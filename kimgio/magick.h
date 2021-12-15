/*  This file is part of the KDE libraries
    Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KIMG_MAGICK_H
#define KIMG_MAGICK_H

#include <QtGui/qimageiohandler.h>

#include <Magick++/Image.h>

class MagickHandler : public QImageIOHandler
{
public:
    MagickHandler();
    ~MagickHandler();

    bool canRead() const final;
    bool read(QImage *image) final;
    bool write(const QImage &image) final;

    QByteArray name() const final;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const final;
    bool supportsOption(QImageIOHandler::ImageOption option) const final;

    bool jumpToNextImage() final;
    bool jumpToImage(int imageNumber) final;
    int loopCount() const final;
    int imageCount() const final;
    int nextImageDelay() const final;
    int currentImageNumber() const final;

private:
    int m_loopcount;
    int m_imagecount;
    int m_imagedelay;
    int m_currentimage;
    mutable std::vector<Magick::Image> m_magickimages;
    mutable QIODevice* m_device;
};

class MagickPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const final;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const final;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const final;
};

#endif // KIMG_MAGICK_H
 
