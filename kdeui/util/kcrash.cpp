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

#include "config.h"
#include "kcrash.h"
#include "kcmdlineargs.h"
#include "kde_file.h"
#include "kaboutdata.h"
#include "kcomponentdata.h"
#include "kstandarddirs.h"
#include "kdebug.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QX11Info>

#include <signal.h>
#include <X11/Xlib.h>
#include "fixx11h.h"

// NOTE: keep in sync with:
// kde-workspace/kcrash/kded/kded_kcrash.cpp

// see kdebug.areas
static const int s_kcrasharea = 1410;

static KCrash::HandlerType s_crashhandler = nullptr;
static KCrash::CrashFlags s_crashflags = 0;
static QString s_crashtmp;
static QString s_crashfile;
static QMap<QByteArray,QString> s_crashdata;

static const int s_signals[] = {
#ifdef SIGSEGV
    SIGSEGV,
#endif
#ifdef SIGBUS
    SIGBUS,
#endif
#ifdef SIGFPE
    SIGFPE,
#endif
#ifdef SIGILL
    SIGILL,
#endif
#ifdef SIGABRT
    SIGABRT,
#endif
    0
};

void KCrash::setFlags(KCrash::CrashFlags flags)
{
    s_crashflags = flags;
    if (s_crashflags & KCrash::AutoRestart || s_crashflags & KCrash::Notify || s_crashflags & KCrash::Log) {
        // Default crash handler is required for the flags to work but one may be set already
        if (!s_crashhandler) {
            KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
            if (!args || args->isSet("crashhandler")) {
                setCrashHandler(defaultCrashHandler);
            }
        }

        if (s_crashtmp.isEmpty()) {
            const QString tmppath = KGlobal::dirs()->saveLocation("tmp", "kcrash/");
            s_crashtmp = QString::fromLatin1("%1/%2_%3.tmp").arg(
                tmppath,
                QCoreApplication::applicationName(),
                QString::number(QCoreApplication::applicationPid())
            );
            s_crashfile = s_crashtmp;
            s_crashfile.chop(4);
            s_crashfile.append(QLatin1String(".kcrash"));
        }

        if (s_crashdata.isEmpty()) {
            // start up on the correct display
            const char* dpy = nullptr;
            if (QX11Info::display()) {
                dpy = XDisplayString(QX11Info::display());
            } else {
                dpy = ::getenv("DISPLAY");
            }
            if (dpy) {
                s_crashdata["display"] = QString::fromLatin1(dpy);
            }
            s_crashdata["appname"] = QCoreApplication::applicationName();
            s_crashdata["apppath"] = QCoreApplication::applicationFilePath();
            s_crashdata["pid"] = QString::number(QCoreApplication::applicationPid());
            const KComponentData kcomponentdata = KGlobal::mainComponent();
            const KAboutData *kaboutdata = kcomponentdata.isValid() ? kcomponentdata.aboutData() : nullptr;
            if (kaboutdata) {
                s_crashdata["appversion"] = kaboutdata->version();
                s_crashdata["programname"] = kaboutdata->programName();
                s_crashdata["bugaddress"] = kaboutdata->bugAddress();
                s_crashdata["homepage"] = kaboutdata->homepage();
            }
        }
        // flags may be updated
        s_crashdata["flags"] = QString::number(int(s_crashflags));
    }
}

KCrash::CrashFlags KCrash::flags()
{
    return s_crashflags;
}

void KCrash::setCrashHandler(HandlerType handler)
{
    if (!handler) {
        handler = SIG_DFL;
    }

    s_crashhandler = handler;

    sigset_t handlermask;
    ::sigemptyset(&handlermask);
    int counter = 0;
    while (s_signals[counter]) {
        KDE_signal(s_signals[counter], s_crashhandler);
        ::sigaddset(&handlermask, s_signals[counter]);
        counter++;
    }
    ::sigprocmask(SIG_UNBLOCK, &handlermask, NULL);
}

KCrash::HandlerType KCrash::crashHandler()
{
    return s_crashhandler;
}

void KCrash::defaultCrashHandler(int sig)
{
    KDE_signal(sig, SIG_DFL);

    const QByteArray crashtrace = kBacktrace();
    {
        QFile crashfile(s_crashtmp);
        if (!crashfile.open(QFile::WriteOnly)) {
            kError(s_kcrasharea) << "Could not open" << s_crashtmp;
            ::exit(sig);
            return;
        }
        QDataStream crashstream(&crashfile);
        crashstream << s_crashdata;
        crashstream << sig;
        crashstream << crashtrace;
    }
    QFile::rename(s_crashtmp, s_crashfile);

    ::exit(sig);
}
