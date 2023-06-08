/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2006 Daniel Teske <teske@squorn.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include "kbookmarkdialog.h"
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <kstringhandler.h>
#include <krun.h>
#include <kactioncollection.h>

#include <qclipboard.h>
#include <qmimedata.h>
#include <QtCore/QStack>
#include <QtGui/QHeaderView>
#include <QtGui/QApplication>

/********************************************************************/
/********************************************************************/
/********************************************************************/
class KBookmarkMenuPrivate
{
public:
    KBookmarkMenuPrivate()
        : newBookmarkFolder(nullptr)
        , addAddBookmark(nullptr)
        , bookmarksToFolder(nullptr)
        , m_bIsRoot(false)
        , m_bDirty(false)
        , m_pManager(nullptr)
        , m_pOwner(nullptr)
    {
    }

    KAction *newBookmarkFolder;
    KAction *addAddBookmark;
    KAction *bookmarksToFolder;

    bool m_bIsRoot;
    bool m_bDirty;
    KBookmarkManager* m_pManager;
    KBookmarkOwner* m_pOwner;

    KMenu* m_parentMenu;
    QList<KBookmarkMenu*> m_lstSubMenus;
    // This is used to "export" our actions into an actionlist
    // we got in the constructor. So that the program can show our 
    // actions in their shortcut dialog
    KActionCollection* m_actionCollection;
    QList<QAction*> m_actions;

    QString m_parentAddress;
};


KBookmarkMenu::KBookmarkMenu(KBookmarkManager *mgr,
                             KBookmarkOwner *owner, KMenu *parentMenu,
                             KActionCollection *actionCollection)
  : QObject(),
    d(new KBookmarkMenuPrivate())
{
    d->m_bIsRoot = true;
    d->m_pManager = mgr;
    d->m_pOwner = owner;
    d->m_parentMenu = parentMenu;
    d->m_parentAddress = QString(""); //TODO KBookmarkAdress::root
    d->m_actionCollection = actionCollection;
    d->m_parentMenu->setKeyboardShortcutsEnabled(true);

    // kDebug(7043) << this << "address" << m_parentAddress;

    connect(parentMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShow()));

    if (KBookmarkSettings::self()->m_contextmenu) {
        d->m_parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(
            d->m_parentMenu, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenu(QPoint))
        );
    }

    connect(
        d->m_pManager, SIGNAL(changed(QString, QString)),
        this, SLOT(slotBookmarksChanged(QString))
    );

    d->m_bDirty = true;
    addActions();
}

void KBookmarkMenu::addActions()
{
    if (d->m_bIsRoot) {
        addAddBookmark();
        addAddBookmarksList();
        addNewFolder();
        addEditBookmarks();
    } else {
        if (d->m_parentMenu->actions().count() > 0) {
            d->m_parentMenu->addSeparator();
        }

        addOpenInTabs();
        addAddBookmark();
        addAddBookmarksList();
        addNewFolder();
    }
}

KBookmarkMenu::KBookmarkMenu(KBookmarkManager *mgr,
                             KBookmarkOwner *owner, KMenu *parentMenu,
                             const QString &parentAddress)
    : QObject(),
    d (new KBookmarkMenuPrivate())
{
    d->m_bIsRoot = false;
    d->m_pManager = mgr;
    d->m_pOwner = owner;
    d->m_parentMenu = parentMenu;
    d->m_parentAddress = parentAddress;
    d->m_actionCollection = new KActionCollection(this);

    d->m_parentMenu->setKeyboardShortcutsEnabled(true);
    connect(parentMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShow()));
    if (KBookmarkSettings::self()->m_contextmenu) {
        d->m_parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(
            d->m_parentMenu, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenu(QPoint))
        );
    }
    d->m_bDirty = true;
}

KBookmarkMenu::~KBookmarkMenu()
{
    qDeleteAll(d->m_lstSubMenus);
    qDeleteAll(d->m_actions);
    delete d;
}

void KBookmarkMenu::ensureUpToDate()
{
    slotAboutToShow();
}

void KBookmarkMenu::slotAboutToShow()
{
    // Did the bookmarks change since the last time we showed them ?
    if (d->m_bDirty) {
        d->m_bDirty = false;
        clear();
        refill();
        d->m_parentMenu->adjustSize();
    }
}

