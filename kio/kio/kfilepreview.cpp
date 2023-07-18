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
    static const QString kfppluginservice("KFilePreview/Plugin");
    QStringList result;
    foreach (const QString &servicetype, servicetypes) {
        if (servicetype.isEmpty() || servicetype == kfppluginservice) {
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
};

KFilePreview::KFilePreview(QObject *parent)
    : QObject(parent),
    d(new KFilePreviewPrivate())
{
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
    const KService::List kfpplugins = KServiceTypeTrader::self()->query("KFilePreview/Plugin");
    foreach (const KService::Ptr &kfpplugin, kfpplugins) {
        const QString kfpname = kfpplugin->desktopEntryName();
        const bool enable = pluginsgroup.readEntry(kfpname, true);
        if (enable) {
            result.append(kPreviewGlobMimeTypes(kfpplugin->serviceTypes()));
        }
    }
    result.removeDuplicates();
    qSort(result);
    return result;
}

QImage KFilePreview::preview(const KFileItem &item, const QSize &size)
{
    // TODO: caching of previews the size of which is equal or less than 256x256

    KConfig config("kfilepreviewrc", KConfig::NoGlobals);
    KConfigGroup previewgroup = config.group("PreviewSettings");
    const int iconsize = previewgroup.readEntry("IconSize", KIconLoader::global()->currentSize(KIconLoader::Desktop));
    const int iconalpha = previewgroup.readEntry("IconAlpha", int(PreviewDefaults::IconAlpha));
    KConfigGroup pluginsgroup = config.group("Plugins");
    const KMimeType::Ptr itemmimetype = item.determineMimeType();
    const KService::List kfpplugins = KServiceTypeTrader::self()->query("KFilePreview/Plugin");
    foreach (const KService::Ptr &kfpplugin, kfpplugins) {
        const QString kfpname = kfpplugin->desktopEntryName();
        const bool enable = pluginsgroup.readEntry(kfpname, true);
        if (enable) {
            foreach (const QString &kfppluginmime, kPreviewGlobMimeTypes(kfpplugin->serviceTypes())) {
                bool mimematches = false;
                if (kfppluginmime.endsWith('*')) {
                    const QString kfppluginmimeglob = kfppluginmime.mid(0, kfppluginmime.size() - 1);
                    if (itemmimetype && itemmimetype->name().startsWith(kfppluginmimeglob)) {
                        mimematches = true;
                    }
                }

                if (!mimematches && itemmimetype->is(kfppluginmime)) {
                    mimematches = true;
                }

                if (mimematches) {
                    int kfpmaximumsize = 0;
                    if (item.isLocalFile()) {
                        kfpmaximumsize = kfpplugin->property("MaximumLocalSize", QVariant::Int).toInt();
                        if (kfpmaximumsize <= 0) {
                            kfpmaximumsize = (PreviewDefaults::MaxLocalSize * 1024 * 1024);
                        }
                    } else {
                        kfpmaximumsize = kfpplugin->property("MaximumRemoteSize", QVariant::Int).toInt();
                        if (kfpmaximumsize <= 0) {
                            kfpmaximumsize = (PreviewDefaults::MaxRemoteSize * 1024 * 1024);
                        }
                    }
                    if (item.size() >= KIO::filesize_t(kfpmaximumsize)) {
                        kDebug() << "Item size too big for" << item.url() << kfpname << item.size() << kfpmaximumsize;
                        continue;
                    }
                }

                if (mimematches) {
                    kDebug() << "Creating preview via" << kfpname;
                    KFilePreviewPlugin *kfpplugininstance = kfpplugin->createInstance<KFilePreviewPlugin>();
                    if (kfpplugininstance) {
                        QImage result = kfpplugininstance->preview(item.url(), size);
                        const bool kfpiconoverlay = kfpplugin->property("X-KDE-IconOverlay", QVariant::Bool).toBool();
                        if (kfpiconoverlay && itemmimetype && KIconLoader::global()->alphaBlending(KIconLoader::Desktop)) {
                            const QPixmap iconoverlay = KIconLoader::global()->loadMimeTypeIcon(
                                itemmimetype->iconName(), KIconLoader::Desktop, iconsize
                            );
                            kPreviewOverlay(result, iconoverlay.toImage(), iconalpha);
                        }
                        delete kfpplugininstance;
                        return result;
                    } else {
                        kWarning() << "Could not create KFilePreviewPlugin instance";
                    }
                }
            }
        }
    }
    return QImage();
}

#include "moc_kfilepreview.cpp"
