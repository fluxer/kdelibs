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

#include "kfilepreviewjob.h"
#include "kfilepreview.h"
#include "kio/global.h"
#include "kio/job.h"
#include "kio/jobuidelegate.h"
#include "ktemporaryfile.h"
#include "kdebug.h"

#include <QThread>
#include <QThreadPool>
#include <QRunnable>

typedef QPair<KFileItem,KFileItem> KFilePreviewPair;

static const int s_previewsuspendtime = 500;

class KFilePreviewJobThread : public QThread
{
    Q_OBJECT
public:
    KFilePreviewJobThread(const QList<KFilePreviewPair> &items, const QSize &size, QObject *parent);

    void interrupt();
    void suspend();
    void resume();

Q_SIGNALS:
    void failed(const KFileItem &item);
    void preview(const KFileItem &item, const QPixmap &preview);

protected:
    void run() final;

private:
    QList<KFilePreviewPair> m_items;
    QSize m_size;
    bool m_interrupt;
    bool m_suspend;
    KFilePreview* m_filepreview;
};

KFilePreviewJobThread::KFilePreviewJobThread(const QList<KFilePreviewPair> &items, const QSize &size, QObject *parent)
    : QThread(parent),
    m_items(items),
    m_size(size),
    m_interrupt(false),
    m_suspend(false),
    m_filepreview(nullptr)
{
    m_filepreview = new KFilePreview(this);
}

void KFilePreviewJobThread::interrupt()
{
    m_interrupt = true;
}

void KFilePreviewJobThread::suspend()
{
    m_suspend = true;
}

void KFilePreviewJobThread::resume()
{
    m_suspend = false;
}

void KFilePreviewJobThread::run()
{
    kDebug() << "creating previews" << m_items;
    foreach (const KFilePreviewPair &itempair, m_items) {
        if (m_interrupt) {
            kDebug() << "interrupted creation of previews";
            break;
        }
        while (m_suspend) {
            QThread::msleep(s_previewsuspendtime);
        }
        const QImage result = m_filepreview->preview(itempair.first, m_size, KFilePreview::makeKey(itempair.second));
        if (result.isNull()) {
            kDebug() << "failed to create preview for" << itempair.first;
            emit failed(itempair.second);
        } else {
            emit preview(itempair.second, QPixmap::fromImage(result));
        }
    }
    kDebug() << "done creating previews" << m_items;
}

class KFilePreviewJobRunnable : public QObject, public QRunnable
{
    Q_OBJECT
public:
    KFilePreviewJobRunnable(const KFileItemList &items, const KUrl &directory, const QSize &size, QObject *parent);

    void interrupt();
    void suspend();
    void resume();

Q_SIGNALS:
    void failed(const KFileItem &item);
    void preview(const KFileItem &item, const QPixmap &preview);
    void finished();

protected:
    void run() final;

private:
    KFileItemList m_items;
    KUrl m_directory;
    QSize m_size;
    bool m_interrupt;
    bool m_suspend;
    KFilePreview* m_filepreview;
};

KFilePreviewJobRunnable::KFilePreviewJobRunnable(const KFileItemList &items, const KUrl &directory, const QSize &size, QObject *parent)
    : QRunnable(),
    QObject(parent),
    m_items(items),
    m_directory(directory),
    m_size(size),
    m_interrupt(false),
    m_suspend(false),
    m_filepreview(nullptr)
{
    m_filepreview = new KFilePreview(this);
    setAutoDelete(false);
}

void KFilePreviewJobRunnable::interrupt()
{
    m_interrupt = true;
}

void KFilePreviewJobRunnable::suspend()
{
    m_suspend = true;
}

void KFilePreviewJobRunnable::resume()
{
    m_suspend = false;
}

