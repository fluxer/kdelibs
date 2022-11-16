// -*- c++ -*-
// vim: ts=4 sw=4 et
/*  This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>
                  2000 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Malte Starostik <malte.starostik@t-online.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
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

#include "previewjob.h"
#include <kdebug.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtGui/QImage>
#include <QtCore/QTimer>
#include <QtCore/QRegExp>
#include <QtCore/QList>
#include <QtGui/QImageWriter>

#include <kfileitem.h>
#include <kde_file.h>
#include <ktemporaryfile.h>
#include <kservicetypetrader.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kservice.h>
#include <kconfiggroup.h>
#include <kprotocolinfo.h>

#include "jobuidelegate.h"
#include "job_p.h"

namespace KIO { struct PreviewItem; }
using namespace KIO;

static const QByteArray thumbFormat = QImageWriter::defaultImageFormat();
static const QString thumbExt = QLatin1String(".") + thumbFormat;

// NOTE: keep in sync with:
// kde-workspace/dolphin/src/settings/general/previewssettingspage.cpp
// kde-workspace/kioslave/thumbnail/thumbnail.h
enum PreviewDefaults {
    MaxLocalSize = 20, // 20 MB
    MaxRemoteSize = 5, // 5 MB
    IconAlpha = 70
};

struct KIO::PreviewItem
{
    KFileItem item;
    KService::Ptr plugin;
};

class KIO::PreviewJobPrivate: public KIO::JobPrivate
{
public:
    enum { STATE_STATORIG, // if the thumbnail exists
           STATE_GETORIG, // if we create it
           STATE_CREATETHUMB // thumbnail:/ slave
    } state;
    PreviewJob *q;

    KFileItemList initialItems;
    QStringList enabledPlugins;
    // Some plugins support remote URLs, <protocol, mimetypes>
    QHash<QString, QStringList> m_remoteProtocolPlugins;
    // Our todo list :)
    // We remove the first item at every step, so use QList
    QList<PreviewItem> items;
    // The current item
    PreviewItem currentItem;
    // The modification time of that URL
    time_t tOrig;
    // Path to thumbnail cache for the current size
    QString thumbPath;
    // Original URL of current item in TMS format
    // (file:///path/to/file instead of file:/path/to/file)
    QString origName;
    // Thumbnail file name for current item
    QString thumbName;
    // Size of thumbnail
    int width;
    int height;
    // Unscaled size of thumbnail (128 or 256 if cache is enabled)
    int cacheWidth;
    int cacheHeight;
    // Whether the thumbnail should be scaled
    bool bScale;
    // Whether we should save the thumbnail
    bool bSave;
    bool ignoreMaximumSize;
    int sequenceIndex;
    bool succeeded;
    // If the file to create a thumb for was a temp file, this is its name
    QString tempName;
    KIO::filesize_t maximumLocalSize;
    KIO::filesize_t maximumRemoteSize;
    // the size for the icon overlay
    int iconSize;
    // the transparency of the blended mimetype icon
    int iconAlpha;
    // Root of thumbnail cache
    QString thumbRoot;

    void getOrCreateThumbnail();
    bool statResultThumbnail();
    void createThumbnail( const QString& );
    void determineNextFile();
    void emitPreview(const QImage &thumb);

    void startPreview();
    void slotThumbData(KIO::Job *, const QByteArray &);

    Q_DECLARE_PUBLIC(PreviewJob)
};


PreviewJob::PreviewJob(const KFileItemList &items,
                       const QSize &size,
                       const QStringList *enabledPlugins) :
    KIO::Job(*new PreviewJobPrivate)
{
    Q_D(PreviewJob);
    const KConfigGroup globalConfig(KGlobal::config(), "PreviewSettings");
    d->tOrig = 0;
    d->initialItems = items;
    if (enabledPlugins) {
        d->enabledPlugins = *enabledPlugins;
    } else {
        QStringList enabledByDefault;
        const KService::List plugins = KServiceTypeTrader::self()->query(QLatin1String("ThumbCreator"));
        foreach (const KSharedPtr<KService>& service, plugins) {
            const bool enabled = service->property("X-KDE-PluginInfo-EnabledByDefault", QVariant::Bool).toBool();
            if (enabled) {
                enabledByDefault << service->desktopEntryName();
            }
        }

        d->enabledPlugins = globalConfig.readEntry("Plugins", enabledByDefault);
    }
    d->width = size.width();
    d->height = size.height();
    d->cacheWidth = d->width;
    d->cacheHeight = d->height;
    d->iconSize = 0; // when zero KIconLoader::currentSize(KIconLoader::Desktop) is used
    d->iconAlpha = globalConfig.readEntry("IconAlpha", int(PreviewDefaults::IconAlpha));
    d->bScale = true;
    d->bSave = true;
    d->succeeded = false;
    d->thumbRoot = QDir::homePath() + QLatin1String("/.thumbnails/");
    d->ignoreMaximumSize = false;
    d->sequenceIndex = 0;
    d->maximumLocalSize = 0;
    d->maximumRemoteSize = 0;

    // Return to event loop first, determineNextFile() might delete this;
    QTimer::singleShot(0, this, SLOT(startPreview()));
}

PreviewJob::~PreviewJob()
{
}

void PreviewJob::setOverlayIconSize(int size)
{
    Q_D(PreviewJob);
    d->iconSize = size;
}

int PreviewJob::overlayIconSize() const
{
    Q_D(const PreviewJob);
    return d->iconSize;
}

void PreviewJob::setOverlayIconAlpha(int alpha)
{
    Q_D(PreviewJob);
    d->iconAlpha = qBound(0, alpha, 255);
}

int PreviewJob::overlayIconAlpha() const
{
    Q_D(const PreviewJob);
    return d->iconAlpha;
}

void PreviewJob::setScaleType(ScaleType type)
{
    Q_D(PreviewJob);
    switch (type) {
    case Unscaled:
        d->bScale = false;
        d->bSave = false;
        break;
    case Scaled:
        d->bScale = true;
        d->bSave = false;
        break;
    case ScaledAndCached:
        d->bScale = true;
        d->bSave = true;
        break;
    default:
        break;
    }
}

PreviewJob::ScaleType PreviewJob::scaleType() const
{
    Q_D(const PreviewJob);
    if (d->bScale) {
        return d->bSave ? ScaledAndCached : Scaled;
    }
    return Unscaled;
}

void PreviewJobPrivate::startPreview()
{
    Q_Q(PreviewJob);
    // Load the list of plugins to determine which mimetypes are supported
    const KService::List plugins = KServiceTypeTrader::self()->query("ThumbCreator");
    QMap<QString, KService::Ptr> mimeMap;
    QHash<QString, QHash<QString, KService::Ptr> > protocolMap;
    for (KService::List::ConstIterator it = plugins.constBegin(); it != plugins.constEnd(); ++it) {
        QStringList protocols = (*it)->property("X-KDE-Protocols").toStringList();
        const QString p = (*it)->property("X-KDE-Protocol").toString();
        if (!p.isEmpty()) {
            protocols.append(p);
        }
        foreach (const QString &protocol, protocols) {
            QStringList mtypes = (*it)->serviceTypes();
            // Filter out non-mimetype servicetypes
            // TODO KDE5: use KService::mimeTypes()
            foreach (const QString &_mtype, mtypes) {
                if (!((*it)->hasMimeType(_mtype))) {
                    mtypes.removeAll(_mtype);
                }
            }
            // Add supported mimetype for this protocol
            QStringList &_ms = m_remoteProtocolPlugins[protocol];
            foreach (const QString &_m, mtypes) {
                protocolMap[protocol].insert(_m, *it);
                if (!_ms.contains(_m)) {
                    _ms.append(_m);
                }
            }
        }
        if (enabledPlugins.contains((*it)->desktopEntryName())) {
            const QStringList mimeTypes = (*it)->serviceTypes();
            for (QStringList::ConstIterator mt = mimeTypes.constBegin(); mt != mimeTypes.constEnd(); ++mt)
                mimeMap.insert(*mt, *it);
        }
    }

    // Look for images and store the items in our todo list :)
    bool bNeedCache = false;
    foreach ( const KFileItem kit, initialItems )
    {
        PreviewItem item;
        item.item = kit;
        const QString mimeType = item.item.mimetype();
        KService::Ptr plugin(0);

        // look for protocol-specific thumbnail plugins first
        QHash<QString, QHash<QString, KService::Ptr> >::const_iterator it = protocolMap.constFind(item.item.url().protocol());
        if (it != protocolMap.constEnd()) {
            plugin = it.value().value(mimeType);
        }

        if (!plugin) {
            QMap<QString, KService::Ptr>::ConstIterator pluginIt = mimeMap.constFind(mimeType);
            if (pluginIt == mimeMap.constEnd()) {
                QString groupMimeType = mimeType;
                groupMimeType.replace(QRegExp("/.*"), "/*");
                pluginIt = mimeMap.constFind(groupMimeType);

                if (pluginIt == mimeMap.constEnd()) {
                    // check mime type inheritance, resolve aliases
                    const KMimeType::Ptr mimeInfo = KMimeType::mimeType(mimeType);
                    if (mimeInfo) {
                        const QStringList parentMimeTypes = mimeInfo->allParentMimeTypes();
                        Q_FOREACH(const QString& parentMimeType, parentMimeTypes) {
                            pluginIt = mimeMap.constFind(parentMimeType);
                            if (pluginIt != mimeMap.constEnd())
                                break;
                        }
                    }
                }
            }

            if (pluginIt != mimeMap.constEnd()) {
                plugin = *pluginIt;
            }
        }

        if (plugin) {
            item.plugin = plugin;
            items.append(item);
            if (!bNeedCache && bSave &&
                (kit.url().protocol() != "file" ||
                 !kit.url().directory( KUrl::AppendTrailingSlash ).startsWith(thumbRoot)) &&
                plugin->property("CacheThumbnail").toBool()) {
                bNeedCache = true;
            }
        } else {
            emit q->failed( kit );
        }
    }

    KConfigGroup cg( KGlobal::config(), "PreviewSettings" );
    maximumLocalSize = cg.readEntry( "MaximumSize", PreviewDefaults::MaxLocalSize *1024 * 1024LL);
    maximumRemoteSize = cg.readEntry( "MaximumRemoteSize", PreviewDefaults::MaxRemoteSize *1024 * 1024LL );

    if (bNeedCache)
    {
        if (width <= 128 && height <= 128) cacheWidth = cacheHeight = 128;
        else cacheWidth = cacheHeight = 256;
        thumbPath = thumbRoot + (cacheWidth == 128 ? "normal/" : "large/");
        KStandardDirs::makeDir(thumbPath, 0700);
    }
    else
        bSave = false;

    initialItems.clear();
    determineNextFile();
}

void PreviewJob::removeItem( const KUrl& url )
{
    Q_D(PreviewJob);
    for (QList<PreviewItem>::Iterator it = d->items.begin(); it != d->items.end(); ++it)
        if ((*it).item.url() == url)
        {
            d->items.erase(it);
            break;
        }

    if (d->currentItem.item.url() == url)
    {
        KJob* job = subjobs().first();
        job->kill();
        removeSubjob( job );
        d->determineNextFile();
    }
}

void KIO::PreviewJob::setSequenceIndex(int index) {
    d_func()->sequenceIndex = index;
}

int KIO::PreviewJob::sequenceIndex() const {
    return d_func()->sequenceIndex;
}

void PreviewJob::setIgnoreMaximumSize(bool ignoreSize)
{
    d_func()->ignoreMaximumSize = ignoreSize;
}

void PreviewJobPrivate::determineNextFile()
{
    Q_Q(PreviewJob);
    if (!currentItem.item.isNull())
    {
        if (!succeeded)
            emit q->failed( currentItem.item );
    }
    // No more items ?
    if ( items.isEmpty() )
    {
        q->emitResult();
        return;
    }
    else
    {
        // First, stat the orig file
        state = PreviewJobPrivate::STATE_STATORIG;
        currentItem = items.first();
        succeeded = false;
        items.removeFirst();
        KIO::Job *job = KIO::stat( currentItem.item.url(), KIO::HideProgressInfo );
        job->addMetaData( "no-auth-prompt", "true" );
        q->addSubjob(job);
    }
}

void PreviewJob::slotResult( KJob *job )
{
    Q_D(PreviewJob);

    removeSubjob(job);
    Q_ASSERT ( !hasSubjobs() ); // We should have only one job at a time ...
    switch ( d->state )
    {
        case PreviewJobPrivate::STATE_STATORIG:
        {
            if (job->error()) // that's no good news...
            {
                // Drop this one and move on to the next one
                d->determineNextFile();
                return;
            }
            const KIO::UDSEntry entry = static_cast<KIO::StatJob*>(job)->statResult();
            d->tOrig = entry.numberValue( KIO::UDSEntry::UDS_MODIFICATION_TIME, 0 );

            bool skipCurrentItem = false;
            const KIO::filesize_t size = (KIO::filesize_t)entry.numberValue( KIO::UDSEntry::UDS_SIZE, 0 );
            const KUrl itemUrl = d->currentItem.item.mostLocalUrl();

            if (itemUrl.isLocalFile() || KProtocolInfo::protocolClass(itemUrl.protocol()) == QLatin1String(":local"))
            {
                skipCurrentItem = !d->ignoreMaximumSize && size > d->maximumLocalSize
                                  && !d->currentItem.plugin->property("IgnoreMaximumSize").toBool();
            }
            else
            {
                // For remote items the "IgnoreMaximumSize" plugin property is not respected
                skipCurrentItem = !d->ignoreMaximumSize && size > d->maximumRemoteSize;

                // Remote directories are not supported, don't try to do a file_copy on them
                if (!skipCurrentItem) {
                    // TODO update item.mimeType from the UDS entry, in case it wasn't set initially
                    KMimeType::Ptr mime = d->currentItem.item.mimeTypePtr();
                    if (mime && mime->is("inode/directory")) {
                        skipCurrentItem = true;
                    }
                }
            }
            if (skipCurrentItem)
            {
                d->determineNextFile();
                return;
            }

            bool pluginHandlesSequences = d->currentItem.plugin->property("HandleSequences", QVariant::Bool).toBool();
            if ( !d->currentItem.plugin->property( "CacheThumbnail" ).toBool()  || (d->sequenceIndex && pluginHandlesSequences) )
            {
                // This preview will not be cached, no need to look for a saved thumbnail
                // Just create it, and be done
                d->getOrCreateThumbnail();
                return;
            }

            if ( d->statResultThumbnail() )
                return;

            d->getOrCreateThumbnail();
            return;
        }
        case PreviewJobPrivate::STATE_GETORIG:
        {
            if (job->error())
            {
                d->determineNextFile();
                return;
            }

            d->createThumbnail( static_cast<KIO::FileCopyJob*>(job)->destUrl().toLocalFile() );
            return;
        }
        case PreviewJobPrivate::STATE_CREATETHUMB:
        {
            if (!d->tempName.isEmpty())
            {
                QFile::remove(d->tempName);
                d->tempName.clear();
            }
            d->determineNextFile();
            return;
        }
    }
}

bool PreviewJobPrivate::statResultThumbnail()
{
    if ( thumbPath.isEmpty() )
        return false;

    KUrl url = currentItem.item.mostLocalUrl();
    // Don't include the password if any
    url.setPass(QString());
    origName = url.url();

    // NOTE: make sure the algorithm and name match those used in kde-workspace/kioslave/thumbnail/thumbnail.cpp
    const QByteArray hash = QFile::encodeName( origName ).toHex();
    const QString modTime = QString::number(QFileInfo(url.toLocalFile()).lastModified().toTime_t());
    thumbName = hash + modTime + thumbExt;

    QImage thumb;
    if ( !thumb.load( thumbPath + thumbName ) )
        return false;

    // Found it, use it
    emitPreview( thumb );
    succeeded = true;
    determineNextFile();
    return true;
}


void PreviewJobPrivate::getOrCreateThumbnail()
{
    Q_Q(PreviewJob);
    // We still need to load the orig file ! (This is getting tedious) :)
    const KFileItem& item = currentItem.item;
    const QString localPath = item.localPath();
    if (!localPath.isEmpty()) {
        createThumbnail( localPath );
    } else {
        const KUrl fileUrl = item.url();
        // heuristics for remote URL support
        bool supportsProtocol = false;
        if (m_remoteProtocolPlugins.value(fileUrl.scheme()).contains(item.mimetype())) {
            // There's a plugin supporting this protocol and mimetype
            supportsProtocol = true;
        } else if (m_remoteProtocolPlugins.value("KIO").contains(item.mimetype())) {
            // Assume KIO understands any URL, ThumbCreator slaves who have
            // X-KDE-Protocols=KIO will get fed the remote URL directly.
            supportsProtocol = true;
        }

        if (supportsProtocol) {
            createThumbnail(fileUrl.url());
            return;
        }
        // No plugin support access to this remote content, copy the file
        // to the local machine, then create the thumbnail
        state = PreviewJobPrivate::STATE_GETORIG;
        tempName = KTemporaryFile::filePath();
        KUrl localURL;
        localURL.setPath( tempName );
        const KUrl currentURL = item.mostLocalUrl();
        KIO::Job * job = KIO::file_copy( currentURL, localURL, -1, KIO::Overwrite | KIO::HideProgressInfo /* No GUI */ );
        job->addMetaData("thumbnail","1");
        q->addSubjob(job);
    }
}

