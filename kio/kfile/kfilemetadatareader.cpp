/*****************************************************************************
 * Copyright (C) 2011 by Peter Penz <peter.penz19@gmail.com>                 *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "kfilemetadatareader_p.h"

#include <kfilemetainfo.h>
#include <kdebug.h>
#include <klocale.h>

KFileMetaDataReader::KFileMetaDataReader(const QList<KUrl>& urls, QObject* parent) :
    QObject(parent)
{
    if (urls.count() > 1) {
        kWarning() << i18n("more then one URL was passed");
    }
    m_urls = urls;
}

KFileMetaDataReader::~KFileMetaDataReader()
{
}

void KFileMetaDataReader::start()
{
#warning implement multi-URL metadata support
    foreach (const KUrl& url, m_urls) {
        // Currently only the meta-data of one file is supported.
        // It might be an option to read all meta-data and show
        // ranges for each key.

        const QString path = url.toLocalFile();
        KFileMetaInfo metaInfo(path, KFileMetaInfo::Fastest);
        const QHash<QString, KFileMetaInfoItem> metaInfoItems = metaInfo.items();
        foreach (const KFileMetaInfoItem& metaInfoItem, metaInfoItems) {
            const QString uriString = metaInfoItem.name();
            const QVariant value(metaInfoItem.value());
            m_metaData.insert(uriString, value);
        }
    }
    emit finished();
}

QHash<KUrl, QVariant> KFileMetaDataReader::metaData() const
{
    return m_metaData;
}

#include "moc_kfilemetadatareader_p.cpp"