void KBookmarkMenu::slotCustomContextMenu(const QPoint &pos)
{
    QAction* action = d->m_parentMenu->actionAt(pos);
    KMenu* menu = contextMenu(action);
    if (!menu) {
        return;
    }
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(d->m_parentMenu->mapToGlobal(pos));
}

KMenu* KBookmarkMenu::contextMenu(QAction *action)
{
    KBookmarkActionInterface* act = dynamic_cast<KBookmarkActionInterface*>(action);
    if (!act) {
        return nullptr;
    }
    return new KBookmarkContextMenu(act->bookmark(), d->m_pManager, d->m_pOwner);
}

bool KBookmarkMenu::isRoot() const
{
    return d->m_bIsRoot;
}

bool KBookmarkMenu::isDirty() const
{
    return d->m_bDirty;
}

QString KBookmarkMenu::parentAddress() const
{
    return d->m_parentAddress;
}

KBookmarkManager* KBookmarkMenu::manager() const
{
    return d->m_pManager;
}

KBookmarkOwner* KBookmarkMenu::owner() const
{
    return d->m_pOwner;
}

KMenu* KBookmarkMenu::parentMenu() const
{
    return d->m_parentMenu;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkActionInterface::KBookmarkActionInterface(const KBookmark &bk)
    : bm(bk)
{
}

KBookmarkActionInterface::~KBookmarkActionInterface()
{
}

const KBookmark KBookmarkActionInterface::bookmark() const
{
    return bm;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/


KBookmarkContextMenu::KBookmarkContextMenu(const KBookmark &bk, KBookmarkManager *manager, KBookmarkOwner *owner, QWidget *parent)
    : KMenu(parent), bm(bk), m_pManager(manager), m_pOwner(owner)
{
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
}

void KBookmarkContextMenu::slotAboutToShow()
{
    addActions();
}

void KBookmarkContextMenu::addActions()
{
    if (bm.isGroup()) {
        addOpenFolderInTabs();
        addBookmark();
        addFolderActions();
    } else {
        addBookmark();
        addBookmarkActions();
    }
}

KBookmarkContextMenu::~KBookmarkContextMenu()
{
}

void KBookmarkContextMenu::addBookmark()
{
    if (m_pOwner && m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark)) {
        addAction(KIcon("bookmark-new"), i18n("Add Bookmark Here"), this, SLOT(slotInsert()));
    }
}

void KBookmarkContextMenu::addFolderActions()
{
    addAction(i18n("Open Folder in Bookmark Editor"), this, SLOT(slotEditAt()));
    addProperties();
    addSeparator();
    addAction(KIcon("edit-delete"), i18n("Delete Folder"), this, SLOT(slotRemove()));
}


void KBookmarkContextMenu::addProperties()
{
    addAction(i18n("Properties"), this, SLOT(slotProperties()));
}

void KBookmarkContextMenu::addBookmarkActions()
{
    addAction(i18n("Copy Link Address"), this, SLOT(slotCopyLocation()));
    addProperties();
    addSeparator();
    addAction(KIcon("edit-delete"), i18n("Delete Bookmark"), this, SLOT(slotRemove()));
}

void KBookmarkContextMenu::addOpenFolderInTabs()
{
    if (m_pOwner->supportsTabs()) {
        addAction(KIcon("tab-new"), i18n("Open Folder in Tabs"), this, SLOT(slotOpenFolderInTabs()));
    }
}

void KBookmarkContextMenu::slotEditAt()
{
    // kDebug(7043) << m_highlightedAddress;
    m_pManager->slotEditBookmarksAtAddress( bm.address() );
}

void KBookmarkContextMenu::slotProperties()
{
    // kDebug(7043) << m_highlightedAddress;

    KBookmarkDialog* dlg = m_pOwner->bookmarkDialog(m_pManager, QApplication::activeWindow());
    dlg->editBookmark(bm);
    delete dlg;
}

void KBookmarkContextMenu::slotInsert()
{
    // kDebug(7043) << m_highlightedAddress;

    QString url = m_pOwner->currentUrl();
    if (url.isEmpty()) {
        KMessageBox::error(QApplication::activeWindow(), i18n("Cannot add bookmark with empty URL."));
        return;
    }
    QString title = m_pOwner->currentTitle();
    if (title.isEmpty()) {
        title = url;
    }

    if (bm.isGroup()) {
        KBookmarkGroup parentBookmark = bm.toGroup();
        Q_ASSERT(!parentBookmark.isNull());
        parentBookmark.addBookmark(title, KUrl(url));
        m_pManager->emitChanged(parentBookmark);
    } else {
        KBookmarkGroup parentBookmark = bm.parentGroup();
        Q_ASSERT(!parentBookmark.isNull());
        KBookmark newBookmark = parentBookmark.addBookmark(title, KUrl(m_pOwner->currentUrl()));
        parentBookmark.moveBookmark(newBookmark, parentBookmark.previous(bm));
        m_pManager->emitChanged(parentBookmark);
    }
}

void KBookmarkContextMenu::slotRemove()
{
  // kDebug(7043) << "slotRemove" << m_highlightedAddress;

  bool folder = bm.isGroup();

  if (KMessageBox::warningContinueCancel(
          QApplication::activeWindow(),
          folder ? i18n("Are you sure you wish to remove the bookmark folder\n\"%1\"?", bm.text())
                 : i18n("Are you sure you wish to remove the bookmark\n\"%1\"?", bm.text()),
          folder ? i18n("Bookmark Folder Deletion")
                 : i18n("Bookmark Deletion"),
          KStandardGuiItem::del())
        != KMessageBox::Continue
     )
    {
        return;
    }

    KBookmarkGroup parentBookmark = bm.parentGroup();
    parentBookmark.deleteBookmark(bm);
    m_pManager->emitChanged(parentBookmark);
}

void KBookmarkContextMenu::slotCopyLocation()
{
    // kDebug(7043) << "slotCopyLocation" << m_highlightedAddress;

    if (!bm.isGroup()) {
        QMimeData* mimeData = new QMimeData();
        bm.populateMimeData(mimeData);
        QApplication::clipboard()->setMimeData(mimeData, QClipboard::Selection);
        mimeData = new QMimeData();
        bm.populateMimeData(mimeData);
        QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
    }
}

void KBookmarkContextMenu::slotOpenFolderInTabs()
{
    owner()->openFolderinTabs(bookmark().toGroup());
}

KBookmarkManager* KBookmarkContextMenu::manager() const
{
    return m_pManager;
}

KBookmarkOwner* KBookmarkContextMenu::owner() const
{
    return m_pOwner;
}

KBookmark KBookmarkContextMenu::bookmark() const
{
    return bm;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

void KBookmarkMenu::slotBookmarksChanged(const QString &groupAddress)
{
    kDebug(7043) << "slotBookmarksChanged" << groupAddress;
    if (groupAddress == d->m_parentAddress) {
        // kDebug(7043) << "setting m_bDirty on" << groupAddress;
        d->m_bDirty = true;
    } else {
        // Iterate recursively into child menus
        foreach (KBookmarkMenu *it, d->m_lstSubMenus ) {
            it->slotBookmarksChanged(groupAddress);
        }
    }
}

void KBookmarkMenu::clear()
{
    qDeleteAll(d->m_lstSubMenus);
    d->m_lstSubMenus.clear();

    foreach (QAction *it, d->m_actions) {
        d->m_parentMenu->removeAction(it);
        delete it;
    }

    d->m_parentMenu->clear();
    d->m_actions.clear();
}

void KBookmarkMenu::refill()
{
    // kDebug(7043) << m_bIsRoot;
    if (d->m_bIsRoot) {
        addActions();
    }
    fillBookmarks();
    if (!d->m_bIsRoot) {
        addActions();
    }
}

void KBookmarkMenu::addOpenInTabs()
{
    if (!d->m_pOwner || !d->m_pOwner->supportsTabs()) {
        return;
    }

    QString title = i18n("Open Folder in Tabs");

    KAction* paOpenFolderInTabs = new KAction(title, this);
    paOpenFolderInTabs->setIcon(KIcon("tab-new"));
    paOpenFolderInTabs->setHelpText(i18n("Open all bookmarks in this folder as a new tab."));
    connect(paOpenFolderInTabs, SIGNAL(triggered(bool)), this, SLOT( slotOpenFolderInTabs()));

    d->m_parentMenu->addAction(paOpenFolderInTabs);
    d->m_actions.append(paOpenFolderInTabs);
}

void KBookmarkMenu::addAddBookmarksList()
{
    if (!d->m_pOwner || !d->m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark) || !d->m_pOwner->supportsTabs()) {
        return;
    }

    if (!d->bookmarksToFolder) {
        QString title = i18n("Bookmark Tabs as Folder...");
        d->bookmarksToFolder = new KAction(title, this);
        d->m_actionCollection->addAction(d->m_bIsRoot ? "add_bookmarks_list" : 0, d->bookmarksToFolder);
        d->bookmarksToFolder->setIcon(KIcon("bookmark-new-list"));
        d->bookmarksToFolder->setHelpText(i18n("Add a folder of bookmarks for all open tabs."));
        connect(d->bookmarksToFolder, SIGNAL(triggered(bool)), this, SLOT(slotAddBookmarksList()));
    }

    d->m_parentMenu->addAction(d->bookmarksToFolder);
}

void KBookmarkMenu::addAddBookmark()
{
    if (!d->m_pOwner || !d->m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark)) {
        return;
    }

    if (!d->addAddBookmark) {
        d->addAddBookmark = d->m_actionCollection->addAction(
            KStandardAction::AddBookmark,
            d->m_bIsRoot ? "add_bookmark" : 0,
            this,
            SLOT(slotAddBookmark())
        );
        if (!d->m_bIsRoot) {
            d->addAddBookmark->setShortcut(QKeySequence());
        }
    }

    d->m_parentMenu->addAction(d->addAddBookmark);
}

