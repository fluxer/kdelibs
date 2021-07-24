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

#include <config-kio.h>

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
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#depends",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#plainTextContent",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#description",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#hasTag",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#lastModified",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#numericRating",
            "kfileitem#owner",
            "kfileitem#permissions",
            "kfileitem#mimetype",
            0 // mandatory last entry
        };

        static const char* disabledProperties[] = {
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#isPartOf",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#lastModified",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentSize",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#mimeType",
            "http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
            0 // mandatory last entry
        };

        for (int i = 0; enabledProperties[i] != 0; ++i) {
            settings.writeEntry(enabledProperties[i], true);
        }

        for (int i = 0; disabledProperties[i] != 0; ++i) {
            settings.writeEntry(disabledProperties[i], false);
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

    QHash<KUrl, QVariant> data = m_provider->data();

    // Remove all items, that are marked as hidden in kmetainformationrc
    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");
    QHash<KUrl, QVariant>::iterator it = data.begin();
    while (it != data.end()) {
        const QString uriString = it.key().url();
        if (!settings.readEntry(uriString, true)) {
            it = data.erase(it);
        } else {
            ++it;
        }
    }

    // Iterate through all remaining items embed the label
    // and the value as new row in the widget
    int rowIndex = 0;
    const QList<KUrl> keys = data.keys();
    foreach (const KUrl& key, keys) {
        const QVariant value = data.value(key);
        if (value.toString().isEmpty()) {
            continue;
        }
        QString itemLabel = m_provider->label(key);
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

    // Calculate the required width for the labels and values
    int leftWidthMax = 0;
    int rightWidthMax = 0;
    foreach (const Private::Row& row, d->m_rows) {
        const int rightWidth = row.value->sizeHint().width();
        if (rightWidth > rightWidthMax) {
            rightWidthMax = rightWidth;
        }

        const int leftWidth = row.label->sizeHint().width();
        if (leftWidth > leftWidthMax) {
            leftWidthMax = leftWidth;
        }
    }

    // Based on the available width calculate the required height
    int height = d->m_gridLayout->margin() * 2 + d->m_gridLayout->spacing() * (d->m_rows.count() - 1);
    foreach (const Private::Row& row, d->m_rows) {
        const int rowHeight = qMax(row.label->heightForWidth(leftWidthMax),
                                   row.value->heightForWidth(rightWidthMax));
        height += rowHeight;
    }

    const int width = d->m_gridLayout->margin() * 2 + leftWidthMax +
                      d->m_gridLayout->spacing() + rightWidthMax;

    return QSize(width, height);
}

#include "moc_kfilemetadatawidget.cpp"
