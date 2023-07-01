/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdirlister.h"
#include "kdirlister_p.h"

#include <kdebug.h>
#include <klocale.h>
#include <kio/jobuidelegate.h>
#include <kmessagebox.h>
#include "kprotocolmanager.h"
#include <QDirIterator>

// for when models update their indexes based on two signals properly, until then no partial updates
// #define MODELS_ARE_FIXED

KDirListerPrivate::KDirListerPrivate(KDirLister *parent)
    : autoUpdate(true),
    delayedMimeTypes(false),
    autoErrorHandling(true),
    showingDotFiles(false),
    dirOnlyMode(false),
    complete(false),
    window(nullptr),
    listJob(nullptr),
    m_parent(parent),
    m_dirwatch(nullptr),
    m_dirnotify(nullptr)
{
}

void KDirListerPrivate::_k_slotInfoMessage(KJob *job, const QString &msg)
{
    Q_UNUSED(job);
    emit m_parent->infoMessage(msg);
}

void KDirListerPrivate::_k_slotPercent(KJob *job, ulong value)
{
    Q_UNUSED(job);
    emit m_parent->percent(value);
}

void KDirListerPrivate::_k_slotTotalSize(KJob *job, qulonglong value)
{
    Q_UNUSED(job);
    emit m_parent->totalSize(value);
}

void KDirListerPrivate::_k_slotProcessedSize(KJob *job, qulonglong value)
{
    Q_UNUSED(job);
    emit m_parent->processedSize(value);
}

void KDirListerPrivate::_k_slotSpeed(KJob *job, ulong value)
{
    Q_UNUSED(job);
    emit m_parent->speed(value);
}

void KDirListerPrivate::_k_slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries)
{
    Q_ASSERT(listJob == qobject_cast<KIO::ListJob*>(job));
    kDebug(7003) << "got entries for" << listJob->url();
    foreach (const KIO::UDSEntry &it, entries) {
        const QString name = it.stringValue(KIO::UDSEntry::UDS_NAME);
        if (name.isEmpty()) {
            continue;
        }
        KFileItem item(it, listJob->url(), delayedMimeTypes, true);
        if (name == QLatin1String(".")) {
            rootFileItem = item;
            continue;
        } else if (name == QLatin1String("..")) {
            continue;
        }
        allItems.append(item);
        if (m_parent->matchesFilter(item) && m_parent->matchesMimeFilter(item)) {
            kDebug(7003) << "filtered entry" << item;
            filteredItems.append(item);
        }
    }
}

void KDirListerPrivate::_k_slotResult(KJob *job)
{
    Q_ASSERT(listJob == qobject_cast<KIO::ListJob*>(job));
    kDebug(7003) << "done listing" << listJob->url();
    complete = true;
    if (job->error() != KJob::KilledJobError && job->error() != KJob::NoError) {
        m_parent->handleError(listJob);
    }
    listJob = nullptr;
    job->deleteLater();

    if (m_dirwatch) {
        m_dirwatch->disconnect(m_parent);
        delete m_dirwatch;
        m_dirwatch = nullptr;
    }
    if (m_dirnotify) {
        // TODO: this has to be done when opening new directory
        org::kde::KDirNotify::emitLeftDirectory(url.url());
        // m_dirnotify->disconnect(m_parent);
        delete m_dirnotify;
        m_dirnotify = nullptr;
    }

    if (!filteredItems.isEmpty()) {
        emit m_parent->itemsAdded(filteredItems);
    }
    emit m_parent->completed();

    if (autoUpdate) {
        if (url.isLocalFile()) {
            kDebug(7003) << "watching" << url.toLocalFile();
            m_dirwatch = new KDirWatch(m_parent);
            m_dirwatch->addDir(url.toLocalFile());
            m_parent->connect(
                m_dirwatch, SIGNAL(dirty(QString)),
                m_parent, SLOT(_k_slotDirty(QString))
            );
        } else {
            kDebug(7003) << "watching remote" << url;
            m_dirnotify = new org::kde::KDirNotify(QString(), QString(), QDBusConnection::sessionBus(), m_parent);
            m_parent->connect(
                m_dirnotify, SIGNAL(FileRenamed(QString,QString)),
                m_parent, SLOT(_k_slotFileRenamed(QString,QString))
            );
            m_parent->connect(
                m_dirnotify, SIGNAL(FilesAdded(QString)),
                m_parent, SLOT(_k_slotFilesAdded(QString))
            );
            m_parent->connect(
                m_dirnotify, SIGNAL(FilesChanged(QStringList)),
                m_parent, SLOT(_k_slotFilesChanged(QStringList))
            );
            m_parent->connect(
                m_dirnotify, SIGNAL(FilesRemoved(QStringList)),
                m_parent, SLOT(_k_slotFilesRemoved(QStringList))
            );
            org::kde::KDirNotify::emitEnteredDirectory(url.url());
        }
    }
}

