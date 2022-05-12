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
#include "kfilemetadata.h"

#include <kurl.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kmimetype.h>

#include <QFileInfo>
#include <QStringList>

static const KFileMetaInfoItem nullitem;
static KFileMetaInfoItem mutablenullitem;

class KFileMetaInfoPrivate : public QSharedData
{
public:
    KFileMetaInfoItemList items;
    KUrl m_url;

    void init(const QString &filename, const KUrl& url, KFileMetaInfo::WhatFlags w);
    void operator=(const KFileMetaInfoPrivate& kfmip) {
        items = kfmip.items;
        m_url = kfmip.m_url;
    }
};

void KFileMetaInfoPrivate::init(const QString &filename, const KUrl &url, KFileMetaInfo::WhatFlags w)
{
    m_url = url;

    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup pluginsgroup = config.group("Plugins");
    const KMimeType::Ptr filemimetype = KMimeType::findByUrl(url);
    const KService::List kfmdplugins = KServiceTypeTrader::self()->query("KFileMetaData/Plugin");
    foreach (const KService::Ptr &kfmdplugin, kfmdplugins) {
        const QString key = kfmdplugin->desktopEntryName();
        const bool enable = pluginsgroup.readEntry(key, true);
        if (enable) {
            KFileMetaDataPlugin *kfmdplugininstance = kfmdplugin->createInstance<KFileMetaDataPlugin>();
            if (kfmdplugininstance) {
                // qDebug() << Q_FUNC_INFO << filemimetype->name() << kfmdplugininstance->mimeTypes();
                foreach (const QString &kfmdpluginmime, kfmdplugininstance->mimeTypes()) {
                    if (filemimetype->is(kfmdpluginmime)) {
                        items.append(kfmdplugininstance->metaData(url, w));
                        break;
                    }
                }
                delete kfmdplugininstance;
            } else {
                kWarning() << "Could not create KFileMetaDataPlugin instance";
            }
        }
    }

    // remove duplicates. first comes, first serves
    KFileMetaInfoItemList::iterator it = items.begin();
    QStringList itemkeys;
    itemkeys.reserve(items.size());
    while (it != items.end()) {
        if (!itemkeys.contains(it->key())) {
            itemkeys.append(it->key());
            it++;
        } else {
            kDebug() << "Multiple entries for the same key" << it->key();
            it = items.erase(it);
        }
    }

    // for compatibility
    const KFileMetaInfoItem kfmi(
        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName"),
        filename
    );
    items.append(kfmi);
    const QString kfmiurl = url.prettyUrl();
    const KFileMetaInfoItem kfmi2(
        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url"),
        kfmiurl
    );
    items.append(kfmi2);
}

KFileMetaInfo::KFileMetaInfo(const QString& path, KFileMetaInfo::WhatFlags w)
    : d(new KFileMetaInfoPrivate())
{
    QFileInfo fileinfo(path);
    // only open the file if it is not a pipe
    if (fileinfo.isFile() || fileinfo.isDir() || fileinfo.isSymLink()) {
        d->init(fileinfo.fileName(), KUrl(path), w);
    }
}

