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

#include "kfilemetainfo.h"
#include "kfilemetainfoitem.h"
#include "kfilemetainfoitem_p.h"

#ifndef KIO_NO_LIBEXTRACTOR
#include <extractor.h>
#endif

#include <kurl.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <QFileInfo>
#include <QDateTime>
#include <QStringList>

QDataStream& operator>>(QDataStream& s, KFileMetaInfo& kfmi)
{
    return s;
}

QDataStream& operator<<(QDataStream& s, const KFileMetaInfo& kfmi)
{
    return s;
}

#ifndef KIO_NO_LIBEXTRACTOR
static const KFileMetaInfoItem nullitem;

class KFileMetaInfoPrivate : public QSharedData
{
public:
    QHash<QString, KFileMetaInfoItem> items;
    KUrl m_url;

    void init(const QByteArray &filepath, const KUrl& url, KFileMetaInfo::WhatFlags w);
    void operator=(const KFileMetaInfoPrivate& kfmip) {
        items = kfmip.items;
    }

    static QVariant variant(enum EXTRACTOR_MetaFormat format,
                            const char *data, size_t data_len);
    static int metadata(void *cls,
                        const char *plugin_name,
                        enum EXTRACTOR_MetaType type,
                        enum EXTRACTOR_MetaFormat format,
                        const char *data_mime_type,
                        const char *data,
                        size_t data_len);
};

QVariant KFileMetaInfoPrivate::variant(enum EXTRACTOR_MetaFormat format,
                                       const char *data, size_t data_len)
{
    switch (format) {
        case EXTRACTOR_METAFORMAT_UTF8: {
            return QVariant(QString::fromUtf8(data, data_len - 1));
        }
        case EXTRACTOR_METAFORMAT_C_STRING: {
            return QVariant(QString::fromLocal8Bit(data, data_len - 1));
        }
        case EXTRACTOR_METAFORMAT_BINARY: {
            return QVariant();
        }
        case EXTRACTOR_METAFORMAT_UNKNOWN:
        default: {
            kWarning() << "Unknown metadata format";
            return QVariant();
        }
    }
    Q_UNREACHABLE();
    return QVariant();
}

int KFileMetaInfoPrivate::metadata(void *cls,
                                   const char *plugin_name,
                                   enum EXTRACTOR_MetaType type,
                                   enum EXTRACTOR_MetaFormat format,
                                   const char *data_mime_type,
                                   const char *data,
                                   size_t data_len)
{
    // qDebug() << Q_FUNC_INFO << type << format << data_mime_type << data_len;
    KFileMetaInfoPrivate* kfmip = static_cast<KFileMetaInfoPrivate*>(cls);
    switch (type) {
        case EXTRACTOR_METATYPE_MIMETYPE: {
            const KFileMetaInfoItem kfmi("kfileitem#mimetype", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("kfileitem#mimetype", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_FILENAME: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName", kfmi);
            break;
        }
        case EXTRACTOR_METATYPE_COMMENT: {
            const KFileMetaInfoItem kfmi("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment", KFileMetaInfoPrivate::variant(format, data, data_len));
            kfmip->items.insert("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment", kfmi);
            break;
        }
        // TODO: handle more cases
    }
    return 0;
}

void KFileMetaInfoPrivate::init(const QByteArray &filename, const KUrl& url, KFileMetaInfo::WhatFlags w)
{
    m_url = url;
    struct EXTRACTOR_PluginList *extractorplugins = EXTRACTOR_plugin_add_defaults(EXTRACTOR_OPTION_DEFAULT_POLICY);
    // not interested in the metadata provided by these plugins
    extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "archive");
    extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "zip");
    extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "deb");
    extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "rpm");
    extractorplugins = EXTRACTOR_plugin_remove(extractorplugins, "thumbnailgtk");
    // TODO: filter more based on flags
    EXTRACTOR_extract(
        extractorplugins,
        filename.constData(),
        NULL, //data
        0, // size
        KFileMetaInfoPrivate::metadata, this
    );
    EXTRACTOR_plugin_remove_all(extractorplugins);
}

