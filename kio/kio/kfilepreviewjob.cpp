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

typedef QPair<KFileItem,KFileItem> KFilePreviewPair;

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
    qDebug() << "creating previews" << m_items;
    foreach (const KFilePreviewPair &itempair, m_items) {
        if (m_interrupt) {
            kDebug() << "interrupted creation of previews";
            break;
        }
        while (m_suspend) {
            QThread::msleep(500);
        }
        const QImage result = m_filepreview->preview(itempair.first, m_size, KFilePreview::makeKey(itempair.second));
        if (result.isNull()) {
            qDebug() << "failed to create preview for" << itempair.first;
            emit failed(itempair.second);
        } else {
            emit preview(itempair.second, QPixmap::fromImage(result));
        }
    }
    qDebug() << "done creating previews" << m_items;
}

class KFilePreviewJobPrivate
{
public:
    KFilePreviewJobPrivate();

    KFileItemList items;
    QList<KFilePreviewPair> remoteitems;
    QSize size;
    KFilePreviewJobThread* localthread;
    KFilePreviewJobThread* remotethread;
};

KFilePreviewJobPrivate::KFilePreviewJobPrivate()
    : localthread(nullptr),
    remotethread(nullptr)
{
}


KFilePreviewJob::KFilePreviewJob(const KFileItemList &items, const QSize &size, QObject *parent)
    : KCompositeJob(parent),
    d(new KFilePreviewJobPrivate())
{
    qRegisterMetaType<KFileItem>();
    setUiDelegate(new KIO::JobUiDelegate());
    setCapabilities(KJob::Killable | KJob::Suspendable);

    d->items = items;
    d->size = size;

    QList<KFilePreviewPair> localitems;
    foreach (const KFileItem &item, items) {
        if (item.isDir()) {
#if 0
            kDebug() << "directory item" << item.url();
            KIO::ListJob* listjob = KIO::listDir(item.url(), KIO::HideProgressInfo);
            addSubjob(listjob);
#endif
        } else if (item.isLocalFile()) {
            kDebug() << "local item" << item.url();
            localitems.append(qMakePair(item, item));
        } else {
            kDebug() << "remote item" << item.url();
            KIO::FileCopyJob* filecopyjob = KIO::file_copy(
                item.mostLocalUrl(), KUrl::fromPath(KTemporaryFile::filePath()),
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
    foreach (const KFileItem &item, d->items) {
        if (item.isDir()) {
            kWarning() << "directories not supported";
            emit failed(item);
        }
    }

    if (d->localthread) {
        d->localthread->start();
    }
}

KFilePreviewJob::~KFilePreviewJob()
{
    if (d->localthread) {
        d->localthread->wait();
    }
    if (d->remotethread) {
        d->remotethread->wait();
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
    return true;
}

void KFilePreviewJob::slotFinished()
{
    if (!hasSubjobs()
        && (!d->localthread || d->localthread->isFinished())
        && (!d->remotethread || d->remotethread->isFinished())) {
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
