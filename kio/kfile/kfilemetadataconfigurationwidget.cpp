/*****************************************************************************
 * Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>                      *
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

#include "kfilemetadataconfigurationwidget.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfilemetainfo.h>
#include <klocale.h>
#include "kfilemetadataprovider_p.h"

#include <QEvent>
#include <QListWidget>
#include <QVBoxLayout>

class KFileMetaDataConfigurationWidget::Private
{
public:
    Private(KFileMetaDataConfigurationWidget* parent);
    ~Private();

    void init();
    void loadMetaData();
    void addItem(const QString& uri);

    /**
     * Is invoked after the meta data model has finished the loading of
     * meta data. The meta data labels will be added to the configuration
     * list.
     */
    void slotLoadingFinished();

    KFileItemList m_fileItems;
    KFileMetaDataProvider* m_provider;
    QListWidget* m_metaDataList;

private:
    KFileMetaDataConfigurationWidget* const q;
};

KFileMetaDataConfigurationWidget::Private::Private(KFileMetaDataConfigurationWidget* parent)
    : m_fileItems(),
    m_provider(nullptr),
    m_metaDataList(nullptr),
    q(parent)
{
    m_metaDataList = new QListWidget(q);
    m_metaDataList->setSelectionMode(QAbstractItemView::NoSelection);
    m_metaDataList->setSortingEnabled(true);

    QVBoxLayout* layout = new QVBoxLayout(q);
    layout->addWidget(m_metaDataList);

    m_provider = new KFileMetaDataProvider(q);
    connect(m_provider, SIGNAL(loadingFinished()),
            q, SLOT(slotLoadingFinished()));
}

KFileMetaDataConfigurationWidget::Private::~Private()
{
}

void KFileMetaDataConfigurationWidget::Private::loadMetaData()
{
    m_provider->setItems(m_fileItems);
}

void KFileMetaDataConfigurationWidget::Private::addItem(const QString& uri)
{
    // Meta information provided by KFileMetaInfo that is already
    // available from KFileItem as "fixed item" (see above)
    // should not be shown as second entry.
    static const char* const hiddenProperties[] = {
        "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#lastModified",     // = fixed item kfileitem#modified
        "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#mimeType",         // = fixed item kfileitem#mimetype
        "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName",         // hide this property always
        0 // mandatory last entry
    };

    int i = 0;
    while (hiddenProperties[i] != 0) {
        if (uri == QLatin1String(hiddenProperties[i])) {
            // the item is hidden
            return;
        }
        ++i;
    }

    // the item is not hidden, add it to the list
    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");

    const QString label = KFileMetaInfo::name(uri);
    QListWidgetItem* item = new QListWidgetItem(label, m_metaDataList);
    item->setData(Qt::UserRole, uri);
    const bool show = settings.readEntry(uri, true);
    item->setCheckState(show ? Qt::Checked : Qt::Unchecked);
}

void KFileMetaDataConfigurationWidget::Private::slotLoadingFinished()
{
    // Get all meta information labels that are available for
    // the currently shown file item and add them to the list.
    Q_ASSERT(m_provider != 0);

    foreach (const KFileMetaInfoItem &it, m_provider->data()) {
        addItem(it.key());
    }
}

KFileMetaDataConfigurationWidget::KFileMetaDataConfigurationWidget(QWidget* parent) :
    QWidget(parent),
    d(new Private(this))
{
}

KFileMetaDataConfigurationWidget::~KFileMetaDataConfigurationWidget()
{
    delete d;
}

void KFileMetaDataConfigurationWidget::setItems(const KFileItemList& items)
{
    d->m_fileItems = items;
}

KFileItemList KFileMetaDataConfigurationWidget::items() const
{
    return d->m_fileItems;
}

void KFileMetaDataConfigurationWidget::save()
{
    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup showGroup = config.group("Show");

    const int count = d->m_metaDataList->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem* item = d->m_metaDataList->item(i);
        const bool show = (item->checkState() == Qt::Checked);
        const QString key = item->data(Qt::UserRole).toString();
        showGroup.writeEntry(key, show);
    }

    showGroup.sync();
}

bool KFileMetaDataConfigurationWidget::event(QEvent* event)
{
    if (event->type() == QEvent::Polish) {
        // loadMetaData() must be invoked asynchronously, as the list
        // must finish it's initialization first
        QMetaObject::invokeMethod(this, "loadMetaData", Qt::QueuedConnection);
    }
    return QWidget::event(event);;
}

QSize KFileMetaDataConfigurationWidget::sizeHint() const
{
    return d->m_metaDataList->sizeHint();
}


#include "moc_kfilemetadataconfigurationwidget.cpp"
