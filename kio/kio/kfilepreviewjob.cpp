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
#include "kjobtrackerinterface.h"
#include "kdebug.h"

#include <QThread>

class KFilePreviewJobPrivate : public QThread
{
    Q_OBJECT
public:
    KFilePreviewJobPrivate(const KFileItemList &items, const QSize &size, QObject *parent);

Q_SIGNALS:
    void failed(const KFileItem &item);
    void preview(const KFileItem &item, const QPixmap &preview);

protected:
    void run() final;

private:
    KFileItemList m_items;
    QSize m_size;
};

KFilePreviewJobPrivate::KFilePreviewJobPrivate(const KFileItemList &items, const QSize &size, QObject *parent)
    : QThread(parent),
    m_items(items),
    m_size(size)
{
}

void KFilePreviewJobPrivate::run()
{
    kDebug() << "creating previews";
    KFilePreview kfilepreview;
    foreach (const KFileItem &item, m_items) {
        const QImage result = kfilepreview.preview(item, m_size);
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
    KIO::getJobTracker()->registerJob(this);

    KFileItemList localitems;
    foreach (const KFileItem &item, items) {
        if (item.isDir()) {
            kWarning() << "directories not supported";
            continue;
#if 0
            kDebug() << "directory item" << item.url();
            KIO::ListJob* listjob = KIO::listDir(item.url(), KIO::HideProgressInfo);
            addSubjob(listjob);
#endif
        } else if (item.isLocalFile()) {
            kDebug() << "local item" << item.url();
            localitems.append(item);
        } else {
            kWarning() << "remote items not supported";
            continue;
#if 0
            kDebug() << "remote item" << item.url();
            KIO::TransferJob* getjob = KIO::get(item.url(), KIO::NoReload, KIO::HideProgressInfo);
            addSubjob(getjob);
#endif
        }
    }

    d = new KFilePreviewJobPrivate(localitems, size, this);

    connect(d, SIGNAL(failed(KFileItem)), this, SIGNAL(failed(KFileItem)));
    connect(d, SIGNAL(preview(KFileItem,QPixmap)), this, SIGNAL(gotPreview(KFileItem,QPixmap)));
    connect(d, SIGNAL(finished()), this, SLOT(slotFinished()));

    start();
}

void KFilePreviewJob::start()
{
    d->start();
}

KFilePreviewJob::~KFilePreviewJob()
{
    KIO::getJobTracker()->unregisterJob(this);
    delete d;
}

void KFilePreviewJob::slotFinished()
{
    qDebug() << Q_FUNC_INFO << hasSubjobs();
    if (!hasSubjobs()) {
        emitResult();
    }
}

void KFilePreviewJob::slotResult(KJob *job)
{
    qDebug() << Q_FUNC_INFO << job << hasSubjobs();
    KCompositeJob::slotResult(job);
    if (!hasSubjobs()) {
        emitResult();
    }
}

#include "moc_kfilepreviewjob.cpp"
#include "kfilepreviewjob.moc"
