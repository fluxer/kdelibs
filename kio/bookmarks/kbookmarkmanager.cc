// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>
   Copyright (C) 2008 Norbert Frese <nf2@scheinwelt.at>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkmanager.h"
#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include "kbookmarkdialog.h"
#include "kbookmarkmanageradaptor_p.h"

#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegExp>
#include <QTextStream>
#include <QMutex>
#include <QTextCodec>
#include <QThread>
#include <QApplication>
#include <QDBusConnection>

#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kmimetype.h>

#define BOOKMARK_CHANGE_NOTIFY_INTERFACE "org.kde.KIO.KBookmarkManager"

class KBookmarkManagerList : public QList<KBookmarkManager*>
{
public:
    ~KBookmarkManagerList() {
        qDeleteAll(begin() , end()); // auto-delete functionality
    }

    QMutex mutex;
};

K_GLOBAL_STATIC(KBookmarkManagerList, s_pSelf)

class KBookmarkMap : private KBookmarkGroupTraverser
{
public:
    KBookmarkMap() : m_mapNeedsUpdate(true) {}
    void setNeedsUpdate() { m_mapNeedsUpdate = true; }
    void update(KBookmarkManager*);
    QList<KBookmark> find( const QString &url ) const
    { return m_bk_map.value(url); }
private:
    virtual void visit(const KBookmark &);
    virtual void visitEnter(const KBookmarkGroup &) { ; }
    virtual void visitLeave(const KBookmarkGroup &) { ; }
private:
    typedef QList<KBookmark> KBookmarkList;
    QMap<QString, KBookmarkList> m_bk_map;
    bool m_mapNeedsUpdate;
};

void KBookmarkMap::update(KBookmarkManager *manager)
{
    if (m_mapNeedsUpdate) {
        m_mapNeedsUpdate = false;

        m_bk_map.clear();
        KBookmarkGroup root = manager->root();
        traverse(root);

        QMutableMapIterator<QString, KBookmarkList> iter(m_bk_map);
        while (iter.hasNext()) {
            iter.next();
            foreach (KBookmark &it, iter.value()) {
                kDebug() << "updating favicon for" << it.url();
                it.setIcon(KMimeType::favIconForUrl(it.url()));
            }
        }
    }
}

void KBookmarkMap::visit(const KBookmark &bk)
{
    if (!bk.isSeparator()) {
        // add bookmark to url map
        m_bk_map[bk.internalElement().attribute("href")].append(bk);
    }
}

// #########################
// KBookmarkManagerPrivate
class KBookmarkManagerPrivate
{
public:
    KBookmarkManagerPrivate(bool bDocIsloaded, const QString &dbusObjectName = QString())
      : m_doc("xbel")
      , m_dbusObjectName(dbusObjectName)
      , m_docIsLoaded(bDocIsloaded)
      , m_update(false)
      , m_dialogAllowed(true)
      , m_dialogParent(nullptr)
      , m_browserEditor(false)
      , m_kDirWatch(nullptr)
    {}

    ~KBookmarkManagerPrivate() {
        delete m_kDirWatch;
    }

    mutable QDomDocument m_doc;
    QString m_bookmarksFile;
    QString m_dbusObjectName;
    mutable bool m_docIsLoaded;
    bool m_update;
    bool m_dialogAllowed;
    QWidget *m_dialogParent;

    bool m_browserEditor;
    QString m_editorCaption;

    KDirWatch* m_kDirWatch; // for external bookmark files

    KBookmarkMap m_map;
};

// ################
// KBookmarkManager
static KBookmarkManager* lookupExisting(const QString &bookmarksFile)
{
    for (KBookmarkManagerList::ConstIterator bmit = s_pSelf->constBegin(), bmend = s_pSelf->constEnd();
         bmit != bmend; ++bmit)
    {
        if ((*bmit)->path() == bookmarksFile) {
            return *bmit;
        }
    }
    return nullptr;
}

KBookmarkManager* KBookmarkManager::managerForFile(const QString &bookmarksFile, const QString &dbusObjectName)
{
    QMutexLocker lock(&s_pSelf->mutex);
    KBookmarkManager* mgr = lookupExisting(bookmarksFile);
    if (mgr) {
        return mgr;
    }

    mgr = new KBookmarkManager(bookmarksFile, dbusObjectName);
    s_pSelf->append(mgr);
    return mgr;
}

KBookmarkManager* KBookmarkManager::managerForExternalFile(const QString &bookmarksFile)
{
    QMutexLocker lock(&s_pSelf->mutex);
    KBookmarkManager* mgr = lookupExisting(bookmarksFile);
    if (mgr) {
        return mgr;
    }

    mgr = new KBookmarkManager(bookmarksFile);
    s_pSelf->append(mgr);
    return mgr;
}

