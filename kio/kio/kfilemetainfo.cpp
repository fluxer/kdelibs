/* This file is part of the KDE libraries

   Copyright (c) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
                 2007 Jos van den Oever <jos@vandenoever.info>
                 2010 Sebastian Trueg <trueg@kde.org>

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

#include "kfilemetainfo.h"
#include "kfilemetainfoitem.h"
#include "kfilemetainfoitem_p.h"
#include "kfilewriteplugin.h"
#include "kfilewriteplugin_p.h"

#ifndef KIO_NO_STRIGI
#include <strigi/bufferedstream.h>
#include <strigi/analyzerconfiguration.h>
#include <strigi/indexwriter.h>
#include <strigi/analysisresult.h>
#include <strigi/fieldtypes.h>
#endif

#include <kurl.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <QFileInfo>
#include <QDateTime>
#include <QStringList>

class KFileMetaInfoGroupPrivate : public QSharedData {
public:
    QString name;
};

KFileMetaInfoGroup::~KFileMetaInfoGroup()
{
}

KFileMetaInfoGroup::KFileMetaInfoGroup ( KFileMetaInfoGroup const& g )
{
    d = g.d;
}

QDataStream& operator >> ( QDataStream& s, KFileMetaInfo& )
{
    return s;
}

QDataStream& operator << ( QDataStream& s, const KFileMetaInfo& )
{
    return s;
}
#ifndef KIO_NO_STRIGI
/**
 * @brief Wrap a QIODevice in a Strigi stream.
 **/
class QIODeviceInputStream : public Strigi::BufferedInputStream {
private:
    QIODevice& in;
    const qint64 m_maxRead;
    qint64 m_read;
    int32_t fillBuffer ( char* start, int32_t space );
public:
    QIODeviceInputStream ( QIODevice& i, qint64 max );
};

int32_t
QIODeviceInputStream::fillBuffer ( char* start, int32_t space )
{
    if ( !in.isOpen() || !in.isReadable() )
        return -1;

    // we force a max stream read length according to the config since some Strigi
    // plugins simply ignore the value which will lead to frozen client apps
    qint64 max = m_maxRead;
    if(max < 0)
        max = space;
    else
        max = qMin(qint64(space), qMax(max-m_read,qint64(0)));

    // read into the buffer
    int32_t nwritten = in.read ( start, max );

    // check the file stream status
    if ( nwritten < 0 ) {
        m_error = "Could not read from QIODevice.";
        in.close();
        return -1;
    }
    if ( nwritten == 0 || in.atEnd() ) {
        in.close();
    }
    m_read += nwritten;
    return nwritten;
}

QIODeviceInputStream::QIODeviceInputStream ( QIODevice& i, qint64 max )
    : in ( i ),
      m_maxRead(max),
      m_read(0)
{
    // determine if we have a character device, which will likely never eof and thereby
    // potentially cause an infinite loop.
    if ( i.isSequential() ) {
        in.close(); // cause fillBuffer to return -1
    }
}

/**
 * @brief KMetaInfoWriter handles the data returned by the Strigi analyzers and
 * store it in a KFileMetaInfo.
 **/
class KMetaInfoWriter : public Strigi::IndexWriter {
public:
    // irrelevant for KFileMetaInfo
    void startAnalysis(const Strigi::AnalysisResult*) {
    }

