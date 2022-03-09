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

#include "kfilemetainfo.h"
#include "kfilemetainfoitem.h"
#include "kfilemetainfoitem_p.h"

#ifndef KIO_NO_LIBEXTRACTOR
#include <extractor.h>
#endif

#include <kurl.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <QFileInfo>
#include <QDateTime>
#include <QStringList>

QDataStream& operator>>(QDataStream& s, KFileMetaInfo& kfmi)
{
    return s;
}

QDataStream& operator<<(QDataStream& s, const KFileMetaInfo& kfmi)
{
    return s;
}

#ifndef KIO_NO_LIBEXTRACTOR
static const KFileMetaInfoItem nullitem;

class KFileMetaInfoPrivate : public QSharedData
{
public:
    QHash<QString, KFileMetaInfoItem> items;
    KUrl m_url;

    void init(const QByteArray &filepath, const KUrl& url, KFileMetaInfo::WhatFlags w);
    void operator=(const KFileMetaInfoPrivate& kfmip) {
        items = kfmip.items;
    }

    static QVariant variant(enum EXTRACTOR_MetaFormat format,
                            const char *data, size_t data_len);
    static int metadata(void *cls,
                        const char *plugin_name,
                        enum EXTRACTOR_MetaType type,
                        enum EXTRACTOR_MetaFormat format,
                        const char *data_mime_type,
                        const char *data,
                        size_t data_len);
};

QVariant KFileMetaInfoPrivate::variant(enum EXTRACTOR_MetaFormat format,
                                       const char *data, size_t data_len)
{
    switch (format) {
        case EXTRACTOR_METAFORMAT_UTF8: {
            return QVariant(QString::fromUtf8(data, data_len - 1));
        }
        case EXTRACTOR_METAFORMAT_C_STRING: {
            return QVariant(QString::fromLocal8Bit(data, data_len - 1));
        }
        case EXTRACTOR_METAFORMAT_BINARY: {
            return QVariant();
        }
        case EXTRACTOR_METAFORMAT_UNKNOWN:
        default: {
            kWarning() << "Unknown metadata format";
            return QVariant();
        }
    }
    Q_UNREACHABLE();
    return QVariant();
}

int KFileMetaInfoPrivate::metadata(void *cls,
                                   const char *plugin_name,
                                   enum EXTRACTOR_MetaType type,
                                   enum EXTRACTOR_MetaFormat format,
                                   const char *data_mime_type,
                                   const char *data,
                                   size_t data_len)
{
    // qDebug() << Q_FUNC_INFO << type << format << data_mime_type << data_len;
    KFileMetaInfoPrivate* kfmip = static_cast<KFileMetaInfoPrivate*>(cls);
    switch (type) {
        case EXTRACTOR_METATYPE_MIMETYPE: {
            // kfileitem#mimetype, handled internally
            break;
        }
        case EXTRACTOR_METATYPE_FILENAME: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_COMMENT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_TITLE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_PAGE_COUNT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount", kfmi);
            break;
        }
        // TODO: or http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress?
        case EXTRACTOR_METATYPE_AUTHOR_EMAIL: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Email", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Email", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_PUBLISHER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_URL: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_HASH_MD4: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", kfmi);
            const KFileMetaInfoItem kfmi2("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", QVariant(QString::fromLatin1("MD4")));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", kfmi2);
            break;
        }
        case EXTRACTOR_METATYPE_HASH_MD5: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", kfmi);
            const KFileMetaInfoItem kfmi2("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", QVariant(QString::fromLatin1("MD5")));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", kfmi2);
            break;
        }
        case EXTRACTOR_METATYPE_HASH_SHA0: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", kfmi);
            const KFileMetaInfoItem kfmi2("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", QVariant(QString::fromLatin1("SHA-0")));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", kfmi2);
            break;
        }
        case EXTRACTOR_METATYPE_HASH_SHA1: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", kfmi);
            const KFileMetaInfoItem kfmi2("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", QVariant(QString::fromLatin1("SHA-1")));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", kfmi2);
            break;
        }
        case EXTRACTOR_METATYPE_GPS_LATITUDE_REF: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_GPS_LONGITUDE_REF: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_LOCATION_COUNTRY: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#country", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#country", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_DESCRIPTION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_COPYRIGHT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_KEYWORDS: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_SUMMARY: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#summary", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#summary", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_SUBJECT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CREATOR: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CREATED_BY_SOFTWARE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CREATION_DATE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_MODIFICATION_DATE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentLastModified", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentLastModified", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CAMERA_MODEL: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_EXPOSURE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_EXPOSURE_BIAS: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_APERTURE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_FLASH: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_FOCAL_LENGTH: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_FOCAL_LENGTH_35MM: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ISO_SPEED: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_EXPOSURE_MODE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_METERING_MODE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_WHITE_BALANCE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ORIENTATION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_IMAGE_RESOLUTION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#resolution", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#resolution", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CHARACTER_SET: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#characterSet", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#characterSet", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_LINE_COUNT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#lineCount", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#lineCount", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_WORD_COUNT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#wordCount", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#wordCount", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CHARACTER_COUNT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#characterCount", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#characterCount", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_REVISION_NUMBER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#sequence", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#sequence", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_DURATION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ALBUM: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ARTIST: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_GENRE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_TRACK_NUMBER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_PERFORMER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CONTACT_INFORMATION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_DISCLAIMER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#disclaimer", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#disclaimer", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_WRITER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#textWriter", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#textWriter", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CONTRIBUTOR_NAME: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contributor", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contributor", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_MOVIE_DIRECTOR: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#director", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#director", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CONDUCTOR: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#conductor", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#conductor", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_INTERPRETATION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#interpretedBy", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#interpretedBy", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_COMPOSER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#composer", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#composer", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_BEATS_PER_MINUTE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#beatsPerMinute", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#beatsPerMinute", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ENCODED_BY: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encodedBy", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encodedBy", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ORIGINAL_ARTIST: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalArtist", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalArtist", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ORIGINAL_WRITER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalTextWriter", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalTextWriter", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ORIGINAL_RELEASE_YEAR: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalReleaseYear", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalReleaseYear", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_LYRICS: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#SynchronizedText", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#SynchronizedText", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_LICENSEE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#licensee", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#licensee", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_MUSICIAN_CREDITS_LIST: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#InvolvedPerson", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#InvolvedPerson", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_SUBTITLE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#subtitle", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#subtitle", kfmi);
            break;
        }
        // TODO: handle more cases
    }
    return 0;
}

