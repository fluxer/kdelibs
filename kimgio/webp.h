/*
QImageIO Routines to read/write WebP images.

Copyright (c) 2012,2013 Martin Koller <kollix@aon.at>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WEBP_H
#define WEBP_H

#include <QStringList>
#include <QImageIOHandler>

class WebPHandler : public QImageIOHandler
{
public:
    WebPHandler();

    bool canRead() const final;
    bool read(QImage *image) final;
    bool write(const QImage &image) final;

    QByteArray name() const final;

    bool supportsOption(QImageIOHandler::ImageOption option) const final;
    QVariant option(QImageIOHandler::ImageOption option) const final;
    void setOption(QImageIOHandler::ImageOption option, const QVariant &value) final;

    static bool canRead(QIODevice *device);

private:
    int quality;
};


class WebPPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    QList<QByteArray> mimeTypes() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const final;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const final;
};

#endif
