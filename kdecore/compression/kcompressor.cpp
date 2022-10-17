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

#include "config-compression.h"
#include "kcompressor.h"
#include "klocale.h"
#include "kmimetype.h"
#include "kdebug.h"
#include <qplatformdefs.h>

#include <libdeflate.h>

#if defined(HAVE_BZIP2_SUPPORT)
#  include <bzlib.h>
#endif

#if defined(HAVE_XZ_SUPPORT)
#  include <lzma.h>
#endif

// for reference:
// http://linux.math.tifr.res.in/manuals/html/manual_3.html

class KCompressorPrivate
{
public:
    KCompressorPrivate();

    KCompressor::KCompressorType m_type;
    int m_level;
    QByteArray m_result;
    QString m_errorstring;
};

KCompressorPrivate::KCompressorPrivate()
    : m_type(KCompressor::TypeUnknown),
    m_level(1)
{
}


KCompressor::KCompressor()
    : d(new KCompressorPrivate())
{
}

KCompressor::~KCompressor()
{
    delete d;
}

KCompressor::KCompressorType KCompressor::type() const
{
    return d->m_type;
}

bool KCompressor::setType(const KCompressorType type)
{
    if (type == KCompressor::TypeUnknown) {
        return false;
    }
    d->m_type = type;
    return true;
}

int KCompressor::level() const
{
    return d->m_level;
}

bool KCompressor::setLevel(const int level)
{
    d->m_errorstring.clear();
    if (level < 0 || level > 9) {
        d->m_errorstring = i18n("Compression level not in the 0-9 range: %1", level);
        return false;
    }
    d->m_level = level;
    return true;
}

