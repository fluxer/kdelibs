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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <fixx11h.h>

#include "kpixmap.h"
#include "kxerrorhandler.h"
#include "kdebug.h"

#include <QX11Info>
#include <QPainter>

class KPixmapPrivate
{
public:
    KPixmapPrivate();

    QSize size;
    Qt::HANDLE handle;
};

KPixmapPrivate::KPixmapPrivate()
    : handle(XNone)
{
}

KPixmap::KPixmap()
    : d(new KPixmapPrivate())
{
}

KPixmap::KPixmap(const Qt::HANDLE pixmap)
    : d(new KPixmapPrivate())
{
    d->handle = pixmap;
    if (pixmap) {
        Window x11window;
        int x11x = 0;
        int x11y = 0;
        uint x11width = 0;
        uint x11height = 0;
        uint x11borderwidth = 0;
        uint x11depth = 0;
        KXErrorHandler kx11errorhandler;
        XGetGeometry(
            QX11Info::display(), pixmap,
            &x11window, &x11x, &x11y,
            &x11width, &x11height,
            &x11borderwidth, &x11depth
        );
        d->size = QSize(x11width, x11height);
        if (kx11errorhandler.error(true)) {
            kWarning(240) << KXErrorHandler::errorMessage(kx11errorhandler.errorEvent());
            // in the worst case the pixmap will be leaked
            d->handle = XNone;
            d->size = QSize();
        }
    }
}

KPixmap::KPixmap(const QPixmap &pixmap)
    : d(new KPixmapPrivate())
{
    if (!pixmap.isNull()) {
        d->handle = pixmap.toX11Pixmap();
        if (!d->handle) {
            kWarning(240) << "Could not convert pixmap";
            return;
        }
        d->size = pixmap.size();
    }
}

KPixmap::KPixmap(const KPixmap &pixmap)
    : d(new KPixmapPrivate(*pixmap.d))
{
}

KPixmap::KPixmap(const QSize &size)
    : d(new KPixmapPrivate())
{
    if (!size.isEmpty()) {
        KXErrorHandler kx11errorhandler;
        d->handle = XCreatePixmap(
            QX11Info::display(), QX11Info::appRootWindow(),
            size.width(), size.height(),
            32
        );
        if (kx11errorhandler.error(true)) {
            kWarning(240) << KXErrorHandler::errorMessage(kx11errorhandler.errorEvent());
            d->handle = XNone;
            return;
        }
        d->size = size;
        GC x11gc = XCreateGC(QX11Info::display(), d->handle, 0, 0);
        XSetForeground(QX11Info::display(), x11gc, XBlackPixel(QX11Info::display(), QX11Info::appScreen()));
        XFillRectangle(QX11Info::display(), d->handle, x11gc, 0, 0, size.width(), size.height());
        XFreeGC(QX11Info::display(), x11gc);
    }
}

KPixmap::~KPixmap()
{
    delete d;
}

bool KPixmap::isNull() const
{
    return (d->handle == XNone);
}

QSize KPixmap::size() const
{
    return d->size;
}

Qt::HANDLE KPixmap::handle() const
{
    return d->handle;
}

void KPixmap::release()
{
    if (d->handle == XNone) {
        kDebug(240) << "No handle";
        return;
    }
    // NOTE: catching errors here is done to not get fatal I/O
    KXErrorHandler kx11errorhandler;
    XFreePixmap(QX11Info::display(), d->handle);
    if (kx11errorhandler.error(true)) {
        kWarning(240) << KXErrorHandler::errorMessage(kx11errorhandler.errorEvent());
    }
    d->handle = XNone;
    d->size = QSize();
}

QImage KPixmap::toImage() const
{
    if (isNull()) {
        kWarning(240) << "Null pixmap";
        return QImage();
    }
    return QPixmap::fromX11Pixmap(d->handle).toImage();
}

KPixmap& KPixmap::operator=(const KPixmap &other)
{
    d->handle = other.d->handle;
    d->size = other.d->size;
    return *this;
}