    // irrelevant for KFileMetaInfo
    // we do not store text as metainfo
    void addText(const Strigi::AnalysisResult*, const char* /*s*/, int32_t /*n*/) {
    }
    void addValue(const Strigi::AnalysisResult* idx, const Strigi::RegisteredField* field,
            const std::string& value) {
        if (idx->writerData()) {
            const std::string name(field->key());
            static const char* orientationfield = "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation";
            if (qstrncmp(name.c_str(), orientationfield, name.size()) == 0) {
                const QByteArray intbyte(value.c_str(), value.size());
                QString orientationstring;
                // for reference:
                // https://exiv2.org/tags-xmp-tiff.html
                switch (intbyte.toInt()) {
                    case 1: {
                        orientationstring = i18n("Row at top, column at left");
                        break;
                    }
                    case 2: {
                        orientationstring = i18n("Row at top, column at right");
                        break;
                    }
                    case 3: {
                        orientationstring = i18n("Row at bottom, column at right");
                        break;
                    }
                    case 4: {
                        orientationstring = i18n("Row at bottom, column at left");
                        break;
                    }
                    case 5: {
                        orientationstring = i18n("Row at left, column at top");
                        break;
                    }
                    case 6: {
                        orientationstring = i18n("Row at right, column at top");
                        break;
                    }
                    case 7: {
                        orientationstring = i18n("Row at right, column at bottom");
                        break;
                    }
                    case 8: {
                        orientationstring = i18n("Row at left, column at bottom");
                        break;
                    }
                    default: {
                        return;
                    }
                }
                addValue(idx, field, QVariant(orientationstring));
                return;
            }

            QString val = QString::fromUtf8(value.c_str(), value.size());
            if( !val.startsWith(':') )
                addValue(idx, field, val);
        }
    }
    void addValue(const Strigi::AnalysisResult* idx, const Strigi::RegisteredField* field,
        const unsigned char* data, uint32_t size) {
        if (idx->writerData()) {
            QByteArray d((const char*)data, size);
            addValue(idx, field, QVariant(d));
        }
    }
    void addValue(const Strigi::AnalysisResult* idx, const Strigi::RegisteredField* field,
            uint32_t value) {
        if (idx->writerData()) {
            // the only duration field relevant to files
            static const char* durationfield = "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration";
            // these use different measures
            static const char* avaragebitratefield = "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate";
            static const char* frameratefield = "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#frameRate";
            static const char* sampleratefield = "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleRate";
            // datetime field
            static const char* contentcreatedfield = "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated";
            const std::string name(field->key());
            if (qstrncmp(name.c_str(), durationfield, name.size()) == 0) {
                const QString durationstring = KGlobal::locale()->prettyFormatDuration(value * 1000);
                addValue(idx, field, QVariant(durationstring));
                return;
            } else if (qstrncmp(name.c_str(), avaragebitratefield, name.size()) == 0) {
                const QString bitratestring = i18n("%1 per second").arg(KGlobal::locale()->formatByteSize(value));
                addValue(idx, field, QVariant(bitratestring));
                return;
            } else if (qstrncmp(name.c_str(), frameratefield, name.size()) == 0) {
                const QString bitratestring = i18n("%1 per second").arg(value);
                addValue(idx, field, QVariant(bitratestring));
                return;
            } else if (qstrncmp(name.c_str(), sampleratefield, name.size()) == 0) {
                const QString bitratestring = i18n("%1 kHz").arg(value / 1000);
                addValue(idx, field, QVariant(bitratestring));
                return;
            } else if (qstrncmp(name.c_str(), contentcreatedfield, name.size()) == 0) {
                const QDateTime datetime = QDateTime::fromTime_t(value);
                // NOTE: keep in sync with kdelibs/kio/kfile/kfilemetadataprovider.cpp
                const QString datestring = KGlobal::locale()->formatDateTime(datetime, KLocale::FancyLongDate);
                addValue(idx, field, QVariant(datestring));
                return;
            }

            addValue(idx, field, QVariant((quint32)value));
        }
    }
    void addValue(const Strigi::AnalysisResult* idx, const Strigi::RegisteredField* field,
            int32_t value) {
        if (idx->writerData()) {
            addValue(idx, field, QVariant((qint32)value));
        }
    }
    void addValue(const Strigi::AnalysisResult* idx, const Strigi::RegisteredField* field,
            double value) {
        if (idx->writerData()) {
            addValue(idx, field, QVariant(value));
        }
    }
    void addValue(const Strigi::AnalysisResult* idx,
                  const Strigi::RegisteredField* field, const QVariant& value) {
        QHash<QString, KFileMetaInfoItem>* info
            = static_cast<QHash<QString, KFileMetaInfoItem>*>(idx->writerData());
        if (info) {
            std::string name(field->key());
            QString key = QString::fromUtf8(name.c_str(), name.size());
            QHash<QString, KFileMetaInfoItem>::iterator i = info->find(key);
            if (i == info->end()) {
                info->insert(key, KFileMetaInfoItem(key, value, 0, true));
            } else {
                i.value().addValue(value);
            }
        }
    }
    void addValue(const Strigi::AnalysisResult* ar,
                  const Strigi::RegisteredField* field, const std::string& name,
            const std::string& value) {
        if (ar->writerData()) {
            QVariantMap m;
            m.insert ( name.c_str(), value.c_str() );
            addValue ( ar, field, m );
        }
    }

