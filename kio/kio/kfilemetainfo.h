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

#ifndef KFILEMETAINFO_H
#define KFILEMETAINFO_H

#include "kfilemetainfoitem.h"

class KUrl;

typedef QList<KFileMetaInfoItem> KFileMetaInfoItemList;

class KFileMetaInfoPrivate;
/**
 * KFileMetaInfo provides metadata extracted from a file or other resource.
 *
 * When instantiating an instance of this class, the metadata related to it
 * will be retrieved and stored in the instance. The data can be inspected
 * through KFileMetaInfoItem objects.
 **/
class KIO_EXPORT KFileMetaInfo
{
public:
    /**
     * @brief Construct a KFileMetaInfo that contains metainformation about
     * the resource pointed to by @p path.
     *
     * @note The path must be full (start with a slash) or relative, e.g.
     * "/home/joe/pic.png" or "../joe/pic.png". If it starts with a URL scheme,
     * e.g. "file:///home/joe/pic.png" it will be considered invalid. If it may
     * start with URL scheme use the constructor that takes KUrl.
     **/
    explicit KFileMetaInfo(const QString &path);
    /**
     * @brief Construct a KFileMetaInfo that contains metainformation about
     * the resource pointed to by @p url.
     * @note that c'tor is not thread-safe
     **/
    KFileMetaInfo(const KUrl &url);
    /**
     * @brief Construct an empty, invalid KFileMetaInfo instance.
     **/
    KFileMetaInfo();
    /**
     * @brief Construct a KFileMetaInfo instance from another one.
     **/
    KFileMetaInfo(const KFileMetaInfo &kfmi);
    /**
     * @brief Destructor.
     **/
    ~KFileMetaInfo();
    /**
     * @brief Copy a KFileMetaInfo instance from another one.
     **/
    KFileMetaInfo& operator=(const KFileMetaInfo &kfmi);

    /**
     * @brief Retrieve all the items.
     **/
    const KFileMetaInfoItemList& items() const;
    KFileMetaInfoItem& item(const QString &key);
    const KFileMetaInfoItem& item(const QString &key) const;
    bool isValid() const;
    QStringList preferredKeys() const;
    QStringList keys() const;
    const KUrl& url() const;

    /**
     * @brief Returns all supported keys
     **/
    static QStringList supportedKeys();
    /**
     * @brief Returns localized name of @p key
     **/
    static QString name(const QString &key);
private:
    QSharedDataPointer<KFileMetaInfoPrivate> d;
};

#endif // KFILEMETAINFO_H