void KBookmarkMenu::addEditBookmarks()
{
    if (d->m_pOwner && !d->m_pOwner->enableOption(KBookmarkOwner::ShowEditBookmark)) {
        return;
    }

    KAction* m_paEditBookmarks = d->m_actionCollection->addAction(KStandardAction::EditBookmarks, "edit_bookmarks",
                                                                  d->m_pManager, SLOT(slotEditBookmarks()));
    d->m_parentMenu->addAction(m_paEditBookmarks);
    m_paEditBookmarks->setHelpText(i18n( "Edit your bookmark collection in a separate window"));
}

void KBookmarkMenu::addNewFolder()
{
    if (!d->m_pOwner || !d->m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark)) {
        return;
    }

    if (!d->newBookmarkFolder) {
        d->newBookmarkFolder = new KAction(i18n("New Bookmark Folder..."), this);
        d->newBookmarkFolder->setIcon(KIcon("folder-new"));
        d->newBookmarkFolder->setHelpText(i18n("Create a new bookmark folder in this menu"));
        connect(d->newBookmarkFolder, SIGNAL(triggered(bool)), this, SLOT(slotNewFolder()));
    }

    d->m_parentMenu->addAction(d->newBookmarkFolder);
}

void KBookmarkMenu::fillBookmarks()
{
    KBookmarkGroup parentBookmark = d->m_pManager->findByAddress(d->m_parentAddress).toGroup();
    Q_ASSERT(!parentBookmark.isNull());

    if (d->m_bIsRoot && !parentBookmark.first().isNull()) { // at least one bookmark
        d->m_parentMenu->addSeparator();
    }

    for (KBookmark bm = parentBookmark.first(); !bm.isNull(); bm = parentBookmark.next(bm)) {
        d->m_parentMenu->addAction(actionForBookmark(bm));
    }
}