    /* irrelevant for KFileMetaInfo: These triples does not convey information
     * about this file, so we ignore it
     */
    void addTriplet ( const std::string& /*subject*/,
                      const std::string& /*predicate*/, const std::string& /*object*/ ) {
    }

    // irrelevant for KFileMetaInfo
    void finishAnalysis(const Strigi::AnalysisResult*) {}
    // irrelevant for KFileMetaInfo
    void deleteEntries(const std::vector<std::string>&) {}
    // irrelevant for KFileMetaInfo
    void deleteAllEntries() {}
};


class KFileMetaInfoPrivate : public QSharedData
{
public:
    QHash<QString, KFileMetaInfoItem> items;
    KUrl m_url;

    void init ( QIODevice& stream, const KUrl& url, time_t mtime, KFileMetaInfo::WhatFlags w );
    void initWriters ( const KUrl& /*file*/ );
    void operator= ( const KFileMetaInfoPrivate& k ) {
        items = k.items;
    }
};
static const KFileMetaInfoItem nullitem;

class KFileMetaInfoAnalysisConfiguration : public Strigi::AnalyzerConfiguration
{
public:
    KFileMetaInfoAnalysisConfiguration( KFileMetaInfo::WhatFlags indexDetail )
        : m_indexDetail(indexDetail) {
    }

    int64_t maximalStreamReadLength ( const Strigi::AnalysisResult& ar ) {
        if(ar.depth() > 0)
            return 0; // ignore all data that has a depth > 0, i.e. files in archives
        else if(m_indexDetail == KFileMetaInfo::Everything)
            return -1;
        else
            return 65536; // do not read the whole file - this is used for on-the-fly analysis
    }

    bool useFactory(Strigi::StreamAnalyzerFactory* factory) const {
        if ((m_indexDetail & KFileMetaInfo::ContentInfo) == 0 && factory) {
            if (qstrcmp(factory->name(), "CppLineAnalyzer") == 0) {
                return false;
            } else if (qstrcmp(factory->name(), "TxtLineAnalyzer") == 0) {
                return false;
            } else if (qstrcmp(factory->name(), "TextEndAnalyzer") == 0) {
                return false;
            }
        }
        return true;
    }

private:
    KFileMetaInfo::WhatFlags m_indexDetail;
};

void KFileMetaInfoPrivate::init ( QIODevice& stream, const KUrl& url, time_t mtime, KFileMetaInfo::WhatFlags w )
{
    m_url = url;

    // get data from Strigi
    KFileMetaInfoAnalysisConfiguration c( w );
    Strigi::StreamAnalyzer indexer ( c );
    KMetaInfoWriter writer;
    kDebug ( 7033 ) << url;
    Strigi::AnalysisResult idx ( url.toLocalFile().toUtf8().constData(), mtime, writer, indexer );
    idx.setWriterData ( &items );

    QIODeviceInputStream strigiStream ( stream, c.maximalStreamReadLength(idx) );
    indexer.analyze ( idx, &strigiStream );
}

void KFileMetaInfoPrivate::initWriters ( const KUrl& file )
{
    QHash<QString, KFileMetaInfoItem>::iterator i;
    for ( i = items.begin(); i != items.end(); ++i ) {
        KFileWritePlugin *w =
            KFileWriterProvider::self()->loadPlugin ( i.key() );
        if ( w && w->canWrite ( file, i.key() ) ) {
            i.value().d->writer = w;
        }
    }
}

KFileMetaInfo::KFileMetaInfo ( const QString& path, KFileMetaInfo::WhatFlags w )
        : d ( new KFileMetaInfoPrivate() )
{
    QFileInfo fileinfo ( path );
    QFile file ( path );
    // only open the file if it is a filetype Qt understands
    // if e.g. the path points to a pipe, it is not opened
    if ( ( fileinfo.isFile() || fileinfo.isDir() || fileinfo.isSymLink() )
            && file.open ( QIODevice::ReadOnly ) ) {
        KUrl u ( path );
        d->init ( file, u, fileinfo.lastModified().toTime_t(), w );
        if ( fileinfo.isWritable() ) {
            d->initWriters ( u );
        }
    }
}

