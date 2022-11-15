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

#include "kselectionowner.h"
#include "kxerrorhandler.h"
#include "kdebug.h"
#include "netwm.h"

#include <QApplication>
#include <QThread>

#include <unistd.h>
#include <signal.h>

#define KSELECTIONOWNER_TIMEOUT 500
#define KSELECTIONOWNER_SLEEPTIME 500
#define KSELECTIONOWNER_CHECKTIME 250

static Window kWaitForOwner(Display* x11display, const Atom x11atom, Window currentowner)
{
    ushort counter = 0;
    while (currentowner != None && counter < 10) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KSELECTIONOWNER_TIMEOUT);
        QThread::msleep(KSELECTIONOWNER_SLEEPTIME);
        counter++;
        currentowner = XGetSelectionOwner(x11display, x11atom);
    }
    return currentowner;
}

class KSelectionOwnerPrivate
{
public:
    KSelectionOwnerPrivate();
    ~KSelectionOwnerPrivate();

    QByteArray atomname;
    Atom x11atom;
    Display* x11display;
    int x11screen;
    Window x11window;
    int timerid;
};

KSelectionOwnerPrivate::KSelectionOwnerPrivate()
    : x11atom(None),
    x11display(nullptr),
    x11screen(0),
    x11window(None),
    timerid(0)
{
    x11display = XOpenDisplay(NULL);
    if (!x11display) {
        kFatal() << "Could not open X11 display";
    }
    x11screen = DefaultScreen(x11display);
}

KSelectionOwnerPrivate::~KSelectionOwnerPrivate()
{
    if (x11display) {
        XCloseDisplay(x11display);
    }
}

KSelectionOwner::KSelectionOwner(const char* const atomname, const int screen, QObject *parent)
    : QObject(parent),
    d(new KSelectionOwnerPrivate())
{
    d->atomname = atomname;
    d->x11atom = XInternAtom(d->x11display, d->atomname.constData(), False);
    if (screen >= 0) {
        d->x11screen = screen;
    }
}

KSelectionOwner::~KSelectionOwner()
{
    release();
    delete d;
}

Window KSelectionOwner::ownerWindow() const
{
    kDebug(240) << "Current" << d->atomname << "owner is" << d->x11window;
    return d->x11window;
}

bool KSelectionOwner::claim(const bool force)
{
    Window currentowner = XGetSelectionOwner(d->x11display, d->x11atom);
    if (currentowner != None && !force) {
        kDebug(240) << d->atomname << "is owned";
        return false;
    }
    if (currentowner != None) {
        kDebug(240) << d->atomname << "is owned, killing owner via kill()";
        KXErrorHandler kx11errorhandler(d->x11display);
        KXErrorHandler handler;
        NETWinInfo netwininfo(d->x11display, currentowner, QX11Info::appRootWindow(), NET::WMPid);
        if (kx11errorhandler.error(true)) {
            kWarning(240) << KXErrorHandler::errorMessage(kx11errorhandler.errorEvent());
            return false;
        }
        const int currentownerpid = netwininfo.pid();
        if (currentownerpid > 0) {
            ::kill(currentownerpid, SIGKILL);
            ::kill(currentownerpid, SIGTERM);
            // ::kill(currentownerpid, YOUSHALLDIEONEWAYORTHEOTHER);
            kDebug(240) << "Waiting for" << d->atomname << "owner";
            currentowner = kWaitForOwner(d->x11display, d->x11atom, currentowner);
        } else {
            kDebug(240) << "Invalid" << d->atomname << "PID";
        }
    }
    if (currentowner != None) {
        kDebug(240) << d->atomname << "is owned, killing owner via XKillClient()";
        KXErrorHandler kx11errorhandler(d->x11display);
        XKillClient(d->x11display, currentowner);
        XFlush(d->x11display);
        kDebug(240) << "Waiting for" << d->atomname << "owner";
        currentowner = kWaitForOwner(d->x11display, d->x11atom, currentowner);
    }
    if (currentowner != None) {
        kWarning(240) << d->atomname << "is still owned";
        return false;
    }
    kDebug(240) << "Creating" << d->atomname << "owner";
    KXErrorHandler kx11errorhandler(d->x11display);
    d->x11window = XCreateSimpleWindow(
        d->x11display, RootWindow(d->x11display, d->x11screen),
        0, 0, // x and y
        1, 1, // width and height
        0, 0, 0 // border width, border and background pixels
    );
    if (kx11errorhandler.error(true)) {
        kWarning(240) << KXErrorHandler::errorMessage(kx11errorhandler.errorEvent());
        return false;
    }
    NETWinInfo netwininfo(d->x11display, d->x11window, QX11Info::appRootWindow(), NET::WMPid);
    netwininfo.setPid(::getpid());
    XSelectInput(d->x11display, d->x11window, NoEventMask);
    XSetSelectionOwner(d->x11display, d->x11atom, d->x11window, CurrentTime);
    XFlush(d->x11display);
    d->timerid = startTimer(KSELECTIONOWNER_CHECKTIME);
    return true;
}

void KSelectionOwner::release()
{
    if (d->x11window == None) {
        kDebug(240) << "No" << d->atomname << "owner";
        return;
    }
    if (d->timerid > 0) {
        killTimer(d->timerid);
        d->timerid = 0;
    }
    kDebug(240) << "Destroying" << d->atomname << "owner window";
    KXErrorHandler kx11errorhandler(d->x11display);
    XDestroyWindow(d->x11display, d->x11window);
    XFlush(d->x11display);
    if (kx11errorhandler.error(true)) {
        kWarning(240) << KXErrorHandler::errorMessage(kx11errorhandler.errorEvent());
    }
    d->x11window = None;
}

void KSelectionOwner::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d->timerid) {
        // kDebug(240) << "Checking owner for" << d->atomname;
        Q_ASSERT(d->x11window != None);
        Window currentowner = XGetSelectionOwner(d->x11display, d->x11atom);
        if (currentowner != d->x11window) {
            kDebug(240) << d->atomname << "owner changed";
            killTimer(d->timerid);
            d->timerid = 0;
            emit lostOwnership();
            // NOTE: catching errors here is done to not get fatal I/O
            KXErrorHandler kx11errorhandler(d->x11display);
            XDestroyWindow(d->x11display, d->x11window);
            XFlush(d->x11display);
            d->x11window = None;
            if (kx11errorhandler.error(true)) {
                kDebug(240) << KXErrorHandler::errorMessage(kx11errorhandler.errorEvent());
            }
        }
        event->accept();
    } else {
        event->ignore();
    }
}
