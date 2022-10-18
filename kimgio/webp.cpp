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

#include "webp.h"
#include "kdebug.h"
#include <QImage>
#include <QVariant>

#include <webp/decode.h>
#include <webp/encode.h>

WebPHandler::WebPHandler()
    : quality(100)
{
}

bool WebPHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("webp");
        return true;
    }
    return false;
}

bool WebPHandler::read(QImage *image)
{
    const QByteArray data = device()->readAll();

    WebPBitstreamFeatures features;
    const VP8StatusCode ret = WebPGetFeatures(reinterpret_cast<const uint8_t*>(data.constData()),
                                              data.size(), &features);
    if (ret != VP8_STATUS_OK) {
        kWarning() << "Could not get image features";
        return false;
    }

    if (features.has_alpha) {
        *image = QImage(features.width, features.height, QImage::Format_ARGB32);
    } else {
        *image = QImage(features.width, features.height, QImage::Format_RGB32);
    }
    if (Q_UNLIKELY(image->isNull())) {
        // out of memory
        kWarning() << "Could not create image";
        return false;
    }

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    const uint8_t* output = WebPDecodeARGBInto(
        reinterpret_cast<const uint8_t*>(data.constData()), data.size(),
        reinterpret_cast<uint8_t*>(image->bits()), image->byteCount(),
        image->bytesPerLine()
    );
#else
    const uint8_t* output = WebPDecodeBGRAInto(
        reinterpret_cast<const uint8_t*>(data.constData()), data.size(),
        reinterpret_cast<uint8_t*>(image->bits()), image->byteCount(),
        image->bytesPerLine()
    );
#endif
    if (Q_UNLIKELY(!output)) {
        kWarning() << "Could not decode image";
        return false;
    }

    return true;
}

bool WebPHandler::write(const QImage &image)
{
    if (image.isNull()) {
        return false;
    } else if (Q_UNLIKELY(image.height() >= WEBP_MAX_DIMENSION || image.width() >= WEBP_MAX_DIMENSION)) {
        // limitation in WebP
        kWarning() << "Image dimension limit";
        return false;
    }

    QImage image32 = image;
    if (image32.depth() != 32) {
        image32 = image32.convertToFormat(QImage::Format_RGB32);
    }

    size_t idx = 0;
    uint8_t *imageData = new uint8_t[image32.width() * image32.height() * (3 + image32.hasAlphaChannel())];
    for (int y = 0; y < image32.height(); y++) {
        const QRgb *scanline = reinterpret_cast<const QRgb*>(image32.constScanLine(y));
        for (int x = 0; x < image32.width(); x++) {
            imageData[idx++] = qRed(scanline[x]);
            imageData[idx++] = qGreen(scanline[x]);
            imageData[idx++] = qBlue(scanline[x]);

            if (image32.hasAlphaChannel()) {
                imageData[idx++] = qAlpha(scanline[x]);
            }
        }
    }

    size_t size = 0;
    uint8_t *output = nullptr;
    if (image32.hasAlphaChannel()) {
        size = WebPEncodeRGBA(imageData, image32.width(), image32.height(), image32.width() * 4, quality, &output);
    } else {
        size = WebPEncodeRGB(imageData, image32.width(), image32.height(), image32.width() * 3, quality, &output);
    }
    delete []imageData;

    if (Q_UNLIKELY(size == 0)) {
        kWarning() << "Could not encode image";
        WebPFree(output);
        return false;
    }

    if (Q_UNLIKELY(device()->write(reinterpret_cast<const char*>(output), size) != size)) {
        kWarning() << "Could not write image";
        WebPFree(output);
        return false;
    }
    WebPFree(output);

    return true;
}

QByteArray WebPHandler::name() const
{
    return "webp";
}

bool WebPHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
    return (option == QImageIOHandler::Quality) || (option == QImageIOHandler::Size);
}

QVariant WebPHandler::option(QImageIOHandler::ImageOption option) const
{
    switch (option) {
        case QImageIOHandler::Quality: {
            return quality;
        }
        case QImageIOHandler::Size: {
            const QByteArray data = device()->peek(26);
            int width = 0, height = 0;
            if (WebPGetInfo(reinterpret_cast<const uint8_t*>(data.constData()),
                            data.size(), &width, &height) == 0) {
                return QSize(); // header error
            }
            return QSize(width, height);
        }
        default: {
            return QVariant();
        }
    }
    Q_UNREACHABLE();
}

void WebPHandler::setOption(QImageIOHandler::ImageOption option, const QVariant &value)
{
    if (option == QImageIOHandler::Quality) {
        const int newquality = value.toInt();
        // -1 means default
        if (newquality == -1) {
            quality = 100;
        } else {
            quality = qBound(0, newquality, 100);
        }
    }
}

bool WebPHandler::canRead(QIODevice *device)
{
    if (Q_UNLIKELY(!device)) {
        qWarning("WebPHandler::canRead() called with no device");
        return false;
    }

    // WebP file header: 4 bytes "RIFF", 4 bytes length, 4 bytes "WEBP"
    const QByteArray header = device->peek(12);
    return (header.size() == 12) && header.startsWith("RIFF") && header.endsWith("WEBP");
}

QStringList WebPPlugin::keys() const
{
    return QStringList() << "webp";
}

QList<QByteArray> WebPPlugin::mimeTypes() const
{
    return QList<QByteArray>() << "image/webp";
}

QImageIOPlugin::Capabilities WebPPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "webp") {
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead | QImageIOPlugin::CanWrite);
    }
    if (!device || !device->isOpen()) {
        return 0;
    }
    QImageIOPlugin::Capabilities cap;
    if (device->isReadable() && WebPHandler::canRead(device)) {
        cap |= QImageIOPlugin::CanRead;
    }
    if (device->isWritable()) {
        cap |= QImageIOPlugin::CanWrite;
    }
    return cap;
}

QImageIOHandler *WebPPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new WebPHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

//---------------------------------------------------------------------

Q_EXPORT_PLUGIN2(webp, WebPPlugin)
