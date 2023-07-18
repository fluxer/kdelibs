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

#ifndef KFILEPREVIEWJOB_H
#define KFILEPREVIEWJOB_H

#include <kio/kio_export.h>
#include <kcompositejob.h>
#include <kfileitem.h>
#include <kjob.h>

#include <QPixmap>

class KFilePreviewJobPrivate;

/*!
    File preview job class

    @since 4.23
    @warning the API is subject to change
*/
class KIO_EXPORT KFilePreviewJob : public KCompositeJob
{
    Q_OBJECT
public:
    KFilePreviewJob(const KFileItemList &items, const QSize &size, QObject *parent = nullptr);
    ~KFilePreviewJob();

    void start() final;

Q_SIGNALS:
    void failed(const KFileItem &item);
    void gotPreview(const KFileItem &item, const QPixmap &preview);

private Q_SLOTS:
    void slotFinished();
    void slotResult(KJob *job);

private:
    Q_DISABLE_COPY(KFilePreviewJob);
    KFilePreviewJobPrivate* d;
};

#endif // KFILEPREVIEWJOB_H
