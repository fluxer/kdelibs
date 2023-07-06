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
#include "klocale.h"
#include "kio/jobuidelegate.h"
#include "kmessagebox.h"
#include "kprotocolmanager.h"
#include "kdesktopfile.h"
#include "kdebug.h"

#include <QFileInfo>

KDirListerPrivate::KDirListerPrivate(KDirLister *parent)
    : autoUpdate(true),
    delayedMimeTypes(false),
    autoErrorHandling(true),
    showingDotFiles(false),
    dirOnlyMode(false),
    recursive(false),
    complete(true),
    window(nullptr),
    listJob(nullptr),
    updateJob(nullptr),
    dirWatch(nullptr),
    dirNotify(nullptr),
    pendingUpdateTimer(new QTimer(parent)),
    m_parent(parent)
{
    pendingUpdateTimer->setSingleShot(true);
    m_parent->connect(
        pendingUpdateTimer, SIGNAL(timeout()),
        m_parent, SLOT(_k_slotUpdateDirectory())
    );
}

void KDirListerPrivate::processEntries(KIO::Job *job, const KIO::UDSEntryList &entries,
                                       KFileItem &rootItem, KFileItemList &allItems, KFileItemList &filteredItems,
                                       const bool watch)
{
    KIO::ListJob* processJob = qobject_cast<KIO::ListJob*>(job);
    Q_ASSERT(processJob);
    foreach (const KIO::UDSEntry &it, entries) {
        const QString name = it.stringValue(KIO::UDSEntry::UDS_NAME);
        if (name.isEmpty()) {
            continue;
        }
        const KFileItem item(it, processJob->url(), delayedMimeTypes, true);
        if (name == QLatin1String(".")) {
            rootItem = item;
            continue;
        } else if (name == QLatin1String("..")) {
            continue;
        }
        allItems.append(item);
        if (m_parent->matchesFilter(item) && m_parent->matchesMimeFilter(item)) {
            kDebug(7003) << "filtered entry" << item;
            filteredItems.append(item);
            if (watch && recursive && item.isDir()) {
                watchUrl(item.url());
            }
        }

        if (watch && item.isDesktopFile()) {
            const QString itempath = item.localPath();
            kDebug(7003) << "desktop file entry" << itempath;
            KDesktopFile desktopfile(itempath);
            const KUrl desktopurl = desktopfile.readUrl();
            if (desktopurl.isValid()) {
                watchUrl(desktopurl);
            }
        }
    }
}

void KDirListerPrivate::watchUrl(const KUrl &it)
{
    if (!autoUpdate) {
        return;
    }

    if (it.isLocalFile()) {
        const QString localfile = it.toLocalFile();
        kDebug(7003) << "watching" << localfile;
        if (!dirWatch) {
            dirWatch = new KDirWatch(m_parent);
            m_parent->connect(
                dirWatch, SIGNAL(dirty(QString)),
                m_parent, SLOT(_k_slotDirty(QString))
            );
        }
        const QFileInfo localinfo(localfile);
        if (localinfo.isDir()) {
            dirWatch->addDir(localfile);
        } else {
            dirWatch->addFile(localfile);
        }
    } else {
        kDebug(7003) << "watching remote" << it;
        if (!dirNotify) {
            dirNotify = new org::kde::KDirNotify(
                QString(), QString(), QDBusConnection::sessionBus(),
                m_parent
            );
            m_parent->connect(
                dirNotify, SIGNAL(FileRenamed(QString,QString)),
                m_parent, SLOT(_k_slotFileRenamed(QString,QString))
            );
            m_parent->connect(
                dirNotify, SIGNAL(FilesAdded(QString)),
                m_parent, SLOT(_k_slotFilesAdded(QString))
            );
            m_parent->connect(
                dirNotify, SIGNAL(FilesChanged(QStringList)),
                m_parent, SLOT(_k_slotFilesChangedOrRemoved(QStringList))
            );
            m_parent->connect(
                dirNotify, SIGNAL(FilesRemoved(QStringList)),
                m_parent, SLOT(_k_slotFilesChangedOrRemoved(QStringList))
            );
        }
        org::kde::KDirNotify::emitEnteredDirectory(it.url());
        watchedUrls.append(it);
    }
}