void KFilePreviewJobRunnable::run()
{
    KFileItem directoryitem(m_directory, QString(), KFileItem::Unknown);
    kDebug() << "creating directory previews" << directoryitem;
    QList<QImage> previews;
    foreach (const KFileItem &item, m_items) {
        if (m_interrupt) {
            kDebug() << "interrupted creation of directory previews";
            break;
        }
        while (m_suspend) {
            QThread::msleep(s_previewsuspendtime);
        }
        const QImage result = m_filepreview->preview(item, m_size, KFilePreview::makeKey(item));
        if (result.isNull()) {
            kDebug() << "failed to create preview for" << item;
        } else {
            previews.append(result);
        }
    }
    // TODO: combine 4 of the previews
    kDebug() << "done creating directory previews" << directoryitem;
    emit failed(directoryitem);
    emit finished();
}


class KFilePreviewJobPrivate
{
public:
    KFilePreviewJobPrivate();

    QList<KFilePreviewPair> remoteitems;
    QMap<KUrl, KFileItemList> directoryitems;
    QSize size;
    KFilePreviewJobThread* localthread;
    KFilePreviewJobThread* remotethread;
    QList<KFilePreviewJobRunnable*> directorythreads;
    QThreadPool* directorypool;
};

KFilePreviewJobPrivate::KFilePreviewJobPrivate()
    : localthread(nullptr),
    remotethread(nullptr),
    directorypool(nullptr)
{
}


KFilePreviewJob::KFilePreviewJob(const KFileItemList &items, const QSize &size, QObject *parent)
    : KCompositeJob(parent),
    d(new KFilePreviewJobPrivate())
{
    qRegisterMetaType<KFileItem>();
    setUiDelegate(new KIO::JobUiDelegate());
    setCapabilities(KJob::Killable | KJob::Suspendable);

    d->size = size;

    QList<KFilePreviewPair> localitems;
    foreach (const KFileItem &item, items) {
        if (item.isDir()) {
            kDebug() << "directory item" << item.url();
            KIO::ListJob* listjob = KIO::listDir(item.url(), KIO::HideProgressInfo);
            connect(listjob, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)), this, SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)));
            addSubjob(listjob);
        } else if (item.isLocalFile()) {
            kDebug() << "local item" << item.url();
            localitems.append(qMakePair(item, item));
        } else {
            kDebug() << "remote item" << item.url();
            // TODO: extension for MIME glob matching
            const KUrl desturl = KUrl::fromPath(KTemporaryFile::filePath());
            KIO::FileCopyJob* filecopyjob = KIO::file_copy(
                item.mostLocalUrl(), desturl,
                -1, KIO::Overwrite | KIO::HideProgressInfo
            );
            // checked for by camera KIO slave
            filecopyjob->addMetaData("thumbnail", "1");
            addSubjob(filecopyjob);
        }
    }

    if (!localitems.isEmpty()) {
        d->localthread = new KFilePreviewJobThread(localitems, d->size, this);
        connect(d->localthread, SIGNAL(failed(KFileItem)), this, SIGNAL(failed(KFileItem)));
        connect(d->localthread, SIGNAL(preview(KFileItem,QPixmap)), this, SIGNAL(gotPreview(KFileItem,QPixmap)));
        connect(d->localthread, SIGNAL(finished()), this, SLOT(slotFinished()));
    }
}

void KFilePreviewJob::start()
{
    if (d->localthread) {
        d->localthread->start();
    }
}

KFilePreviewJob::~KFilePreviewJob()
{
    // FIXME: no events processing
    if (d->localthread) {
        d->localthread->wait();
    }
    if (d->remotethread) {
        d->remotethread->wait();
    }
    if (d->directorypool) {
        d->directorypool->waitForDone();
    }
    delete d;
}

bool KFilePreviewJob::doKill()
{
    if (d->localthread) {
        d->localthread->interrupt();
    }
    if (d->remotethread) {
        d->remotethread->interrupt();
    }
    QListIterator<KFilePreviewJobRunnable*> directoryiterator(d->directorythreads);
    while (directoryiterator.hasNext()) {
        KFilePreviewJobRunnable *directorythread = directoryiterator.next();
        directorythread->interrupt();
    }
    return true;
}

