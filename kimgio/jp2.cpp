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

#include "jp2.h"

#include <QImage>
#include <qplatformdefs.h>
#include <kdebug.h>

#include <openjpeg.h>

static const char* const s_jp2pluginformat = "jp2";
static const OPJ_SIZE_T s_ojbuffersize = QT_BUFFSIZE;

static const ushort s_peekbuffsize = 32;
// for reference:
// https://en.wikipedia.org/wiki/List_of_file_signatures
// https://www.rfc-editor.org/rfc/rfc3745
// openjpeg/tests/j2k_random_tile_access.c
static const uchar s_jp2rfc3745header[] = { 0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a };
static const uchar s_jp2header[] = { 0x0d, 0x0a, 0x87, 0x0a };
static const uchar s_j2kheader[] = { 0xff, 0x4f, 0xff, 0x51 };

static const struct HeadersTblData {
    const uchar *header;
    const int headersize;
    const OPJ_CODEC_FORMAT ojcodec;
} HeadersTbl[] = {
    { s_jp2rfc3745header, 12, OPJ_CODEC_JP2 },
    { s_jp2header, 4, OPJ_CODEC_JP2 },
    { s_j2kheader, 4, OPJ_CODEC_J2K }
    // TODO: OPJ_CODEC_JPT
};
static const qint16 HeadersTblSize = sizeof(HeadersTbl) / sizeof(HeadersTblData);

static OPJ_CODEC_FORMAT guessOJCodec(const QByteArray &data)
{
    for (int i = 0; i < HeadersTblSize; i++) {
        if (data.size() >= HeadersTbl[i].headersize &&
            ::memcmp(data.constData(), HeadersTbl[i].header, HeadersTbl[i].headersize) == 0) {
            kDebug() << "Codec detected" << HeadersTbl[i].ojcodec;
            return HeadersTbl[i].ojcodec;
        }
    }
    return OPJ_CODEC_UNKNOWN;
}

static void oj_info_callback(const char *msg, void *data)
{
    Q_UNUSED(data);
    kDebug() << msg;
}

static void oj_warning_callback(const char *msg, void *data)
{
    Q_UNUSED(data);
    kWarning() << msg;
}

static void oj_error_callback(const char *msg, void *data)
{
    Q_UNUSED(data);
    kError() << msg;
}

static OPJ_SIZE_T oj_read_callback(void *buffer, OPJ_SIZE_T size, void *data)
{
    QIODevice* device = static_cast<QIODevice*>(data);
    char* ojbuffer = static_cast<char*>(buffer);
    const qint64 result = device->read(ojbuffer, size);
    if (result == 0) {
        return -1;
    }
    return result;
}

static OPJ_BOOL oj_seek_callback(OPJ_OFF_T size, void *data)
{
    QIODevice* device = static_cast<QIODevice*>(data);
    return device->seek(size);
}

static OPJ_OFF_T oj_skip_callback(OPJ_OFF_T size, void *data)
{
    kWarning() << "Not implemented";
    Q_UNUSED(size);
    Q_UNUSED(data);
    return 0;
}


JP2Handler::JP2Handler()
{
}

JP2Handler::~JP2Handler()
{
}

bool JP2Handler::canRead() const
{
    if (JP2Handler::canRead(device())) {
        setFormat(s_jp2pluginformat);
        return true;
    }
    return false;
}

