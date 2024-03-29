/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "windoweffects.h"
#include "theme.h"

#include <kwindowsystem.h>

#include <vector>

#ifdef Q_WS_X11
#  include <X11/Xlib.h>
#  include <X11/Xatom.h>
#  include <X11/Xutil.h>
#  include <QtGui/qx11info_x11.h>
#endif

namespace Plasma
{

namespace WindowEffects
{

//FIXME: check if this works for any atom?
bool isEffectAvailable(Effect effect)
{
    if (!Plasma::Theme::defaultTheme()->windowTranslucencyEnabled()) {
        return false;
    }
#ifdef Q_WS_X11
    // hackish way to find out if KWin has the effect enabled,
    // TODO provide proper support

    Display *dpy = QX11Info::display();
    Atom atom = None;
    switch (effect) {
        case Slide: {
            atom = XInternAtom(dpy, "_KDE_SLIDE", False);
            break;
        }
        case WindowPreview: {
            atom = XInternAtom(dpy, "_KDE_WINDOW_PREVIEW", False);
            break;
        }
        case PresentWindows: {
            atom = XInternAtom(dpy, "_KDE_PRESENT_WINDOWS_DESKTOP", False);
            break;
        }
        case PresentWindowsGroup: {
            atom = XInternAtom(dpy, "_KDE_PRESENT_WINDOWS_GROUP", False);
            break;
        }
        case HighlightWindows: {
            atom = XInternAtom(dpy, "_KDE_WINDOW_HIGHLIGHT", False);
            break;
        }
        default: {
            return false;
        }
    }

    int cnt = 0;
    Atom *list = XListProperties(dpy, DefaultRootWindow(dpy), &cnt);
    if (list != NULL) {
        bool ret = (qFind(list, list + cnt, atom) != list + cnt);
        XFree(list);
        return ret;
    }
#endif
    return false;
}

void slideWindow(WId id, Plasma::Location location, int offset)
{
#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_SLIDE", False);
    std::vector<long> data(2);

    data[0] = offset;

    switch (location) {
    case LeftEdge:
        data[1] = 0;
        break;
    case TopEdge:
        data[1] = 1;
        break;
    case RightEdge:
        data[1] = 2;
        break;
    case BottomEdge:
        data[1] = 3;
    default:
        break;
    }

    if (location == Desktop || location == Floating) {
        XDeleteProperty(dpy, id, atom);
    } else {
        XChangeProperty(dpy, id, atom, atom, 32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(data.data()), data.size());
    }
#endif
}

void slideWindow(QWidget *widget, Plasma::Location location)
{
    WindowEffects::slideWindow(widget->effectiveWinId(), location, -1);
}

QList<QSize> windowSizes(const QList<WId> &ids)
{
    QList<QSize> windowSizes;
    foreach (WId id, ids) {
#ifdef Q_WS_X11
        if (id > 0) {
            KWindowInfo info = KWindowSystem::windowInfo(id, NET::WMGeometry|NET::WMFrameExtents);
            windowSizes.append(info.frameGeometry().size());
        } else {
            windowSizes.append(QSize());
        }
#else
        windowSizes.append(QSize());
#endif
    }
    return windowSizes;
}

void showWindowThumbnails(WId parent, const QList<WId> &windows, const QList<QRect> &rects)
{
    if (windows.size() != rects.size()) {
        return;
    }
#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_WINDOW_PREVIEW", False);
    if (windows.isEmpty()) {
        XDeleteProperty(dpy, parent, atom);
        return;
    }

    int numWindows = windows.size();

    // 64 is enough for 10 windows and is a nice base 2 number
    std::vector<long> data(1 + (6 * numWindows));
    data[0] = numWindows;

    QList<QRect>::const_iterator rectsIt = rects.constBegin();
    int i = 0;
    foreach(const WId windowsIt, windows) {

        const int start = (i * 6) + 1;
        const QRect thumbnailRect = (*rectsIt);

        data[start] = 5;
        data[start+1] = windowsIt;
        data[start+2] = thumbnailRect.x();
        data[start+3] = thumbnailRect.y();
        data[start+4] = thumbnailRect.width();
        data[start+5] = thumbnailRect.height();
        ++rectsIt;
        ++i;
    }

    XChangeProperty(dpy, parent, atom, atom, 32, PropModeReplace,
                    reinterpret_cast<unsigned char *>(data.data()), data.size());
#endif
}

void presentWindows(WId controller, const QList<WId> &ids)
{
#ifdef Q_WS_X11
    const int numWindows = ids.count();
    std::vector<long> data(numWindows);
    for (int i = 0; i < numWindows; ++i) {
        data[i] = ids.at(i);
    }
    if (data.size() > 0) {
        Display *dpy = QX11Info::display();
        Atom atom = XInternAtom(dpy, "_KDE_PRESENT_WINDOWS_GROUP", False);
        XChangeProperty(dpy, controller, atom, atom, 32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(data.data()), data.size());
    }
#endif
}

void presentWindows(WId controller, int desktop)
{
#ifdef Q_WS_X11
    std::vector<long> data(1);
    data[0] = desktop;
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_PRESENT_WINDOWS_DESKTOP", False);
    XChangeProperty(dpy, controller, atom, atom, 32, PropModeReplace,
                    reinterpret_cast<unsigned char *>(data.data()), data.size());
#endif
}

void highlightWindows(WId controller, const QList<WId> &ids)
{
#ifdef Q_WS_X11
    const int numWindows = ids.count();
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_WINDOW_HIGHLIGHT", False);

    if (numWindows == 0) {
        XDeleteProperty(dpy, controller, atom);
        return;
    }

    std::vector<long> data(numWindows);
    for (int i = 0; i < numWindows; ++i) {
        data[i] = ids.at(i);

    }
    if (data.size() > 0) {
        XChangeProperty(dpy, controller, atom, atom, 32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(data.data()), data.size());
    }
#endif
}

}

}