bool KCompressor::process(const QByteArray &data)
{
    d->m_errorstring.clear();
    d->m_result.clear();

    switch (d->m_type) {
        case KCompressor::TypeUnknown: {
            return false;
        }
        case KCompressor::TypeDeflate: {
            struct libdeflate_compressor* comp = libdeflate_alloc_compressor(d->m_level);
            if (Q_UNLIKELY(!comp)) {
                d->m_errorstring = i18n("Could not allocate compressor");
                return false;
            }

            const size_t boundresult = libdeflate_deflate_compress_bound(comp, data.size());
            if (Q_UNLIKELY(boundresult <= 0)) {
                d->m_errorstring = i18n("Compression boundary is negative or zero");
                libdeflate_free_compressor(comp);
                return false;
            }

            d->m_result.resize(boundresult);
            const size_t compresult = libdeflate_deflate_compress(
                comp,
                data.constData(), data.size(),
                d->m_result.data(), d->m_result.size()
            );
            libdeflate_free_compressor(comp);

            if (Q_UNLIKELY(compresult <= 0)) {
                d->m_errorstring = i18n("Could not compress data");
                d->m_result.clear();
                return false;
            }

            d->m_result.resize(compresult);
            return true;
        }
        case KCompressor::TypeZlib: {
            struct libdeflate_compressor* comp = libdeflate_alloc_compressor(d->m_level);
            if (Q_UNLIKELY(!comp)) {
                d->m_errorstring = i18n("Could not allocate compressor");
                return false;
            }

            const size_t boundresult = libdeflate_zlib_compress_bound(comp, data.size());
            if (Q_UNLIKELY(boundresult <= 0)) {
                d->m_errorstring = i18n("Compression boundary is negative or zero");
                libdeflate_free_compressor(comp);
                return false;
            }

            d->m_result.resize(boundresult);
            const size_t compresult = libdeflate_zlib_compress(
                comp,
                data.constData(), data.size(),
                d->m_result.data(), d->m_result.size()
            );
            libdeflate_free_compressor(comp);

            if (Q_UNLIKELY(compresult <= 0)) {
                d->m_errorstring = i18n("Could not compress data");
                d->m_result.clear();
                return false;
            }

            d->m_result.resize(compresult);
            return true;
        }
        case KCompressor::TypeGZip: {
            struct libdeflate_compressor* comp = libdeflate_alloc_compressor(d->m_level);
            if (Q_UNLIKELY(!comp)) {
                d->m_errorstring = i18n("Could not allocate compressor");
                return false;
            }

            const size_t boundresult = libdeflate_gzip_compress_bound(comp, data.size());
            if (Q_UNLIKELY(boundresult <= 0)) {
                d->m_errorstring = i18n("Compression boundary is negative or zero");
                libdeflate_free_compressor(comp);
                return false;
            }
        
            d->m_result.resize(boundresult);
            const size_t compresult = libdeflate_gzip_compress(
                comp,
                data.constData(), data.size(),
                d->m_result.data(), d->m_result.size()
            );
            libdeflate_free_compressor(comp);

            if (Q_UNLIKELY(compresult <= 0)) {
                d->m_errorstring = i18n("Could not compress data");
                d->m_result.clear();
                return false;
            }

            d->m_result.resize(compresult);
            return true;
        }
#if defined(HAVE_BZIP2_SUPPORT)
        case KCompressor::TypeBZip2: {
            d->m_result.resize(data.size() + QT_BUFFSIZE);
            uint compsize = d->m_result.size();

            int compresult = compresult = BZ2_bzBuffToBuffCompress(
                d->m_result.data(), &compsize,
                (char*)data.constData(), data.size(),
                d->m_level, 0, 0
            );

            if (Q_UNLIKELY(compresult < BZ_OK || compresult > BZ_STREAM_END)) {
                d->m_errorstring = i18n("Could not compress data");
                return false;
            }

            d->m_result.resize(compsize);
            return true;
        }
#endif // HAVE_BZIP2_SUPPORT
#if defined(HAVE_XZ_SUPPORT)
        case KCompressor::TypeXZ: {
            d->m_result.resize(data.size() + QT_BUFFSIZE);
            size_t compsize = d->m_result.size();

            lzma_stream comp = LZMA_STREAM_INIT;
            comp.next_in = (const uint8_t*)data.constData();
            comp.avail_in = data.size();
            comp.next_out = (uint8_t*)d->m_result.data();
            comp.avail_out = compsize;

            lzma_ret compresult = lzma_easy_encoder(&comp, d->m_level, LZMA_CHECK_CRC32);
            if (Q_UNLIKELY(compresult != LZMA_OK)) {
                d->m_errorstring = i18n("Could not initialize compressor");
                d->m_result.clear();
                return false;
            }

            compresult = lzma_code(&comp, LZMA_FINISH);
            if (Q_UNLIKELY(compresult != LZMA_OK && compresult != LZMA_STREAM_END)) {
                d->m_errorstring = i18n("Could not compress data");
                d->m_result.clear();
                return false;
            }
            compsize = comp.total_out;
            lzma_end(&comp);

            d->m_result.resize(compsize);
            return true;
        }
#endif // HAVE_XZ_SUPPORT
        default: {
            kWarning() << "Unsupported type" << d->m_type;
            return false;
        }
    }
    Q_UNREACHABLE();
}

QByteArray KCompressor::result() const
{
    return d->m_result;
}

QString KCompressor::errorString() const
{
    return d->m_errorstring;
}

KCompressor::KCompressorType KCompressor::typeForMime(const QString &mime)
{
    const KMimeType::Ptr kmimetype = KMimeType::mimeType(mime);
    if (kmimetype) {
        if (kmimetype->is(QString::fromLatin1("application/x-gzip"))) {
            return KCompressor::TypeGZip;
        } else if (kmimetype->is(QString::fromLatin1("application/x-bzip"))) {
            return KCompressor::TypeBZip2;
        } else if (kmimetype->is(QString::fromLatin1("application/x-xz"))) {
            return KCompressor::TypeXZ;
        }
    }
    return KCompressor::TypeUnknown;
}

KCompressor::KCompressorType KCompressor::typeForFile(const QString &filepath)
{
    const KMimeType::Ptr kmimetype = KMimeType::findByPath(filepath);
    if (kmimetype) {
        return KCompressor::typeForMime(kmimetype->name());
    }
    return KCompressor::TypeUnknown;
}