bool JP2Handler::read(QImage *image)
{
    opj_stream_t *ojstream = opj_stream_create(s_ojbuffersize, OPJ_TRUE);
    if (Q_UNLIKELY(!ojstream)) {
        kWarning() << "Could not create stream";
        return false;
    }

    opj_stream_set_user_data(ojstream, device(), NULL);
    opj_stream_set_user_data_length(ojstream, device()->size());
    opj_stream_set_read_function(ojstream, oj_read_callback);
    opj_stream_set_seek_function(ojstream, oj_seek_callback);
    opj_stream_set_skip_function(ojstream, oj_skip_callback);

    opj_codec_t* ojcodec = opj_create_decompress(guessOJCodec(device()->peek(s_peekbuffsize)));
    if (Q_UNLIKELY(!ojcodec)) {
        kWarning() << "Could not create codec";
        opj_stream_destroy(ojstream);
        return false;
    }

    opj_set_info_handler(ojcodec, oj_info_callback, NULL);
    opj_set_warning_handler(ojcodec, oj_warning_callback, NULL);
    opj_set_error_handler(ojcodec, oj_error_callback, NULL);

    opj_dparameters_t ojparameters;
    opj_set_default_decoder_parameters(&ojparameters);
    ojparameters.m_verbose = true;
    if (Q_UNLIKELY(opj_setup_decoder(ojcodec, &ojparameters) == OPJ_FALSE)) {
        kWarning() << "Could not setup decoder";
        opj_destroy_codec(ojcodec);
        opj_stream_destroy(ojstream);
        return false;
    }

    opj_image_t* ojimage = NULL;
    if (Q_UNLIKELY(opj_read_header(ojstream, ojcodec, &ojimage) == OPJ_FALSE)) {
        kWarning() << "Could not read header";
        opj_destroy_codec(ojcodec);
        opj_stream_destroy(ojstream);
        opj_image_destroy(ojimage);
        return false;
    }

    if (Q_UNLIKELY(opj_decode(ojcodec, ojstream, ojimage) == OPJ_FALSE)) {
        kWarning() << "Could not decode stream";
        opj_destroy_codec(ojcodec);
        opj_stream_destroy(ojstream);
        opj_image_destroy(ojimage);
        return false;
    }

    if (Q_UNLIKELY(opj_end_decompress(ojcodec, ojstream) == OPJ_FALSE)) {
        kWarning() << "Could not end decompression";
        opj_destroy_codec(ojcodec);
        opj_stream_destroy(ojstream);
        opj_image_destroy(ojimage);
        return false;
    }

    *image = QImage(ojimage->comps->w, ojimage->comps->h, QImage::Format_ARGB32);
    if (Q_UNLIKELY(image->isNull())) {
        kWarning() << "Could not create image QImage";
        opj_destroy_codec(ojcodec);
        opj_stream_destroy(ojstream);
        opj_image_destroy(ojimage);
        return false;
    }

    switch (ojimage->numcomps) {
        case 4: {
            QRgb* imagebits = reinterpret_cast<QRgb*>(image->bits());
            const uint bitscount = (ojimage->comps->h * ojimage->comps->w * ojimage->numcomps);
            OPJ_INT32* ojr = ojimage->comps[0].data;
            OPJ_INT32* ojg = ojimage->comps[1].data;
            OPJ_INT32* ojb = ojimage->comps[2].data;
            OPJ_INT32* oja = ojimage->comps[3].data;
            for (uint i = 0 ; i < bitscount; i += ojimage->numcomps) {
                *imagebits = qRgba(*ojr, *ojg, *ojb, *oja);
                ojr++;
                ojg++;
                ojb++;
                oja++;
                imagebits++;
            }
            break;
        }
        case 3: {
            QRgb* imagebits = reinterpret_cast<QRgb*>(image->bits());
            const uint bitscount = (ojimage->comps->h * ojimage->comps->w * ojimage->numcomps);
            OPJ_INT32* ojr = ojimage->comps[0].data;
            OPJ_INT32* ojg = ojimage->comps[1].data;
            OPJ_INT32* ojb = ojimage->comps[2].data;
            for (uint i = 0 ; i < bitscount; i += ojimage->numcomps) {
                *imagebits = qRgba(*ojr, *ojg, *ojb, 0xff);
                ojr++;
                ojg++;
                ojb++;
                imagebits++;
            }
            break;
        }
        // gray with alpha
        case 2: {
            QRgb* imagebits = reinterpret_cast<QRgb*>(image->bits());
            const uint bitscount = (ojimage->comps->h * ojimage->comps->w * ojimage->numcomps);
            OPJ_INT32* ojg = ojimage->comps[0].data;
            OPJ_INT32* oja = ojimage->comps[1].data;
            for (uint i = 0 ; i < bitscount; i += ojimage->numcomps) {
                *imagebits = qRgba(*ojg, *ojg, *ojg, *oja);
                ojg++;
                oja++;
                imagebits++;
            }
            break;
        }
        // gray only
        case 1: {
            QRgb* imagebits = reinterpret_cast<QRgb*>(image->bits());
            const uint bitscount = (ojimage->comps->h * ojimage->comps->w * ojimage->numcomps);
            OPJ_INT32* ojg = ojimage->comps[0].data;
            for (uint i = 0 ; i < bitscount; i += ojimage->numcomps) {
                *imagebits = qRgba(*ojg, *ojg, *ojg, 0xff);
                ojg++;
                imagebits++;
            }
            break;
        }
        default: {
            kWarning() << "Invalid color component count" << ojimage->numcomps;
            *image = QImage();
            break;
        }
    }

    opj_destroy_codec(ojcodec);
    opj_stream_destroy(ojstream);
    opj_image_destroy(ojimage);

    return true;
}

bool JP2Handler::write(const QImage &image)
{
    // this plugin is a read-only kind of plugin
    return false;
}

QByteArray JP2Handler::name() const
{
    return s_jp2pluginformat;
}

bool JP2Handler::canRead(QIODevice *device)
{
    if (Q_UNLIKELY(!device)) {
        kWarning() << "Called with no device";
        return false;
    }

    const QByteArray data = device->peek(s_peekbuffsize);

    if (Q_UNLIKELY(data.isEmpty())) {
        return false;
    }

    return (guessOJCodec(data) != OPJ_CODEC_UNKNOWN);
}

QStringList JP2Plugin::keys() const
{
    return QStringList() << s_jp2pluginformat;
}

QList<QByteArray> JP2Plugin::mimeTypes() const
{
    static const QList<QByteArray> list = QList<QByteArray>()
        << "image/jp2";
    return list;
}

QImageIOPlugin::Capabilities JP2Plugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == s_jp2pluginformat)
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead);
    if (!device->isOpen())
        return 0;

    QImageIOPlugin::Capabilities cap;
    if (device->isReadable() && JP2Handler::canRead(device))
        cap |= QImageIOPlugin::CanRead;
    return cap;
}

QImageIOHandler *JP2Plugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new JP2Handler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN2(jp2, JP2Plugin)