void KDirListerPrivate::_k_slotRedirection(KIO::Job *job, const KUrl &url)
{
    Q_UNUSED(job);
    emit m_parent->redirection(url);
}

void KDirListerPrivate::_k_slotDirty(const QString &path)
{
    kDebug(7003) << "dirty path" << path;
    m_parent->updateDirectory();
}

void KDirListerPrivate::_k_slotFileRenamed(const QString &path, const QString &path2)
{
    kDebug(7003) << "file renamed" << path << path2;
    QMutableListIterator<KFileItem> it(filteredItems);
    while (it.hasNext()) {
        const KFileItem item = it.next();
        if (item.url() == KUrl(path)) {
            m_parent->updateDirectory();
            break;
        }
    }
}

void KDirListerPrivate::_k_slotFilesAdded(const QString &path)
{
    kDebug(7003) << "file added" << path;
    const KUrl pathurl(path);
    const KUrl pathdirectory = pathurl.directory();
    if (pathdirectory == url || pathurl == url) {
        m_parent->updateDirectory();
    }
}
void KDirListerPrivate::_k_slotFilesChanged(const QStringList &paths)
{
    kDebug(7003) << "files changed" << paths;
    foreach (const QString &it, paths) {
        foreach (const KFileItem &it2, filteredItems) {
            if (it2.url() == KUrl(it)) {
                m_parent->updateDirectory();
                break;
            }
        }
    }
}
void KDirListerPrivate::_k_slotFilesRemoved(const QStringList &paths)
{
    kDebug(7003) << "files removed" << paths;
    foreach (const QString &it, paths) {
        _k_slotFileRenamed(it, QString());
    }
}


KDirLister::KDirLister(QObject* parent)
    : QObject(parent),
    d(new KDirListerPrivate(this))
{
}

KDirLister::~KDirLister()
{
    stop();
    delete d;
}

bool KDirLister::openUrl(const KUrl &url, OpenUrlFlags flags)
{
    Q_UNUSED(flags);
    kDebug(7003) << "opening" << url << flags;
    if (!url.isValid()) {
        // this happens a lot, invalid starting directory as URL
        return false;
    }
    stop();
    d->url = url;
    d->allItems.clear();
    if (!d->filteredItems.isEmpty()) {
        emit itemsDeleted(d->filteredItems);
        d->filteredItems.clear();
    }
    emit clear();
    if (d->autoErrorHandling) {
        if (!KProtocolManager::supportsListing(url)) {
            KMessageBox::error(d->window, i18n("URL cannot be listed\n%1", url.prettyUrl()));
            return false;
        }
    }
    d->complete = false;
    d->listJob = KIO::listDir(url, KIO::HideProgressInfo);
    d->listJob->setAutoDelete(false);
    if (d->window) {
        d->listJob->ui()->setWindow(d->window);
    }

    // proxies
    connect(
        d->listJob, SIGNAL(infoMessage(KJob*,QString,QString)),
        this, SLOT(_k_slotInfoMessage(KJob*,QString))
    );
    connect(
        d->listJob, SIGNAL(percent(KJob*,ulong)),
        this, SLOT(_k_slotPercent(KJob*,ulong))
    );
    connect(
        d->listJob, SIGNAL(totalSize(KJob*, qulonglong)),
        this, SLOT(_k_slotTotalSize(KJob*, qulonglong))
    );
    connect(
        d->listJob, SIGNAL(processedSize(KJob*, qulonglong)),
        this, SLOT(_k_slotProcessedSize(KJob*, qulonglong))
    );
    connect(
        d->listJob, SIGNAL(speed(KJob*, ulong)),
        this, SLOT(_k_slotSpeed(KJob*, ulong))
    );

    // private slots
    connect(
        d->listJob, SIGNAL(entries(KIO::Job*, KIO::UDSEntryList)),
        this, SLOT(_k_slotEntries(KIO::Job*, KIO::UDSEntryList))
    );
    connect(
        d->listJob, SIGNAL(result(KJob*)),
        this, SLOT(_k_slotResult(KJob*))
    );
    connect(
        d->listJob, SIGNAL(redirection(KIO::Job*,KUrl)),
        this, SLOT(_k_slotRedirection(KIO::Job*,KUrl))
    );

    emit started();
    return true;
}

