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

#include "tiff.h"

#include <QImage>
#include <kdebug.h>

#include <stdio.h>
#include <tiffio.h>

static const char* const s_tiffpluginformat = "tiff";

static const ushort s_peekbuffsize = 32;
// for reference:
// https://en.wikipedia.org/wiki/List_of_file_signatures
static const uchar s_tiffleheader[] = { 0x49, 0x49, 0x2A, 0x00 };
static const uchar s_tiffbeheader[] = { 0x49, 0x49, 0x00, 0x2A };

static const struct HeadersTblData {
    const uchar *header;
    const int headersize;
} HeadersTbl[] = {
    { s_tiffleheader, 4 },
    { s_tiffbeheader, 4 }
};
static const qint16 HeadersTblSize = sizeof(HeadersTbl) / sizeof(HeadersTblData);

static TIFFErrorHandler s_tifferrorhandler = NULL;
static TIFFErrorHandler s_tiffwarninghandler = NULL;

void tiff_error_handler(const char* tiffmodule, const char* tiffformat, va_list tiffva)
{
    char vsnprintfbuff[1024];
    ::memset(vsnprintfbuff, '\0', sizeof(vsnprintfbuff));
    ::vsnprintf(vsnprintfbuff, sizeof(vsnprintfbuff), tiffformat, tiffva);
    kError() << tiffmodule << vsnprintfbuff;
}

void tiff_warning_handler(const char* tiffmodule, const char* tiffformat, va_list tiffva)
{
    char vsnprintfbuff[1024];
    ::memset(vsnprintfbuff, '\0', sizeof(vsnprintfbuff));
    ::vsnprintf(vsnprintfbuff, sizeof(vsnprintfbuff), tiffformat, tiffva);
    kWarning() << tiffmodule << vsnprintfbuff;
}

static tmsize_t tiff_read_proc(thandle_t tiffhandle, void* tiffptr, tmsize_t tiffsize)
{
    QIODevice* device = static_cast<QIODevice*>(tiffhandle);
    return device->read(static_cast<char*>(tiffptr), tiffsize);
}

static tmsize_t tiff_write_proc(thandle_t tiffhandle, void* tiffptr, tmsize_t tiffsize)
{
    // dummy
    Q_UNUSED(tiffhandle);
    Q_UNUSED(tiffptr);
    Q_UNUSED(tiffsize);
    return 0;
}

static toff_t tiff_seek_proc(thandle_t tiffhandle, toff_t tiffoffset, int tiffwhence)
{
    bool result = false;
    QIODevice* device = static_cast<QIODevice*>(tiffhandle);
    switch (tiffwhence) {
        case SEEK_SET: {
            result = device->seek(tiffoffset);
            break;
        }
        case SEEK_CUR: {
            result = device->seek(device->pos() + tiffoffset);
            break;
        }
        case SEEK_END: {
            result = device->seek(device->size() + tiffoffset);
            break;
        }
        default: {
            kWarning() << "Invalid whence value" << tiffwhence;
            result = false;
            break;
        }
    }
    if (Q_UNLIKELY(!result)) {
        kWarning() << "Could not seek" << tiffoffset << tiffwhence;
    }
    return device->pos();
}

static int tiff_close_proc(thandle_t tiffhandle)
{
    // nothing to do
    return 0;
}

static toff_t tiff_size_proc(thandle_t tiffhandle)
{
    QIODevice* device = static_cast<QIODevice*>(tiffhandle);
    return device->size();
}

static int tiff_mapfile_proc(thandle_t tiffhandle, void** tiffptr, toff_t* tiffsize)
{
    // dummy
    Q_UNUSED(tiffhandle);
    Q_UNUSED(tiffptr);
    Q_UNUSED(tiffsize);
    return 0;
}

static void tiff_unmapfile_proc(thandle_t tiffhandle, void* tiffptr, toff_t tiffsize)
{
    // dummy
    Q_UNUSED(tiffhandle);
    Q_UNUSED(tiffptr);
    Q_UNUSED(tiffsize);
}

TIFFHandler::TIFFHandler()
{
}

TIFFHandler::~TIFFHandler()
{
}

bool TIFFHandler::canRead() const
{
    if (TIFFHandler::canRead(device())) {
        setFormat(s_tiffpluginformat);
        return true;
    }
    return false;
}

