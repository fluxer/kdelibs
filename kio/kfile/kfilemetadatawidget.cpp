/*****************************************************************************
 * Copyright (C) 2008-2010 by Sebastian Trueg <trueg@kde.org>                *
 * Copyright (C) 2009-2010 by Peter Penz <peter.penz@gmx.at>                 *
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

#include "kfilemetadatawidget.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfileitem.h>
#include <klocale.h>
#include "kfilemetadataprovider_p.h"

#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QSpacerItem>

class KFileMetaDataWidget::Private
{
public:
    struct Row
    {
        QLabel* label;
        QWidget* value;
    };

    Private(KFileMetaDataWidget* parent);
    ~Private();

    /**
     * Initializes the configuration file "kmetainformationrc"
     * with proper default settings for the first start in
     * an uninitialized environment.
     */
    void initMetaInfoSettings();

    /**
     * Parses the configuration file "kmetainformationrc" and
     * updates the visibility of all rows that got their data
     * from KFileItem.
     */
    void updateFileItemRowsVisibility();

    void deleteRows();

    void slotLoadingFinished();
    void slotLinkActivated(const QString& link);
    void slotDataChangeStarted();
    void slotDataChangeFinished();

    QList<Row> m_rows;
    KFileMetaDataProvider* m_provider;
    QGridLayout* m_gridLayout;

private:
    KFileMetaDataWidget* const q;
};

KFileMetaDataWidget::Private::Private(KFileMetaDataWidget* parent) :
    m_rows(),
    m_provider(0),
    m_gridLayout(0),
    q(parent)
{
    initMetaInfoSettings();

    // TODO: If KFileMetaDataProvider might get a public class in future KDE releases,
    // the following code should be moved into KFileMetaDataWidget::setModel():
    m_provider = new KFileMetaDataProvider(q);
    connect(m_provider, SIGNAL(loadingFinished()), q, SLOT(slotLoadingFinished()));
    connect(m_provider, SIGNAL(urlActivated(KUrl)), q, SIGNAL(urlActivated(KUrl)));
}

KFileMetaDataWidget::Private::~Private()
{
    deleteRows();
}

void KFileMetaDataWidget::Private::initMetaInfoSettings()
{
    // increase version, if the blacklist of disabled properties should be updated
    static const int currentVersion = 7;

    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    if (config.group("Misc").readEntry("version", 0) < currentVersion) {
        // The resource file is read the first time. Assure
        // that some meta information is enabled per default.

        // clear old info
        config.deleteGroup("Show");
        KConfigGroup settings = config.group("Show");

        static const char* enabledProperties[] = {
            "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title",
            "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber",
            "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitDepth",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitRate",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitDepth",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#beatsPerMinute",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#codec",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoCodec",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioCodec",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encoder",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#lastModified",
            "kfileitem#owner",
            "kfileitem#permissions",
            "kfileitem#mimetype",
            0 // mandatory last entry
        };

        foreach (const QString &key, KFileMetaInfo::supportedKeys()) {
            settings.writeEntry(key, false);
        }

        for (int i = 0; enabledProperties[i] != 0; ++i) {
            settings.writeEntry(enabledProperties[i], true);
        }

        // mark the group as initialized
        config.group("Misc").writeEntry("version", currentVersion);
    }
}

void KFileMetaDataWidget::Private::deleteRows()
{
    foreach (const Row& row, m_rows) {
        delete row.label;
        delete row.value;
    }
    m_rows.clear();
}

void KFileMetaDataWidget::Private::slotLoadingFinished()
{
    deleteRows();

    if (m_gridLayout == 0) {
        m_gridLayout = new QGridLayout(q);
        m_gridLayout->setMargin(0);
        m_gridLayout->setSpacing(q->fontMetrics().height() / 4);
    }

    KFileMetaInfoItemList data = m_provider->data();

    // Remove all items, that are marked as hidden in kmetainformationrc
    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");
    KFileMetaInfoItemList::iterator it = data.begin();
    while (it != data.end()) {
        if (!settings.readEntry(it->key(), true)) {
            it = data.erase(it);
        } else {
            ++it;
        }
    }

    // Iterate through all remaining items embed the label
    // and the value as new row in the widget
    int rowIndex = 0;
    foreach (const KFileMetaInfoItem& item, data) {
        const QString key = item.key();
        const QString value = item.value();
        if (value.isEmpty()) {
            continue;
        }
        QString itemLabel = KFileMetaInfo::name(key);
        itemLabel.append(QLatin1Char(':'));

        // Create label
        QLabel* label = new QLabel(itemLabel, q);
        label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        label->setForegroundRole(q->foregroundRole());
        label->setFont(q->font());
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignTop | Qt::AlignRight);

        // Create value-widget
        QWidget* valueWidget = m_provider->createValueWidget(key, value, q);

        // Add the label and value-widget to grid layout
        m_gridLayout->addWidget(label, rowIndex, 0, Qt::AlignRight);
        m_gridLayout->addWidget(valueWidget, rowIndex, 1, Qt::AlignLeft);

        // Remember the label and value-widget as row
        Row row;
        row.label = label;
        row.value = valueWidget;
        m_rows.append(row);

        ++rowIndex;
    }

    q->updateGeometry();
    emit q->metaDataRequestFinished(m_provider->items());
}

void KFileMetaDataWidget::Private::slotLinkActivated(const QString& link)
{
    const KUrl url(link);
    if (url.isValid()) {
        emit q->urlActivated(url);
    }
}

void KFileMetaDataWidget::Private::slotDataChangeStarted()
{
    q->setEnabled(false);
}

void KFileMetaDataWidget::Private::slotDataChangeFinished()
{
    q->setEnabled(true);
}

KFileMetaDataWidget::KFileMetaDataWidget(QWidget* parent) :
    QWidget(parent),
    d(new Private(this))
{
}

KFileMetaDataWidget::~KFileMetaDataWidget()
{
    delete d;
}

void KFileMetaDataWidget::setItems(const KFileItemList& items)
{
    d->m_provider->setItems(items);
}

KFileItemList KFileMetaDataWidget::items() const
{
    return d->m_provider->items();
}

QSize KFileMetaDataWidget::sizeHint() const
{
    if (d->m_gridLayout == 0) {
        return QWidget::sizeHint();
    }

    return d->m_gridLayout->sizeHint();
}

#include "moc_kfilemetadatawidget.cpp"