KFileMetaInfo::KFileMetaInfo(const KUrl& url, KFileMetaInfo::WhatFlags w)
    : d(new KFileMetaInfoPrivate())
{
    const QString filename = QFileInfo(url.toLocalFile()).fileName();
    d->init(filename, url, w);
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

const KFileMetaInfoItemList& KFileMetaInfo::items() const
{
    return d->items;
}

const KFileMetaInfoItem& KFileMetaInfo::item(const QString& key) const
{
    for (int i = 0; i < d->items.size(); i++) {
        if (d->items.at(i).key() == key) {
            return d->items.at(i);
        }
    }
    return nullitem;
}

QStringList KFileMetaInfo::keys() const
{
    QStringList result;
    result.reserve(d->items.size());
    for (int i = 0; i < d->items.size(); i++) {
        result.append(d->items.at(i).key());
    }
    return result;
}

KFileMetaInfoItem& KFileMetaInfo::item(const QString& key)
{
    for (int i = 0; i < d->items.size(); i++) {
        if (d->items.at(i).key() == key) {
            return d->items[i];
        }
    }
    return mutablenullitem;
}

bool KFileMetaInfo::isValid() const
{
    return !d->m_url.isEmpty();
}

QStringList KFileMetaInfo::preferredKeys() const
{
    QStringList result;
    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup showgroup = config.group("Show");
    foreach (const QString &key, supportedKeys()) {
        const bool show = showgroup.readEntry(key, true);
        if (show) {
            result.append(key);
        }
    }
    return result;
}

QStringList KFileMetaInfo::supportedKeys()
{
    QStringList keys = QStringList()
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url");

    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup pluginsgroup = config.group("Plugins");
    const KService::List kfmdplugins = KServiceTypeTrader::self()->query("KFileMetaData/Plugin");
    foreach (const KService::Ptr &kfmdplugin, kfmdplugins) {
        const QString key = kfmdplugin->desktopEntryName();
        const bool enable = pluginsgroup.readEntry(key, true);
        if (enable) {
            KFileMetaDataPlugin *kfmdplugininstance = kfmdplugin->createInstance<KFileMetaDataPlugin>();
            if (kfmdplugininstance) {
                keys.append(kfmdplugininstance->keys());
                delete kfmdplugininstance;
            } else {
                kWarning() << "Could not create KFileMetaDataPlugin instance";
            }
        }
    }
    keys.removeDuplicates();
    qSort(keys);
    
    return keys;
}

QString KFileMetaInfo::name(const QString& key)
{
    I18N_NOOP2("kfilemetadata", "%1 kb/s");
    I18N_NOOP2("kfilemetadata", "%1 Hz");

    typedef std::map<QString,QString> TranslationMap;

    static const TranslationMap s_translations = {
        { "kfileitem#modified", i18nc("@label", "Modified") },
        { "kfileitem#owner", i18nc("@label", "Owner") },
        { "kfileitem#permissions", i18nc("@label", "Permissions") },
        { "kfileitem#size", i18nc("@label", "Size") },
        { "kfileitem#totalSize", i18nc("@label", "Total Size") },
        { "kfileitem#type", i18nc("@label", "Type") },
        { "kfileitem#mimetype", i18nc("@label", "MIME Type") },
        // implemented so far
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName", i18nc("@label", "Filename") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url", i18nc("@label file URL", "URL") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width", i18nc("@label", "Width") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height", i18nc("@label", "Height") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate", i18nc("@label", "Average Bitrate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleRate", i18nc("@label", "Sample Rate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels", i18nc("@label", "Channels") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration", i18nc("@label", "Duration") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#frameRate", i18nc("@label", "Frame Rate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoCodec",  i18nc("@label", "Video Codec") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioCodec",  i18nc("@label", "Audio Codec") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#subtitleCodec",  i18nc("@label", "Subtitle Codec") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitRate", i18nc("@label", "Audio Bit Rate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitRate", i18nc("@label", "Video Bit Rate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount", i18nc("@label", "Page Count") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright", i18nc("@label", "Copyright") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment", i18nc("@label", "Comment") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title", i18nc("@label music title", "Title") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword", i18nc("@label", "Keyword") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description", i18nc("@label", "Description") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator", i18nc("@label Software used to generate content", "Generator") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentLastModified", i18nc("@label modification date", "Modified") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated", i18nc("@label creation date", "Created") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject", i18nc("@label", "Subject") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum", i18nc("@label music album", "Album") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre",  i18nc("@label music genre", "Genre") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer", i18nc("@label", "Performer") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber", i18nc("@label music track number", "Track") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#composer", i18nc("@label music composer", "Composer") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encoder", i18nc("@label", "Encoder") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encodedBy", i18nc("@label", "Encoded By") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalReleaseYear", i18nc("@label", "Original Release Year") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#uniqueFileIdentifier", i18nc("@label", "URI") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#textWriter", i18nc("@label", "Text Writer") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher", i18nc("@label", "Publisher") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator", i18nc("@label", "Creator") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contributor", i18nc("@label", "Contributor") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make", i18nc("@label EXIF", "Manufacturer") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model", i18nc("@label EXIF", "Model") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation", i18nc("@label EXIF", "Orientation") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist", i18nc("@label music or image artist", "Artist") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef", i18nc("@label", "GPS Latitude Reference") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef", i18nc("@label", "GPS Longitude Reference") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash", i18nc("@label EXIF", "Flash") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime", i18nc("@label EXIF", "Exposure Time") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue", i18nc("@label EXIF", "Exposure Bias Value") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode", i18nc("@label EXIF", "Exposure Mode") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue", i18nc("@label EXIF aperture value", "Aperture") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength", i18nc("@label EXIF", "Focal Length") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm", i18nc("@label EXIF", "Focal Length 35 mm") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings", i18nc("@label EXIF", "ISO Speed Ratings") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode", i18nc("@label EXIF", "Metering Mode") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance", i18nc("@label EXIF", "White Balance") },
        // to be used by plugins
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Email", i18nc("@label", "Creator E-Mail") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", i18nc("@label", "Hash Value") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", i18nc("@label", "Hash Algorithm") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#country", i18nc("@label", "Country") },
        { "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#summary", i18nc("@label", "Summary") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#characterSet", i18nc("@label", "Character Set") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#lineCount", i18nc("@label number of lines", "Lines") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#wordCount", i18nc("@label number of words", "Words") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#characterCount", i18nc("@label", "Character Count") },
        { "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#sequence", i18nc("@label", "Revision") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact", i18nc("@label", "Contact") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#disclaimer", i18nc("@label", "Disclaimer") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#director", i18nc("@label video director", "Director") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#conductor", i18nc("@label", "Conductor") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#interpretedBy", i18nc("@label", "Interpreted By") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#beatsPerMinute", i18nc("@label", "Beats Per Minute") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalArtist", i18nc("@label", "Original Artist") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalTextWriter", i18nc("@label", "Original Text Writer") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#SynchronizedText", i18nc("@label", "Lyrics") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#licensee", i18nc("@label", "Licensee") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#InvolvedPerson", i18nc("@label", "Musician Credits") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#subtitle", i18nc("@label", "Subtitle") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#codec",  i18nc("@label", "Codec") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#belongsToContainer",  i18nc("@label", "Container Format") },
        { "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#UnionOfEventJournalTodo", i18nc("@label", "Grouping") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitDepth", i18nc("@label", "Audio Bit Depth") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitDepth", i18nc("@label", "Video Bit Depth") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#aspectRatio", i18nc("@label", "Aspect Ratio") },
    };

    const TranslationMap::const_iterator it = s_translations.find(key);
    if (it != s_translations.cend()) {
        return it->second;
    }

    // fallback if the URI is not translated
    const int index = key.indexOf(QChar('#'));
    if (index >= 0) {
        return key.right(key.size() - index - 1);
    }
    return key;
}
