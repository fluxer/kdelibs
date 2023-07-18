/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "kfilepreview.h"
#include "kfilepreviewplugin.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kservicetypetrader.h"
#include "kservice.h"
#include "kiconloader.h"
#include "kdebug.h"

#include <QPainter>

enum PreviewDefaults {
    MaxLocalSize = 20, // 20 MB
    MaxRemoteSize = 5, // 5 MB
    IconAlpha = 125
};

// NOTE: same as kdelibs/kio/kio/kfilemetainfo.cpp except the service string
static QStringList kPreviewGlobMimeTypes(const QStringList &servicetypes)
{
    static const QString pluginservice("KFilePreview/Plugin");
    QStringList result;
    foreach (const QString &servicetype, servicetypes) {
        if (servicetype.isEmpty() || servicetype == pluginservice) {
            continue;
        }
        result.append(servicetype);
    }
    return result;
}

static void kPreviewOverlay(QImage &preview, const QImage &overlay, const int overlayalpha)
{
    // TODO: ratio check
    QPainter painter(&preview);
    const int x = qMax(preview.width() - overlay.width() - 4, 0);
    const int y = qMax(preview.height() - overlay.height() - 4, 0);
    painter.setOpacity(qreal(overlayalpha) / qreal(255.0));
    painter.drawImage(x, y, overlay);
}

class KFilePreviewPrivate
{
public:
    int iconsize;
    int iconalpha;
    KService::List plugins;
    QStringList enabledplugins;
};

KFilePreview::KFilePreview(QObject *parent)
    : QObject(parent),
    d(new KFilePreviewPrivate())
{
    KConfig config("kfilepreviewrc", KConfig::NoGlobals);
    KConfigGroup previewgroup = config.group("PreviewSettings");
    d->iconsize = previewgroup.readEntry("IconSize", KIconLoader::global()->currentSize(KIconLoader::Desktop));
    d->iconalpha = previewgroup.readEntry("IconAlpha", int(PreviewDefaults::IconAlpha));
    d->plugins = KServiceTypeTrader::self()->query("KFilePreview/Plugin");
    KConfigGroup pluginsgroup = config.group("Plugins");
    foreach (const KService::Ptr &plugin, d->plugins) {
        const QString pluginname = plugin->desktopEntryName();
        const bool pluginenabled = pluginsgroup.readEntry(pluginname, true);
        if (pluginenabled) {
            d->enabledplugins.append(pluginname);
        }
    }       
}

KFilePreview::~KFilePreview()
{
    delete d;
}

QStringList KFilePreview::supportedMimeTypes()
{
    QStringList result;
    KConfig config("kfilepreviewrc", KConfig::NoGlobals);
    KConfigGroup pluginsgroup = config.group("Plugins");
    const KService::List plugins = KServiceTypeTrader::self()->query("KFilePreview/Plugin");
    foreach (const KService::Ptr &plugin, plugins) {
        const QString pluginname = plugin->desktopEntryName();
        const bool pluginenabled = pluginsgroup.readEntry(pluginname, true);
        if (pluginenabled) {
            result.append(kPreviewGlobMimeTypes(plugin->serviceTypes()));
        }
    }
    result.removeDuplicates();
    qSort(result);
    return result;
}

QImage KFilePreview::preview(const KFileItem &item, const QSize &size)
{
    // TODO: caching of previews the size of which is equal or less than 256x256

    const KMimeType::Ptr itemmimetype = item.determineMimeType();
    foreach (const KService::Ptr &plugin, d->plugins) {
        const QString pluginname = plugin->desktopEntryName();
        if (!d->enabledplugins.contains(pluginname)) {
            continue;
        }

        foreach (const QString &pluginmime, kPreviewGlobMimeTypes(plugin->serviceTypes())) {
            bool mimematches = false;
            if (pluginmime.endsWith('*')) {
                const QString pluginmimeglob = pluginmime.mid(0, pluginmime.size() - 1);
                if (itemmimetype && itemmimetype->name().startsWith(pluginmimeglob)) {
                    mimematches = true;
                }
            }

            if (!mimematches && itemmimetype->is(pluginmime)) {
                mimematches = true;
            }

            if (mimematches) {
                int pluginmaximumsize = 0;
                if (item.isLocalFile()) {
                    pluginmaximumsize = plugin->property("MaximumLocalSize", QVariant::Int).toInt();
                    if (pluginmaximumsize <= 0) {
                        pluginmaximumsize = (PreviewDefaults::MaxLocalSize * 1024 * 1024);
                    }
                } else {
                    pluginmaximumsize = plugin->property("MaximumRemoteSize", QVariant::Int).toInt();
                    if (pluginmaximumsize <= 0) {
                        pluginmaximumsize = (PreviewDefaults::MaxRemoteSize * 1024 * 1024);
                    }
                }
                if (item.size() >= KIO::filesize_t(pluginmaximumsize)) {
                    kDebug() << "Item size too big for" << item.url() << pluginname << item.size() << pluginmaximumsize;
                    continue;
                }
            }

            if (mimematches) {
                kDebug() << "Creating preview via" << pluginname;
                KFilePreviewPlugin *plugininstance = plugin->createInstance<KFilePreviewPlugin>();
                if (plugininstance) {
                    QImage result = plugininstance->preview(item.url(), size);
                    const bool kfpiconoverlay = plugin->property("X-KDE-IconOverlay", QVariant::Bool).toBool();
                    if (kfpiconoverlay && itemmimetype && KIconLoader::global()->alphaBlending(KIconLoader::Desktop)) {
                        const QPixmap iconoverlay = KIconLoader::global()->loadMimeTypeIcon(
                            itemmimetype->iconName(), KIconLoader::Desktop, d->iconsize
                        );
                        kPreviewOverlay(result, iconoverlay.toImage(), d->iconalpha);
                    }
                    delete plugininstance;
                    return result;
                } else {
                    kWarning() << "Could not create KFilePreviewPlugin instance";
                }
            }
        }
    }
    return QImage();
}

#include "moc_kfilepreview.cpp"
