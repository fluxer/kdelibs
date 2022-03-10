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
class KIO_EXPORT KFileMetaInfo {
public:
    /**
     * This is used to specify what a KFileMetaInfo object should read, so
     * you can specify if you want to read "expensive" items or not.
     * This is like a preset which can be customized by passing additional
     * parameters to constructors.
     */
    enum What {
        TechnicalInfo = 0x1,   /** extract technical details about the file, like
                                 e.g. play time, resolution or a compression type */
        ContentInfo = 0x2,     /** read information about the content of the file
                                 like comments or id3 tags */
        ExternalSources = 0x4, /** read external metadata sources such as
                                 filesystem based extended attributes if
                                 they are supported for the filesystem;
                                 RDF storages etc */
        Everything = TechnicalInfo | ContentInfo | ExternalSources // read everything, even if it might take a while

    };
    Q_DECLARE_FLAGS(WhatFlags, What)

    /**
     * @brief Construct a KFileMetaInfo that contains metainformation about
     * the resource pointed to by @p path.
     *
     * When w is not Everything, a limit of 64kbytes is imposed on the file size.
     * @note The path must be full (start with a slash) or relative, e.g.
     * "/home/joe/pic.png" or "../joe/pic.png". If it starts with a URL scheme,
     * e.g. "file:///home/joe/pic.png" it will be considered invalid. If it may
     * start with URL scheme use the constructor that takes KUrl.
     **/
    explicit KFileMetaInfo(const QString& path, WhatFlags w = Everything);
    /**
     * @brief Construct a KFileMetaInfo that contains metainformation about
     * the resource pointed to by @p url.
     * @note that c'tor is not thread-safe
     **/
    KFileMetaInfo(const KUrl& url, WhatFlags w = Everything);
    /**
     * @brief Construct an empty, invalid KFileMetaInfo instance.
     **/
    KFileMetaInfo();
    /**
     * @brief Construct a KFileMetaInfo instance from another one.
     **/
    KFileMetaInfo(const KFileMetaInfo& kfmi);
    /**
     * @brief Destructor.
     **/
    ~KFileMetaInfo();
    /**
     * @brief Copy a KFileMetaInfo instance from another one.
     **/
    KFileMetaInfo& operator=(KFileMetaInfo const& kfmi);
    /**
     * @brief Retrieve all the items.
     **/
    const KFileMetaInfoItemList& items() const;
    KFileMetaInfoItem& item(const QString& key);
    const KFileMetaInfoItem& item(const QString& key) const;
    bool isValid() const;
    QStringList preferredKeys() const;
    QStringList supportedKeys() const;
    QStringList keys() const;
    const KUrl& url() const;

private:
    QSharedDataPointer<KFileMetaInfoPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KFileMetaInfo::WhatFlags)

#endif // KFILEMETAINFO_H
