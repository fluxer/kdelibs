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

// NOTE: same as kdelibs/kio/kio/previewjob.cpp except the service string
static QStringList kMetaGlobMimeTypes(const QStringList &servicetypes)
{
    static const QString kfimetadatapluginservice("KFileMetaData/Plugin");

    QStringList result;
    foreach (const QString &servicetype, servicetypes) {
        if (servicetype.isEmpty() || servicetype == kfimetadatapluginservice) {
            continue;
        }
        result.append(servicetype);
    }
    // qDebug() << Q_FUNC_INFO << result;
    return result;
}


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
        const QString kfmdname = kfmdplugin->desktopEntryName();
        const bool enable = pluginsgroup.readEntry(kfmdname, true);
        if (enable) {
            // qDebug() << Q_FUNC_INFO << filemimetype->name() << kfmdname;
            foreach (const QString &kfmdpluginmime, kMetaGlobMimeTypes(kfmdplugin->serviceTypes())) {
                bool mimematches = false;
                if (kfmdpluginmime.endsWith('*')) {
                    const QString kfmdpluginmimeglob = kfmdpluginmime.mid(0, kfmdpluginmime.size() - 1);
                    if (filemimetype && filemimetype->name().startsWith(kfmdpluginmimeglob)) {
                        mimematches = true;
                    }
                }

                if (!mimematches && filemimetype->is(kfmdpluginmime)) {
                    mimematches = true;
                }

                if (mimematches) {
                    kDebug() << "Extracting metadata via" << kfmdname;
                    KFileMetaDataPlugin *kfmdplugininstance = kfmdplugin->createInstance<KFileMetaDataPlugin>();
                    if (kfmdplugininstance) {
                        items.append(kfmdplugininstance->metaData(url, w));
                        delete kfmdplugininstance;
                    } else {
                        kWarning() << "Could not create KFileMetaDataPlugin instance";
                    }
                    break;
                }
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
        const QString kfmdname = kfmdplugin->desktopEntryName();
        const bool enable = pluginsgroup.readEntry(kfmdname, true);
        if (enable) {
            keys.append(kfmdplugin->property("X-KDE-MetadataKeys", QVariant::StringList).toStringList());
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

    static const struct TranslationsTblData {
        const QLatin1String key;
        const QString translation;
    } TranslationsTbl[] = {
        { QLatin1String("kfileitem#modified"), i18nc("@label", "Modified") },
        { QLatin1String("kfileitem#owner"), i18nc("@label", "Owner") },
        { QLatin1String("kfileitem#permissions"), i18nc("@label", "Permissions") },
        { QLatin1String("kfileitem#size"), i18nc("@label", "Size") },
        { QLatin1String("kfileitem#totalSize"), i18nc("@label", "Total Size") },
        { QLatin1String("kfileitem#type"), i18nc("@label", "Type") },
        { QLatin1String("kfileitem#mimetype"), i18nc("@label", "MIME Type") },
        // implemented so far
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName"), i18nc("@label", "Filename") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url"), i18nc("@label file URL", "URL") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width"), i18nc("@label", "Width") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height"), i18nc("@label", "Height") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate"), i18nc("@label", "Average Bitrate") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleRate"), i18nc("@label", "Sample Rate") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels"), i18nc("@label", "Channels") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration"), i18nc("@label", "Duration") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#frameRate"), i18nc("@label", "Frame Rate") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoCodec"),  i18nc("@label", "Video Codec") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioCodec"),  i18nc("@label", "Audio Codec") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#subtitleCodec"),  i18nc("@label", "Subtitle Codec") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitRate"), i18nc("@label", "Audio Bit Rate") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitRate"), i18nc("@label", "Video Bit Rate") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount"), i18nc("@label", "Page Count") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright"), i18nc("@label", "Copyright") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment"), i18nc("@label", "Comment") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title"), i18nc("@label music title", "Title") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword"), i18nc("@label", "Keyword") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description"), i18nc("@label", "Description") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator"), i18nc("@label Software used to generate content", "Generator") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentLastModified"), i18nc("@label modification date", "Modified") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated"), i18nc("@label creation date", "Created") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject"), i18nc("@label", "Subject") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum"), i18nc("@label music album", "Album") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre"),  i18nc("@label music genre", "Genre") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer"), i18nc("@label", "Performer") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber"), i18nc("@label music track number", "Track") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#composer"), i18nc("@label music composer", "Composer") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encoder"), i18nc("@label", "Encoder") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encodedBy"), i18nc("@label", "Encoded By") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalReleaseYear"), i18nc("@label", "Original Release Year") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#uniqueFileIdentifier"), i18nc("@label", "URI") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#textWriter"), i18nc("@label", "Text Writer") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher"), i18nc("@label", "Publisher") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator"), i18nc("@label", "Creator") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contributor"), i18nc("@label", "Contributor") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make"), i18nc("@label EXIF", "Manufacturer") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model"), i18nc("@label EXIF", "Model") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation"), i18nc("@label EXIF", "Orientation") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist"), i18nc("@label music or image artist", "Artist") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef"), i18nc("@label", "GPS Latitude Reference") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef"), i18nc("@label", "GPS Longitude Reference") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash"), i18nc("@label EXIF", "Flash") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime"), i18nc("@label EXIF", "Exposure Time") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue"), i18nc("@label EXIF", "Exposure Bias Value") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode"), i18nc("@label EXIF", "Exposure Mode") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue"), i18nc("@label EXIF aperture value", "Aperture") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength"), i18nc("@label EXIF", "Focal Length") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm"), i18nc("@label EXIF", "Focal Length 35 mm") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings"), i18nc("@label EXIF", "ISO Speed Ratings") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode"), i18nc("@label EXIF", "Metering Mode") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance"), i18nc("@label EXIF", "White Balance") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fontFamily"), i18nc("@label", "Family") },
        // to be used by plugins
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Email"), i18nc("@label", "Creator E-Mail") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue"), i18nc("@label", "Hash Value") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm"), i18nc("@label", "Hash Algorithm") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#country"), i18nc("@label", "Country") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#summary"), i18nc("@label", "Summary") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#characterSet"), i18nc("@label", "Character Set") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#lineCount"), i18nc("@label number of lines", "Lines") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#wordCount"), i18nc("@label number of words", "Words") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#characterCount"), i18nc("@label", "Character Count") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#sequence"), i18nc("@label", "Revision") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact"), i18nc("@label", "Contact") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#disclaimer"), i18nc("@label", "Disclaimer") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#director"), i18nc("@label video director", "Director") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#conductor"), i18nc("@label", "Conductor") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#interpretedBy"), i18nc("@label", "Interpreted By") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#beatsPerMinute"), i18nc("@label", "Beats Per Minute") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalArtist"), i18nc("@label", "Original Artist") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalTextWriter"), i18nc("@label", "Original Text Writer") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#SynchronizedText"), i18nc("@label", "Lyrics") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#licensee"), i18nc("@label", "Licensee") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#InvolvedPerson"), i18nc("@label", "Musician Credits") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#subtitle"), i18nc("@label", "Subtitle") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#codec"),  i18nc("@label", "Codec") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#belongsToContainer"),  i18nc("@label", "Container Format") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#UnionOfEventJournalTodo"), i18nc("@label", "Grouping") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitDepth"), i18nc("@label", "Audio Bit Depth") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitDepth"), i18nc("@label", "Video Bit Depth") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#aspectRatio"), i18nc("@label", "Aspect Ratio") },
        { QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#foundry"), i18nc("@label", "Foundry") },
    };
    static const qint16 TranslationsTblSize = sizeof(TranslationsTbl) / sizeof(TranslationsTblData);

    for (qint16 i = 0; i < TranslationsTblSize; i++) {
        if (key == TranslationsTbl[i].key) {
            return TranslationsTbl[i].translation;
        }
    }

    // fallback if the URI is not translated
    const int index = key.indexOf(QChar('#'));
    if (index >= 0) {
        return key.right(key.size() - index - 1);
    }
    return key;
}