void KDirListerPrivate::unwatchUrl(const KUrl &it)
{
    if (!autoUpdate) {
        return;
    }

    if (it.isLocalFile()) {
        const QString localfile = it.toLocalFile();
        const QFileInfo localinfo(localfile);
        kDebug(7003) << "no longer watching" << localfile;
        // not tracking what it is, KDirWatch does tho
        dirWatch->removeDir(localfile);
        dirWatch->removeFile(localfile);
    } else {
        kDebug(7003) << "no longer watching remote" << it.url();
        org::kde::KDirNotify::emitLeftDirectory(it.url());
        watchedUrls.removeAll(it);
    }
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

void KDirListerPrivate::_k_slotRedirection(KIO::Job *job, const KUrl &url)
{
    Q_UNUSED(job);
    emit m_parent->redirection(url);
}

void KDirListerPrivate::_k_slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries)
{
    Q_ASSERT(listJob == qobject_cast<KIO::ListJob*>(job));
    kDebug(7003) << "got entries for" << listJob->url();
    processEntries(job, entries, rootFileItem, allFileItems, filteredFileItems, autoUpdate);
}

void KDirListerPrivate::_k_slotResult(KJob *job)
{
    Q_ASSERT(listJob == qobject_cast<KIO::ListJob*>(job));
    kDebug(7003) << "done listing" << listJob->url();
    if (job->error() != KJob::KilledJobError && job->error() != KJob::NoError) {
        m_parent->handleError(listJob);
    }
    listJob = nullptr;
    job->deleteLater();

    if (!filteredFileItems.isEmpty()) {
        emit m_parent->itemsAdded(filteredFileItems);
    }
    complete = true;
    emit m_parent->completed();

    watchUrl(url);
}

void KDirListerPrivate::_k_slotUpdateEntries(KIO::Job *job, const KIO::UDSEntryList &entries)
{
    Q_ASSERT(updateJob == qobject_cast<KIO::ListJob*>(job));
    kDebug(7003) << "got update entries for" << updateJob->url();
    processEntries(job, entries, updateRootFileItem, updateAllFileItems, updateFilteredFileItems, false);
}

void KDirListerPrivate::_k_slotUpdateResult(KJob *job)
{
    Q_ASSERT(updateJob == qobject_cast<KIO::ListJob*>(job));
    kDebug(7003) << "done updating" << updateJob->url();
    if (job->error() != KJob::KilledJobError && job->error() != KJob::NoError) {
        m_parent->handleError(updateJob);
    }
    updateJob = nullptr;
    job->deleteLater();

    KFileItemList addedItems;
    KFileItemList deletedItems;
    QList<QPair<KFileItem, KFileItem>> refreshedItems;

    if (!updateRootFileItem.isNull() && rootFileItem != updateRootFileItem) {
        // no way its deleted if it is not null, wasn't added either
        kDebug(7003) << "updated root entry" << rootFileItem;
        refreshedItems.append(qMakePair(rootFileItem, updateRootFileItem));
    }

    foreach (const KFileItem &item, filteredFileItems) {
        KFileItem founditem = updateFilteredFileItems.findByUrl(item.url());
        if (founditem.isNull()) {
            kDebug(7003) << "deleted entry" << item;
            deletedItems.append(item);
            unwatchUrl(item.url());
        } else if (item != founditem) {
            kDebug(7003) << "updated entry" << item;
            refreshedItems.append(qMakePair(item, founditem));
        }
    }
    foreach (const KFileItem &item, updateFilteredFileItems) {
        KFileItem founditem = filteredFileItems.findByUrl(item.url());
        if (founditem.isNull()) {
            kDebug(7003) << "added entry" << item;
            addedItems.append(item);
            if (autoUpdate && recursive && item.isDir()) {
                watchUrl(item.url());
            }
            if (autoUpdate && item.isDesktopFile()) {
                KDesktopFile desktopfile(item.localPath());
                const KUrl desktopurl = desktopfile.readUrl();
                if (desktopurl.isValid()) {
                    watchUrl(desktopurl);
                }
            }
        }
    }

    foreach (const KFileItem &item, updateFilteredFileItems) {
        // if an update was triggered might aswell refresh all .desktop files
        if (item.isDesktopFile()) {
            const KFileItem olditem = filteredFileItems.findByUrl(item.url());
            kDebug(7003) << "updating desktop entry" << item;
            refreshedItems.append(qMakePair(olditem, item));
        }
    }

    if (!addedItems.isEmpty()) {
        emit m_parent->itemsAdded(addedItems);
    }
    if (!deletedItems.isEmpty()) {
        emit m_parent->itemsDeleted(deletedItems);
    }
    if (!refreshedItems.isEmpty()) {
        emit m_parent->refreshItems(refreshedItems);
    }

    rootFileItem = updateRootFileItem;
    allFileItems = updateAllFileItems;
    filteredFileItems = updateFilteredFileItems;

    complete = true;
    emit m_parent->completed();
}