KFileMetaInfo::KFileMetaInfo(const QString& path, KFileMetaInfo::WhatFlags w)
    : d(new KFileMetaInfoPrivate())
{
    QFileInfo fileinfo(path);
    // only open the file if it is not a pipe
    if (fileinfo.isFile() || fileinfo.isDir() || fileinfo.isSymLink()) {
        d->init(path.toLocal8Bit(), KUrl(path), w);
    }
}

KFileMetaInfo::KFileMetaInfo(const KUrl& url, KFileMetaInfo::WhatFlags w)
    : d(new KFileMetaInfoPrivate())
{
    const QString path = url.toLocalFile();
    d->init(path.toLocal8Bit(), url, w);
}

KFileMetaInfo::KFileMetaInfo()
    : d(new KFileMetaInfoPrivate())
{
}

KFileMetaInfo::KFileMetaInfo(const KFileMetaInfo& kfmi)
    : d(kfmi.d)
{
}

KFileMetaInfo& KFileMetaInfo::operator=(KFileMetaInfo const& kfmi)
{
    d = kfmi.d;
    return *this;
}

KFileMetaInfo::~KFileMetaInfo()
{
}

const KUrl& KFileMetaInfo::url() const
{
    return d->m_url;
}

const QHash<QString, KFileMetaInfoItem>& KFileMetaInfo::items() const
{
    return d->items;
}

const KFileMetaInfoItem& KFileMetaInfo::item(const QString& key) const
{
    QHash<QString, KFileMetaInfoItem>::const_iterator i = d->items.constFind(key);
    return (i == d->items.constEnd()) ? nullitem : i.value();
}

QStringList KFileMetaInfo::keys() const
{
    return d->items.keys();
}

KFileMetaInfoItem& KFileMetaInfo::item(const QString& key)
{
    return d->items[key];
}

bool KFileMetaInfo::isValid() const
{
    return !d->m_url.isEmpty();
}

QStringList KFileMetaInfo::preferredKeys() const
{
#warning TODO: implement
    return QStringList();
}

QStringList KFileMetaInfo::supportedKeys() const
{
#warning TODO: implement properly
    return keys();
}

#else //KIO_NO_LIBEXTRACTOR

class KFileMetaInfoPrivate : public QSharedData
{
public:
};

KFileMetaInfo::KFileMetaInfo(const QString& path, KFileMetaInfo::WhatFlags w)
{
    Q_UNUSED(path);
    Q_UNUSED(w);
}

KFileMetaInfo::KFileMetaInfo(const KUrl& url, KFileMetaInfo::WhatFlags w)
{
    Q_UNUSED(url);
    Q_UNUSED(w);
}

KFileMetaInfo::KFileMetaInfo()
{
}

KFileMetaInfo::KFileMetaInfo(const KFileMetaInfo& kfmi)
{
    Q_UNUSED(kfmi);
}

KFileMetaInfo& KFileMetaInfo::operator=(KFileMetaInfo const& kfmi)
{
    Q_UNUSED(kfmi);
    return *this;
}

KFileMetaInfo::~KFileMetaInfo()
{
}

const KUrl& KFileMetaInfo::url() const
{
    static const KUrl item;
    return item;
}

const QHash<QString, KFileMetaInfoItem>& KFileMetaInfo::items() const
{
    static const QHash<QString, KFileMetaInfoItem> items;
    return items;
}

const KFileMetaInfoItem& KFileMetaInfo::item(const QString& key) const
{
    Q_UNUSED(key);
    static const KFileMetaInfoItem item;
    return item;
}

QStringList KFileMetaInfo::keys() const
{
    return QStringList();
}

KFileMetaInfoItem& KFileMetaInfo::item(const QString& key)
{
    Q_UNUSED(key);
    static KFileMetaInfoItem item;
    return item;
}

bool KFileMetaInfo::isValid() const
{
    return false;
}

QStringList KFileMetaInfo::preferredKeys() const
{
    return QStringList();
}

QStringList KFileMetaInfo::supportedKeys() const
{
    return QStringList();
}
#endif // KIO_NO_LIBEXTRACTOR
