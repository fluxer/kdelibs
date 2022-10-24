/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "wallpaper.h"

#include "config-plasma.h"

#include <QColor>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QAction>
#include <QQueue>
#include <QTimer>
#include <QPainter>
#include <QDateTime>
#include <QImageWriter>

#include <kdebug.h>
#include <kglobal.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>

#ifndef PLASMA_NO_KIO
#include <kio/job.h>
#endif

#include "plasma/plasma.h"
#include "plasma/package.h"
#include "plasma/private/dataengineconsumer_p.h"
#include "plasma/private/packages_p.h"
#include "plasma/private/wallpaper_p.h"

static const QByteArray imageFormat = QImageWriter::defaultImageFormat();

namespace Plasma
{

PackageStructure::Ptr WallpaperPrivate::s_packageStructure(0);

Wallpaper::Wallpaper(QObject * parentObject)
    : d(new WallpaperPrivate(KService::serviceByStorageId(QString()), this))
{
    setParent(parentObject);
}

Wallpaper::Wallpaper(QObject *parentObject, const QVariantList &args)
    : d(new WallpaperPrivate(KService::serviceByStorageId(args.count() > 0 ?
                             args[0].toString() : QString()), this))
{
    // now remove first item since those are managed by Wallpaper and subclasses shouldn't
    // need to worry about them. yes, it violates the constness of this var, but it lets us add
    // or remove items later while applets can just pretend that their args always start at 0
    QVariantList &mutableArgs = const_cast<QVariantList &>(args);
    if (!mutableArgs.isEmpty()) {
        mutableArgs.removeFirst();
    }

    setParent(parentObject);
}

Wallpaper::~Wallpaper()
{
    delete d;
}

void Wallpaper::addUrls(const KUrl::List &urls)
{
    // provide compatibility with urlDropped
    foreach (const KUrl &url, urls) {
        emit urlDropped(url);
    }
}

void Wallpaper::setUrls(const KUrl::List &urls)
{
    if (!d->initialized) {
        d->pendingUrls = urls;
    } else {
       QMetaObject::invokeMethod(this, "addUrls", Q_ARG(KUrl::List, urls));
    }
}

KPluginInfo::List Wallpaper::listWallpaperInfo(const QString &formFactor)
{
    QString constraint;
    if (!formFactor.isEmpty()) {
        constraint.append("[X-Plasma-FormFactors] ~~ '").append(formFactor).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Wallpaper", constraint);
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List Wallpaper::listWallpaperInfoForMimetype(const QString &mimetype, const QString &formFactor)
{
    QString constraint = QString("'%1' in [X-Plasma-DropMimeTypes]").arg(mimetype);
    if (!formFactor.isEmpty()) {
        constraint.append("[X-Plasma-FormFactors] ~~ '").append(formFactor).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Wallpaper", constraint);
    kDebug() << offers.count() << constraint;
    return KPluginInfo::fromServices(offers);
}

bool Wallpaper::supportsMimetype(const QString &mimetype) const
{
    return d->wallpaperDescription.isValid() &&
           d->wallpaperDescription.service()->hasMimeType(mimetype);
}

Wallpaper *Wallpaper::load(const QString &wallpaperName, const QVariantList &args)
{
    if (wallpaperName.isEmpty()) {
        return 0;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(wallpaperName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Wallpaper", constraint);

    if (offers.isEmpty()) {
        kDebug() << "offers is empty for " << wallpaperName;
        return 0;
    }

    KService::Ptr offer = offers.first();
    QVariantList allArgs;
    allArgs << offer->storageId() << args;

    QString error;
    Wallpaper *wallpaper = offer->createInstance<Plasma::Wallpaper>(0, allArgs, &error);

    if (!wallpaper) {
        kDebug() << "Couldn't load wallpaper \"" << wallpaperName << "\"! reason given: " << error;
    }

    return wallpaper;
}

Wallpaper *Wallpaper::load(const KPluginInfo &info, const QVariantList &args)
{
    if (!info.isValid()) {
        return 0;
    }
    return load(info.pluginName(), args);
}

PackageStructure::Ptr Wallpaper::packageStructure(Wallpaper *paper)
{
    if (paper) {
        PackageStructure::Ptr package(new WallpaperPackage(paper));
        return package;
    }

    if (!WallpaperPrivate::s_packageStructure) {
        WallpaperPrivate::s_packageStructure = new WallpaperPackage();
    }

    return WallpaperPrivate::s_packageStructure;
}

QString Wallpaper::name() const
{
    if (!d->wallpaperDescription.isValid()) {
        return i18n("Unknown Wallpaper");
    }

    return d->wallpaperDescription.name();
}

QString Wallpaper::icon() const
{
    if (!d->wallpaperDescription.isValid()) {
        return QString();
    }

    return d->wallpaperDescription.icon();
}

QString Wallpaper::pluginName() const
{
    if (!d->wallpaperDescription.isValid()) {
        return QString();
    }

    return d->wallpaperDescription.pluginName();
}

KServiceAction Wallpaper::renderingMode() const
{
    return d->mode;
}

QList<KServiceAction> Wallpaper::listRenderingModes() const
{
    if (!d->wallpaperDescription.isValid()) {
        return QList<KServiceAction>();
    }

    return d->wallpaperDescription.service()->actions();
}

QRectF Wallpaper::boundingRect() const
{
    return d->boundingRect;
}

bool Wallpaper::isInitialized() const
{
    return d->initialized;
}

void Wallpaper::setBoundingRect(const QRectF &boundingRect)
{
    d->boundingRect = boundingRect;

    if (d->targetSize != boundingRect.size()) {
        d->targetSize = boundingRect.size();
        emit renderHintsChanged();
    }
}

void Wallpaper::setRenderingMode(const QString &mode)
{
    if (d->mode.name() == mode) {
        return;
    }

    d->mode = KServiceAction();
    if (!mode.isEmpty()) {
        QList<KServiceAction> modes = listRenderingModes();

        foreach (const KServiceAction &action, modes) {
            if (action.name() == mode) {
                d->mode = action;
                break;
            }
        }
    }
}

void Wallpaper::restore(const KConfigGroup &config)
{
    init(config);
    d->initialized = true;
    if (!d->pendingUrls.isEmpty()) {
        setUrls(d->pendingUrls);
        d->pendingUrls.clear();
    }
}

void Wallpaper::init(const KConfigGroup &config)
{
}

void Wallpaper::save(KConfigGroup &config)
{
}

QWidget *Wallpaper::createConfigurationInterface(QWidget *parent)
{
    return 0;
}

void Wallpaper::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
}

void Wallpaper::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
}

void Wallpaper::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
}

void Wallpaper::wheelEvent(QGraphicsSceneWheelEvent *event)
{
}

DataEngine *Wallpaper::dataEngine(const QString &name) const
{
    return d->dataEngine(name);
}

bool Wallpaper::configurationRequired() const
{
    return d->needsConfig;
}

void Wallpaper::setConfigurationRequired(bool needsConfig, const QString &reason)
{
    //TODO: implement something for reason. first, we need to decide where/how
    //      to communicate it to the user
    Q_UNUSED(reason)

    if (d->needsConfig == needsConfig) {
        return;
    }

    d->needsConfig = needsConfig;
    emit configurationRequired(needsConfig);
}

bool Wallpaper::isUsingRenderingCache() const
{
    return d->cacheRendering;
}

void Wallpaper::setUsingRenderingCache(bool useCache)
{
    d->cacheRendering = useCache;
}

void Wallpaper::setResizeMethodHint(Wallpaper::ResizeMethod resizeMethod)
{
    const ResizeMethod method = qBound(ScaledResize, resizeMethod, LastResizeMethod);
    if (method != d->lastResizeMethod) {
        d->lastResizeMethod = method;
        emit renderHintsChanged();
    }
}

Wallpaper::ResizeMethod Wallpaper::resizeMethodHint() const
{
    return d->lastResizeMethod;
}

void Wallpaper::setTargetSizeHint(const QSizeF &targetSize)
{
    if (targetSize != d->targetSize) {
        d->targetSize = targetSize;
        emit renderHintsChanged();
    }
}

QSizeF Wallpaper::targetSizeHint() const
{
    return d->targetSize;
}

void Wallpaper::render(const QImage &image, const QSize &size,
                       Wallpaper::ResizeMethod resizeMethod, const QColor &color)
{
    if (image.isNull()) {
        return;
    }

    d->renderWallpaper(QString(), image, size, resizeMethod, color);
}

void Wallpaper::render(const QString &sourceImagePath, const QSize &size,
                       Wallpaper::ResizeMethod resizeMethod, const QColor &color)
{
    if (sourceImagePath.isEmpty() || !QFile::exists(sourceImagePath)) {
        //kDebug() << "failed on:" << sourceImagePath;
        return;
    }

    d->renderWallpaper(sourceImagePath, QImage(), size, resizeMethod, color);
}

void WallpaperPrivate::renderWallpaper(const QString &sourceImagePath, const QImage &image, const QSize &size,
                                       Wallpaper::ResizeMethod resizeMethod, const QColor &color)
{
    resizeMethod = qBound(Wallpaper::ScaledResize, resizeMethod, Wallpaper::LastResizeMethod);
    if (lastResizeMethod != resizeMethod) {
        lastResizeMethod = resizeMethod;
        emit q->renderHintsChanged();
    }

    if (cacheRendering) {
        QFileInfo info(sourceImagePath);
        QString cache = cacheKey(sourceImagePath, size, resizeMethod, color);
        if (findInCache(cache, info.lastModified().toTime_t())) {
            return;
        }
    }

    kDebug() << "rendering wallpaper" << sourceImagePath;
    QImage result(size, QImage::Format_ARGB32_Premultiplied);
    result.fill(color.rgba());

    if (sourceImagePath.isEmpty() && image.isNull() && !QFile::exists(sourceImagePath)) {
        kDebug() << "wrong request or file does not exist";
        return;
    }

    QPoint pos(0, 0);
    //const float ratio = qMax(float(1), size.width() / float(size.height()));
    bool tiled = false;
    QSize scaledSize;
    QImage img;

    // set image size
    QSize imgSize(1, 1);
    if (!image.isNull()) {
        img = image;
        kDebug() << "going to resize the img" << img.size();
        imgSize = imgSize.expandedTo(img.size());
    } else {
        // otherwise, use the natural size of the loaded image
        img = QImage(sourceImagePath);
        imgSize = imgSize.expandedTo(img.size());
        //kDebug() << "loaded with" << imgSize << ratio;
    }

    // set render parameters according to resize mode
    switch (resizeMethod) {
        case Wallpaper::ScaledResize: {
            scaledSize = size;
            break;
        }
        case Wallpaper::CenteredResize: {
            scaledSize = imgSize;
            pos = QPoint((size.width() - scaledSize.width()) / 2,
                         (size.height() - scaledSize.height()) / 2);

            //If the picture is bigger than the screen, shrink it
            if (size.width() < imgSize.width() && imgSize.width() > imgSize.height()) {
                int width = size.width();
                int height = width * scaledSize.height() / imgSize.width();
                scaledSize = QSize(width, height);
                pos = QPoint((size.width() - scaledSize.width()) / 2,
                        (size.height() - scaledSize.height()) / 2);
            } else if (size.height() < imgSize.height()) {
                int height = size.height();
                int width = height * imgSize.width() / imgSize.height();
                scaledSize = QSize(width, height);
                pos = QPoint((size.width() - scaledSize.width()) / 2,
                             (size.height() - scaledSize.height()) / 2);
            }

            break;
        }
        case Wallpaper::MaxpectResize: {
            float xratio = (float) size.width() / imgSize.width();
            float yratio = (float) size.height() / imgSize.height();
            if (xratio > yratio) {
                int height = size.height();
                int width = height * imgSize.width() / imgSize.height();
                scaledSize = QSize(width, height);
            } else {
                int width = size.width();
                int height = width * imgSize.height() / imgSize.width();
                scaledSize = QSize(width, height);
            }

            pos = QPoint((size.width() - scaledSize.width()) / 2,
                         (size.height() - scaledSize.height()) / 2);
            break;
        }
        case Wallpaper::ScaledAndCroppedResize: {
            float xratio = (float) size.width() / imgSize.width();
            float yratio = (float) size.height() / imgSize.height();
            if (xratio > yratio) {
                int width = size.width();
                int height = width * imgSize.height() / imgSize.width();
                scaledSize = QSize(width, height);
            } else {
                int height = size.height();
                int width = height * imgSize.width() / imgSize.height();
                scaledSize = QSize(width, height);
            }
            pos = QPoint((size.width() - scaledSize.width()) / 2,
                         (size.height() - scaledSize.height()) / 2);
            break;
        }
        case Wallpaper::TiledResize: {
            scaledSize = imgSize;
            tiled = true;
            break;
        }
        case Wallpaper::CenterTiledResize: {
            scaledSize = imgSize;
            pos = QPoint(-scaledSize.width() + ((size.width() - scaledSize.width()) / 2) % scaledSize.width(),
                         -scaledSize.height() + ((size.height() - scaledSize.height()) / 2) % scaledSize.height());
            tiled = true;
            break;
        }
    }

    QPainter p(&result);
    // kDebug() << sourceImagePath << scalable << scaledSize << imgSize;
    if (scaledSize != imgSize) {
        img = img.scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    if (tiled) {
        for (int x = pos.x(); x < size.width(); x += scaledSize.width()) {
            for (int y = pos.y(); y < size.height(); y += scaledSize.height()) {
                p.drawImage(QPoint(x, y), img);
            }
        }
    } else {
        p.drawImage(pos, img);
    }

    if (cacheRendering) {
        q->insertIntoCache(cacheKey(sourceImagePath, size, resizeMethod, color), result);
    }

    //kDebug() << "rendering complete!";
    emit q->renderCompleted(result);
}

WallpaperPrivate::WallpaperPrivate(KService::Ptr service, Wallpaper *wallpaper) :
    q(wallpaper),
    wallpaperDescription(service),
    lastResizeMethod(Wallpaper::ScaledResize),
    cacheRendering(false),
    initialized(false),
    needsConfig(false),
    previewing(false),
    needsPreviewDuringConfiguration(false)
{
}

QString WallpaperPrivate::cacheKey(const QString &sourceImagePath, const QSize &size,
                                   int resizeMethod, const QColor &color) const
{
    const QString id = QString("%5_%3_%4_%1x%2")
                              .arg(size.width()).arg(size.height()).arg(color.name())
                              .arg(resizeMethod).arg(sourceImagePath);
    return id;
}

QString WallpaperPrivate::cachePath(const QString &key) const
{
    return KGlobal::dirs()->locateLocal("cache", "plasma-wallpapers/" + key + "." + imageFormat);
}

bool WallpaperPrivate::findInCache(const QString &key, unsigned int lastModified)
{
    if (cacheRendering) {
        QString cache = cachePath(key);
        QFileInfo cacheinfo(cache);
        if (cacheinfo.exists() && cacheinfo.isFile()) {
            if (lastModified > 0) {
                if (cacheinfo.lastModified().toTime_t() < lastModified) {
                    return false;
                }
            }

            QImage image(cache, imageFormat);
            emit q->renderCompleted(image);

            return true;
        }
    }

    return false;
}

bool Wallpaper::findInCache(const QString &key, QImage &image, unsigned int lastModified)
{
    if (d->cacheRendering) {
        QString cache = d->cachePath(key);
        QFileInfo cacheinfo(cache);
        if (cacheinfo.exists() && cacheinfo.isFile()) {
            if (lastModified > 0) {
                if (cacheinfo.lastModified().toTime_t() < lastModified) {
                    return false;
                }
            }

            image.load(cache);
            return true;
        }
    }

    return false;
}

void Wallpaper::insertIntoCache(const QString& key, const QImage &image)
{
    //TODO: cache limits?
    if (key.isEmpty()) {
        return;
    }

    if (d->cacheRendering) {
        if (image.isNull()) {
#ifndef PLASMA_NO_KIO
            KIO::file_delete(d->cachePath(key));
#else
            QFile f(d->cachePath(key));
            f.remove();
#endif
        } else {
            image.save(d->cachePath(key), imageFormat, 100);
        }
    }
}

QList<QAction*> Wallpaper::contextualActions() const
{
    return d->contextActions;
}

void Wallpaper::setContextualActions(const QList<QAction*> &actions)
{
    d->contextActions = actions;
}

bool Wallpaper::isPreviewing() const
{
    return d->previewing;
}

void Wallpaper::setPreviewing(bool previewing)
{
    d->previewing = previewing;
}

bool Wallpaper::needsPreviewDuringConfiguration() const
{
    return d->needsPreviewDuringConfiguration;
}

void Wallpaper::setPreviewDuringConfiguration(const bool preview)
{
    d->needsPreviewDuringConfiguration = preview;
}

} // Plasma namespace

#include "moc_wallpaper.cpp"
