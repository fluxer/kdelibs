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
#include "kglobal.h"
#include "klocale.h"
#include "kdebug.h"

static QString getExiv2Time(const QString &exiv2time)
{
    const QDateTime qdatetime = QDateTime::fromString(exiv2time, Qt::ISODate);
    return KGlobal::locale()->formatDateTime(qdatetime, QLocale::NarrowFormat);
}

KFileMetaDataExiv2Plugin::KFileMetaDataExiv2Plugin(QObject* parent, const QVariantList &args)
    : KFileMetaDataPlugin(parent)
{
    Q_UNUSED(args);
}

KFileMetaDataExiv2Plugin::~KFileMetaDataExiv2Plugin()
{
}

QList<KFileMetaInfoItem> KFileMetaDataExiv2Plugin::metaData(const QString &path)
{
    QList<KFileMetaInfoItem> result;
    const KExiv2 kexiv2(path);
    foreach (const KExiv2Property &kexiv2property, kexiv2.metadata()) {
        // qDebug() << Q_FUNC_INFO << kexiv2property.name << kexiv2property.value;
        // for reference:
        // https://exiv2.org/tags.html
        if (kexiv2property.name == "Exif.Image.ImageWidth") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.ImageLength") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.FrameRate") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#frameRate"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.Copyright") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.UserComment" || kexiv2property.name == "Exif.Photo.XPComment") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Photo.XPTitle") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.XPKeywords") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword"),
                    kexiv2property.value
                )
            );

        } else if (kexiv2property.name == "Exif.Image.XPSubject") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.ImageDescription") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.Software") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.DateTimeOriginal" || kexiv2property.name == "Exif.Photo.DateTimeOriginal") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated"),
                    getExiv2Time(kexiv2property.value)
                )
            );
        } else if (kexiv2property.name == "Exif.Image.Make") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.Model") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.Orientation") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.Artist") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.GPSInfo.GPSLatitudeRef") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.GPSInfo.GPSLongitudeRef") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.Flash") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Photo.ExposureTime") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.ExposureBiasValue") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Photo.ExposureMode") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Photo.ApertureValue") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.FocalLength") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Photo.FocalLengthIn35mmFilm") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Image.ISOSpeedRatings") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Photo.MeteringMode") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Photo.WhiteBalance") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance"),
                    kexiv2property.value
                )
            );
        } else if (kexiv2property.name == "Exif.Photo.ImageUniqueID") {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#uniqueFileIdentifier"),
                    kexiv2property.value
                )
            );
        }
    }
    return result;
}

K_PLUGIN_FACTORY(KFileMetaDataExiv2PluginFactory, registerPlugin<KFileMetaDataExiv2Plugin>();)
K_EXPORT_PLUGIN(KFileMetaDataExiv2PluginFactory("kfilemetadata_exiv2"))

#include "moc_kfilemetadata_exiv2.cpp"
