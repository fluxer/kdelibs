/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "ico.h"

#include <QImage>
#include <kdebug.h>

#include <limits.h>

static const char* const s_icopluginformat = "ico";

static const ushort s_peekbuffsize = 32;
static const uchar s_pngheader[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
static const ushort s_pngheadersize = 8;
// for reference:
// https://en.wikipedia.org/wiki/List_of_file_signatures
static const uchar s_icoheader[] = { 0x0, 0x0, 0x1, 0x0, 0x0 };
static const uchar s_icoheader1[] = { 0x0, 0x0, 0x1, 0x0, 0x01 };
static const uchar s_icoheader2[] = { 0x0, 0x0, 0x1, 0x0, 0x02 };
static const uchar s_icoheader3[] = { 0x0, 0x0, 0x1, 0x0, 0x03 };
static const uchar s_icoheader4[] = { 0x0, 0x0, 0x1, 0x0, 0x04 };
static const uchar s_icoheader5[] = { 0x0, 0x0, 0x1, 0x0, 0x05 };
static const uchar s_icoheader6[] = { 0x0, 0x0, 0x1, 0x0, 0x06 };
static const uchar s_icoheader7[] = { 0x0, 0x0, 0x1, 0x0, 0x07 };
static const uchar s_icoheader8[] = { 0x0, 0x0, 0x1, 0x0, 0x08 };
static const uchar s_icoheader9[] = { 0x0, 0x0, 0x1, 0x0, 0x09 };

static const struct HeadersTblData {
    const uchar *header;
    const int headersize;
} HeadersTbl[] = {
    { s_icoheader, 5 },
    { s_icoheader1, 5 },
    { s_icoheader2, 5 },
    { s_icoheader3, 5 },
    { s_icoheader4, 5 },
    { s_icoheader5, 5 },
    { s_icoheader6, 5 },
    { s_icoheader7, 5 },
    { s_icoheader8, 5 },
    { s_icoheader9, 5 }
};
static const qint16 HeadersTblSize = sizeof(HeadersTbl) / sizeof(HeadersTblData);

enum ICOType {
    TypeIcon = 1,
    TypeCursor = 2
};

enum BMPCompression {
    CompressionRGB = 0,
    CompressionRLE8 = 1,
    CompressionRLE4 = 2,
    CompressionBitFields = 3,
    CompressionJPEG = 4,
    CompressionPNG = 5,
    CompressionAlphaBitFields = 6,
    CompressionCMYK = 7,
    CompressionCMYKRLE8 = 8,
    CompressionCMYKRLE4 = 9,
};

ICOHandler::ICOHandler()
{
}

ICOHandler::~ICOHandler()
{
}

bool ICOHandler::canRead() const
{
    if (ICOHandler::canRead(device())) {
        setFormat(s_icopluginformat);
        return true;
    }
    return false;
}

// for reference:
// https://en.wikipedia.org/wiki/ICO_(file_format)
// https://en.wikipedia.org/wiki/BMP_file_format
bool ICOHandler::read(QImage *image)
{
    Q_ASSERT(sizeof(uchar) == 1);
    Q_ASSERT(sizeof(ushort) == 2);
    Q_ASSERT(sizeof(uint) == 4);

    QDataStream datastream(device());
    datastream.setByteOrder(QDataStream::LittleEndian);
    ushort icoreserved = 0;
    ushort icotype = 0;
    ushort iconimages = 0;
    datastream >> icoreserved;
    datastream >> icotype;
    datastream >> iconimages;

    if (icotype == ICOType::TypeCursor) {
        kWarning() << "Cursor icons are not supported";
        return false;
    } else if (icotype != ICOType::TypeIcon) {
        kWarning() << "Invalid icon type" << icotype;
        return false;
    } else if (iconimages < 1) {
        kDebug() << "No images" << iconimages;
        return false;
    }

    for (ushort ii = 0; ii < iconimages; ii++) {
        uchar icowidth = 0;
        uchar icoheight = 0;
        uchar icocolors = 0;
        uchar icoreserved2 = 0;
        ushort icoplaneorhhs = 0;
        ushort icobpporvhs = 0;
        uint icoimagesize = 0;
        uint icoimageoffset = 0;
        datastream >> icowidth;
        datastream >> icoheight;
        datastream >> icocolors;
        datastream >> icoreserved2;
        datastream >> icoplaneorhhs;
        datastream >> icobpporvhs;
        datastream >> icoimagesize;
        datastream >> icoimageoffset;

        if (Q_UNLIKELY(icoimageoffset > datastream.device()->size())) {
            kWarning() << "Invalid image offset" << icoimageoffset;
            return false;
        }

        if (Q_UNLIKELY(icoimagesize >= INT_MAX)) {
            kWarning() << "ICO image size is too big" << icoimagesize;
            return false;
        }

        datastream.device()->seek(icoimageoffset);
        QByteArray imagebytes(icoimagesize, char(0));
        if (Q_UNLIKELY(datastream.readRawData(imagebytes.data(), icoimagesize) != icoimagesize)) {
            kWarning() << "Could not read image data";
            return false;
        }

        if (imagebytes.size() > s_pngheadersize &&
            ::memcmp(imagebytes.constData(), s_pngheader, s_pngheadersize) != 0) {
            datastream.device()->seek(icoimageoffset);
            uint bmpheadersize = 0;
            uint bmpwidth = 0;
            uint bmpheight = 0;
            ushort bmpplanes = 0;
            ushort bmpbpp = 0;
            uint bmpcompression = 0;
            uint bmpimagesize = 0;
            uint bmphppm = 0;
            uint bmpvppm = 0;
            uint bmpncolors = 0;
            uint bmpnimportantcolors = 0;
            datastream >> bmpheadersize;
            datastream >> bmpwidth;
            datastream >> bmpheight;
            datastream >> bmpplanes;
            datastream >> bmpbpp;
            datastream >> bmpcompression;
            datastream >> bmpimagesize;
            datastream >> bmphppm;
            datastream >> bmpvppm;
            datastream >> bmpncolors;
            datastream >> bmpnimportantcolors;

            if (bmpheadersize != 40) {
                kWarning() << "Invalid BMP info header size" << bmpheadersize;
                return false;
            }

            if (bmpcompression != BMPCompression::CompressionRGB) {
                kWarning() << "Unsupported BMP compression" << bmpcompression;
                return false;
            }

            if (Q_UNLIKELY(bmpimagesize >= INT_MAX)) {
                kWarning() << "BMP image size is too big" << bmpimagesize;
                return false;
            }

            imagebytes.resize(bmpimagesize);
            if (Q_UNLIKELY(datastream.readRawData(imagebytes.data(), bmpimagesize) != bmpimagesize)) {
                kWarning() << "Could not read BMP image data";
                return false;
            }

            // fallbacks
            const int imagewidth = (icowidth ? icowidth : bmpwidth);
            const int imageheight = (icoheight ? icoheight : bmpheight);

            int imagebounds = 0;
            QImage::Format imageformat = QImage::Format_ARGB32;
            switch (bmpbpp) {
                case 32: {
                    imagebounds = (imagewidth * imageheight * 4);
                    break;
                }
                case 24: {
                    imageformat = QImage::Format_RGB32;
                    imagebounds = (imagewidth * imageheight * 3);
                    break;
                }
                default: {
                    kWarning() << "Unsupported BMP bits per-pixel" << bmpbpp;
                    return false;
                }
            }

            QImage bmpimage(imagewidth, imageheight, imageformat);
            if (Q_UNLIKELY(bmpimage.isNull())) {
                kWarning() << "Could not create BMP image" << imagewidth << imageheight << imageformat;
                return false;
            }

            if (bmpimagesize != imagebounds) {
                // data may be padded
                kDebug() << "BMP and QImage bytes count mismatch" << bmpimagesize << imagebounds;
            }

            switch (bmpbpp) {
                case 32: {
                    QRgb* bmpimagebits = reinterpret_cast<QRgb*>(bmpimage.bits());
                    for (uint bi = 0; bi < bmpimagesize && bi < imagebounds; bi += 4) {
                        *bmpimagebits = qRgba(imagebytes.at(bi + 2), imagebytes.at(bi + 1), imagebytes.at(bi), imagebytes.at(bi + 3));
                        bmpimagebits++;
                    }
                    break;
                }
                case 24: {
                    QRgb* bmpimagebits = reinterpret_cast<QRgb*>(bmpimage.bits());
                    for (uint bi = 0; bi < bmpimagesize && bi < imagebounds; bi += 3) {
                        *bmpimagebits = qRgb(imagebytes.at(bi + 2), imagebytes.at(bi + 1), imagebytes.at(bi));
                        bmpimagebits++;
                    }
                    break;
                }
                default: {
                    Q_ASSERT(false);
                    break;
                }
            }
            // pixel data is backwards so flip the image vertically
            *image = bmpimage.mirrored(false, true);
            kDebug() << "Valid BMP image" << ii;
            return true;
        }

        const QImage pngimage = QImage::fromData(imagebytes.constData(), imagebytes.size(), "PNG");
        if (Q_LIKELY(!pngimage.isNull())) {
            kDebug() << "Valid PNG image" << ii;
            *image = pngimage;
            return true;
        }
    }

    kWarning() << "No images could be loaded";
    return false;
}

bool ICOHandler::write(const QImage &image)
{
    // this plugin is a read-only kind of plugin
    return false;
}

QByteArray ICOHandler::name() const
{
    return s_icopluginformat;
}

bool ICOHandler::canRead(QIODevice *device)
{
    if (Q_UNLIKELY(!device)) {
        kWarning() << "Called with no device";
        return false;
    }

    const QByteArray data = device->peek(s_peekbuffsize);

    // ICONDIR, ICONDIRENTRY and one bit of data
    if (data.size() < 23) {
        kDebug() << "Not enough data for ICO";
        return false;
    }

    for (int i = 0; i < HeadersTblSize; i++) {
        if (data.size() >= HeadersTbl[i].headersize &&
            ::memcmp(data.constData(), HeadersTbl[i].header, HeadersTbl[i].headersize) == 0) {
            kDebug() << "Header detected";
            return true;
        }
    }

    return false;
}

QStringList ICOPlugin::keys() const
{
    return QStringList() << s_icopluginformat;
}

QList<QByteArray> ICOPlugin::mimeTypes() const
{
    static const QList<QByteArray> list = QList<QByteArray>()
        << "image/vnd.microsoft.icon";
    return list;
}

QImageIOPlugin::Capabilities ICOPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == s_icopluginformat) {
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead);
    }
    if (!device || !device->isOpen()) {
        return 0;
    }
    if (device->isReadable() && ICOHandler::canRead(device)) {
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead);
    }
    return 0;
}

QImageIOHandler *ICOPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new ICOHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN2(ico, ICOPlugin)