bool TIFFHandler::read(QImage *image)
{
    s_tifferrorhandler = TIFFSetErrorHandler(tiff_error_handler);
    s_tiffwarninghandler = TIFFSetWarningHandler(tiff_warning_handler);

    TIFF* tiffclient = TIFFClientOpen(
        "TIFFHandler", "r",
        device(),
        tiff_read_proc,
        tiff_write_proc,
        tiff_seek_proc,
        tiff_close_proc,
        tiff_size_proc,
        tiff_mapfile_proc,
        tiff_unmapfile_proc
    );
    if (!Q_UNLIKELY(tiffclient)) {
        kWarning() << "Could not open client";
        TIFFSetErrorHandler(s_tifferrorhandler);
        TIFFSetWarningHandler(s_tiffwarninghandler);
        return false;
    }

    // NOTE: TIFFReadRGBA* functions do internal conversion (e.g. YCbCr to RGBA) which does not
    // work for all images
    char tifferror[1024];
    ::memset(tifferror, '\0', sizeof(tifferror));
    int tiffresult = TIFFRGBAImageOK(tiffclient, tifferror);
    if (tiffresult != 1) {
        kWarning() << "Image is not OK" << tifferror;
        TIFFClose(tiffclient);
        TIFFSetErrorHandler(s_tifferrorhandler);
        TIFFSetWarningHandler(s_tiffwarninghandler);
        return false;
    }

    uint32_t tiffwidth = 0;
    tiffresult = TIFFGetField(tiffclient, TIFFTAG_IMAGEWIDTH, &tiffwidth);
    if (Q_UNLIKELY(tiffresult != 1)) {
        kWarning() << "Could not get image width";
        TIFFClose(tiffclient);
        TIFFSetErrorHandler(s_tifferrorhandler);
        TIFFSetWarningHandler(s_tiffwarninghandler);
        return false;
    }

    uint32_t tiffheight = 0;
    tiffresult = TIFFGetField(tiffclient, TIFFTAG_IMAGELENGTH, &tiffheight);
    if (Q_UNLIKELY(tiffresult != 1)) {
        kWarning() << "Could not get image length";
        TIFFClose(tiffclient);
        TIFFSetErrorHandler(s_tifferrorhandler);
        TIFFSetWarningHandler(s_tiffwarninghandler);
        return false;
    }

    *image = QImage(tiffwidth, tiffheight, QImage::Format_ARGB32);
    if (Q_UNLIKELY(image->isNull())) {
        kWarning() << "Could not create image";
        TIFFClose(tiffclient);
        TIFFSetErrorHandler(s_tifferrorhandler);
        TIFFSetWarningHandler(s_tiffwarninghandler);
        return false;
    }

    tiffresult = TIFFReadRGBAImageOriented(
        tiffclient,
        tiffwidth, tiffheight,
        reinterpret_cast<uint32_t*>(image->bits()), ORIENTATION_TOPLEFT,
        1
    );
    if (Q_UNLIKELY(tiffresult != 1)) {
        kWarning() << "Could not read image";
        *image = QImage();
        TIFFClose(tiffclient);
        TIFFSetErrorHandler(s_tifferrorhandler);
        TIFFSetWarningHandler(s_tiffwarninghandler);
        return false;
    }

    TIFFClose(tiffclient);
    TIFFSetErrorHandler(s_tifferrorhandler);
    TIFFSetWarningHandler(s_tiffwarninghandler);
    return true;
}

bool TIFFHandler::write(const QImage &image)
{
    // this plugin is a read-only kind of plugin
    return false;
}

QByteArray TIFFHandler::name() const
{
    return s_tiffpluginformat;
}

bool TIFFHandler::canRead(QIODevice *device)
{
    if (Q_UNLIKELY(!device)) {
        kWarning() << "Called with no device";
        return false;
    }

    const QByteArray data = device->peek(s_peekbuffsize);

    for (int i = 0; i < HeadersTblSize; i++) {
        if (data.size() >= HeadersTbl[i].headersize &&
            ::memcmp(data.constData(), HeadersTbl[i].header, HeadersTbl[i].headersize) == 0) {
            kDebug() << "Header detected";
            return true;
        }
    }

    return false;
}

QList<QByteArray> TIFFPlugin::mimeTypes() const
{
    static const QList<QByteArray> list = QList<QByteArray>()
        << "image/tiff";
    return list;
}

QImageIOPlugin::Capabilities TIFFPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == s_tiffpluginformat) {
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead);
    }
    if (!device || !device->isOpen()) {
        return 0;
    }
    if (device->isReadable() && TIFFHandler::canRead(device)) {
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead);
    }
    return 0;
}

QImageIOHandler *TIFFPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new TIFFHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN(TIFFPlugin)
