/* This file is part of the KDE libraries

   Copyright (c) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
                 2007 Jos van den Oever <jos@vandenoever.info>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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

#include "predicateproperties.h"
#include "kfilemetainfoitem.h"

class KUrl;

typedef QList<KFileMetaInfoItem> KFileMetaInfoItemList;

class KFileMetaInfoGroupPrivate;
class KIO_EXPORT KFileMetaInfoGroup {
public:
    KFileMetaInfoGroup();
    KFileMetaInfoGroup(const KFileMetaInfoGroup&);
    ~KFileMetaInfoGroup();
    const KFileMetaInfoGroup& operator=(const KFileMetaInfoGroup&);
    KFileMetaInfoItemList items() const;
    const QString& name() const;
    const QStringList& keys() const;
private:
    QSharedDataPointer<KFileMetaInfoGroupPrivate> d;
};

typedef QList<KFileMetaInfoGroup> KFileMetaInfoGroupList;

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
    enum What
    {
      Fastest       = 0x1,  /**< do the fastest possible read and omit all items
                                 that might need a significantly longer time
                                 than the others */
      TechnicalInfo = 0x2,  /**< extract technical details about the file, like
                                 e.g. play time, resolution or a compressioni
                                 type */
      ContentInfo   = 0x4,  /**< read information about the content of the file
                                 like comments or id3 tags */
      ExternalSources = 0x8, /**<read external metadata sources such as
                                 filesystem based extended attributes if
                                 they are supported for the filesystem;
                                 RDF storages etc */
      Thumbnail     = 0x10, /**< only read the file's thumbnail, if it contains
                                 one */
      LinkedData    = 0x40, //< extract linked/related files like html links, source #include etc
      Everything    = 0xffff ///< read everything, even if it might take a while

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
    KFileMetaInfo(const KUrl& url);
    /**
     * @brief Construct an empty, invalid KFileMetaInfo instance.
     **/
    KFileMetaInfo();
    /**
     * @brief Construct a KFileMetaInfo instance from another one.
     **/
    KFileMetaInfo(const KFileMetaInfo&);
    /**
     * @brief Destructor.
     **/
    ~KFileMetaInfo();
    /**
     * @brief Copy a KFileMetaInfo instance from another one.
     **/
    KFileMetaInfo& operator=(KFileMetaInfo const& kfmi);
    /**
     * @brief Save the changes made to this KFileMetaInfo instance.
     */
    bool applyChanges();
    /**
     * @brief Retrieve all the items.
     **/
    const QHash<QString, KFileMetaInfoItem>& items() const;
    KFileMetaInfoItem& item(const QString& key);
    const KFileMetaInfoItem& item(const QString& key) const;
    bool isValid() const;
    QStringList preferredKeys() const;
    QStringList supportedKeys() const;
    KIO_EXPORT friend QDataStream& operator >>(QDataStream& s, KFileMetaInfo& )
;
    KIO_EXPORT friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfo&);
    KFileMetaInfoGroupList groups() const;
    QStringList keys() const;
    const KUrl& url() const;

private:
    QSharedDataPointer<KFileMetaInfoPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KFileMetaInfo::WhatFlags)


#endif
