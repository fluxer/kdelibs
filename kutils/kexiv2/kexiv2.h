/*  This file is part of the KDE libraries
    Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KEXIV2_H
#define KEXIV2_H

#include "kexiv2_export.h"

#include <QList>
#include <QString>
#include <QImage>

class KExiv2Private;


/*!
    Structure that holds information about Exif, IPTC or XMP property.

    @since 4.22
    @see KExiv2
    @link https://exiv2.org/metadata.html
*/
struct KExiv2Property
{
    QByteArray name;
    QString value;
    QString label;
};

typedef QList<KExiv2Property> KExiv2PropertyList;

/*!
    Class to obtain Exif, IPTC and XMP metadata, preview and rotate images based on the metadata.

    @note Initialization and cleanup of the Exiv2 library resources is automatic
    @since 4.20
    @see KExiv2Property
*/
class KEXIV2_EXPORT KExiv2
{
public:
    /*!
        @brief Contructs object from @p path
    */
    KExiv2(const QString &path);
    ~KExiv2();

    static bool isSupported();

    /*!
        @return Largest preview image if provided in the metadata, the image is not rotated
        automatically and may be null
    */
    QImage preview() const;

    /*!
        @return Rotates @p image according to the orientation metadata
    */
    bool rotateImage(QImage &image) const;

    /*!
        @return List of all Exif, IPTC and XMP metadata properties
    */
    KExiv2PropertyList metadata() const;

private:
    Q_DISABLE_COPY(KExiv2);
    KExiv2Private * const d;
};

#endif // KEXIV2_H