#define PI_DATA "version=\"1.0\" encoding=\"UTF-8\""

static QDomElement createXbelTopLevelElement(QDomDocument &doc)
{
    QDomElement topLevel = doc.createElement("xbel");
    topLevel.setAttribute("xmlns:mime", "http://www.freedesktop.org/standards/shared-mime-info");
    topLevel.setAttribute("xmlns:bookmark", "http://www.freedesktop.org/standards/desktop-bookmarks");
    topLevel.setAttribute("xmlns:kdepriv", "http://www.kde.org/kdepriv");
    doc.appendChild( topLevel );
    doc.insertBefore( doc.createProcessingInstruction("xml", PI_DATA), topLevel);
    return topLevel;
}

KBookmarkManager::KBookmarkManager(const QString &bookmarksFile, const QString &dbusObjectName)
    : d(new KBookmarkManagerPrivate(false, dbusObjectName))
{
    if (dbusObjectName.isNull()) {
        // get dbusObjectName from file
        if (QFile::exists(d->m_bookmarksFile) ) {
            // sets d->m_dbusObjectName
            parse();
        }
    }

    new KBookmarkManagerAdaptor(this);
    QDBusConnection::sessionBus().registerObject(d->m_dbusObjectName, this);
    QDBusConnection::sessionBus().connect(
        QString(), d->m_dbusObjectName, BOOKMARK_CHANGE_NOTIFY_INTERFACE, "bookmarksChanged",
        this, SLOT(notifyChanged(QString,QDBusMessage))
    );
    QDBusConnection::sessionBus().connect(
        QString(), d->m_dbusObjectName, BOOKMARK_CHANGE_NOTIFY_INTERFACE, "bookmarkConfigChanged",
        this, SLOT(notifyConfigChanged())
    );

    d->m_update = true;

    Q_ASSERT(!bookmarksFile.isEmpty());
    d->m_bookmarksFile = bookmarksFile;

    if (!QFile::exists(d->m_bookmarksFile)) {
        QDomElement topLevel = createXbelTopLevelElement(d->m_doc);
        topLevel.setAttribute("dbusName", dbusObjectName);
        d->m_docIsLoaded = true;
    }
}

KBookmarkManager::KBookmarkManager(const QString &bookmarksFile)
    : d(new KBookmarkManagerPrivate(false))
{
    // use KDirWatch to monitor this bookmarks file
    d->m_update = true;

    Q_ASSERT(!bookmarksFile.isEmpty());
    d->m_bookmarksFile = bookmarksFile;

    if (!QFile::exists(d->m_bookmarksFile)) {
        createXbelTopLevelElement(d->m_doc);
    } else {
        parse();
    }
    d->m_docIsLoaded = true;

    // start KDirWatch
    d->m_kDirWatch = new KDirWatch();
    d->m_kDirWatch->addFile(d->m_bookmarksFile);
    QObject::connect(
        d->m_kDirWatch, SIGNAL(dirty(QString)),
        this, SLOT(slotFileChanged(QString))
    );
    kDebug(7043) << "starting KDirWatch for " << d->m_bookmarksFile;
}

void KBookmarkManager::slotFileChanged(const QString &path)
{
    if (path == d->m_bookmarksFile) {
        kDebug(7043) << "file changed (KDirWatch)" << path;
        // Reparse
        parse();
        // Tell our GUI
        // (emit where group is "" to directly mark the root menu as dirty)
        emit changed("", QString());
    }
}

KBookmarkManager::~KBookmarkManager()
{
    if (!s_pSelf.isDestroyed()) {
        s_pSelf->removeAll(this);
    }

    delete d;
}

bool KBookmarkManager::autoErrorHandlingEnabled() const
{
    return d->m_dialogAllowed;
}

void KBookmarkManager::setAutoErrorHandlingEnabled(bool enable, QWidget *parent)
{
    d->m_dialogAllowed = enable;
    d->m_dialogParent = parent;
}

void KBookmarkManager::setUpdate(bool update)
{
    d->m_update = update;
}

QDomDocument KBookmarkManager::internalDocument() const
{
    if (!d->m_docIsLoaded) {
        parse();
    }
    return d->m_doc;
}

