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

#ifndef KFILEMETADATAMODEL_H
#define KFILEMETADATAMODEL_H

#include <kurl.h>
#include <kfileitem.h>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QWidget>

/**
 * @brief Provides the data for the KMetaDataWidget.
 *
 * @see KFileMetaDataWidget, KFileMetaInfo
 */
class KFileMetaDataProvider : public QObject
{
    Q_OBJECT

public:
    explicit KFileMetaDataProvider(QObject* parent = 0);
    ~KFileMetaDataProvider();

    /**
     * Sets the items, where the meta data should be
     * requested. The loading of the meta data is done
     * asynchronously. The signal loadingFinished() is
     * emitted, as soon as the loading has been finished.
     * The meta data can be retrieved by
     * KFileMetaDataProvider::data() afterwards. The label for
     * each item can be retrieved by KFileMetaDataProvider::label().
     */
    void setItems(const KFileItemList& items);
    KFileItemList items() const;

    /**
     * @return Translated string for the label of the meta data represented
     *         by \p metaDataUri.
     */
    QString label(const KUrl& metaDataUri) const;

    /**
     * @return Meta data for the items that have been set by
     *         KFileMetaDataProvider::setItems(). The method should
     *         be invoked after the signal loadingFinished() has
     *         been received (otherwise no data will be returned).
     */
    QHash<KUrl, QVariant> data() const;

    /**
     * @return Factory method that returns a widget that should be used
     *         to show the meta data represented by \p metaDataUri. A
     *         QLabel will be returned.
     */
    QWidget* createValueWidget(const KUrl& metaDataUri,
                               const QVariant& value,
                               QWidget* parent) const;

Q_SIGNALS:
    /**
     * Is emitted after the loading triggered by KFileMetaDataProvider::setItems()
     * has been finished.
     */
    void loadingFinished();

    void urlActivated(const KUrl& url);

private Q_SLOTS:
    void slotLinkActivated(const QString&);

private:
    /*!
     * @return The number of subdirectories for the directory \a path.
     */
    static int subDirectoriesCount(const QString &path);

    QList<KFileItem> m_fileItems;
    QHash<KUrl, QVariant> m_data;
};

#endif // KFILEMETADATAMODEL_H