KFileMetaInfo::KFileMetaInfo ( const KUrl& url, KFileMetaInfo::WhatFlags w )
        : d ( new KFileMetaInfoPrivate() )
{
    QFileInfo fileinfo ( url.toLocalFile() );
    QFile file ( url.toLocalFile() );
    if ( file.open ( QIODevice::ReadOnly ) ) {
        d->init ( file, url, fileinfo.lastModified().toTime_t(), w );
        if ( fileinfo.isWritable() ) {
            d->initWriters ( url );
        }
    }
}

KFileMetaInfo::KFileMetaInfo() : d ( new KFileMetaInfoPrivate() )
{
}

KFileMetaInfo::KFileMetaInfo ( const KFileMetaInfo& k ) : d ( k.d )
{
}

KFileMetaInfo& KFileMetaInfo::operator= ( KFileMetaInfo const & kfmi )
{
    d = kfmi.d;
    return *this;
}

KFileMetaInfo::~KFileMetaInfo()
{
}

bool KFileMetaInfo::applyChanges()
{
    // go through all editable fields and group them by writer
    QHash<KFileWritePlugin*, QVariantMap> data;
    QHashIterator<QString, KFileMetaInfoItem> i(d->items);
    while ( i.hasNext() ) {
        i.next();
        if ( i.value().isModified() && i.value().d->writer ) {
            data[i.value().d->writer][i.key() ] = i.value().value();
        }
    }

    // call the writers on the data they can write
    bool ok = true;
    QHashIterator<KFileWritePlugin*, QVariantMap> j(data);
    while ( j.hasNext() ) {
        j.next();
        ok &= j.key()->write ( d->m_url, j.value() );
    }
    return ok;
}

const KUrl& KFileMetaInfo::url() const
{
    return d->m_url;
}

const QHash<QString, KFileMetaInfoItem>& KFileMetaInfo::items() const
{
    return d->items;
}

const KFileMetaInfoItem& KFileMetaInfo::item ( const QString& key ) const
{
    QHash<QString, KFileMetaInfoItem>::const_iterator i = d->items.constFind ( key );
    return ( i == d->items.constEnd() ) ? nullitem : i.value();
}

QStringList KFileMetaInfo::keys() const
{
    return d->items.keys();
}

KFileMetaInfoItem& KFileMetaInfo::item ( const QString& key )
{
    return d->items[key];
}

bool KFileMetaInfo::isValid() const
{
    return !d->m_url.isEmpty();
}

QStringList KFileMetaInfo::preferredKeys() const
{
    return QStringList();
}

QStringList KFileMetaInfo::supportedKeys() const
{
    return QStringList();
}


#else //KIO_NO_STRIGI

class KFileMetaInfoPrivate : public QSharedData
{
public:
};

KFileMetaInfo::KFileMetaInfo ( const QString& path, KFileMetaInfo::WhatFlags w )
{
    Q_UNUSED(path);
    Q_UNUSED(w);
}

KFileMetaInfo::KFileMetaInfo ( const KUrl& url, KFileMetaInfo::WhatFlags w )
{
    Q_UNUSED(url);
    Q_UNUSED(w);
}

KFileMetaInfo::KFileMetaInfo()
{
}

KFileMetaInfo::KFileMetaInfo ( const KFileMetaInfo& k )
{
    Q_UNUSED(k);
}

KFileMetaInfo& KFileMetaInfo::operator= ( KFileMetaInfo const & kfmi )
{
    Q_UNUSED(kfmi);
    return *this;
}

KFileMetaInfo::~KFileMetaInfo()
{
}

bool KFileMetaInfo::applyChanges()
{
    return false;
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

const KFileMetaInfoItem& KFileMetaInfo::item ( const QString& key ) const
{
    Q_UNUSED(key);
    static const KFileMetaInfoItem item;
    return item;
}

QStringList KFileMetaInfo::keys() const
{
    return QStringList();
}

KFileMetaInfoItem& KFileMetaInfo::item ( const QString& key )
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
#endif

KFileMetaInfoItemList KFileMetaInfoGroup::items() const
{
    return KFileMetaInfoItemList();
}

const QString& KFileMetaInfoGroup::name() const
{
    return d->name;
}