void KBookmarkManager::parse() const
{
    d->m_docIsLoaded = true;
    // kDebug(7043) << "parsing" << d->m_bookmarksFile;
    QFile file(d->m_bookmarksFile);
    if (!file.open(QIODevice::ReadOnly)) {
        kWarning() << "Can't open" << d->m_bookmarksFile;
        return;
    }
    d->m_doc = QDomDocument("xbel");
    d->m_doc.setContent(&file);

    if (d->m_doc.documentElement().isNull()) {
        kWarning() << "main tag is missing, creating default" << d->m_bookmarksFile;
        QDomElement element = d->m_doc.createElement("xbel");
        d->m_doc.appendChild(element);
    }

    QDomElement docElem = d->m_doc.documentElement();

    QString mainTag = docElem.tagName();
    if (mainTag != "xbel" ) {
        kWarning() << "unknown main tag" << mainTag;
    }

    if (d->m_dbusObjectName.isNull()) {
        d->m_dbusObjectName = docElem.attribute("dbusName");
    } else if(docElem.attribute("dbusName") != d->m_dbusObjectName) {
        docElem.setAttribute("dbusName", d->m_dbusObjectName);
        save();
    }

    QDomNode n = d->m_doc.documentElement().previousSibling();
    if (n.isProcessingInstruction()) {
        QDomProcessingInstruction pi = n.toProcessingInstruction();
        pi.parentNode().removeChild(pi);
    }

    QDomProcessingInstruction pi;
    pi = d->m_doc.createProcessingInstruction("xml", PI_DATA);
    d->m_doc.insertBefore(pi, docElem);

    file.close();

    d->m_map.setNeedsUpdate();
}

bool KBookmarkManager::save() const
{
    return saveAs(d->m_bookmarksFile);
}

bool KBookmarkManager::saveAs(const QString &filename) const
{
    kDebug(7043) << "save as" << filename;

    KSaveFile file(filename);
    if (file.open()) {
        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));
        stream << internalDocument().toString();
        stream.flush();
        if (file.finalize()) {
            return true;
        }
    }

    static int hadSaveError = false;
    file.abort();
    if (!hadSaveError) {
        QString err = i18n("Unable to save bookmarks in %1. Reported error was: %2. "
                             "This error message will only be shown once. The cause "
                             "of the error needs to be fixed as quickly as possible, "
                             "which is most likely a full hard drive.",
                         filename, file.errorString());

        if (d->m_dialogAllowed && qApp->type() != QApplication::Tty && QThread::currentThread() == qApp->thread()) {
            KMessageBox::error(QApplication::activeWindow(), err);
        }

        kError() << QString("Unable to save bookmarks in %1. File reported the following error-code: %2.").arg(filename).arg(file.error());
        emit const_cast<KBookmarkManager*>(this)->error(err);
    }
    hadSaveError = true;
    return false;
}

QString KBookmarkManager::path() const
{
    return d->m_bookmarksFile;
}

KBookmarkGroup KBookmarkManager::root() const
{
    return KBookmarkGroup(internalDocument().documentElement());
}

KBookmark KBookmarkManager::findByAddress(const QString &address)
{
    // kDebug(7043) << "findByAddress" << address;
    KBookmark result = root();
    // The address is something like /5/10/2+
    const QStringList addresses = address.split(QRegExp("[/+]"),QString::SkipEmptyParts);
    // kWarning() << addresses.join(",");
    foreach (const QString &it, addresses) {
       bool append = (it == "+");
       uint number = it.toUInt();
       Q_ASSERT(result.isGroup());
       KBookmarkGroup group = result.toGroup();
       KBookmark bk = group.first(), lbk = bk; // last non-null bookmark
       for (uint i = 0 ; (i < number || append) && !bk.isNull() ; ++i) {
           lbk = bk;
           bk = group.next(bk);
         // kWarning() << i;
       }
       // kWarning() << "found section";
       result = bk;
    }
    if (result.isNull()) {
        kWarning() << "couldn't find item" << address;
    } else {
        kDebug() << "updating favicon for" << result.url();
        result.setIcon(KMimeType::favIconForUrl(result.url()));
    }
    // kDebug() << "found " << result.address();
    return result;
 }

void KBookmarkManager::emitChanged()
{
    emitChanged(root());
}

void KBookmarkManager::emitChanged(const KBookmarkGroup &group)
{
    (void) save(); // TODO: emitChanged should return a bool? Maybe rename it to saveAndEmitChanged?

    // Tell the other processes too
    // kDebug(7043) << "broadcasting change " << group.address();

    emit bookmarksChanged(group.address());

    // We do get our own broadcast, so no need for this anymore
    // emit changed(group);
}

void KBookmarkManager::emitConfigChanged()
{
    emit bookmarkConfigChanged();
}

// DBUS call
void KBookmarkManager::notifyCompleteChange(const QString &caller)
{
    if (!d->m_update) {
        return;
    }

    kDebug(7043) << "notifyCompleteChange";
    // The bk editor tells us we should reload everything
    // Reparse
    parse();
    // Tell our GUI
    // (emit where group is "" to directly mark the root menu as dirty)
    emit changed("", caller);
}

