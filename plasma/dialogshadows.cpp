/*
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2, 
*   or (at your option) any later version.
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

#include "dialogshadows.h"
#include "kpixmap.h"
#include "kdebug.h"

#include <QWidget>
#include <QPainter>

#ifdef Q_WS_X11
#include <QtGui/qx11info_x11.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

namespace Plasma
{

class DialogShadowsPrivate
{
public:
    DialogShadowsPrivate(DialogShadows *shadows)
        : q(shadows)
    {
    }

    ~DialogShadowsPrivate()
    {
        freeX11Pixmaps();
    }

    void freeX11Pixmaps();
    void clearPixmaps();
    void setupPixmaps();
    void initPixmap(const QString &element);
    KPixmap initEmptyPixmap(const QSize &size);
    void updateShadow(const QWidget *window, Plasma::FrameSvg::EnabledBorders);
    void clearShadow(const QWidget *window);
    void updateShadows();
    void windowDestroyed(QObject *deletedObject);
    void setupData(Plasma::FrameSvg::EnabledBorders enabledBorders);

    DialogShadows *q;
    QList<KPixmap> m_shadowPixmaps;

    KPixmap m_emptyCornerPix;
    KPixmap m_emptyCornerLeftPix;
    KPixmap m_emptyCornerTopPix;
    KPixmap m_emptyCornerRightPix;
    KPixmap m_emptyCornerBottomPix;
    KPixmap m_emptyVerticalPix;
    KPixmap m_emptyHorizontalPix;

    QHash<Plasma::FrameSvg::EnabledBorders, QVector<unsigned long> > data;
    QHash<const QWidget *, Plasma::FrameSvg::EnabledBorders> m_windows;
};

DialogShadows::DialogShadows(QObject *parent, const QString &prefix)
    : Plasma::Svg(parent),
      d(new DialogShadowsPrivate(this))
{
    setImagePath(prefix);
    connect(this, SIGNAL(repaintNeeded()), this, SLOT(updateShadows()));
}

DialogShadows::~DialogShadows()
{
    delete d;
}

void DialogShadows::addWindow(const QWidget *window, Plasma::FrameSvg::EnabledBorders enabledBorders)
{
    if (!window || !window->isWindow()) {
        return;
    }

    d->m_windows[window] = enabledBorders;
    d->updateShadow(window, enabledBorders);
    connect(window, SIGNAL(destroyed(QObject*)),
            this, SLOT(windowDestroyed(QObject*)), Qt::UniqueConnection);
}

void DialogShadows::removeWindow(const QWidget *window)
{
    if (!d->m_windows.contains(window)) {
        return;
    }

    d->m_windows.remove(window);
    disconnect(window, 0, this, 0);
    d->clearShadow(window);

    if (d->m_windows.isEmpty()) {
        d->clearPixmaps();
    }
}

void DialogShadowsPrivate::windowDestroyed(QObject *deletedObject)
{
    m_windows.remove(static_cast<QWidget *>(deletedObject));

    if (m_windows.isEmpty()) {
        clearPixmaps();
    }
}

void DialogShadowsPrivate::updateShadows()
{
    setupPixmaps();
    QHashIterator<const QWidget *, Plasma::FrameSvg::EnabledBorders> it(m_windows);
    while (it.hasNext()) {
        it.next();
        updateShadow(it.key(), it.value());
    }
}

void DialogShadowsPrivate::initPixmap(const QString &element)
{
    m_shadowPixmaps << KPixmap(q->pixmap(element));
}

KPixmap DialogShadowsPrivate::initEmptyPixmap(const QSize &size)
{
    return KPixmap(size);
}

void DialogShadowsPrivate::setupPixmaps()
{
    clearPixmaps();
    initPixmap("shadow-top");
    initPixmap("shadow-topright");
    initPixmap("shadow-right");
    initPixmap("shadow-bottomright");
    initPixmap("shadow-bottom");
    initPixmap("shadow-bottomleft");
    initPixmap("shadow-left");
    initPixmap("shadow-topleft");

    m_emptyCornerPix = initEmptyPixmap(QSize(1,1));
    m_emptyCornerLeftPix = initEmptyPixmap(QSize(q->elementSize("shadow-topleft").width(), 1));
    m_emptyCornerTopPix = initEmptyPixmap(QSize(1, q->elementSize("shadow-topleft").height()));
    m_emptyCornerRightPix = initEmptyPixmap(QSize(q->elementSize("shadow-bottomright").width(), 1));
    m_emptyCornerBottomPix = initEmptyPixmap(QSize(1, q->elementSize("shadow-bottomright").height()));
    m_emptyVerticalPix = initEmptyPixmap(QSize(1, q->elementSize("shadow-left").height()));
    m_emptyHorizontalPix = initEmptyPixmap(QSize(q->elementSize("shadow-top").width(), 1));

}


void DialogShadowsPrivate::setupData(Plasma::FrameSvg::EnabledBorders enabledBorders)
{
#ifdef Q_WS_X11
    //shadow-top
    if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        data[enabledBorders] << m_shadowPixmaps[0].handle();
    } else {
        data[enabledBorders] << m_emptyHorizontalPix.handle();
    }

    //shadow-topright
    if (enabledBorders & Plasma::FrameSvg::TopBorder &&
        enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << m_shadowPixmaps[1].handle();
    } else if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        data[enabledBorders] << m_emptyCornerTopPix.handle();
    } else if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << m_emptyCornerRightPix.handle();
    } else {
        data[enabledBorders] << m_emptyCornerPix.handle();
    }

    //shadow-right
    if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << m_shadowPixmaps[2].handle();
    } else {
        data[enabledBorders] << m_emptyVerticalPix.handle();
    }

    //shadow-bottomright
    if (enabledBorders & Plasma::FrameSvg::BottomBorder &&
        enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << m_shadowPixmaps[3].handle();
    } else if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        data[enabledBorders] << m_emptyCornerBottomPix.handle();
    } else if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << m_emptyCornerRightPix.handle();
    } else {
        data[enabledBorders] << m_emptyCornerPix.handle();
    }

    //shadow-bottom
    if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        data[enabledBorders] << m_shadowPixmaps[4].handle();
    } else {
        data[enabledBorders] << m_emptyHorizontalPix.handle();
    }

    //shadow-bottomleft
    if (enabledBorders & Plasma::FrameSvg::BottomBorder &&
        enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << m_shadowPixmaps[5].handle();
    } else if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        data[enabledBorders] << m_emptyCornerBottomPix.handle();
    } else if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << m_emptyCornerLeftPix.handle();
    } else {
        data[enabledBorders] << m_emptyCornerPix.handle();
    }

    //shadow-left
    if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << m_shadowPixmaps[6].handle();
    } else {
        data[enabledBorders] << m_emptyVerticalPix.handle();
    }

    //shadow-topleft
    if (enabledBorders & Plasma::FrameSvg::TopBorder &&
        enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << m_shadowPixmaps[7].handle();
    } else if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        data[enabledBorders] << m_emptyCornerTopPix.handle();
    } else if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << m_emptyCornerLeftPix.handle();
    } else {
        data[enabledBorders] << m_emptyCornerPix.handle();
    }
#endif

    int left, top, right, bottom = 0;

    QSize marginHint;
    if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        marginHint = q->elementSize("shadow-hint-top-margin");
        if (marginHint.isValid()) {
            top = marginHint.height();
        } else {
            top = m_shadowPixmaps[0].height(); // top
        }
    } else {
        top = 1;
    }

    if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        marginHint = q->elementSize("shadow-hint-right-margin");
        if (marginHint.isValid()) {
            right = marginHint.width();
        } else {
            right = m_shadowPixmaps[2].width(); // right
        }
    } else {
        right = 1;
    }

    if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        marginHint = q->elementSize("shadow-hint-bottom-margin");
        if (marginHint.isValid()) {
            bottom = marginHint.height();
        } else {
            bottom = m_shadowPixmaps[4].height(); // bottom
        }
    } else {
        bottom = 1;
    }

    if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        marginHint = q->elementSize("shadow-hint-left-margin");
        if (marginHint.isValid()) {
            left = marginHint.width();
        } else {
            left = m_shadowPixmaps[6].width(); // left
        }
    } else {
        left = 1;
    }

    data[enabledBorders] << top << right << bottom << left;
}

void DialogShadowsPrivate::freeX11Pixmaps()
{
#ifdef Q_WS_X11
    if (!QX11Info::display()) {
        return;
    }

    foreach (KPixmap &pixmap, m_shadowPixmaps) {
        if (!pixmap.isNull()) {
            pixmap.release();
        }
    }

    if (!m_emptyCornerPix.isNull()) {
        m_emptyCornerPix.release();
    }
    if (!m_emptyCornerBottomPix.isNull()) {
        m_emptyCornerBottomPix.release();
    }
    if (!m_emptyCornerLeftPix.isNull()) {
        m_emptyCornerLeftPix.release();
    }
    if (!m_emptyCornerRightPix.isNull()) {
        m_emptyCornerRightPix.release();
    }
    if (!m_emptyCornerTopPix.isNull()) {
        m_emptyCornerTopPix.release();
    }
    if (!m_emptyVerticalPix.isNull()) {
        m_emptyVerticalPix.release();
    }
    if (!m_emptyHorizontalPix.isNull()) {
        m_emptyHorizontalPix.release();
    }
#endif
}

void DialogShadowsPrivate::clearPixmaps()
{
#ifdef Q_WS_X11
    freeX11Pixmaps();

    m_emptyCornerPix = KPixmap();
    m_emptyCornerBottomPix = KPixmap();
    m_emptyCornerLeftPix = KPixmap();
    m_emptyCornerRightPix = KPixmap();
    m_emptyCornerTopPix = KPixmap();
    m_emptyVerticalPix = KPixmap();
    m_emptyHorizontalPix = KPixmap();
#endif
    m_shadowPixmaps.clear();
    data.clear();
}

void DialogShadowsPrivate::updateShadow(const QWidget *window, Plasma::FrameSvg::EnabledBorders enabledBorders)
{
#ifdef Q_WS_X11
    if (m_shadowPixmaps.isEmpty()) {
        setupPixmaps();
    }

    if (!data.contains(enabledBorders)) {
        setupData(enabledBorders);
    }

    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_NET_WM_SHADOW", False);

    //kDebug() << "going to set the shadow of" << winId() << "to" << data;
    XChangeProperty(dpy, window->winId(), atom, XA_CARDINAL, 32, PropModeReplace,
                    reinterpret_cast<const unsigned char *>(data[enabledBorders].constData()), data[enabledBorders].size());
#endif
}

void DialogShadowsPrivate::clearShadow(const QWidget *window)
{
#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_NET_WM_SHADOW", False);
    XDeleteProperty(dpy, window->winId(), atom);
#endif
}

bool DialogShadows::enabled() const
{
     return hasElement("shadow-left");
}

} // Plasma namespace

#include "moc_dialogshadows.cpp"