void KDirListerPrivate::_k_slotDirty(const QString &path)
{
    kDebug(7003) << "dirty path" << path;
    _k_slotUpdateDirectory();
}

void KDirListerPrivate::_k_slotFileRenamed(const QString &path, const QString &path2)
{
    kDebug(7003) << "file renamed" << path << path2;
    _k_slotFilesChangedOrRemoved(QStringList() << path);
    _k_slotFilesAdded(path2);
}

void KDirListerPrivate::_k_slotFilesAdded(const QString &path)
{
    kDebug(7003) << "file added" << path;
    const KUrl pathurl(path);
    const KUrl pathdirectory = pathurl.directory();
    foreach (const KUrl &it, watchedUrls) {
        if (it == pathdirectory || it == pathurl) {
            _k_slotUpdateDirectory();
            return;
        }
    }
}

void KDirListerPrivate::_k_slotFilesChangedOrRemoved(const QStringList &paths)
{
    kDebug(7003) << "files changed" << paths;
    foreach (const QString &it, paths) {
        const KUrl pathurl(it);
        foreach (const KUrl &it2, watchedUrls) {
            if (it2 == pathurl) {
                _k_slotUpdateDirectory();
                return;
            }
        }
        foreach (const KFileItem &it2, filteredFileItems) {
            if (it2.url() == pathurl) {
                _k_slotUpdateDirectory();
                return;
            }
        }
    }
}