void PreviewJobPrivate::createThumbnail( const QString &pixPath )
{
    Q_Q(PreviewJob);
    state = PreviewJobPrivate::STATE_CREATETHUMB;
    KUrl thumbURL;
    thumbURL.setProtocol("thumbnail");
    thumbURL.setPath(pixPath);
    KIO::TransferJob *job = KIO::get(thumbURL, NoReload, HideProgressInfo);
    q->addSubjob(job);
    q->connect(job, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(slotThumbData(KIO::Job*,QByteArray)));
    bool save = bSave && currentItem.plugin->property("CacheThumbnail").toBool() && !sequenceIndex;
    job->addMetaData("mimeType", currentItem.item.mimetype());
    job->addMetaData("width", QString::number(save ? cacheWidth : width));
    job->addMetaData("height", QString::number(save ? cacheHeight : height));
    job->addMetaData("iconSize", QString::number(save ? 64 : iconSize));
    job->addMetaData("iconAlpha", QString::number(iconAlpha));
    job->addMetaData("plugin", currentItem.plugin->library());
    if(sequenceIndex)
        job->addMetaData("sequence-index", QString::number(sequenceIndex));
}

void PreviewJobPrivate::slotThumbData(KIO::Job *, const QByteArray &data)
{
    bool save = bSave &&
                currentItem.plugin->property("CacheThumbnail").toBool() &&
                (currentItem.item.url().protocol() != "file" ||
                 !currentItem.item.url().directory( KUrl::AppendTrailingSlash ).startsWith(thumbRoot)) && !sequenceIndex;
    QImage thumb;
    QDataStream s(data);
    s >> thumb;

    QString tempFileName;
    bool savedCorrectly = false;
    if (save) {
        // Only try to write out the thumbnail if we actually created the temp file.
        tempFileName = KTemporaryFile::filePath(QString::fromLatin1("XXXXXXXXXX%1").arg(thumbExt));
        savedCorrectly = thumb.save(tempFileName, thumbFormat);
    }
    if (savedCorrectly) {
        Q_ASSERT(!tempFileName.isEmpty());
        KDE::rename(tempFileName, thumbPath + thumbName);
    }
    emitPreview( thumb );
    succeeded = true;
}

void PreviewJobPrivate::emitPreview(const QImage &thumb)
{
    Q_Q(PreviewJob);
    QPixmap pix;
    if (thumb.width() > width || thumb.height() > height)
        pix = QPixmap::fromImage( thumb.scaled(QSize(width, height), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
    else
        pix = QPixmap::fromImage( thumb );
    emit q->gotPreview(currentItem.item, pix);
}

QStringList PreviewJob::availablePlugins()
{
    QStringList result;
    const KService::List plugins = KServiceTypeTrader::self()->query("ThumbCreator");
    for (KService::List::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        if (!result.contains((*it)->desktopEntryName()))
            result.append((*it)->desktopEntryName());
    return result;
}

QStringList PreviewJob::supportedMimeTypes()
{
    QStringList result;
    const KService::List plugins = KServiceTypeTrader::self()->query("ThumbCreator");
    for (KService::List::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        result += (*it)->serviceTypes();
    return result;
}


PreviewJob *KIO::filePreview(const KFileItemList &items, const QSize &size, const QStringList *enabledPlugins)
{
    return new PreviewJob(items, size, enabledPlugins);
}


#include "moc_previewjob.cpp"
