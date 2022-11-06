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

#include <QX11Info>
#include <QApplication>
#include <QThread>

#define KSELECTIONOWNER_TIMEOUT 500
#define KSELECTIONOWNER_SLEEPTIME 500
#define KSELECTIONOWNER_CHECKTIME 250

class KSelectionOwnerPrivate
{
public:
    KSelectionOwnerPrivate();

    Atom x11atom;
    Display* x11display;
    int x11screen;
    Window x11window;
    int timerid;
};

KSelectionOwnerPrivate::KSelectionOwnerPrivate()
    : x11atom(None),
    x11display(QX11Info::display()),
    x11screen(QX11Info::appScreen()),
    x11window(None),
    timerid(0)
{
}


KSelectionOwner::KSelectionOwner(const char* const atomname, const int screen, QObject *parent)
    : QObject(parent),
    d(new KSelectionOwnerPrivate())
{
    d->x11atom = XInternAtom(d->x11display, atomname, False);
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
    kDebug() << "Current selection owner is" << d->x11window;
    return d->x11window;
}

bool KSelectionOwner::claim(const bool force)
{
    Window currentowner = XGetSelectionOwner(d->x11display, d->x11atom);
    if (currentowner != None && !force) {
        kDebug() << "Selection is owned";
        return false;
    }
    if (currentowner != None) {
        kDebug() << "Selection is owned, clearing owner";
        XSetSelectionOwner(d->x11display, d->x11atom, None, CurrentTime);
        XFlush(d->x11display);
        ushort counter = 0;
        kDebug() << "Waiting for owner";
        while (currentowner != None && counter < 10) {
            currentowner = XGetSelectionOwner(d->x11display, d->x11atom);
            QCoreApplication::processEvents(QEventLoop::AllEvents, KSELECTIONOWNER_TIMEOUT);
            QThread::msleep(KSELECTIONOWNER_SLEEPTIME);
            counter++;
        }
    }
    if (currentowner != None) {
        kDebug() << "Selection is owned, killing owner";
        KXErrorHandler kx11errorhandler;
        XKillClient(d->x11display, currentowner);
        XFlush(d->x11display);
        if (kx11errorhandler.error(true)) {
            kWarning() << KXErrorHandler::errorMessage(kx11errorhandler.errorEvent());
            return false;
        }
    }
    kDebug() << "Creating selection owner";
    d->x11window = XCreateSimpleWindow(
        d->x11display, RootWindow(d->x11display, d->x11screen),
        0, 0, // x and y
        1, 1, // width and height
        0, 0, 0 // border width, border and background pixels
    );
    XSetSelectionOwner(d->x11display, d->x11atom, d->x11window, CurrentTime);
    XFlush(d->x11display);
    d->timerid = startTimer(KSELECTIONOWNER_CHECKTIME);
    return true;
}

void KSelectionOwner::release()
{
    if (d->x11window == None) {
        kDebug() << "No owner";
        return;
    }
    if (d->timerid > 0) {
        killTimer(d->timerid);
        d->timerid = 0;
    }
    kDebug() << "Destroying owner window";
    XDestroyWindow(d->x11display, d->x11window);
    XSetSelectionOwner(d->x11display, d->x11atom, None, CurrentTime);
    XFlush(d->x11display);
    d->x11window = None;
}

void KSelectionOwner::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d->timerid) {
        // kDebug() << "Checking selection owner for" << XGetAtomName(d->x11display, d->x11atom);
        Q_ASSERT(d->x11window != None);
        Window currentowner = XGetSelectionOwner(d->x11display, d->x11atom);
        if (currentowner != d->x11window) {
            kDebug() << "Selection owner changed";
            killTimer(d->timerid);
            d->timerid = 0;
            emit lostOwnership();
            // NOTE: catching errors here is done to not get fatal I/O
            KXErrorHandler kx11errorhandler;
            XDestroyWindow(d->x11display, d->x11window);
            XFlush(d->x11display);
            d->x11window = None;
            if (kx11errorhandler.error(true)) {
                kDebug() << KXErrorHandler::errorMessage(kx11errorhandler.errorEvent());
            }
        }
        event->accept();
    } else {
        event->ignore();
    }
}
