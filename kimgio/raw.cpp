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

#include "raw.h"

#include <QImage>
#include <kmimetype.h>
#include <kdebug.h>

#include <libraw/libraw.h>

static const char* const s_rawpluginformat = "raw";

RAWHandler::RAWHandler()
{
}

RAWHandler::~RAWHandler()
{
}

bool RAWHandler::canRead() const
{
    if (RAWHandler::canRead(device())) {
        setFormat(s_rawpluginformat);
        return true;
    }
    return false;
}

bool RAWHandler::read(QImage *image)
{
    QByteArray data = device()->readAll();

    if (Q_UNLIKELY(data.isEmpty())) {
        return false;
    }

    try {
        LibRaw raw;
        raw.imgdata.params.output_color = LIBRAW_COLORSPACE_sRGB;

        int rawresult = raw.open_buffer(data.data(), data.size());
        if (Q_UNLIKELY(rawresult != LIBRAW_SUCCESS)) {
            kWarning() << "Could not open buffer" << libraw_strerror(rawresult);
            raw.recycle();
            return false;
        }

        rawresult = raw.unpack();
        if (Q_UNLIKELY(rawresult != LIBRAW_SUCCESS)) {
            kWarning() << "Could not unpack" << libraw_strerror(rawresult);
            raw.recycle();
            return false;
        }

        rawresult = raw.dcraw_process();
        if (Q_UNLIKELY(rawresult != LIBRAW_SUCCESS)) {
            kWarning() << "Could not process" << libraw_strerror(rawresult);
            raw.recycle();
            return false;
        }

        libraw_processed_image_t* rawimg = raw.dcraw_make_mem_image(&rawresult);
        if (Q_UNLIKELY(!rawimg || rawresult != LIBRAW_SUCCESS)) {
            kWarning() << "Could not make image" << libraw_strerror(rawresult);
            raw.recycle();
            return false;
        }

        if (rawimg->colors != 3) {
            kWarning() << "Color components count not supported" << rawimg->colors;
            raw.dcraw_clear_mem(rawimg);
            raw.recycle();
            return false;
        }

        *image = QImage(rawimg->width, rawimg->height, QImage::Format_ARGB32);
        if (Q_UNLIKELY(image->isNull())) {
            kWarning() << "Could not create QImage";
            raw.dcraw_clear_mem(rawimg);
            raw.recycle();
            return false;
        }

        QRgb* imagebits = reinterpret_cast<QRgb*>(image->bits());
        for (uint i = 0 ; i < rawimg->data_size; i += 3) {
            *imagebits = qRgba(rawimg->data[i], rawimg->data[i + 1], rawimg->data[i + 2], 0xff);
            imagebits++;
        }

        raw.dcraw_clear_mem(rawimg);
        raw.recycle();
    } catch (...) {
        kWarning() << "Exception raised";
        return false;
    }

    return true;
}

bool RAWHandler::write(const QImage &image)
{
    // this plugin is a read-only kind of plugin
    return false;
}

QByteArray RAWHandler::name() const
{
    return s_rawpluginformat;
}

bool RAWHandler::canRead(QIODevice *device)
{
    if (Q_UNLIKELY(!device)) {
        kWarning() << "Called with no device";
        return false;
    }

    const qint64 devicepos = device->pos();
    QByteArray data = device->readAll();
    device->seek(devicepos);

    if (Q_UNLIKELY(data.isEmpty())) {
        return false;
    }

    try {
        LibRaw raw;
        raw.imgdata.params.output_color = LIBRAW_COLORSPACE_sRGB;

        const int rawresult = raw.open_buffer(data.data(), data.size());
        if (rawresult == LIBRAW_FILE_UNSUPPORTED) {
            kDebug() << libraw_strerror(rawresult);
            raw.recycle();
            return false;
        } else if (Q_UNLIKELY(rawresult != LIBRAW_SUCCESS)) {
            kWarning() << "Could not open buffer" << libraw_strerror(rawresult);
            raw.recycle();
            return false;
        }
        
        raw.recycle();
    } catch (...) {
        kWarning() << "Exception raised";
        return false;
    }

    return true;
}

QStringList RAWPlugin::keys() const
{
    return QStringList() << s_rawpluginformat;
}

QList<QByteArray> RAWPlugin::mimeTypes() const
{
    static QList<QByteArray> list;
    if (list.isEmpty()) {
        foreach (const KMimeType::Ptr &mime, KMimeType::allMimeTypes()) {
            // NOTE: RAW MIME types are sub-class of image/x-dcraw
            if (mime->is(QString::fromLatin1("image/x-dcraw"))
                && mime->name() != QLatin1String("image/x-dcraw")) {
                list.append(mime->name().toLatin1());
            }
        }
    }
    return list;
}

QImageIOPlugin::Capabilities RAWPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == s_rawpluginformat)
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    QImageIOPlugin::Capabilities cap;
    if (device->isReadable() && RAWHandler::canRead(device))
        cap |= QImageIOPlugin::CanRead;
    return cap;
}

QImageIOHandler *RAWPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new RAWHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN2(raw, RAWPlugin)
