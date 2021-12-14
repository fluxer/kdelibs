/**
* kimgio.h -- Implementation of interface to the KDE Image IO library.
* Copyright (c) 1998 Sirtaj Singh Kang <taj@kde.org>
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#include "kimageio.h"

#include "kmimetype.h"
#include <kservicetypetrader.h>
#include <klocale.h>
#include <kdebug.h>

#include <QImageReader>
#include <QImageWriter>

// https://www.iana.org/assignments/media-types/media-types.xhtml#image
static const struct ImageFormatTblData {
    const char *mime;
    const QLatin1String format;
} ImageFormatTbl[] = {
    { "image/png", QLatin1String("png") },
    { "image/x-portable-pixmap", QLatin1String("ppm") },
    { "image/x-portable-graymap", QLatin1String("pgm") },
    { "image/x-portable-bitmap", QLatin1String("pbm") },
    { "image/x-xbitmap", QLatin1String("xbm") },
    { "image/x-xpixmap", QLatin1String("xpm") },
    { "image/gif", QLatin1String("gif") },
    { "image/svg+xml", QLatin1String("svg") },
    { "image/svg+xml-compressed", QLatin1String("svgz") },
};
static const qint16 ImageFormatTblSize = sizeof(ImageFormatTbl) / sizeof(ImageFormatTblData);

static QStringList toolkitSupported(KImageIO::Mode mode) {
    QStringList formats;
    if (mode == KImageIO::Reading) {
        foreach(const QByteArray &format, QImageReader::supportedImageFormats()) {
            formats << QString::fromLatin1(format.constData());
        }
    } else {
        foreach(const QByteArray &format, QImageWriter::supportedImageFormats()) {
            formats << QString::fromLatin1(format.constData());
        }
    }
    return formats;
}

static QStringList toolkitSupportedMimeTypes(KImageIO::Mode mode) {
    QStringList mimeTypes;
    foreach(const QString &format, toolkitSupported(mode)) {
        for(int i = 0; i < ImageFormatTblSize; i++) {
            if (ImageFormatTbl[i].format == format) {
                mimeTypes << QString::fromLatin1(ImageFormatTbl[i].mime);
            }
        }
    }
    return mimeTypes;
}

static QStringList toolkitSupportedTypes(const QString &mimeType) {
    QStringList types;
    const QStringList supportedMimeTypes = toolkitSupportedMimeTypes(KImageIO::Reading);
    if (!supportedMimeTypes.contains(mimeType)) {
        return types;
    }
    const QByteArray latinMimeType = mimeType.toLatin1();
    for(int i = 0; i < ImageFormatTblSize; i++) {
        if (ImageFormatTbl[i].mime == latinMimeType) {
            types << ImageFormatTbl[i].format;
        }
    }

    return types;
}

QString KImageIO::pattern(Mode mode)
{
    QStringList patterns;
    QString allPatterns;
    QString separator("|");

    foreach(const QString &mimeType, toolkitSupportedMimeTypes(mode)) {
        KMimeType::Ptr mime = KMimeType::mimeType( mimeType );
        if (!mime) {
            kWarning() << "unknown toolkit mimetype " << mimeType;
        } else {
            QString pattern = mime->patterns().join(" ");
            patterns.append( pattern + separator + mime->comment() );
            if (!allPatterns.isEmpty() )
                allPatterns += ' ';
            allPatterns += pattern;
        }
    }

    const KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    foreach(const KService::Ptr &service, services)
    {
        if ( (service->property("X-KDE-Read").toBool() && mode == Reading) ||
             (service->property("X-KDE-Write").toBool() && mode == Writing ) ) {

            QString mimeType = service->property("X-KDE-MimeType").toString();
            if ( mimeType.isEmpty() ) continue;
            KMimeType::Ptr mime = KMimeType::mimeType( mimeType );
            if (!mime) {
                kWarning() << service->entryPath() << " specifies unknown mimetype " << mimeType;
            } else {
                QString pattern = mime->patterns().join(" ");
                patterns.append( pattern + separator + mime->comment() );
                if (!allPatterns.isEmpty() )
                    allPatterns += ' ';
                allPatterns += pattern;
            }
        }
    }

    allPatterns = allPatterns + separator + i18n("All Pictures");
    patterns.sort();
    patterns.prepend(allPatterns);

    return patterns.join(QLatin1String("\n"));
}

QStringList KImageIO::typeForMime(const QString& mimeType)
{
    if ( mimeType.isEmpty() )
        return QStringList();

    QStringList toolkitTypes = toolkitSupportedTypes(mimeType);
    if (!toolkitTypes.isEmpty()) {
        return toolkitTypes;
    }

    const KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    foreach(const KService::Ptr &service, services) {
        if ( mimeType == service->property("X-KDE-MimeType").toString() )
            return ( service->property("X-KDE-ImageFormat").toStringList() );
    }

    return QStringList();
}

QStringList KImageIO::mimeTypes( Mode mode )
{
    QStringList mimeList = toolkitSupportedMimeTypes(mode);

    const KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    foreach(const KService::Ptr &service, services) {
        if ( (service->property("X-KDE-Read").toBool() && mode == Reading) ||
             (service->property("X-KDE-Write").toBool() && mode == Writing ) ) {

            const QString mime = service->property("X-KDE-MimeType").toString();
            if ( !mime.isEmpty() )
                mimeList.append( mime );
        }
    }

    return mimeList;
}

QStringList KImageIO::types( Mode mode )
{
    QStringList imagetypes = toolkitSupported(mode);

    const KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    foreach(const KService::Ptr &service, services) {
        if ( (service->property("X-KDE-Read").toBool() && mode == Reading) ||
             (service->property("X-KDE-Write").toBool() && mode == Writing ) ) {

            imagetypes += service->property("X-KDE-ImageFormat").toStringList();
        }
    }
    return imagetypes;
}

bool KImageIO::isSupported( const QString& mimeType, Mode mode )
{
    if (mimeType.isEmpty() )
        return false;

    foreach(const QString &mime, toolkitSupportedMimeTypes(mode)) {
        if (mimeType == mime) {
            return true;
        }
    }

    const KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    foreach(const KService::Ptr &service, services) {
        if ( mimeType == service->property("X-KDE-MimeType").toString() ) {

            if ( (service->property("X-KDE-Read").toBool() && mode == Reading) ||
                 (service->property("X-KDE-Write").toBool() && mode == Writing ) ) {

                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}
