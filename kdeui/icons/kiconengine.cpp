/* This file is part of the KDE libraries
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kiconengine_p.h"
#include "kiconloader.h"
#include "kdebug.h"

#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QApplication>


KIconEngine::KIconEngine(const QString &iconName, KIconLoader* iconLoader, const QStringList &overlays)
    : mIconName(iconName),
      mOverlays(overlays), 
      mIconLoader(iconLoader)
{
    
}

KIconEngine::KIconEngine(const QString &iconName, KIconLoader* iconLoader)
    : mIconName(iconName),
    mIconLoader(iconLoader)
{
}

static inline int qIconModeToKIconState(QIcon::Mode mode)
{
    int kstate;
    switch (mode) {
    case QIcon::Active:
        kstate = KIconLoader::ActiveState;
        break;
    case QIcon::Disabled:
        kstate = KIconLoader::DisabledState;
        break;
    case QIcon::Normal:
    default:
        kstate = KIconLoader::DefaultState;
        break;
    }
    return kstate;
}

QSize KIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state);
    Q_UNUSED(mode);
    const int iconSize = qMin(size.width(), size.height());
    return QSize(iconSize, iconSize);
}

QList<QSize> KIconEngine::availableSizes(QIcon::Mode mode, QIcon::State state) const
{
    Q_UNUSED(mode);
    Q_UNUSED(state);

    // TODO: maybe cache via mAvailableSizes member
    static QList<QSize> avaiablesizes;
    if (mIconLoader && !mIconName.isEmpty() && avaiablesizes.isEmpty()) {
        static const int s_stdiconsizes[] = {
            KIconLoader::SizeSmall,
            KIconLoader::SizeSmallMedium,
            KIconLoader::SizeMedium,
            KIconLoader::SizeLarge,
            KIconLoader::SizeHuge,
            KIconLoader::SizeEnormous,
            0
        };
        int counter = 0;
        while (s_stdiconsizes[counter]) {
            const QString iconpath = mIconLoader.data()->iconPath(mIconName, -s_stdiconsizes[counter], true);
            if (!iconpath.isEmpty()) {
                avaiablesizes.append(QSize(s_stdiconsizes[counter], s_stdiconsizes[counter]));
            }
            counter++;
        }
    }
    return avaiablesizes;
}

void KIconEngine::paint(QPainter* painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state);

    if (!mIconLoader) {
        return;
    }

    const int kstate = qIconModeToKIconState(mode);
    KIconLoader::Group group = KIconLoader::Desktop;

    if (QWidget* targetWidget = dynamic_cast<QWidget*>(painter->device())) {
        if (qobject_cast<QMenu*>(targetWidget)) {
            group = KIconLoader::Small;
        } else if (qobject_cast<QToolBar*>(targetWidget->parent())) {
            group = KIconLoader::Toolbar;
        }
    }

    const int iconSize = qMin(rect.width(), rect.height());
    const QPixmap pix = mIconLoader.data()->loadIcon(mIconName, group, iconSize, kstate, mOverlays);
    painter->drawPixmap(rect, pix);
}

QPixmap KIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state);

    if (!mIconLoader) {
        return QPixmap();
    }

    const int kstate = qIconModeToKIconState(mode);
    const int iconSize = qMin(size.width(), size.height());
    const QPixmap pix = mIconLoader.data()->loadIcon(mIconName, KIconLoader::Desktop, iconSize, kstate, mOverlays);
    if (pix.isNull()) {
        return pix;
    }
    if (pix.size() == size) {
        return pix;
    }
    return pix.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QString KIconEngine::key() const
{
    return QString::fromLatin1("KIconEngine");
}

QString KIconEngine::iconName() const
{
    if (mIconName.contains(QLatin1Char('/'))) {
        // e.g. favicon, KIcon exclusive that QIcon::fromTheme() will not be able to load
        return QString();
    }
    return mIconName;
}

QIconEngineV2 *KIconEngine::clone() const
{
    return new KIconEngine(mIconName, mIconLoader.data(), mOverlays);
}

bool KIconEngine::read(QDataStream &in)
{
    in >> mIconName >> mOverlays;
    return true;
}

bool KIconEngine::write(QDataStream &out) const
{
    out << mIconName << mOverlays;
    return true;
}
