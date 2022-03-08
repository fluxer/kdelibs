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

#ifndef KFILEMETAINFOITEM_H
#define KFILEMETAINFOITEM_H

#include <kio/kio_export.h>

#include <QSharedDataPointer>
#include <QVariant>

class KFileMetaInfoItemPrivate;

class KIO_EXPORT KFileMetaInfoItem
{
    friend class KFileMetaInfo;
    friend class KFileMetaInfoPrivate;
public:
    /**
     * @brief Default constructor
     **/
    KFileMetaInfoItem();
    /**
     * @brief Copy constructor
     **/
    KFileMetaInfoItem(const KFileMetaInfoItem& item);
    /**
     * @brief Destructor
     **/
    ~KFileMetaInfoItem();
    /**
     * @brief Copy operator
     **/
    const KFileMetaInfoItem& operator=(const KFileMetaInfoItem& item);
    /**
     * @brief Retrieve the key of this item
     **/
    const QString& key() const;
    /**
     * @brief Retrieve the value of this item
     **/
    const QVariant& value() const;
    /**
     * @brief Is this a valid item.
     **/
    bool isValid() const;
    /**
     * @brief Returns localized name of the key
     **/
    const QString& name() const;
    /**
     * @brief Returns string suitable for displaying the value of the item
     */
    QString toString() const;
private:
    QSharedDataPointer<KFileMetaInfoItemPrivate> d;

    KFileMetaInfoItem(const QString& key, const QVariant& value);
};

#endif // KFILEMETAINFOITEM_H
