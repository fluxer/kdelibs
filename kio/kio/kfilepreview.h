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

#ifndef KFILEPREVIEW_H
#define KFILEPREVIEW_H

#include <kio/kio_export.h>
#include <kfileitem.h>

#include <QObject>
#include <QImage>

class KFilePreviewPrivate;

/*!
    File preview class

    @since 4.23
    @warning the API is subject to change
*/
class KIO_EXPORT KFilePreview : public QObject
{
    Q_OBJECT
public:
    KFilePreview(QObject *parent = nullptr);
    ~KFilePreview();

    QImage preview(const KFileItem &item, const QSize &size);

    static QStringList supportedMimeTypes();

private:
    Q_DISABLE_COPY(KFilePreview);
    KFilePreviewPrivate* d;
};

#endif // KFILEPREVIEW_H