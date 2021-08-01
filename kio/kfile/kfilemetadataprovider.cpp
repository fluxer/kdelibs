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

#include <kfileitem.h>
#include <knfotranslator_p.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kdebug.h>

#include <QLabel>
#include <QDir>

class KFileMetaDataProvider::Private
{

public:
    Private(KFileMetaDataProvider* parent);
    ~Private();

    void readMetadata();

    void slotMetaDataUpdateDone();
    void slotLinkActivated(const QString& link);

    /**
     * Disables the metadata widget and starts the job that
     * changes the meta data asynchronously. After the job
     * has been finished, the metadata widget gets enabled again.
     */
    void startChangeDataJob(KJob* job);

    QList<QString> resourceList() const;   
    QWidget* createValueWidget(const QString& value, QWidget* parent);

    /*
     * @return The number of subdirectories for the directory \a path.
     */
    static int subDirectoriesCount(const QString &path);

    QList<KFileItem> m_fileItems;

    QList<KUrl> m_urls;
    QHash<KUrl, QVariant> m_data;

private:
    KFileMetaDataProvider* const q;
};

KFileMetaDataProvider::Private::Private(KFileMetaDataProvider* parent) :
    m_fileItems(),
    m_data(),
    q(parent)
{
}

KFileMetaDataProvider::Private::~Private()
{
}

void KFileMetaDataProvider::Private::readMetadata()
{
#warning implement multi-URL metadata support
    if (m_urls.count() > 1) {
        kWarning() << "the API does not handle multile URLs metadata";
    }
    const QString path = m_urls.first().toLocalFile();
    KFileMetaInfo metaInfo(path, KFileMetaInfo::TechnicalInfo);
    const QHash<QString, KFileMetaInfoItem> metaInfoItems = metaInfo.items();
    foreach (const KFileMetaInfoItem& metaInfoItem, metaInfoItems) {
        const QString uriString = metaInfoItem.name();
        const QVariant value = metaInfoItem.value();
        m_data.insert(uriString, value);
    }

    if (m_fileItems.count() == 1) {
        // TODO: Handle case if remote URLs are used properly. isDir() does
        // not work, the modification date needs also to be adjusted...
        const KFileItem& item = m_fileItems.first();

        if (item.isDir()) {
            const int count = subDirectoriesCount(item.url().pathOrUrl());
            if (count == -1) {
                m_data.insert(KUrl("kfileitem#size"), QString("Unknown"));
            } else {
                const QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
                m_data.insert(KUrl("kfileitem#size"), itemCountString);
            }
        } else {
            m_data.insert(KUrl("kfileitem#size"), KIO::convertSize(item.size()));
        }
        m_data.insert(KUrl("kfileitem#type"), item.mimeComment());
        m_data.insert(KUrl("kfileitem#modified"), KGlobal::locale()->formatDateTime(item.time(KFileItem::ModificationTime), KLocale::FancyLongDate));
        m_data.insert(KUrl("kfileitem#owner"), item.user());
        m_data.insert(KUrl("kfileitem#permissions"), item.permissionsString());
        m_data.insert(KUrl("kfileitem#mimetype"), item.mimetype());
    } else if (m_fileItems.count() > 1) {
        // Calculate the size of all items
        quint64 totalSize = 0;
        foreach (const KFileItem& item, m_fileItems) {
            if (!item.isDir() && !item.isLink()) {
                totalSize += item.size();
            }
        }
        m_data.insert(KUrl("kfileitem#totalSize"), KIO::convertSize(totalSize));
    }

    emit q->loadingFinished();
}

void KFileMetaDataProvider::Private::slotLinkActivated(const QString& link)
{
    emit q->urlActivated(KUrl(link));
}

void KFileMetaDataProvider::Private::startChangeDataJob(KJob* job)
{
    job->start();
}

QList<QString> KFileMetaDataProvider::Private::resourceList() const
{
    QList<QString> list;
    foreach (const KFileItem& item, m_fileItems) {
        const KUrl url = item.url();
        if(url.isValid())
            list.append(url.prettyUrl());
    }
    return list;
}

QWidget* KFileMetaDataProvider::Private::createValueWidget(const QString& value, QWidget* parent)
{
    QLabel* valueWidget = new QLabel(parent);
    valueWidget->setWordWrap(true);
    valueWidget->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    valueWidget->setTextFormat(Qt::PlainText);
    valueWidget->setText(value);
    connect(valueWidget, SIGNAL(linkActivated(QString)), q, SLOT(slotLinkActivated(QString)));
    return valueWidget;
}

KFileMetaDataProvider::KFileMetaDataProvider(QObject* parent) :
    QObject(parent),
    d(new Private(this))
{
}

KFileMetaDataProvider::~KFileMetaDataProvider()
{
    delete d;
}

void KFileMetaDataProvider::setItems(const KFileItemList& items)
{
    d->m_fileItems = items;

    if (items.isEmpty()) {
        return;
    }
    d->m_urls.clear();
    foreach (const KFileItem& item, items) {
        const KUrl url = item.url();
        if (url.isValid()) {
            d->m_urls.append(url);
        }
    }
    d->readMetadata();
}

QString KFileMetaDataProvider::label(const KUrl& metaDataUri) const
{
    return KNfoTranslator::translation(metaDataUri.url());
}

QString KFileMetaDataProvider::group(const KUrl& metaDataUri) const
{
    QString group; // return value

    const QString uri = metaDataUri.url();
    if (uri == QLatin1String("kfileitem#type")) {
        group = QLatin1String("0FileItemA");
    } else if (uri == QLatin1String("kfileitem#size")) {
        group = QLatin1String("0FileItemB");
    } else if (uri == QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width")) {
        group = QLatin1String("0SizeA");
    } else if (uri == QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height")) {
        group = QLatin1String("0SizeB");
    }

    return group;
}

KFileItemList KFileMetaDataProvider::items() const
{
    return d->m_fileItems;
}

QHash<KUrl, QVariant> KFileMetaDataProvider::data() const
{
    return d->m_data;
}

QWidget* KFileMetaDataProvider::createValueWidget(const KUrl& metaDataUri,
                                                  const QVariant& value,
                                                  QWidget* parent) const
{
    Q_ASSERT(parent != 0);
    QWidget* widget = 0;

    if (widget == 0) {
        widget = d->createValueWidget(value.toString(), parent);
    }

    widget->setForegroundRole(parent->foregroundRole());
    widget->setFont(parent->font());

    return widget;
}

int KFileMetaDataProvider::Private::subDirectoriesCount(const QString& path)
{
    QDir dir(path);
    return dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System).count();
}

#include "moc_kfilemetadataprovider_p.cpp"
