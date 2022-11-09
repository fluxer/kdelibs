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

#ifndef KPIXMAP_H
#define KPIXMAP_H

#include <kdeui_export.h>

#include <QImage>
#include <QPixmap>

class KPixmapPrivate;

/*!
    Class to deal with X11 pixmaps.
*/
class KDEUI_EXPORT KPixmap
{
public:
    /*!
        @brief Constructs null object.
    */
    KPixmap();
    /*!
        @brief Constructs object from X11 pixmap @p pixmap, the @p pixmap is not deep-copied.
    */
    KPixmap(const Qt::HANDLE pixmap);
    /*!
        @brief Constructs object from QPixmap @p pixmap, the @p pixmap is deep-copied and must be released.
    */
    KPixmap(const QPixmap &pixmap);
    /*!
        @brief Constructs object from other KPixmap @p pixmap, the @p pixmap is not deep-copied.
    */
    KPixmap(const KPixmap &pixmap);
    /*!
        @brief Constructs object for X11 pixmap with size @p size, the @p pixmap must be released.
    */
    KPixmap(const QSize &size);
    ~KPixmap();

    bool isNull() const;
    QSize size() const;
    int width() const { return size().width(); }
    int height() const { return size().height(); }
    Qt::HANDLE handle() const;

    /*!
        @brief Releases the X11 pixmap if not null.
    */
    void release();
    /*!
        @brief Returns QImage copy of the X11 pixmap.
    */
    QImage toImage() const;

    KPixmap &operator=(const KPixmap &other);

private:
    KPixmapPrivate* d;
};

#endif // KPIXMAP_H
