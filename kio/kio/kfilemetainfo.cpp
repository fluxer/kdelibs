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
#include <kconfiggroup.h>

#include <QFileInfo>
#include <QDateTime>
#include <QStringList>

#ifndef KIO_NO_LIBEXTRACTOR
static const KFileMetaInfoItem nullitem;
static KFileMetaInfoItem mutablenullitem;

class KFileMetaInfoPrivate : public QSharedData
{
public:
    KFileMetaInfoItemList items;
    KUrl m_url;

    void init(const QByteArray &filepath, const KUrl& url, KFileMetaInfo::WhatFlags w);
    void operator=(const KFileMetaInfoPrivate& kfmip) {
        items = kfmip.items;
        m_url = kfmip.m_url;
    }

    static QString string(enum EXTRACTOR_MetaFormat format,
                          const char *data, size_t data_len);
    static QString time(enum EXTRACTOR_MetaFormat format,
                        const char *data, size_t data_len);
    static QString frameRate(enum EXTRACTOR_MetaFormat format,
                             const char *data, size_t data_len);
    static QString sampleRate(enum EXTRACTOR_MetaFormat format,
                              const char *data, size_t data_len);
    static QString bitRate(enum EXTRACTOR_MetaFormat format,
                           const char *data, size_t data_len);
    static int metadata(void *cls,
                        const char *plugin_name,
                        enum EXTRACTOR_MetaType type,
                        enum EXTRACTOR_MetaFormat format,
                        const char *data_mime_type,
                        const char *data,
                        size_t data_len);
};

QString KFileMetaInfoPrivate::string(enum EXTRACTOR_MetaFormat format,
                                     const char *data, size_t data_len)
{
    switch (format) {
        case EXTRACTOR_METAFORMAT_UTF8: {
            return QString::fromUtf8(data, data_len - 1);
        }
        case EXTRACTOR_METAFORMAT_C_STRING: {
            return QString::fromLocal8Bit(data, data_len - 1);
        }
        case EXTRACTOR_METAFORMAT_BINARY: {
            return QString();
        }
        case EXTRACTOR_METAFORMAT_UNKNOWN:
        default: {
            kWarning() << "Unknown metadata format";
            return QString();
        }
    }
    Q_UNREACHABLE();
    return QString();
}

QString KFileMetaInfoPrivate::time(enum EXTRACTOR_MetaFormat format,
                                   const char *data, size_t data_len)
{
    const QString timestring = KFileMetaInfoPrivate::string(format, data, data_len);
    const QStringList splittimestring = timestring.split(QLatin1Char('.'));
    if (splittimestring.size() == 2) {
        return splittimestring.at(0);
    }
    kDebug() << "Unexpected time" << timestring;
    return timestring;
}

QString KFileMetaInfoPrivate::frameRate(enum EXTRACTOR_MetaFormat format,
                                        const char *data, size_t data_len)
{
    const QString frameratestring = KFileMetaInfoPrivate::string(format, data, data_len);
    const QStringList splitframeratestring = frameratestring.split(QLatin1Char('/'));
    if (splitframeratestring.size() == 2) {
        return i18n("%1 per second").arg(splitframeratestring.at(0));
    }
    kDebug() << "Unexpected frame rate" << frameratestring;
    return frameratestring;
}

QString KFileMetaInfoPrivate::sampleRate(enum EXTRACTOR_MetaFormat format,
                                         const char *data, size_t data_len)
{
    const QString ratestring = KFileMetaInfoPrivate::string(format, data, data_len);
    return i18n("%1 kHz").arg(ratestring.toInt() / 1000);
}

QString KFileMetaInfoPrivate::bitRate(enum EXTRACTOR_MetaFormat format,
                                         const char *data, size_t data_len)
{
    const QString ratestring = KFileMetaInfoPrivate::string(format, data, data_len);
    return i18n("%1 Kbps").arg(ratestring.toInt() / 1000);
}

