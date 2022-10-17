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
#include "kdecompressor.h"
#include "klocale.h"
#include "kmimetype.h"
#include "kdebug.h"
#include <qplatformdefs.h>

#include <limits.h>
#include <libdeflate.h>

#if defined(HAVE_BZIP2_SUPPORT)
#  include <bzlib.h>
#endif

#if defined(HAVE_XZ_SUPPORT)
#  include <lzma.h>
#endif

// for reference:
// http://linux.math.tifr.res.in/manuals/html/manual_3.html

class KDecompressorPrivate
{
public:
    KDecompressorPrivate();

    KDecompressor::KDecompressorType m_type;
    QByteArray m_result;
    QString m_errorstring;
};

KDecompressorPrivate::KDecompressorPrivate()
    : m_type(KDecompressor::TypeUnknown)
{
}


KDecompressor::KDecompressor()
    : d(new KDecompressorPrivate())
{
}

KDecompressor::~KDecompressor()
{
    delete d;
}

KDecompressor::KDecompressorType KDecompressor::type() const
{
    return d->m_type;
}

bool KDecompressor::setType(const KDecompressorType type)
{
    if (type == KDecompressor::TypeUnknown) {
        return false;
    }
    d->m_type = type;
    return true;
}

bool KDecompressor::process(const QByteArray &data)
{
    d->m_errorstring.clear();
    d->m_result.clear();

    switch (d->m_type) {
        case KDecompressor::TypeUnknown: {
            return false;
        }
        case KDecompressor::TypeDeflate: {
            struct libdeflate_decompressor* decomp = libdeflate_alloc_decompressor();
            if (Q_UNLIKELY(!decomp)) {
                d->m_errorstring = i18n("Could not allocate decompressor");
                return false;
            }

            size_t speculativesize = (data.size() * 2);
            d->m_result.resize(speculativesize);

            libdeflate_result decompresult = LIBDEFLATE_INSUFFICIENT_SPACE;
            while (decompresult == LIBDEFLATE_INSUFFICIENT_SPACE) {
                decompresult = libdeflate_deflate_decompress(
                    decomp,
                    data.constData(), data.size(),
                    d->m_result.data(), d->m_result.size(),
                    &speculativesize
                );

                if (decompresult == LIBDEFLATE_INSUFFICIENT_SPACE) {
                    speculativesize = (speculativesize + QT_BUFFSIZE);
                    d->m_result.resize(speculativesize);
                }

                if (speculativesize >= INT_MAX) {
                    break;
                }
            }
            libdeflate_free_decompressor(decomp);

            if (decompresult != LIBDEFLATE_SUCCESS) {
                d->m_errorstring = i18n("Could not decompress data");
                d->m_result.clear();
                return false;
            }

            d->m_result.resize(speculativesize);
            return true;
        }
        case KDecompressor::TypeZlib: {
            struct libdeflate_decompressor* decomp = libdeflate_alloc_decompressor();
            if (Q_UNLIKELY(!decomp)) {
                d->m_errorstring = i18n("Could not allocate decompressor");
                return false;
            }

            size_t speculativesize = (data.size() * 2);
            d->m_result.resize(speculativesize);

            libdeflate_result decompresult = LIBDEFLATE_INSUFFICIENT_SPACE;
            while (decompresult == LIBDEFLATE_INSUFFICIENT_SPACE) {
                decompresult = libdeflate_zlib_decompress(
                    decomp,
                    data.constData(), data.size(),
                    d->m_result.data(), d->m_result.size(),
                    &speculativesize
                );

                if (decompresult == LIBDEFLATE_INSUFFICIENT_SPACE) {
                    speculativesize = (speculativesize + QT_BUFFSIZE);
                    d->m_result.resize(speculativesize);
                }

                if (speculativesize >= INT_MAX) {
                    break;
                }
            }
            libdeflate_free_decompressor(decomp);

            if (decompresult != LIBDEFLATE_SUCCESS) {
                d->m_errorstring = i18n("Could not decompress data");
                d->m_result.clear();
                return false;
            }

            d->m_result.resize(speculativesize);
            return true;
        }
        case KDecompressor::TypeGZip: {
            struct libdeflate_decompressor* decomp = libdeflate_alloc_decompressor();
            if (Q_UNLIKELY(!decomp)) {
                d->m_errorstring = i18n("Could not allocate decompressor");
                return false;
            }

            size_t speculativesize = (data.size() * 2);
            d->m_result.resize(speculativesize);

            libdeflate_result decompresult = LIBDEFLATE_INSUFFICIENT_SPACE;
            while (decompresult == LIBDEFLATE_INSUFFICIENT_SPACE) {
                decompresult = libdeflate_gzip_decompress(
                    decomp,
                    data.constData(), data.size(),
                    d->m_result.data(), d->m_result.size(),
                    &speculativesize
                );

                if (decompresult == LIBDEFLATE_INSUFFICIENT_SPACE) {
                    speculativesize = (speculativesize + QT_BUFFSIZE);
                    d->m_result.resize(speculativesize);
                }

                if (speculativesize >= INT_MAX) {
                    break;
                }
            }
            libdeflate_free_decompressor(decomp);

            if (decompresult != LIBDEFLATE_SUCCESS) {
                d->m_errorstring = i18n("Could not decompress data");
                d->m_result.clear();
                return false;
            }

            d->m_result.resize(speculativesize);
            return true;
        }
#if defined(HAVE_BZIP2_SUPPORT)
        case KDecompressor::TypeBZip2: {
            uint speculativesize = (data.size() * 2);
            d->m_result.resize(speculativesize);
            int decompresult = BZ_OUTBUFF_FULL;
            while (decompresult == BZ_OUTBUFF_FULL) {
                decompresult = BZ2_bzBuffToBuffDecompress(
                    d->m_result.data(), &speculativesize,
                    (char*)data.constData(), data.size(),
                    0, 0
                );

                if (decompresult == BZ_OUTBUFF_FULL) {
                    speculativesize = (speculativesize + QT_BUFFSIZE);
                    d->m_result.resize(speculativesize);
                }

                if (speculativesize >= INT_MAX) {
                    break;
                }
            }

            if (Q_UNLIKELY(decompresult < BZ_OK || decompresult > BZ_STREAM_END)) {
                d->m_errorstring = i18n("Could not decompress data");
                d->m_result.clear();
                return false;
            }

            d->m_result.resize(speculativesize);
            return true;
        }
#endif // HAVE_BZIP2_SUPPORT
#if defined(HAVE_XZ_SUPPORT)
        case KDecompressor::TypeXZ: {
            size_t speculativesize = (data.size() * 2);
            d->m_result.resize(speculativesize);

            lzma_stream decomp = LZMA_STREAM_INIT;
            decomp.next_in = (const uint8_t*)data.constData();
            decomp.avail_in = data.size();
            decomp.next_out = (uint8_t*)d->m_result.data();
            decomp.avail_out = speculativesize;

            lzma_ret decompresult = lzma_auto_decoder(&decomp, UINT64_MAX, 0);
            if (Q_UNLIKELY(decompresult != LZMA_OK)) {
                d->m_errorstring = i18n("Could not initialize decompressor");
                d->m_result.clear();
                return false;
            }

            decompresult = LZMA_MEM_ERROR;
            while (decompresult == LZMA_MEM_ERROR) {
                decompresult = lzma_code(&decomp, LZMA_FINISH);

                if (decompresult == LZMA_MEM_ERROR) {
                    speculativesize = (speculativesize + QT_BUFFSIZE);
                    d->m_result.resize(speculativesize);
                }

                if (speculativesize >= INT_MAX) {
                    break;
                }
            }

            if (Q_UNLIKELY(decompresult != LZMA_OK && decompresult != LZMA_STREAM_END)) {
                d->m_errorstring = i18n("Could not decompress data");
                d->m_result.clear();
                return false;
            }
            speculativesize = decomp.total_out;
            lzma_end(&decomp);

            d->m_result.resize(speculativesize);
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

QByteArray KDecompressor::result() const
{
    return d->m_result;
}

QString KDecompressor::errorString() const
{
    return d->m_errorstring;
}

KDecompressor::KDecompressorType KDecompressor::typeForMime(const QString &mime)
{
    const KMimeType::Ptr kmimetype = KMimeType::mimeType(mime);
    if (kmimetype) {
        if (kmimetype->is(QString::fromLatin1("application/x-gzip"))) {
            return KDecompressor::TypeGZip;
        } else if (kmimetype->is(QString::fromLatin1("application/x-bzip"))) {
            return KDecompressor::TypeBZip2;
        } else if (kmimetype->is(QString::fromLatin1("application/x-xz"))) {
            return KDecompressor::TypeXZ;
        }
    }
    return KDecompressor::TypeUnknown;
}

KDecompressor::KDecompressorType KDecompressor::typeForFile(const QString &filepath)
{
    const KMimeType::Ptr kmimetype = KMimeType::findByPath(filepath);
    if (kmimetype) {
        return KDecompressor::typeForMime(kmimetype->name());
    }
    return KDecompressor::TypeUnknown;
}
