/*****************************************************************************
 * Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>                      *
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

#include "kfilemetadataprovider_p.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <QLabel>
#include <QDir>

KFileMetaDataProvider::KFileMetaDataProvider(QObject* parent)
    : QObject(parent)
{
}

KFileMetaDataProvider::~KFileMetaDataProvider()
{
}

void KFileMetaDataProvider::setItems(const KFileItemList& items)
{
    m_fileItems = items;
    m_data.clear();

    if (items.isEmpty()) {
        return;
    }

    if (m_fileItems.count() == 1) {
        // TODO: Handle case if remote URLs are used properly. isDir() does
        // not work, the modification date needs also to be adjusted...
        const KFileItem& item = m_fileItems.first();

        if (item.isDir()) {
            const int count = subDirectoriesCount(item.url().pathOrUrl());
            if (count == -1) {
                m_data.append(KFileMetaInfoItem("kfileitem#size", QString::fromLatin1("Unknown")));
            } else {
                const QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
                m_data.append(KFileMetaInfoItem("kfileitem#size", itemCountString));
            }
        } else {
            const KFileMetaInfo metaInfo(item.url(), KFileMetaInfo::TechnicalInfo);
            m_data = metaInfo.items();

            m_data.append(KFileMetaInfoItem("kfileitem#size", KIO::convertSize(item.size())));
        }
        m_data.append(KFileMetaInfoItem("kfileitem#type", item.mimeComment()));
        m_data.append(KFileMetaInfoItem("kfileitem#modified", KGlobal::locale()->formatDateTime(item.time(KFileItem::ModificationTime), KLocale::FancyLongDate)));
        m_data.append(KFileMetaInfoItem("kfileitem#owner", item.user()));
        m_data.append(KFileMetaInfoItem("kfileitem#permissions", item.permissionsString()));
        m_data.append(KFileMetaInfoItem("kfileitem#mimetype", item.mimetype()));
    } else if (m_fileItems.count() > 1) {
        // Calculate the size of all items
        quint64 totalSize = 0;
        foreach (const KFileItem& item, m_fileItems) {
            if (!item.isDir() && !item.isLink()) {
                totalSize += item.size();
            }
        }
        m_data.append(KFileMetaInfoItem("kfileitem#totalSize", KIO::convertSize(totalSize)));
    }

    qSort(m_data);

    emit loadingFinished();
}

QString KFileMetaDataProvider::label(const QString& metaDataUri) const
{
    return KFileMetaInfo::name(metaDataUri);
}

KFileItemList KFileMetaDataProvider::items() const
{
    return m_fileItems;
}

KFileMetaInfoItemList KFileMetaDataProvider::data() const
{
    return m_data;
}

QWidget* KFileMetaDataProvider::createValueWidget(const QString& metaDataUri,
                                                  const QString& value,
                                                  QWidget* parent) const
{
    Q_ASSERT(parent != 0);
    QLabel* widget = new QLabel(parent);

    widget->setWordWrap(true);
    widget->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    widget->setTextFormat(Qt::PlainText);
    widget->setText(value);
    widget->setForegroundRole(parent->foregroundRole());
    widget->setFont(parent->font());
    connect(widget, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)));

    return widget;
}

void KFileMetaDataProvider::slotLinkActivated(const QString& link)
{
    emit urlActivated(KUrl(link));
}

int KFileMetaDataProvider::subDirectoriesCount(const QString& path)
{
    QDir dir(path);
    return dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System).count();
}

#include "moc_kfilemetadataprovider_p.cpp"