int KFileMetaInfoPrivate::metadata(void *cls,
                                   const char *plugin_name,
                                   enum EXTRACTOR_MetaType type,
                                   enum EXTRACTOR_MetaFormat format,
                                   const char *data_mime_type,
                                   const char *data,
                                   size_t data_len)
{
    // kDebug() << type << format << data_mime_type << data_len;
    KFileMetaInfoPrivate* kfmip = static_cast<KFileMetaInfoPrivate*>(cls);
    switch (type) {
        case EXTRACTOR_METATYPE_MIMETYPE: {
            // kfileitem#mimetype, handled internally
            break;
        }
        case EXTRACTOR_METATYPE_FILENAME: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_COMMENT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_TITLE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_PAGE_COUNT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        // TODO: or http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress?
        case EXTRACTOR_METATYPE_AUTHOR_EMAIL: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Email", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_PUBLISHER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_URL: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_URI: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#uniqueFileIdentifier", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_HASH_MD4: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            const KFileMetaInfoItem kfmi2("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", QString::fromLatin1("MD4"));
            kfmip->items.append(kfmi2);
            break;
        }
        case EXTRACTOR_METATYPE_HASH_MD5: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            const KFileMetaInfoItem kfmi2("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", QString::fromLatin1("MD5"));
            kfmip->items.append(kfmi2);
            break;
        }
        case EXTRACTOR_METATYPE_HASH_SHA0: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            const KFileMetaInfoItem kfmi2("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", QString::fromLatin1("SHA-0"));
            kfmip->items.append(kfmi2);
            break;
        }
        case EXTRACTOR_METATYPE_HASH_SHA1: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            const KFileMetaInfoItem kfmi2("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", QString::fromLatin1("SHA-1"));
            kfmip->items.append(kfmi2);
            break;
        }
        case EXTRACTOR_METATYPE_GPS_LATITUDE_REF: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_GPS_LONGITUDE_REF: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_LOCATION_COUNTRY: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#country", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_DESCRIPTION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_COPYRIGHT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_KEYWORDS: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_SUMMARY: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#summary", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_SUBJECT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CREATOR: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CREATED_BY_SOFTWARE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CREATION_DATE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_MODIFICATION_DATE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentLastModified", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_EXPOSURE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_EXPOSURE_BIAS: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_APERTURE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_FLASH: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_FOCAL_LENGTH: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_FOCAL_LENGTH_35MM: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ISO_SPEED: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_EXPOSURE_MODE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_METERING_MODE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_WHITE_BALANCE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ORIENTATION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_IMAGE_RESOLUTION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#resolution", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CHARACTER_SET: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#characterSet", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_LINE_COUNT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#lineCount", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_WORD_COUNT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#wordCount", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CHARACTER_COUNT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#characterCount", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_REVISION_NUMBER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#sequence", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ALBUM: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ARTIST: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_GENRE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_TRACK_NUMBER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_PERFORMER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CONTACT_INFORMATION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_DISCLAIMER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#disclaimer", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_WRITER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#textWriter", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CONTRIBUTOR_NAME: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contributor", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_MOVIE_DIRECTOR: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#director", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CONDUCTOR: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#conductor", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_INTERPRETATION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#interpretedBy", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_COMPOSER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#composer", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_BEATS_PER_MINUTE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#beatsPerMinute", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ENCODED_BY: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encodedBy", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ORIGINAL_ARTIST: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalArtist", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ORIGINAL_WRITER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalTextWriter", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ORIGINAL_RELEASE_YEAR: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalReleaseYear", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_LYRICS: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#SynchronizedText", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_LICENSEE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#licensee", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_MUSICIAN_CREDITS_LIST: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#InvolvedPerson", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_SUBTITLE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#subtitle", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CODEC: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#codec", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_VIDEO_CODEC: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoCodec", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_AUDIO_CODEC: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioCodec", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_SUBTITLE_CODEC: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#subtitleCodec", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CONTAINER_FORMAT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#belongsToContainer", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_BITRATE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate", KFileMetaInfoPrivate::bitRate(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_ENCODER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encoder", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_GROUPING: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#UnionOfEventJournalTodo", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_DEVICE_MANUFACTURER: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CAMERA_MODEL:
        case EXTRACTOR_METATYPE_DEVICE_MODEL: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_CHANNELS: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_SAMPLE_RATE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleRate", KFileMetaInfoPrivate::sampleRate(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_AUDIO_DEPTH: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitDepth", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_AUDIO_BITRATE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitRate", KFileMetaInfoPrivate::bitRate(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_VIDEO_DEPTH: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitDepth", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_FRAME_RATE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#frameRate", KFileMetaInfoPrivate::frameRate(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_PIXEL_ASPECT_RATIO: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#aspectRatio", KFileMetaInfoPrivate::string(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_VIDEO_BITRATE: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitRate", KFileMetaInfoPrivate::bitRate(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_DURATION:
        case EXTRACTOR_METATYPE_VIDEO_DURATION:
        case EXTRACTOR_METATYPE_AUDIO_DURATION: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration", KFileMetaInfoPrivate::time(format, data, data_len));
            kfmip->items.append(kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_IMAGE_DIMENSIONS:
        case EXTRACTOR_METATYPE_VIDEO_DIMENSIONS: {
            const QString dimensions = KFileMetaInfoPrivate::string(format, data, data_len);
            const QStringList splitdimensions = dimensions.split(QLatin1Char('x'));
            if (splitdimensions.size() == 2) {
                const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width", splitdimensions.at(0));
                kfmip->items.append(kfmi);
                const KFileMetaInfoItem kfmi2("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height", splitdimensions.at(1));
                kfmip->items.append(kfmi2);
            } else {
                kWarning() << "Invalid dimensions" << dimensions;
            }
            break;
        }
    }
    return 0;
}

