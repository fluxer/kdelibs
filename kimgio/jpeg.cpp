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

#include "jpeg.h"

#include <QImage>
#include <kdebug.h>

#include <turbojpeg.h>

static const char* const s_jpegpluginformat = "jpg";

static const ushort s_peekbuffsize = 32;
static const TJPF s_jpegpixelformat = TJPF_ARGB;
// for reference:
// https://en.wikipedia.org/wiki/List_of_file_signatures
static const uchar s_jpgjfifheader[] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01 };
static const uchar s_jpgheader[] = { 0xFF, 0xD8, 0xFF, 0xE0 };
static const uchar s_jpg2header[] = { 0xFF, 0xD8, 0xFF, 0xEE };
static const uchar s_jpegexifheader[] = { 0xFF, 0xD8, 0xFF, 0xE1 };

static const struct HeadersTblData {
    const uchar *header;
    const int headersize;
} HeadersTbl[] = {
    { s_jpgjfifheader, 12 },
    { s_jpgheader, 4 },
    { s_jpg2header, 4 },
    { s_jpegexifheader, 4 }
};
static const qint16 HeadersTblSize = sizeof(HeadersTbl) / sizeof(HeadersTblData);

JPEGHandler::JPEGHandler()
{
}

JPEGHandler::~JPEGHandler()
{
}

bool JPEGHandler::canRead() const
{
    if (canRead(device())) {
        setFormat(s_jpegpluginformat);
        return true;
    }
    return false;
}

bool JPEGHandler::read(QImage *image)
{
    const QByteArray data = device()->readAll();

    if (Q_UNLIKELY(data.isEmpty())) {
        return false;
    }

    tjhandle jpegdecomp = tjInitDecompress();
    if (!jpegdecomp) {
        kWarning() << "Could not initialize decompressor" << tjGetErrorStr();
        return false;
    }

    int jpegwidth = 0;
    int jpegheight = 0;
    int jpegsubsamp = 0;
    int jpegcolorspace = 0;
    int jpegstatus = tjDecompressHeader3(
        jpegdecomp,
        reinterpret_cast<const uchar*>(data.constData()), data.size(),
        &jpegwidth, &jpegheight,
        &jpegsubsamp, &jpegcolorspace
    );
    if (jpegstatus != 0) {
        kWarning() << "Could not decompress header" << tjGetErrorStr2(jpegdecomp);
        (void)tjDestroy(jpegdecomp);
        return false;
    }

    int jpegbuffersize = (jpegwidth * jpegheight * tjPixelSize[s_jpegpixelformat]);
    unsigned char *jpegbuffer = tjAlloc(jpegbuffersize);
    if (!jpegbuffer) {
        kWarning() << "Could not allocate buffer" << tjGetErrorStr2(jpegdecomp);
        (void)tjDestroy(jpegdecomp);
        return false;
    }

    jpegstatus = tjDecompress2(
        jpegdecomp,
        reinterpret_cast<const uchar*>(data.constData()), data.size(),
        jpegbuffer,
        jpegwidth, 0 , jpegheight,
        s_jpegpixelformat,
        TJFLAG_FASTDCT
    );
    if (jpegstatus != 0) {
        kWarning() << "Could not decompress" << tjGetErrorStr2(jpegdecomp);
        tjFree(jpegbuffer);
        (void)tjDestroy(jpegdecomp);
        return false;
    }

    *image = QImage(jpegwidth, jpegheight, QImage::Format_ARGB32);
    if (image->isNull()) {
        tjFree(jpegbuffer);
        (void)tjDestroy(jpegdecomp);
        return false;
    }

    QRgb* imagebits = reinterpret_cast<QRgb*>(image->bits());
    for (uint i = 0; i < jpegbuffersize; i += 4) {
        *imagebits = qRgba(jpegbuffer[i + 1], jpegbuffer[i + 2], jpegbuffer[i + 3], jpegbuffer[i]);
        imagebits++;
    }

    tjFree(jpegbuffer);
    (void)tjDestroy(jpegdecomp);

    return true;
}

bool JPEGHandler::write(const QImage &image)
{
    // this plugin is a read-only kind of plugin
    return false;
}

QByteArray JPEGHandler::name() const
{
    return s_jpegpluginformat;
}

bool JPEGHandler::canRead(QIODevice *device)
{
    if (Q_UNLIKELY(!device)) {
        kWarning() << "Called with no device";
        return false;
    }

    const QByteArray data = device->peek(s_peekbuffsize);

    if (Q_UNLIKELY(data.isEmpty())) {
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

QStringList JPEGPlugin::keys() const
{
    return QStringList() << s_jpegpluginformat;
}

QList<QByteArray> JPEGPlugin::mimeTypes() const
{
    static const QList<QByteArray> list = QList<QByteArray>()
        << "image/jpeg";
    return list;
}

QImageIOPlugin::Capabilities JPEGPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == s_jpegpluginformat)
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    QImageIOPlugin::Capabilities cap;
    if (device->isReadable() && JPEGHandler::canRead(device))
        cap |= QImageIOPlugin::CanRead;
    return cap;
}

QImageIOHandler *JPEGPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new JPEGHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN2(jpeg, JPEGPlugin)
