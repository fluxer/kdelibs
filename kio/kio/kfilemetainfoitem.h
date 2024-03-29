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
#include <QString>

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
    KFileMetaInfoItem(const KFileMetaInfoItem& other);
    /**
     * @brief Constructor used by plugins
     **/
    KFileMetaInfoItem(const QString& key, const QString& value);
    /**
     * @brief Destructor
     **/
    ~KFileMetaInfoItem();
    /**
     * @brief Copy operator
     **/
    const KFileMetaInfoItem& operator=(const KFileMetaInfoItem& other);
    /**
     * @brief Retrieve the key of this item
     **/
    const QString& key() const;
    /**
     * @brief Retrieve the value of this item
     **/
    const QString& value() const;
    /**
     * @brief Returns localized name of the key
     **/
    const QString& name() const;
    /**
     * @brief Is this a valid item.
     **/
    bool isValid() const;
    /**
     * @brief Returns if item is less than other item
     **/
    bool operator<(const KFileMetaInfoItem &other) const;
private:
    QSharedDataPointer<KFileMetaInfoItemPrivate> d;
};

#endif // KFILEMETAINFOITEM_H