void KFileMetaInfoPrivate::init(const QByteArray &filepath, const KUrl& url, KFileMetaInfo::WhatFlags w)
{
    m_url = url;

    struct EXTRACTOR_PluginList *extractorplugins = NULL;
    // since EXTRACTOR_plugin_remove() does not really work explicitly enable most plugins
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "av", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "exiv2", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "flac", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "gif", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "gstreamer", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "jpeg", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "midi", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "mpeg", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "ogg", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "tiff", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "it", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "nsf", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "nsfe", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "png", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "riff", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "s3m", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "sid", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "wav", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "xm", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    if ((w & KFileMetaInfo::ContentInfo) != 0) {
        extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "pdf", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
        extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "ole2", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
        extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "html", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
        extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "dvi", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
        extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "man", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
        extractorplugins = EXTRACTOR_plugin_add(extractorplugins, "ps", NULL, EXTRACTOR_OPTION_DEFAULT_POLICY);
    }

    EXTRACTOR_extract(
        extractorplugins,
        filepath.constData(),
        NULL, //data
        0, // size
        KFileMetaInfoPrivate::metadata, this
    );
    EXTRACTOR_plugin_remove_all(extractorplugins);

    bool hasfilename = false;
    // remove duplicates from the falltrough cases in the extraction which essentially have the
    // same meaning
    KFileMetaInfoItemList::iterator it = items.begin();
    QStringList itemkeys;
    itemkeys.reserve(items.size());
    while (it != items.end()) {
        if (it->key() == QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName")) {
            hasfilename = true;
        }

        if (!itemkeys.contains(it->key())) {
            itemkeys.append(it->key());
            it++;
        } else {
            kDebug() << "Multiple entries for the same key" << it->key();
            it = items.erase(it);
        }
    }

    // for compatibility
    if (!hasfilename) {
        const QString kfmifilename = QFileInfo(filepath).fileName();
        const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName", kfmifilename);
        items.append(kfmi);
    }
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

const KFileMetaInfoItemList& KFileMetaInfo::items() const
{
    return d->items;
}

const KFileMetaInfoItem& KFileMetaInfo::item(const QString& key) const
{
    for (int i = 0; i < d->items.size(); i++) {
        if (d->items.at(i).key() == key) {
            return d->items.at(i);
        }
    }
    return nullitem;
}

QStringList KFileMetaInfo::keys() const
{
    QStringList result;
    result.reserve(d->items.size());
    for (int i = 0; i < d->items.size(); i++) {
        result.append(d->items.at(i).key());
    }
    return result;
}

KFileMetaInfoItem& KFileMetaInfo::item(const QString& key)
{
    for (int i = 0; i < d->items.size(); i++) {
        if (d->items.at(i).key() == key) {
            return d->items[i];
        }
    }
    return mutablenullitem;
}

bool KFileMetaInfo::isValid() const
{
    return !d->m_url.isEmpty();
}

QStringList KFileMetaInfo::preferredKeys() const
{
    QStringList result;
    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");
    foreach (const QString &key, supportedKeys()) {
        const bool show = settings.readEntry(key, true);
        if (show) {
            result.append(key);
        }
    }
    return result;
}

QStringList KFileMetaInfo::supportedKeys()
{
    static const QStringList s_keys = QStringList()
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Email"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#country"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword"
        << "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#summary"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentLastModified"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#resolution"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#characterSet"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#lineCount"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#wordCount"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#characterCount"
        << "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#sequence"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration"
        << "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist"
        << "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre"
        << "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber"
        << "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact"
        << "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#disclaimer"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#textWriter"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contributor"
        << "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#director"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#conductor"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#interpretedBy"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#composer"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#beatsPerMinute"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encodedBy"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalArtist"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalTextWriter"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalReleaseYear"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#SynchronizedText"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#licensee"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#InvolvedPerson"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#subtitle"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#codec"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoCodec"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioCodec"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#subtitleCodec"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#belongsToContainer"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encoder"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#uniqueFileIdentifier"
        << "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#UnionOfEventJournalTodo"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make"
        << "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleRate"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitDepth"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitRate"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitDepth"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#frameRate"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#aspectRatio"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitRate"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width"
        << "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height";
    return s_keys;
}

QString KFileMetaInfo::name(const QString& key)
{
    typedef std::map<QString,QString> TranslationMap;

    static const TranslationMap s_translations = {
        { "kfileitem#modified", i18nc("@label", "Modified") },
        { "kfileitem#owner", i18nc("@label", "Owner") },
        { "kfileitem#permissions", i18nc("@label", "Permissions") },
        { "kfileitem#size", i18nc("@label", "Size") },
        { "kfileitem#totalSize", i18nc("@label", "Total Size") },
        { "kfileitem#type", i18nc("@label", "Type") },
        { "kfileitem#mimetype", i18nc("@label", "MIME Type") },
        // implemented so far
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName", i18nc("@label", "Filename") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment", i18nc("@label", "Comment") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title", i18nc("@label music title", "Title") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount", i18nc("@label", "Page Count") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator", i18nc("@label", "Creator") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Email", i18nc("@label", "Creator E-Mail") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher", i18nc("@label", "Publisher") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url", i18nc("@label file URL", "URL") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", i18nc("@label", "Hash Value") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", i18nc("@label", "Hash Algorithm") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef", i18nc("@label", "GPS Latitude Reference") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef", i18nc("@label", "GPS Longitude Reference") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#country", i18nc("@label", "Country") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description", i18nc("@label", "Description") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright", i18nc("@label", "Copyright") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword", i18nc("@label", "Keyword") },
        { "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#summary", i18nc("@label", "Summary") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject", i18nc("@label", "Subject") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator", i18nc("@label Software used to generate content", "Generator") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated", i18nc("@label creation date", "Created") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentLastModified", i18nc("@label modification date", "Modified") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime", i18nc("@label EXIF", "Exposure Time") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue", i18nc("@label EXIF", "Exposure Bias Value") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue", i18nc("@label EXIF aperture value", "Aperture") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash", i18nc("@label EXIF", "Flash") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength", i18nc("@label EXIF", "Focal Length") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm", i18nc("@label EXIF", "Focal Length 35 mm") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings", i18nc("@label EXIF", "ISO Speed Ratings") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode", i18nc("@label EXIF", "Exposure Mode") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode", i18nc("@label EXIF", "Metering Mode") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance", i18nc("@label EXIF", "White Balance") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation", i18nc("@label EXIF", "Orientation") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#resolution", i18nc("@label EXIF", "Resolution") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#characterSet", i18nc("@label", "Character Set") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#lineCount", i18nc("@label number of lines", "Lines") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#wordCount", i18nc("@label number of words", "Words") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#characterCount", i18nc("@label", "Character Count") },
        { "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#sequence", i18nc("@label", "Revision") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration", i18nc("@label", "Duration") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum", i18nc("@label music album", "Album") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist", i18nc("@label music or image artist", "Artist") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre",  i18nc("@label music genre", "Genre") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber", i18nc("@label music track number", "Track") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer", i18nc("@label", "Performer") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact", i18nc("@label", "Contact") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#disclaimer", i18nc("@label", "Disclaimer") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#textWriter", i18nc("@label", "Text Writer") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contributor", i18nc("@label", "Contributor") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#director", i18nc("@label video director", "Director") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#conductor", i18nc("@label", "Conductor") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#interpretedBy", i18nc("@label", "Interpreted By") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#composer", i18nc("@label music composer", "Composer") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#beatsPerMinute", i18nc("@label", "Beats Per Minute") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encodedBy", i18nc("@label", "Encoded By") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalArtist", i18nc("@label", "Original Artist") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalTextWriter", i18nc("@label", "Original Text Writer") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalReleaseYear", i18nc("@label", "Original Release Year") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#SynchronizedText", i18nc("@label", "Lyrics") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#licensee", i18nc("@label", "Licensee") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#InvolvedPerson", i18nc("@label", "Musician Credits") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#subtitle", i18nc("@label", "Subtitle") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#codec",  i18nc("@label", "Codec") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoCodec",  i18nc("@label", "Video Codec") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioCodec",  i18nc("@label", "Audio Codec") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#subtitleCodec",  i18nc("@label", "Subtitle Codec") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#belongsToContainer",  i18nc("@label", "Container Format") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate", i18nc("@label", "Average Bitrate") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encoder", i18nc("@label", "Encoder") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#uniqueFileIdentifier", i18nc("@label", "URI") },
        { "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#UnionOfEventJournalTodo", i18nc("@label", "Grouping") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make", i18nc("@label EXIF", "Manufacturer") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model", i18nc("@label EXIF", "Model") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels", i18nc("@label", "Channels") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleRate", i18nc("@label", "Sample Rate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitDepth", i18nc("@label", "Audio Bit Depth") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitRate", i18nc("@label", "Audio Bit Rate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitDepth", i18nc("@label", "Video Bit Depth") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#frameRate", i18nc("@label", "Frame Rate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#aspectRatio", i18nc("@label", "Aspect Ratio") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitRate", i18nc("@label", "Video Bit Rate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width", i18nc("@label", "Width") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height", i18nc("@label", "Height") },
    };

    const TranslationMap::const_iterator it = s_translations.find(key);
    if (it != s_translations.cend()) {
        return it->second;
    }

    // fallback if the URI is not translated
    const int index = key.indexOf(QChar('#'));
    if (index >= 0) {
        return key.right(key.size() - index - 1);
    }
    return key;
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

const KFileMetaInfoItemList& KFileMetaInfo::items() const
{
    static const KFileMetaInfoItemList items;
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

QString KFileMetaInfo::name(const QString& key)
{
    Q_UNUSED(key);
    retrun QString();
}

#endif // KIO_NO_LIBEXTRACTOR
