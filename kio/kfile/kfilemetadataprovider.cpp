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
#include <kfilemetadatareader_p.h>
#include "knfotranslator_p.h"
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include <QEvent>
#include <QLabel>

// Required includes for subDirectoriesCount():
#ifdef Q_WS_WIN
    #include <QDir>
#else
    #include <dirent.h>
    #include <QFile>
#endif

namespace {
    static QString plainText(const QString& richText)
    {
        QString plainText;
        plainText.reserve(richText.length());

        bool skip = false;
        for (int i = 0; i < richText.length(); ++i) {
            const QChar c = richText.at(i);
            if (c == QLatin1Char('<')) {
                skip = true;
            } else if (c == QLatin1Char('>')) {
                skip = false;
            } else if (!skip) {
                plainText.append(c);
            }
        }

        return plainText;
    }
}

// The default size hint of QLabel tries to return a square size.
// This does not work well in combination with layouts that use
// heightForWidth(): In this case it is possible that the content
// of a label might get clipped. By specifying a size hint
// with a maximum width that is necessary to contain the whole text,
// using heightForWidth() assures having a non-clipped text.
class ValueWidget : public QLabel
{
public:
    explicit ValueWidget(QWidget* parent = 0);
    virtual QSize sizeHint() const;
};

ValueWidget::ValueWidget(QWidget* parent) :
    QLabel(parent)
{
}

QSize ValueWidget::sizeHint() const
{
    QFontMetrics metrics(font());
    // TODO: QLabel internally provides already a method sizeForWidth(),
    // that would be sufficient. However this method is not accessible, so
    // as workaround the tags from a richtext are removed manually here to
    // have a proper size hint.
    return metrics.size(Qt::TextSingleLine, plainText(text()));
}



class KFileMetaDataProvider::Private
{

public:
    Private(KFileMetaDataProvider* parent);
    ~Private();

    void slotLoadingFinished();

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

    QHash<KUrl, QVariant> m_data;

    QList<KFileMetaDataReader*> m_metaDataReaders;
    KFileMetaDataReader* m_latestMetaDataReader;

private:
    KFileMetaDataProvider* const q;
};

KFileMetaDataProvider::Private::Private(KFileMetaDataProvider* parent) :
    m_fileItems(),
    m_data(),
    m_metaDataReaders(),
    m_latestMetaDataReader(0),
    q(parent)
{
}

KFileMetaDataProvider::Private::~Private()
{
    qDeleteAll(m_metaDataReaders);
}

void KFileMetaDataProvider::Private::slotLoadingFinished()
{
    KFileMetaDataReader* finishedMetaDataReader = qobject_cast<KFileMetaDataReader*>(q->sender());
    // The process that has emitted the finished() signal
    // will get deleted and removed from m_metaDataReaders.
    for (int i = 0; i < m_metaDataReaders.count(); ++i) {
        KFileMetaDataReader* metaDataReader = m_metaDataReaders[i];
        if (metaDataReader == finishedMetaDataReader) {
            m_metaDataReaders.removeAt(i);
            if (metaDataReader != m_latestMetaDataReader) {
                // Ignore data of older processs, as the data got
                // obsolete by m_latestMetaDataReader.
                metaDataReader->deleteLater();
                return;
            }
        }
    }

    m_data = m_latestMetaDataReader->metaData();
    m_latestMetaDataReader->deleteLater();

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
    connect(job, SIGNAL(result(KJob*)),
            q, SIGNAL(dataChangeFinished()));
    emit q->dataChangeStarted();
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
    ValueWidget* valueWidget = new ValueWidget(parent);
    valueWidget->setWordWrap(true);
    valueWidget->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    valueWidget->setText(plainText(value));
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
    Q_PRIVATE_SLOT(d,void slotDataChangeStarted())
    Q_PRIVATE_SLOT(d,void slotDataChangeFinished())
    QList<KUrl> urls;
    foreach (const KFileItem& item, items) {
        const KUrl url = item.url();
        if (url.isValid()) {
            urls.append(url);
        }
    }

    d->m_latestMetaDataReader = new KFileMetaDataReader(urls);
    d->m_latestMetaDataReader->setReadContextData(false);
    connect(d->m_latestMetaDataReader, SIGNAL(finished()), this, SLOT(slotLoadingFinished()));
    d->m_metaDataReaders.append(d->m_latestMetaDataReader);
    d->m_latestMetaDataReader->start();
}

QString KFileMetaDataProvider::label(const KUrl& metaDataUri) const
{
    struct TranslationItem {
        const char* const key;
        const char* const context;
        const char* const value;
    };

    static const TranslationItem translations[] = {
        { "kfileitem#modified", I18N_NOOP2_NOSTRIP("@label", "Modified") },
        { "kfileitem#owner", I18N_NOOP2_NOSTRIP("@label", "Owner") },
        { "kfileitem#permissions", I18N_NOOP2_NOSTRIP("@label", "Permissions") },
        { "kfileitem#size", I18N_NOOP2_NOSTRIP("@label", "Size") },
        { "kfileitem#totalSize", I18N_NOOP2_NOSTRIP("@label", "Total Size") },
        { "kfileitem#type", I18N_NOOP2_NOSTRIP("@label", "Type") },
        { 0, 0, 0} // Mandatory last entry
    };

    static QHash<QString, QString> hash;
    if (hash.isEmpty()) {
        const TranslationItem* item = &translations[0];
        while (item->key != 0) {
            hash.insert(item->key, i18nc(item->context, item->value));
            ++item;
        }
    }

    QString value = hash.value(metaDataUri.url());
    if (value.isEmpty()) {
        value = KNfoTranslator::instance().translation(metaDataUri);
    }

    return value;
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
#ifdef Q_WS_WIN
    QDir dir(path);
    return dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System).count();
#else
    // Taken from kdelibs/kio/kio/kdirmodel.cpp
    // Copyright (C) 2006 David Faure <faure@kde.org>

    int count = -1;
    DIR* dir = ::opendir(QFile::encodeName(path));
    if (dir) {
        count = 0;
        struct dirent *dirEntry = 0;
        while ((dirEntry = ::readdir(dir))) { // krazy:exclude=syscalls
            if (dirEntry->d_name[0] == '.') {
                if (dirEntry->d_name[1] == '\0') {
                    // Skip "."
                    continue;
                }
                if (dirEntry->d_name[1] == '.' && dirEntry->d_name[2] == '\0') {
                    // Skip ".."
                    continue;
                }
            }
            ++count;
        }
        ::closedir(dir);
    }
    return count;
#endif
}

#include "moc_kfilemetadataprovider_p.cpp"
