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

#ifndef KICONENGINE_H
#define KICONENGINE_H

#include <kdeui_export.h>

#include <QtGui/qiconengine.h>

class KIconLoader;

/**
 * \short A class to provide rendering of KDE icons.
 *
 * Currently, this class is not much more than a wrapper around QIconEngine.
 * However, it should not be difficult to extend with features such as SVG
 * rendered icons.
 *
 * @author Hamish Rodda <rodda@kde.org>
 */
class KIconEngine : public QIconEngineV2
{
  public:
    /**
     * Constructs an icon engine for a KDE named icon.
     *
     * @param iconName the name of the icon to load
     * @param iconLoader The KDE icon loader that this engine is to use.
     * @param overlays Add one or more overlays to the icon. See KIconLoader::Overlays.
     *
     * @sa KIconLoader
     */
    KIconEngine(const QString &iconName, KIconLoader* iconLoader, const QStringList &overlays);

    /**
     * \overload
     */
    KIconEngine(const QString &iconName, KIconLoader* iconLoader);

    /// Reimplementations
    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) final;
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) final;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) final;

    QString key() const final;
    QIconEngineV2 *clone() const final;
    bool read(QDataStream &in) final;
    bool write(QDataStream &out) const final;

  private:
    QString mIconName;
    QStringList mOverlays;
    QWeakPointer<KIconLoader> mIconLoader;
};

#endif // KICONENGINE_H
