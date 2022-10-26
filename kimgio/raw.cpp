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
#include <kdebug.h>

#include <libraw/libraw.h>

static const char* const s_rawpluginformat = "raw";

// for reference:
// https://www.libraw.org/docs/API-CXX.html#datastream
class RAWDataStream : public LibRaw_abstract_datastream
{
public:
    RAWDataStream(QIODevice* device, const bool restore);
    ~RAWDataStream();

    int valid() final;
    int read(void* rawptr, size_t rawsize, size_t rawnmemb) final;
    int seek(INT64 rawoffset, int rawwhence) final;
    INT64 tell() final;
    INT64 size() final;
    int get_char() final;
    char *gets(char* rawbuffer, int rawsize) final;
    int scanf_one(const char*, void*) final;
    int eof() final;
    void *make_jas_stream() final;
    const char *fname() final;

private:
    QIODevice* m_device;
    const qint64 m_devicepos;
    const bool m_restore;
};

RAWDataStream::RAWDataStream(QIODevice* device, const bool restore)
    : m_device(device),
    m_devicepos(m_device->pos()),
    m_restore(restore)
{
}

RAWDataStream::~RAWDataStream()
{
    if (m_restore) {
        m_device->seek(m_devicepos);
    }
}

int RAWDataStream::valid()
{
    return m_device->isOpen();
}

int RAWDataStream::read(void* rawptr, size_t rawsize, size_t rawnmemb)
{
    return m_device->read(static_cast<char*>(rawptr), rawnmemb * rawsize);
}

int RAWDataStream::seek(INT64 rawoffset, int rawwhence)
{
    bool result = false;
    switch (rawwhence) {
        case SEEK_SET: {
            result = m_device->seek(rawoffset);
            break;
        }
        case SEEK_CUR: {
            result = m_device->seek(m_device->pos() + rawoffset);
            break;
        }
        case SEEK_END: {
            result = m_device->seek(m_device->size() + rawoffset);
            break;
        }
        default: {
            kWarning() << "Invalid whence value" << rawwhence;
            result = false;
            break;
        }
    }
    if (Q_UNLIKELY(!result)) {
        kWarning() << "Could not seek" << rawoffset << rawwhence;
        return -1;
    }
    return 0;
}

INT64 RAWDataStream::tell()
{
    return m_device->pos();
}

INT64 RAWDataStream::size()
{
    return m_device->size();
}

int RAWDataStream::get_char()
{
    char result = 0;
    if (!m_device->getChar(&result)) {
        return -1;
    }
    return static_cast<uchar>(result);
}

char* RAWDataStream::gets(char* rawbuffer, int rawsize)
{
    const qint64 result = m_device->readLine(rawbuffer, rawsize);
    if (result > 0) {
        return rawbuffer;
    }
    return nullptr;
}

int RAWDataStream::scanf_one(const char*, void*)
{
    // NOTE: not used at all by LibRaw, i.e. don't implement
    KWARNING_NOTIMPLEMENTED
    return 0;
}

int RAWDataStream::eof()
{
    if (m_device->atEnd()) {
        return 1;
    }
    return 0;
}

void* RAWDataStream::make_jas_stream()
{
    // NOTE: used only for RedCine images
    KWARNING_NOTIMPLEMENTED
    return nullptr;
}

const char* RAWDataStream::fname()
{
    return "RAWDataStream";
}


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
    if (Q_UNLIKELY(!device())) {
        kWarning() << "Called with no device";
        return false;
    }

    try {
        LibRaw raw;
        raw.imgdata.params.output_color = LIBRAW_COLORSPACE_sRGB;

        RAWDataStream rawdatastream(device(), false);
        int rawresult = raw.open_datastream(&rawdatastream);
        if (Q_UNLIKELY(rawresult != LIBRAW_SUCCESS)) {
            kWarning() << "Could not open datastream" << libraw_strerror(rawresult);
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

        *image = QImage(rawimg->width, rawimg->height, QImage::Format_RGB32);
        if (Q_UNLIKELY(image->isNull())) {
            kWarning() << "Could not create QImage";
            raw.dcraw_clear_mem(rawimg);
            raw.recycle();
            return false;
        }

        QRgb* imagebits = reinterpret_cast<QRgb*>(image->bits());
        for (uint i = 0 ; i < rawimg->data_size; i += 3) {
            *imagebits = qRgb(rawimg->data[i], rawimg->data[i + 1], rawimg->data[i + 2]);
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

    try {
        LibRaw raw;
        raw.imgdata.params.output_color = LIBRAW_COLORSPACE_sRGB;

        RAWDataStream rawdevicestream(device, true);
        const int rawresult = raw.open_datastream(&rawdevicestream);
        if (rawresult == LIBRAW_FILE_UNSUPPORTED) {
            kDebug() << libraw_strerror(rawresult);
            raw.recycle();
            return false;
        } else if (Q_UNLIKELY(rawresult != LIBRAW_SUCCESS)) {
            kWarning() << "Could not open datastream" << libraw_strerror(rawresult);
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
    static const QList<QByteArray> list = QList<QByteArray>()
        << "image/x-sony-srf"
        << "image/x-fuji-raf"
        << "image/x-adobe-dng"
        << "image/x-olympus-orf"
        << "image/x-panasonic-rw2"
        << "image/x-kodak-dcr"
        << "image/x-kodak-k25"
        << "image/x-sony-arw"
        << "image/x-minolta-mrw"
        << "image/x-kodak-kdc"
        << "image/x-sigma-x3f"
        << "image/x-nikon-nef"
        << "image/x-pentax-pef"
        << "image/x-panasonic-rw"
        << "image/x-canon-crw"
        << "image/x-sony-sr2"
        << "image/x-canon-cr2";
    return list;
}

QImageIOPlugin::Capabilities RAWPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == s_rawpluginformat) {
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead);
    }
    if (!device || !device->isOpen()) {
        return 0;
    }
    if (device->isReadable() && RAWHandler::canRead(device)) {
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead);
    }
    return 0;
}

QImageIOHandler *RAWPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new RAWHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN2(raw, RAWPlugin)
