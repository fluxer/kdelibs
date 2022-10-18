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
#include <webp/demux.h>

static const int s_peekbuffsize = 32;

WebPHandler::WebPHandler()
    : m_quality(100),
    m_loopcount(0),
    m_imagecount(1),
    m_imagedelay(80),
    m_currentimage(0)
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
    // NOTE: QMovie will continuously call read() to get each frame
    const qint64 devicepos = device()->pos();
    const QByteArray data = device()->readAll();
    device()->seek(devicepos);

    const WebPData webpdata = { reinterpret_cast<const uint8_t*>(data.constData()), size_t(data.size()) };
    WebPAnimDecoderOptions webpanimoptions;
    WebPAnimDecoderOptionsInit(&webpanimoptions);
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    webpanimoptions.color_mode = WEBP_CSP_MODE::MODE_ARGB;
#else
    webpanimoptions.color_mode = WEBP_CSP_MODE::MODE_BGRA;
#endif
    WebPAnimDecoder* webpanimdec = WebPAnimDecoderNew(
        &webpdata,
        &webpanimoptions
    );
    if (!webpanimdec) {
        kWarning() << "Could not create animation decoder";
        return false;
    }

    WebPAnimInfo webpaniminfo;
    if (WebPAnimDecoderGetInfo(webpanimdec, &webpaniminfo) == 0) {
        kWarning() << "Could not get animation information";
        WebPAnimDecoderDelete(webpanimdec);
        return false;
    }

    m_loopcount = webpaniminfo.loop_count;
    m_imagecount = webpaniminfo.frame_count;

    const WebPDemuxer* webpdemuxer = WebPAnimDecoderGetDemuxer(webpanimdec);
    WebPIterator webpiter;
    if (WebPDemuxGetFrame(webpdemuxer, m_currentimage, &webpiter) == 0) {
        kWarning() << "Could not get frame";
        WebPAnimDecoderDelete(webpanimdec);
        return false;
    }

    if (webpiter.has_alpha) {
        *image = QImage(webpiter.width, webpiter.height, QImage::Format_ARGB32);
    } else {
        *image = QImage(webpiter.width, webpiter.height, QImage::Format_RGB32);
    }
    if (Q_UNLIKELY(image->isNull())) {
        kWarning() << "Could not create image";
        WebPDemuxReleaseIterator(&webpiter);
        WebPAnimDecoderDelete(webpanimdec);
        return false;
    }

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    const uint8_t* webpoutput = WebPDecodeARGBInto(
        webpiter.fragment.bytes, webpiter.fragment.size,
        reinterpret_cast<uint8_t*>(image->bits()), image->byteCount(),
        image->bytesPerLine()
    );
#else
    const uint8_t* webpoutput = WebPDecodeBGRAInto(
        webpiter.fragment.bytes, webpiter.fragment.size,
        reinterpret_cast<uint8_t*>(image->bits()), image->byteCount(),
        image->bytesPerLine()
    );