void KDirLister::stop()
{
    if (d->listJob) {
        kDebug(7003) << "killing job for" << d->url;
        d->listJob->disconnect(this);
        d->listJob->kill();
        d->listJob->deleteLater();
        d->listJob = nullptr;
        emit canceled();
    }
}

bool KDirLister::autoUpdate() const
{
    return d->autoUpdate;
}

void KDirLister::setAutoUpdate(bool enable)
{
    if (d->autoUpdate == enable) {
        return;
    }
    d->autoUpdate = enable;
}

bool KDirLister::showingDotFiles() const
{
    return d->showingDotFiles;
}

void KDirLister::setShowingDotFiles(bool showDotFile)
{
    if (d->showingDotFiles == showDotFile) {
        return;
    }
    d->showingDotFiles = showDotFile;
}

bool KDirLister::dirOnlyMode() const
{
    return d->dirOnlyMode;
}

void KDirLister::setDirOnlyMode(bool dirsOnly)
{
    if (d->dirOnlyMode == dirsOnly) {
        return;
    }
    d->dirOnlyMode = dirsOnly;
}

bool KDirLister::autoErrorHandlingEnabled() const
{
    return d->autoErrorHandling;
}

void KDirLister::setAutoErrorHandlingEnabled(bool enable, QWidget *parent)
{
    d->autoErrorHandling = enable;
    d->window = parent;
}

KUrl KDirLister::url() const
{
    return d->url;
}

void KDirLister::updateDirectory()
{
#ifdef MODELS_ARE_FIXED
    if (d->url.isLocalFile()) {
        KFileItemList deletedItems;
        QMutableListIterator<KFileItem> it(d->allItems);
        while (it.hasNext()) {
            const KFileItem item = it.next();
            QFileInfo foundinfo(item.url().toLocalFile());
            kDebug(7003) << "checking entry" << item;
            if (!foundinfo.exists()) {
                kDebug(7003) << "deleted entry" << item;
                it.remove();
                d->filteredItems.removeAll(item);
                deletedItems.append(item);
            }
        }
        if (!deletedItems.isEmpty()) {
            emit itemsDeleted(deletedItems);
        }

        QDirIterator diriterator(d->url.toLocalFile());
        KFileItemList addedItems;
        while (diriterator.hasNext()) {
            const QString dirfilepath = diriterator.next();
            const QString dirfilename = diriterator.fileName();
            if (dirfilename == QLatin1String(".") || dirfilename == QLatin1String("..")) {
                continue;
            }
            const KFileItem founditem = d->allItems.findByName(dirfilename);
            if (founditem.isNull()) {
                const KFileItem item(KUrl(dirfilepath), QString(), KFileItem::Unknown);
                kDebug(7003) << "new entry" << item;
                if (matchesFilter(item) && matchesMimeFilter(item)) {
                    kDebug(7003) << "new filtered entry" << item;
                    d->filteredItems.append(item);
                }
                d->allItems.append(item);
                addedItems.append(item);
            }
        }
        if (!addedItems.isEmpty()) {
            emit itemsAdded(addedItems);
        }
        return;
    }
#endif // MODELS_ARE_FIXED
    // NOTE: no partial updates for non-local directories because the signals are bogus for
    // some KIO slaves (such as trash:/, see kde-workspace/kioslave/trash/ktrash.cpp for example)
    openUrl(d->url, KDirLister::NoFlags);
}

bool KDirLister::isFinished() const
{
    return d->complete;
}

KFileItem KDirLister::rootItem() const
{
    return d->rootFileItem;
}