QAction* KBookmarkMenu::actionForBookmark(const KBookmark &bm)
{
    if (bm.isGroup()) {
        // kDebug(7043) << "Creating bookmark submenu named" << bm.text();
        KActionMenu* actionMenu = new KBookmarkActionMenu(bm, this);
        d->m_actions.append(actionMenu);
        KBookmarkMenu *subMenu = new KBookmarkMenu(d->m_pManager, d->m_pOwner, actionMenu->menu(), bm.address());
        d->m_lstSubMenus.append(subMenu);
        return actionMenu;
    } else if (bm.isSeparator()) {
        QAction *sa = new QAction(this);
        sa->setSeparator(true);
        d->m_actions.append(sa);
        return sa;
    }
    // kDebug(7043) << "Creating bookmark menu item for " << bm.text();
    KAction * action = new KBookmarkAction(bm, d->m_pOwner, this);
    d->m_actions.append(action);
    return action;
}

void KBookmarkMenu::slotAddBookmarksList()
{
    if (!d->m_pOwner || !d->m_pOwner->supportsTabs()) {
        return;
    }

    KBookmarkGroup parentBookmark = d->m_pManager->findByAddress(d->m_parentAddress).toGroup();
    KBookmarkDialog * dlg = d->m_pOwner->bookmarkDialog(d->m_pManager, QApplication::activeWindow());
    dlg->addBookmarks(d->m_pOwner->currentBookmarkList(), "", parentBookmark);
    delete dlg;
}

