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

QString KImageIO::pattern(Mode mode)
{
    QStringList patterns;
    QString allPatterns;
    static const QLatin1Char separator('|');

    foreach(const QString &mimeType, KImageIO::mimeTypes(mode)) {
        KMimeType::Ptr mime = KMimeType::mimeType(mimeType);
        if (!mime) {
            kWarning() << "unknown toolkit mimetype " << mimeType;
        } else {
            QString pattern = mime->patterns().join(" ");
            patterns.append(pattern + separator + mime->comment());
            if (!allPatterns.isEmpty() )
                allPatterns += ' ';
            allPatterns += pattern;
        }
    }

    allPatterns = allPatterns + separator + i18n("All Pictures");
    patterns.sort();
    patterns.prepend(allPatterns);

    return patterns.join(QLatin1String("\n"));
}

QStringList KImageIO::typeForMime(const QString &mimeType)
{
    QStringList result;
    if (mimeType.isEmpty()) {
        return result;
    }
    const QByteArray format = QImageReader::formatForMimeType(mimeType.toLatin1());
    if (!format.isEmpty()) {
        result << QString::fromLatin1(format.constData(), format.size());
    }
    return result;
}

QStringList KImageIO::mimeTypes(Mode mode)
{
    QStringList result;
    if (mode == KImageIO::Reading) {
        foreach(const QByteArray &mime, QImageReader::supportedMimeTypes()) {
            result << QString::fromLatin1(mime.constData(), mime.size());
        }
    } else {
        foreach(const QByteArray &mime, QImageWriter::supportedMimeTypes()) {
            result << QString::fromLatin1(mime.constData(), mime.size());
        }
    }
    return result;
}

QStringList KImageIO::types(Mode mode)
{
    QStringList result;
    if (mode == KImageIO::Reading) {
        foreach(const QByteArray &format, QImageReader::supportedImageFormats()) {
            result << QString::fromLatin1(format.constData(), format.size());
        }
    } else {
        foreach(const QByteArray &format, QImageWriter::supportedImageFormats()) {
            result << QString::fromLatin1(format.constData(), format.size());
        }
    }
    return result;
}

bool KImageIO::isSupported(const QString &mimeType, Mode mode)
{
    if (mimeType.isEmpty()) {
        return false;
    }
    foreach(const QString &mime, KImageIO::mimeTypes(mode)) {
        if (mimeType == mime) {
            return true;
        }
    }
    return false;
}
