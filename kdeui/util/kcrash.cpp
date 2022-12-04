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

#include "kcrash.h"
#include "kcmdlineargs.h"
#include "kde_file.h"
#include "kaboutdata.h"
#include "kcomponentdata.h"
#include "kstandarddirs.h"
#include "kdebug.h"

#include <QCoreApplication>
#include <QProcess>
#include <QX11Info>

#include <signal.h>
#include <X11/Xlib.h>
#include "fixx11h.h"

static KCrash::HandlerType s_crashHandler = nullptr;
static KCrash::CrashFlags s_flags = 0;

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
    s_flags = flags;
    if (s_flags & KCrash::AutoRestart || s_flags & KCrash::DrKonqi) {
        // Default crash handler is required for the flags to work but one may be set already
        if (!s_crashHandler) {
            KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
            if (args->isSet("crashhandler")) {
                setCrashHandler(defaultCrashHandler);
            }
        }
    }
}

KCrash::CrashFlags KCrash::flags()
{
    return s_flags;
}

void KCrash::setCrashHandler(HandlerType handler)
{
    if (!handler) {
        handler = SIG_DFL;
    }

    s_crashHandler = handler;

    int counter = 0;
    while (s_signals[counter]) {
        KDE_signal(s_signals[counter], s_crashHandler);
        counter++;
    }
}

KCrash::HandlerType KCrash::crashHandler()
{
    return s_crashHandler;
}

void KCrash::defaultCrashHandler(int sig)
{
    KDE_signal(sig, SIG_DFL);

#if 0
    kFatal() << QCoreApplication::applicationName() << "crashed (" << QCoreApplication::applicationPid() << ")";
    return;
#endif

    if (s_flags & KCrash::AutoRestart) {
        QStringList args;

        // start up on the correct display
        const char* dpy = nullptr;
        if (QX11Info::display()) {
            dpy = XDisplayString(QX11Info::display());
        } else {
            dpy = ::getenv("DISPLAY");
        }

        if (dpy) {
            args.append(QString::fromLatin1("-display"));
            args.append(QString::fromLatin1(dpy));
        }

        QProcess::startDetached(QCoreApplication::applicationFilePath(), args);
    } else if (s_flags & KCrash::DrKonqi) {
        const QString drkonqiexe = KStandardDirs::findExe(QString::fromLatin1("drkonqi"));
        if (drkonqiexe.isEmpty()) {
            return;
        }

        QStringList args;

        args.append(QString::fromLatin1("--signal"));
        args.append(QString::number(sig));
        args.append(QString::fromLatin1("--appname"));
        args.append(QCoreApplication::applicationName());
        args.append(QString::fromLatin1("--apppath"));
        args.append(QCoreApplication::applicationFilePath());
        args.append(QString::fromLatin1("--pid"));
        args.append(QString::number(QCoreApplication::applicationPid()));

        const KComponentData kcomponentdata = KGlobal::mainComponent();
        const KAboutData *kaboutdata = kcomponentdata.isValid() ? kcomponentdata.aboutData() : nullptr;
        if (kaboutdata) {
            if (kaboutdata->internalVersion()) {
                args.append(QString::fromLatin1("--appversion"));
                args.append(kaboutdata->internalVersion());
            }

            if (kaboutdata->internalProgramName()) {
                args.append(QString::fromLatin1("--programname"));
                args.append(kaboutdata->internalProgramName());
            }

            if (kaboutdata->internalBugAddress()) {
                args.append(QString::fromLatin1("--bugaddress"));
                args.append(kaboutdata->internalBugAddress());
            }
        }

        QProcess::execute(drkonqiexe, args);
    }

    ::exit(sig);
}