void KBookmarkMenu::slotAddBookmark()
{
    if (!d->m_pOwner) {
        return;
    }

    KBookmarkGroup parentBookmark = d->m_pManager->findByAddress(d->m_parentAddress).toGroup();
    if (KBookmarkSettings::self()->m_advancedaddbookmark) {
        KBookmarkDialog* dlg = d->m_pOwner->bookmarkDialog(d->m_pManager, QApplication::activeWindow() );
        dlg->addBookmark(d->m_pOwner->currentTitle(), KUrl(d->m_pOwner->currentUrl()), parentBookmark );
        delete dlg;
    } else {
        parentBookmark.addBookmark(d->m_pOwner->currentTitle(), KUrl(d->m_pOwner->currentUrl()));
        d->m_pManager->emitChanged(parentBookmark);
    }
}

void KBookmarkMenu::slotOpenFolderInTabs()
{
    d->m_pOwner->openFolderinTabs(d->m_pManager->findByAddress(d->m_parentAddress).toGroup());
}

void KBookmarkMenu::slotNewFolder()
{
    if (!d->m_pOwner) {
        return; // this view doesn't handle bookmarks...
    }
    KBookmarkGroup parentBookmark = d->m_pManager->findByAddress(d->m_parentAddress).toGroup();
    Q_ASSERT(!parentBookmark.isNull());
    KBookmarkDialog* dlg = d->m_pOwner->bookmarkDialog(d->m_pManager, QApplication::activeWindow());
    dlg->createNewFolder("", parentBookmark);
    delete dlg;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/


KBookmarkAction::KBookmarkAction(const KBookmark &bk, KBookmarkOwner *owner, QObject *parent)
    : KAction( bk.text().replace('&', "&&"), parent),
    KBookmarkActionInterface(bk),
    m_pOwner(owner)
{
    setIcon(KIcon(bookmark().icon()));
    setIconText(text());
    setHelpText(bookmark().url().pathOrUrl());
    const QString description = bk.description();
    if (!description.isEmpty()) {
        setToolTip(description);
    }
    connect(
        this, SIGNAL(triggered(Qt::MouseButtons, Qt::KeyboardModifiers)),
        this, SLOT(slotSelected(Qt::MouseButtons, Qt::KeyboardModifiers))
    );
}

KBookmarkAction::~KBookmarkAction()
{
}

void KBookmarkAction::slotSelected(Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
    if (!m_pOwner) {
        new KRun(bookmark().url(), (QWidget*)0);
    } else {
        m_pOwner->openBookmark( bookmark(), mb, km );
    }
}

KBookmarkActionMenu::KBookmarkActionMenu(const KBookmark &bm, QObject *parent)
    : KActionMenu(KIcon(bm.icon()), bm.text().replace('&', "&&"), parent),
    KBookmarkActionInterface(bm)
{
    setToolTip(bm.description());
    setIconText(text());
}

KBookmarkActionMenu::KBookmarkActionMenu(const KBookmark &bm, const QString &text, QObject *parent)
    : KActionMenu(text, parent),
    KBookmarkActionInterface(bm)
{
}

KBookmarkActionMenu::~KBookmarkActionMenu()
{
}

#include "moc_kbookmarkmenu.cpp"