void KDirListerPrivate::_k_slotUpdateDirectory()
{
    if (!pendingUpdateTimer->isActive()) {
        m_parent->updateDirectory();
    } else {
        pendingUpdateTimer->start(500);
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

bool KDirLister::openUrl(const KUrl &url, bool recursive)
{
    stop();

    foreach (const KUrl &it, d->watchedUrls) {
        d->unwatchUrl(it);
    }
    if (d->dirWatch) {
        d->dirWatch->disconnect(this);
        delete d->dirWatch;
        d->dirWatch = nullptr;
    }
    if (d->dirNotify) {
        // d->dirNotify->disconnect(this);
        delete d->dirNotify;
        d->dirNotify = nullptr;
    }

    kDebug(7003) << "opening" << url << recursive;
    d->url = url;
    d->recursive = recursive;
    d->rootFileItem = KFileItem();
    d->allFileItems.clear();
    d->filteredFileItems.clear();
    d->updateRootFileItem = KFileItem();
    d->updateAllFileItems.clear();
    d->updateFilteredFileItems.clear();
    d->watchedUrls.clear();
    emit clear();
    if (d->autoErrorHandling) {
        if (!url.isValid()) {
            KMessageBox::error(d->window, i18n("Malformed URL\n%1", url.prettyUrl()));
            return false;
        }
        if (!KProtocolManager::supportsListing(url)) {
            KMessageBox::error(d->window, i18n("URL cannot be listed\n%1", url.prettyUrl()));
            return false;
        }
    }
    d->complete = false;
    if (recursive) {
        d->listJob = KIO::listRecursive(url, KIO::HideProgressInfo);
    } else {
        d->listJob = KIO::listDir(url, KIO::HideProgressInfo);
    }
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
    connect(
        d->listJob, SIGNAL(redirection(KIO::Job*,KUrl)),
        this, SLOT(_k_slotRedirection(KIO::Job*,KUrl))
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

    emit started();
    return true;
}

void KDirLister::stop()
{
    if (d->pendingUpdateTimer->isActive()) {
        d->pendingUpdateTimer->stop();
    }
    bool emitcancel = false;
    if (d->updateJob) {
        kDebug(7003) << "killing update job for" << d->url;
        d->updateJob->disconnect(this);
        d->updateJob->kill();
        d->updateJob->deleteLater();
        d->updateJob = nullptr;
        emitcancel = true;
    }
    if (d->listJob) {
        kDebug(7003) << "killing job for" << d->url;
        d->listJob->disconnect(this);
        d->listJob->kill();
        d->listJob->deleteLater();
        d->listJob = nullptr;
        emitcancel = true;
    }
    if (emitcancel) {
        d->complete = true;
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
    if (!d->url.isValid()) {
        // attempt to update before anything was listed or after invalid was listed
        return;
    }

    if (d->updateJob) {
        kDebug(7003) << "updating in progress for" << d->url << d->recursive;
        return;
    }

    kDebug(7003) << "updating" << d->url << d->recursive;
    d->updateRootFileItem = KFileItem();
    d->updateAllFileItems.clear();
    d->updateFilteredFileItems.clear();
    d->complete = false;
    if (d->recursive) {
        d->updateJob = KIO::listRecursive(d->url, KIO::HideProgressInfo);
    } else {
        d->updateJob = KIO::listDir(d->url, KIO::HideProgressInfo);
    }
    d->updateJob->setAutoDelete(false);
    if (d->window) {
        d->updateJob->ui()->setWindow(d->window);
    }

    // private slots
    connect(
        d->updateJob, SIGNAL(entries(KIO::Job*, KIO::UDSEntryList)),
        this, SLOT(_k_slotUpdateEntries(KIO::Job*, KIO::UDSEntryList))
    );
    connect(
        d->updateJob, SIGNAL(result(KJob*)),
        this, SLOT(_k_slotUpdateResult(KJob*))
    );

    emit started();
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
    foreach (const KFileItem &it, d->allFileItems) {
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
    foreach (const KFileItem &it, d->allFileItems) {
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
    if (d->nameFilters.isEmpty()) {
        return true;
    }
    kDebug(7003) << "matchesFilter" << name << d->nameFilters;
    foreach (const QRegExp &it, d->nameFilters) {
        if (it.exactMatch(name)) {
            return true;
        }
    }
    return false;
}

bool KDirLister::matchesMimeFilter(const QString &mime) const
{
    if (d->mimeFilter.isEmpty()) {
        return true;
    }
    kDebug(7003) << "matchesMimeFilter" << mime << d->mimeFilter;
    const KMimeType::Ptr mimeptr = KMimeType::mimeType(mime);
    if (!mimeptr) {
        return false;
    }
    foreach (const QString &it, d->mimeFilter) {
        if (mimeptr->is(it)) {
            return true;
        }
    }
    return false;
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
            return d->allFileItems;
        }
        case KDirLister::FilteredItems: {
            return d->filteredFileItems;
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