#endif
    if (Q_UNLIKELY(!webpoutput)) {
        kWarning() << "Could not decode image";
        WebPDemuxReleaseIterator(&webpiter);
        WebPAnimDecoderDelete(webpanimdec);
        return false;
    }

    m_currentimage++;
    if (m_currentimage >= m_imagecount) {
        m_currentimage = 0;
    }

    WebPDemuxReleaseIterator(&webpiter);
    WebPAnimDecoderDelete(webpanimdec);
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
    uint8_t *webpimagedata = new uint8_t[image32.width() * image32.height() * (3 + image32.hasAlphaChannel())];
    for (int y = 0; y < image32.height(); y++) {
        const QRgb *scanline = reinterpret_cast<const QRgb*>(image32.constScanLine(y));
        for (int x = 0; x < image32.width(); x++) {
            webpimagedata[idx++] = qRed(scanline[x]);
            webpimagedata[idx++] = qGreen(scanline[x]);
            webpimagedata[idx++] = qBlue(scanline[x]);

            if (image32.hasAlphaChannel()) {
                webpimagedata[idx++] = qAlpha(scanline[x]);
            }
        }
    }

    size_t webpsize = 0;
    uint8_t *webpoutput = nullptr;
    if (image32.hasAlphaChannel()) {
        webpsize = WebPEncodeRGBA(webpimagedata, image32.width(), image32.height(), image32.width() * 4, m_quality, &webpoutput);
    } else {
        webpsize = WebPEncodeRGB(webpimagedata, image32.width(), image32.height(), image32.width() * 3, m_quality, &webpoutput);
    }
    delete []webpimagedata;

    if (Q_UNLIKELY(webpsize == 0)) {
        kWarning() << "Could not encode image";
        WebPFree(webpoutput);
        return false;
    }

    if (Q_UNLIKELY(device()->write(reinterpret_cast<const char*>(webpoutput), webpsize) != webpsize)) {
        kWarning() << "Could not write image";
        WebPFree(webpoutput);
        return false;
    }
    WebPFree(webpoutput);

    return true;
}

QByteArray WebPHandler::name() const
{
    return "webp";
}

bool WebPHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
    switch (option) {
        case QImageIOHandler::Quality:
        case QImageIOHandler::Size:
        case QImageIOHandler::Animation: {
            return true;
        }
        default: {
            return false;
        }
    }
    Q_UNREACHABLE();
}

QVariant WebPHandler::option(QImageIOHandler::ImageOption option) const
{
    switch (option) {
        case QImageIOHandler::Quality: {
            return m_quality;
        }
        case QImageIOHandler::Size: {
            const QByteArray data = device()->peek(s_peekbuffsize);

            WebPBitstreamFeatures webpfeatures;
            const VP8StatusCode vp8statusret = WebPGetFeatures(
                reinterpret_cast<const uint8_t*>(data.constData()), data.size(),
                &webpfeatures
            );
            if (vp8statusret != VP8_STATUS_OK) {
                kWarning() << "Could not get image features for size option";
                return QVariant(QSize());
            }
            return QVariant(QSize(webpfeatures.width, webpfeatures.height));
        }
        case QImageIOHandler::Animation: {
            const QByteArray data = device()->peek(s_peekbuffsize);

            WebPBitstreamFeatures webpfeatures;
            const VP8StatusCode vp8statusret = WebPGetFeatures(
                reinterpret_cast<const uint8_t*>(data.constData()), data.size(),
                &webpfeatures
            );
            if (vp8statusret != VP8_STATUS_OK) {
                kWarning() << "Could not get image features for animation option";
                return QVariant(bool(false));
            }
            return QVariant(bool(webpfeatures.has_animation));
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
            m_quality = 100;
        } else {
            m_quality = qBound(0, newquality, 100);
        }
    }
}

bool WebPHandler::jumpToNextImage()
{
    return jumpToImage(m_currentimage + 1);
}

bool WebPHandler::jumpToImage(int imageNumber)
{
    if (imageNumber >= m_imagecount) {
        return false;
    }
    m_currentimage = imageNumber;
    return true;
}

int WebPHandler::loopCount() const
{
    return m_loopcount;
}

int WebPHandler::imageCount() const
{
    return m_imagecount;
}

int WebPHandler::nextImageDelay() const
{
    return m_imagedelay;
}

int WebPHandler::currentImageNumber() const
{
    return m_currentimage;
}

bool WebPHandler::canRead(QIODevice *device)
{
    if (Q_UNLIKELY(!device)) {
        qWarning("WebPHandler::canRead() called with no device");
        return false;
    }

    // WebP file header: 4 bytes "RIFF", 4 bytes length, 4 bytes "WEBP"
    const QByteArray header = device->peek(12);
    if (header.size() == 12 && header.startsWith("RIFF") && header.endsWith("WEBP")) {
        return true;
    }
    return false;
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
