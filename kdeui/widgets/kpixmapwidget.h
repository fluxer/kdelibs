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

#ifndef KPIXMAPWIDGET_H
#define KPIXMAPWIDGET_H

#include <kdeui_export.h>

#include <QWidget>
#include <QPixmap>
#include <QPaintEvent>
#include <QDropEvent>
#include <QMouseEvent>

class KPixmapWidgetPrivate;

/*!
    Class to show a pixmap with drag-n-drop support.

    @note neither drag nor drop is enabled by default, call @p setDragEnabled and
    @p setAcceptDrops to enable the features
    @since 4.24
    @warning the API is subject to change
*/
class KDEUI_EXPORT KPixmapWidget: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool dragEnabled READ dragEnabled WRITE setDragEnabled)
public:
    KPixmapWidget(QWidget *parent = nullptr);
    ~KPixmapWidget();

    void setPixmap(const QPixmap &pixmap);
    QPixmap pixmap() const;

    Qt::Alignment alignment() const;
    void setAlignment(const Qt::Alignment alignment);

    bool dragEnabled() const;
    void setDragEnabled(const bool enable);

    // QWidget reimplementations
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:
    // QWidget reimplementations
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

private:
    Q_DISABLE_COPY(KPixmapWidget);
    KPixmapWidgetPrivate *d;
};

#endif //  KPIXMAPWIDGET_H