bool KFilePreviewJob::doSuspend()
{
    if (d->localthread) {
        d->localthread->suspend();
    }
    if (d->remotethread) {
        d->remotethread->suspend();
    }
    QListIterator<KFilePreviewJobRunnable*> directoryiterator(d->directorythreads);
    while (directoryiterator.hasNext()) {
        KFilePreviewJobRunnable *directorythread = directoryiterator.next();
        directorythread->suspend();
    }
    return true;
}

bool KFilePreviewJob::doResume()
{
    if (d->localthread) {
        d->localthread->resume();
    }
    if (d->remotethread) {
        d->remotethread->resume();
    }
    QListIterator<KFilePreviewJobRunnable*> directoryiterator(d->directorythreads);
    while (directoryiterator.hasNext()) {
        KFilePreviewJobRunnable *directorythread = directoryiterator.next();
        directorythread->resume();
    }
    return true;
}

void KFilePreviewJob::slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries)
{
    KIO::ListJob* listjob = qobject_cast<KIO::ListJob*>(job);
    const KUrl joburl = listjob->url();
    foreach (const KIO::UDSEntry &entry, entries) {
        const QString entryname = entry.stringValue(KIO::UDSEntry::UDS_NAME);
        if (entryname == QLatin1String(".") || entryname == QLatin1String("..")) {
            continue;
        }
        const KFileItem item(entry, joburl, true, true);
        d->directoryitems[joburl].append(item);
    }
}

void KFilePreviewJob::slotFinished()
{
    KFilePreviewJobRunnable* directorythread = qobject_cast<KFilePreviewJobRunnable*>(sender());
    if (directorythread) {
        // qDebug() << Q_FUNC_INFO << directorythread;
        d->directorythreads.removeAll(directorythread);
        delete directorythread;
    }
    // qDebug() << Q_FUNC_INFO << d->directorythreads;
    if (!hasSubjobs()
        && (!d->localthread || d->localthread->isFinished())
        && (!d->remotethread || d->remotethread->isFinished())
        && d->directorythreads.isEmpty()) {
        emitResult();
    }
}

void KFilePreviewJob::slotResult(KJob *job)
{
    KCompositeJob::slotResult(job);
    KIO::FileCopyJob* filecopyjob = qobject_cast<KIO::FileCopyJob*>(job);
    if (filecopyjob) {
        const KFileItem remoteitem(filecopyjob->srcUrl(), QString(), KFileItem::Unknown);
        const KFileItem localitem(filecopyjob->destUrl(), QString(), KFileItem::Unknown);
        d->remoteitems.append(qMakePair(localitem, remoteitem));
    }
    KIO::ListJob* listjob = qobject_cast<KIO::ListJob*>(job);
    if (listjob) {
        const KUrl joburl = listjob->url();
        KFilePreviewJobRunnable* directorythread = new KFilePreviewJobRunnable(
            d->directoryitems[joburl], joburl, d->size, this
        );
        connect(directorythread, SIGNAL(failed(KFileItem)), this, SIGNAL(failed(KFileItem)));
        connect(directorythread, SIGNAL(preview(KFileItem,QPixmap)), this, SIGNAL(gotPreview(KFileItem,QPixmap)));
        connect(directorythread, SIGNAL(finished()), this, SLOT(slotFinished()));
        d->directorythreads.append(directorythread);
        if (!d->directorypool) {
            d->directorypool = new QThreadPool(this);
            // two threads already (possibly) for local and remote items
            d->directorypool->setMaxThreadCount(qMax(d->directorypool->maxThreadCount() - 2, 1));
        }
        d->directorypool->start(directorythread);
    }
    if (!hasSubjobs() && !d->remoteitems.isEmpty()) {
        d->remotethread = new KFilePreviewJobThread(d->remoteitems, d->size, this);
        connect(d->remotethread, SIGNAL(failed(KFileItem)), this, SIGNAL(failed(KFileItem)));
        connect(d->remotethread, SIGNAL(preview(KFileItem,QPixmap)), this, SIGNAL(gotPreview(KFileItem,QPixmap)));
        connect(d->remotethread, SIGNAL(finished()), this, SLOT(slotFinished()));
        d->remotethread->start();
    }
}

#include "moc_kfilepreviewjob.cpp"
#include "kfilepreviewjob.moc"
