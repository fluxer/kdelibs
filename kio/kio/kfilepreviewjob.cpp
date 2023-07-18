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
#include "kjobtrackerinterface.h"
#include "kdebug.h"

#include <QThread>

class KFilePreviewJobPrivate : public QThread
{
    Q_OBJECT
public:
    KFilePreviewJobPrivate(const KFileItemList &items, const KFileItemList &localitems, const QSize &size, QObject *parent);

    void interrupt();
    void suspend();
    void resume();

    KFileItemList items() const;

Q_SIGNALS:
    void failed(const KFileItem &item);
    void preview(const KFileItem &item, const QPixmap &preview);

protected:
    void run() final;

private:
    KFileItemList m_items;
    KFileItemList m_localitems;
    QSize m_size;
    bool m_interrupt;
    bool m_suspend;
    KFilePreview* m_filepreview;
};

KFilePreviewJobPrivate::KFilePreviewJobPrivate(const KFileItemList &items, const KFileItemList &localitems, const QSize &size, QObject *parent)
    : QThread(parent),
    m_items(items),
    m_localitems(localitems),
    m_size(size),
    m_interrupt(false),
    m_suspend(false),
    m_filepreview(nullptr)
{
    m_filepreview = new KFilePreview(this);
}

void KFilePreviewJobPrivate::interrupt()
{
    m_interrupt = true;
}

void KFilePreviewJobPrivate::suspend()
{
    m_suspend = true;
}

void KFilePreviewJobPrivate::resume()
{
    m_suspend = false;
}

KFileItemList KFilePreviewJobPrivate::items() const
{
    return m_items;
}

void KFilePreviewJobPrivate::run()
{
    kDebug() << "creating previews";
    foreach (const KFileItem &item, m_localitems) {
        if (m_interrupt) {
            kDebug() << "interrupted creation of previews";
            break;
        }
        while (m_suspend) {
            QThread::msleep(500);
        }
        const QImage result = m_filepreview->preview(item, m_size);
        if (result.isNull()) {
            emit failed(item);
        } else {
            emit preview(item, QPixmap::fromImage(result));
        }
    }
    kDebug() << "done creating previews";
}

KFilePreviewJob::KFilePreviewJob(const KFileItemList &items, const QSize &size, QObject *parent)
    : KCompositeJob(parent),
    d(nullptr)
{
    qRegisterMetaType<KFileItem>();
    setUiDelegate(new KIO::JobUiDelegate());
    setCapabilities(KJob::Killable | KJob::Suspendable);

    KFileItemList localitems;
    foreach (const KFileItem &item, items) {
        if (item.isDir()) {
#if 0
            if (!item.isLocalFile()) {
                KIO::getJobTracker()->registerJob(this);
            }
            kDebug() << "directory item" << item.url();
            KIO::ListJob* listjob = KIO::listDir(item.url(), KIO::HideProgressInfo);
            addSubjob(listjob);
#endif
        } else if (item.isLocalFile()) {
            kDebug() << "local item" << item.url();
            localitems.append(item);
        } else {
#if 0
            KIO::getJobTracker()->registerJob(this);
            kDebug() << "remote item" << item.url();
            KIO::TransferJob* getjob = KIO::get(item.url(), KIO::NoReload, KIO::HideProgressInfo);
            addSubjob(getjob);
#endif
        }
    }

    d = new KFilePreviewJobPrivate(items, localitems, size, this);

    connect(d, SIGNAL(failed(KFileItem)), this, SIGNAL(failed(KFileItem)));
    connect(d, SIGNAL(preview(KFileItem,QPixmap)), this, SIGNAL(gotPreview(KFileItem,QPixmap)));
    connect(d, SIGNAL(finished()), this, SLOT(slotFinished()));
}

void KFilePreviewJob::start()
{
    foreach (const KFileItem &item, d->items()) {
        if (item.isDir()) {
            kWarning() << "directories not supported";
            emit failed(item);
        } else if (!item.isLocalFile()) {
            kWarning() << "remote items not supported";
            emit failed(item);
        }
    }

    d->start();
}

KFilePreviewJob::~KFilePreviewJob()
{
    // KIO::getJobTracker()->unregisterJob(this);
    d->wait();
    delete d;
}

bool KFilePreviewJob::doKill()
{
    d->interrupt();
    return true;
}

bool KFilePreviewJob::doSuspend()
{
    d->suspend();
    return true;
}

bool KFilePreviewJob::doResume()
{
    d->resume();
    return true;
}

void KFilePreviewJob::slotFinished()
{
    if (!hasSubjobs()) {
        emitResult();
    }
}

void KFilePreviewJob::slotResult(KJob *job)
{
    KCompositeJob::slotResult(job);
    if (!hasSubjobs()) {
        emitResult();
    }
}

#include "moc_kfilepreviewjob.cpp"
#include "kfilepreviewjob.moc"
