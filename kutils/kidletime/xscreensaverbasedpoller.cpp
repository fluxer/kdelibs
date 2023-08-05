/* This file is part of the KDE libraries
   Copyright (C) 2009 Dario Freddi <drf at kde.org>

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

#include "xscreensaverbasedpoller.h"

#include <QtGui/qx11info_x11.h>

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

XScreensaverBasedPoller::XScreensaverBasedPoller(QWidget *parent)
    : WidgetBasedPoller(parent)
{

}

XScreensaverBasedPoller::~XScreensaverBasedPoller()
{
}

int XScreensaverBasedPoller::getIdleTime()
{
    XScreenSaverInfo * mitInfo = 0;
    mitInfo = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(QX11Info::display(), DefaultRootWindow(QX11Info::display()), mitInfo);
    int ret = mitInfo->idle;
    XFree( mitInfo );
    return ret;
}

void XScreensaverBasedPoller::simulateUserActivity()
{
    stopCatchingIdleEvents();
    XResetScreenSaver(QX11Info::display());
    emit resumingFromIdle();
}

#include "moc_xscreensaverbasedpoller.cpp"