KFileItem KDirLister::findByUrl(const KUrl &url) const
{
    if (url == d->rootFileItem.url()) {
        return d->rootFileItem;
    }
    foreach (const KFileItem &it, d->allItems) {
        if (it.url() == url) {
            return it;
        }
    }
    return KFileItem();
}

KFileItem KDirLister::findByName(const QString &name) const
{
    if (name == d->rootFileItem.name()) {
        return d->rootFileItem;
    }
    foreach (const KFileItem &it, d->allItems) {
        if (it.name() == name) {
            return it;
        }
    }
    return KFileItem();
}

void KDirLister::setNameFilter(const QString &nameFilter)
{
    if (d->nameFilter == nameFilter) {
        return;
    }
    d->nameFilter = nameFilter;
    d->nameFilters.clear();
    const QStringList splitFilters = nameFilter.split(QLatin1Char(' '));
    d->nameFilters.reserve(splitFilters.size());
    foreach (const QString &it, splitFilters) {
        QRegExp filterexp(it, Qt::CaseInsensitive, QRegExp::Wildcard);
        if (!filterexp.isValid()) {
            kWarning(7003) << "invalid name filter" << it;
            continue;
        }
        d->nameFilters.append(filterexp);
    }
}

QString KDirLister::nameFilter() const
{
    return d->nameFilter;
}

void KDirLister::setMimeFilter(const QStringList &mimeFilter)
{
    if (d->mimeFilter == mimeFilter) {
        return;
    }
    d->mimeFilter = mimeFilter;
}

void KDirLister::clearMimeFilter()
{
    d->mimeFilter.clear();
}

QStringList KDirLister::mimeFilters() const
{
    return d->mimeFilter;
}

bool KDirLister::matchesFilter(const QString &name) const
{
    return doNameFilter(name, d->nameFilters);
}

bool KDirLister::matchesMimeFilter(const QString &mime) const
{
    return doMimeFilter(mime, d->mimeFilter);
}

bool KDirLister::matchesFilter(const KFileItem &item) const
{
    if (item.text() == "..") {
        return false;
    }
    if (!d->showingDotFiles && item.isHidden()) {
        return false;
    }
    if (d->dirOnlyMode && !item.isDir()) {
        return false;
    }
    if (item.isDir() || d->nameFilter.isEmpty()) {
        return true;
    }
    return matchesFilter(item.text());
}

bool KDirLister::matchesMimeFilter(const KFileItem &item) const
{
    if (item.isDir() || d->mimeFilter.isEmpty()) {
        return true;
    }
    return matchesMimeFilter(item.mimetype());
}

bool KDirLister::doNameFilter(const QString &name, const QList<QRegExp> &filters) const
{
    if (filters.isEmpty()) {
        return true;
    }
    kDebug(7003) << "doNameFilter" << name << filters;
    foreach (const QRegExp &it, filters) {
        if (it.exactMatch(name)) {
            return true;
        }
    }
    return false;
}

bool KDirLister::doMimeFilter(const QString &mime, const QStringList &filters) const
{
    if (filters.isEmpty()) {
        return true;
    }
    kDebug(7003) << "doMimeFilter" << mime << filters;
    const KMimeType::Ptr mimeptr = KMimeType::mimeType(mime);
    if (!mimeptr) {
        return false;
    }
    foreach (const QString &it, filters) {
        if (mimeptr->is(it)) {
            return true;
        }
    }
    return false;
}

void KDirLister::handleError(KIO::Job *job)
{
    if (d->autoErrorHandling) {
        job->uiDelegate()->showErrorMessage();
    }
}

void KDirLister::setMainWindow(QWidget *window)
{
    d->window = window;
}

QWidget *KDirLister::mainWindow()
{
    return d->window;
}

KFileItemList KDirLister::items(WhichItems which) const
{
    switch (which) {
        case KDirLister::AllItems: {
            return d->allItems;
        }
        case KDirLister::FilteredItems: {
            return d->filteredItems;
        }
    }
    return KFileItemList();
}

bool KDirLister::delayedMimeTypes() const
{
    return d->delayedMimeTypes;
}

void KDirLister::setDelayedMimeTypes(bool delayedMimeTypes)
{
    d->delayedMimeTypes = delayedMimeTypes;
}

#include "moc_kdirlister.cpp"