// DBUS call
void KBookmarkManager::notifyConfigChanged()
{
    kDebug() << "reloaded bookmark config!";
    KBookmarkSettings::self()->readSettings();
    parse(); // reload, and thusly recreate the menus
    emit configChanged();
}

// DBUS call
void KBookmarkManager::notifyChanged(const QString &groupAddress, const QDBusMessage &msg)
{
    kDebug() << "bookmark group changed" << groupAddress;
    if (!d->m_update) {
        return;
    }

    // Reparse (the whole file, no other choice)
    // if someone else notified us
    if (msg.service() != QDBusConnection::sessionBus().baseService()) {
        parse();
    }

    // KBookmarkGroup group = findByAddress( groupAddress ).toGroup();
    // Q_ASSERT(!group.isNull());
    emit changed(groupAddress, QString());
}

void KBookmarkManager::setEditorOptions(const QString &caption, bool browser)
{
    d->m_editorCaption = caption;
    d->m_browserEditor = browser;
}

void KBookmarkManager::slotEditBookmarks()
{
    QStringList args;
    if (!d->m_editorCaption.isEmpty()) {
        args << QLatin1String("--customcaption") << d->m_editorCaption;
    }
    if (!d->m_browserEditor) {
        args << QLatin1String("--nobrowser");
    }
    if (!d->m_dbusObjectName.isEmpty()) {
        args << QLatin1String("--dbusObjectName") << d->m_dbusObjectName;
    }
    args << d->m_bookmarksFile;
    QProcess::startDetached("keditbookmarks", args);
}

void KBookmarkManager::slotEditBookmarksAtAddress(const QString &address)
{
    QStringList args;
    if (!d->m_editorCaption.isEmpty()) {
        args << QLatin1String("--customcaption") << d->m_editorCaption;
    }
    if (!d->m_browserEditor) {
        args << QLatin1String("--nobrowser");
    }
    if (!d->m_dbusObjectName.isEmpty()) {
        args << QLatin1String("--dbusObjectName") << d->m_dbusObjectName;
    }
    args << QLatin1String("--address") << address << d->m_bookmarksFile;
    QProcess::startDetached("keditbookmarks", args);
}

///////
bool KBookmarkManager::updateAccessMetadata(const QString &url)
{
    d->m_map.update(this);
    QList<KBookmark> list = d->m_map.find(url);
    foreach (KBookmark &it, list) {
        it.updateAccessMetadata();
    }
    return true;
}

void KBookmarkManager::updateFavicon(const QString &url, const QString &faviconurl)
{
    d->m_map.update(this);
    QList<KBookmark> list = d->m_map.find(url);
    foreach (KBookmark &it, list) {
        KUrl iconurl(faviconurl);
        it.setIcon(KMimeType::favIconForUrl(iconurl));
    }
}

KBookmarkManager* KBookmarkManager::userBookmarksManager()
{
     const QString bookmarksFile = KStandardDirs::locateLocal("data", QString::fromLatin1("konqueror/bookmarks.xml"));
     KBookmarkManager* bookmarkManager = KBookmarkManager::managerForFile( bookmarksFile, "konqueror" );
     bookmarkManager->setEditorOptions(KGlobal::caption(), true);
     return bookmarkManager;
}

KBookmarkSettings* KBookmarkSettings::s_self = nullptr;

void KBookmarkSettings::readSettings()
{
   KConfig config("kbookmarkrc", KConfig::NoGlobals);
   KConfigGroup cg(&config, "Bookmarks");

   // add bookmark dialog usage - no reparse
   s_self->m_advancedaddbookmark = cg.readEntry("AdvancedAddBookmarkDialog", false);

   // this one alters the menu, therefore it needs a reparse
   s_self->m_contextmenu = cg.readEntry("ContextMenuActions", true);
}

KBookmarkSettings *KBookmarkSettings::self()
{
   if (!s_self) {
      s_self = new KBookmarkSettings();
      readSettings();
   }
   return s_self;
}

/////////// KBookmarkOwner

bool KBookmarkOwner::enableOption(BookmarkOption action) const
{
    if (action == ShowAddBookmark) {
        return true;
    }
    if (action == ShowEditBookmark) {
        return true;
    }
    return false;
}

KBookmarkDialog* KBookmarkOwner::bookmarkDialog(KBookmarkManager *mgr, QWidget *parent)
{
    return new KBookmarkDialog(mgr, parent);
}

void KBookmarkOwner::openFolderinTabs(const KBookmarkGroup &)
{
}

#include "moc_kbookmarkmanager.cpp"
