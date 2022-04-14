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

#include "kfilemetadata_exiv2.h"
#include "kpluginfactory.h"
#include "kexiv2.h"

#include <QDebug>

KFileMetaDataExiv2Plugin::KFileMetaDataExiv2Plugin(QObject* parent, const QVariantList &args)
    : KFileMetaDataPlugin(parent)
{
    Q_UNUSED(args);
}

KFileMetaDataExiv2Plugin::~KFileMetaDataExiv2Plugin()
{
}

QStringList KFileMetaDataExiv2Plugin::keys() const
{
    static const QStringList result = QStringList()
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance");
    return result;
}

QStringList KFileMetaDataExiv2Plugin::mimeTypes() const
{
    // for reference:
    // https://dev.exiv2.org/projects/exiv2/wiki/Supported_image_formats
    static const QStringList result = QStringList()
        << QString::fromLatin1("image/jpeg")
        << QString::fromLatin1("image/x-exv")
        << QString::fromLatin1("image/x-canon-cr2")
        << QString::fromLatin1("image/x-canon-crw")
        << QString::fromLatin1("image/x-minolta-mrw")
        << QString::fromLatin1("image/tiff")
        << QString::fromLatin1("image/webp")
        << QString::fromLatin1("image/x-webp")
        << QString::fromLatin1("image/x-nikon-nef")
        << QString::fromLatin1("image/x-pentax-pef")
        << QString::fromLatin1("image/x-panasonic-rw2")
        << QString::fromLatin1("image/x-olympus-orf")
        << QString::fromLatin1("image/png")
        << QString::fromLatin1("image/x-fuji-raf")
        << QString::fromLatin1("image/x-eps")
        << QString::fromLatin1("image/gif")
        << QString::fromLatin1("image/photoshop")
        << QString::fromLatin1("image/x-photoshop")
        << QString::fromLatin1("image/x-tga")
        << QString::fromLatin1("image/bmp")
        << QString::fromLatin1("image/x-bmp")
        << QString::fromLatin1("image/x-ms-bmp")
        << QString::fromLatin1("image/jp2");
    return result;
}

QList<KFileMetaInfoItem> KFileMetaDataExiv2Plugin::metaData(const KUrl &url, const KFileMetaInfo::WhatFlags flags)
{
    Q_UNUSED(flags);
    QList<KFileMetaInfoItem> result;
    const KExiv2 kexiv2(url.toLocalFile());
    const KExiv2::DataMap kexiv2metadata = kexiv2.data();
    foreach (const QByteArray &kexiv2key, kexiv2metadata.keys()) {
        const QString kexiv2value = kexiv2metadata.value(kexiv2key);
        // qDebug() << Q_FUNC_INFO << kexiv2key << kexiv2value;
        // for reference:
        // https://exiv2.org/tags.html
        if (kexiv2key == "Exif.Image.ImageWidth") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.ImageLength") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.Copyright") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.UserComment" || kexiv2key == "Exif.Photo.XPComment") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Photo.XPTitle") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.XPKeywords") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.ImageDescription") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.Software") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.Make") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.Model") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.Orientation") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.Artist") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.GPSInfo.GPSLatitudeRef") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.GPSInfo.GPSLongitudeRef") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.Flash") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Photo.ExposureTime") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.ExposureBiasValue") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Photo.ExposureMode") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Photo.ApertureValue") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.FocalLength") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Photo.FocalLengthIn35mmFilm") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Image.ISOSpeedRatings") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Photo.MeteringMode") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode"),
                    kexiv2value
                )
            );
        } else if (kexiv2key == "Exif.Photo.WhiteBalance") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance"),
                    kexiv2value
                )
            );
        }
    }
    // qDebug() << Q_FUNC_INFO << url << kexiv2metadata;
    return result;
}

K_PLUGIN_FACTORY(KFileMetaDataExiv2PluginFactory, registerPlugin<KFileMetaDataExiv2Plugin>();)
K_EXPORT_PLUGIN(KFileMetaDataExiv2PluginFactory("kfilemetadata_exiv2"))

#include "moc_kfilemetadata_exiv2.cpp"