void KFileMetaInfoPrivate::init(const QByteArray &filepath, const KUrl& url, KFileMetaInfo::WhatFlags w)
{
    m_url = url;
    struct EXTRACTOR_PluginList *extractorplugins = EXTRACTOR_plugin_add_defaults(EXTRACTOR_OPTION_DEFAULT_POLICY);
    // not interested in the metadata provided by these plugins
    extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "archive");
    extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "zip");
    extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "deb");
    extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "rpm");
    extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "thumbnailgtk");
    if ((w & KFileMetaInfo::ContentInfo) == 0) {
        extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "pdf");
        extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "ole2");
        extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "html");
    }
    EXTRACTOR_extract(
        extractorplugins,
        filepath.constData(),
        NULL, //data
        0, // size
        KFileMetaInfoPrivate::metadata, this
    );
    EXTRACTOR_plugin_remove_all(extractorplugins);
}

KFileMetaInfo::KFileMetaInfo(const QString& path, KFileMetaInfo::WhatFlags w)
    : d(new KFileMetaInfoPrivate())
{
    QFileInfo fileinfo(path);
    // only open the file if it is not a pipe
    if (fileinfo.isFile() || fileinfo.isDir() || fileinfo.isSymLink()) {
        d->init(path.toLocal8Bit(), KUrl(path), w);
    }
}

KFileMetaInfo::KFileMetaInfo(const KUrl& url, KFileMetaInfo::WhatFlags w)
    : d(new KFileMetaInfoPrivate())
{
    const QString path = url.toLocalFile();
    d->init(path.toLocal8Bit(), url, w);
}

KFileMetaInfo::KFileMetaInfo()
    : d(new KFileMetaInfoPrivate())
{
}

KFileMetaInfo::KFileMetaInfo(const KFileMetaInfo& kfmi)
    : d(kfmi.d)
{
}

KFileMetaInfo& KFileMetaInfo::operator=(KFileMetaInfo const& kfmi)
{
    d = kfmi.d;
    return *this;
}

KFileMetaInfo::~KFileMetaInfo()
{
}

const KUrl& KFileMetaInfo::url() const
{
    return d->m_url;
}

const QHash<QString, KFileMetaInfoItem>& KFileMetaInfo::items() const
{
    return d->items;
}

const KFileMetaInfoItem& KFileMetaInfo::item(const QString& key) const
{
    QHash<QString, KFileMetaInfoItem>::const_iterator i = d->items.constFind(key);
    return (i == d->items.constEnd()) ? nullitem : i.value();
}

QStringList KFileMetaInfo::keys() const
{
    return d->items.keys();
}

KFileMetaInfoItem& KFileMetaInfo::item(const QString& key)
{
    return d->items[key];
}

bool KFileMetaInfo::isValid() const
{
    return !d->m_url.isEmpty();
}

QStringList KFileMetaInfo::preferredKeys() const
{
#warning TODO: implement
    return QStringList();
}

QStringList KFileMetaInfo::supportedKeys() const
{
#warning TODO: implement properly
    return keys();
}

#else //KIO_NO_LIBEXTRACTOR

class KFileMetaInfoPrivate : public QSharedData
{
public:
};

KFileMetaInfo::KFileMetaInfo(const QString& path, KFileMetaInfo::WhatFlags w)
{
    Q_UNUSED(path);
    Q_UNUSED(w);
}

KFileMetaInfo::KFileMetaInfo(const KUrl& url, KFileMetaInfo::WhatFlags w)
{
    Q_UNUSED(url);
    Q_UNUSED(w);
}

KFileMetaInfo::KFileMetaInfo()
{
}

KFileMetaInfo::KFileMetaInfo(const KFileMetaInfo& kfmi)
{
    Q_UNUSED(kfmi);
}

KFileMetaInfo& KFileMetaInfo::operator=(KFileMetaInfo const& kfmi)
{
    Q_UNUSED(kfmi);
    return *this;
}

KFileMetaInfo::~KFileMetaInfo()
{
}

const KUrl& KFileMetaInfo::url() const
{
    static const KUrl item;
    return item;
}

const QHash<QString, KFileMetaInfoItem>& KFileMetaInfo::items() const
{
    static const QHash<QString, KFileMetaInfoItem> items;
    return items;
}

const KFileMetaInfoItem& KFileMetaInfo::item(const QString& key) const
{
    Q_UNUSED(key);
    static const KFileMetaInfoItem item;
    return item;
}

QStringList KFileMetaInfo::keys() const
{
    return QStringList();
}

KFileMetaInfoItem& KFileMetaInfo::item(const QString& key)
{
    Q_UNUSED(key);
    static KFileMetaInfoItem item;
    return item;
}

bool KFileMetaInfo::isValid() const
{
    return false;
}

QStringList KFileMetaInfo::preferredKeys() const
{
    return QStringList();
}

QStringList KFileMetaInfo::supportedKeys() const
{
    return QStringList();
}
#endif // KIO_NO_LIBEXTRACTOR
