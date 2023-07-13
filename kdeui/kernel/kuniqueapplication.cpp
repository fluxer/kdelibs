/* This file is part of the KDE libraries
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kuniqueapplication.h"
#include "kuniqueapplication_p.h"
#include "kmainwindow.h"
#include "kcmdlineargs.h"
#include "kaboutdata.h"
#include "kconfiggroup.h"
#include "kconfig.h"
#include "kstartupinfo.h"
#include "kdebug.h"

#include <QList>
#include <QTimer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <sys/types.h>
#include <unistd.h>

#if defined Q_WS_X11
#  include <X11/Xlib.h>
#endif


bool KUniqueApplication::Private::s_multipleInstances = false;
bool s_kuniqueapplication_startCalled = false;

bool KUniqueApplication::start(StartFlags flags)
{
    if (s_kuniqueapplication_startCalled) {
        return true;
    }
    s_kuniqueapplication_startCalled = true;

    QString appName = KCmdLineArgs::aboutData()->appName();
    const QStringList parts = KCmdLineArgs::aboutData()->organizationDomain().split(QLatin1Char('.'), QString::SkipEmptyParts);
    if (parts.isEmpty()) {
        appName.prepend(QLatin1String("local."));
    } else {
        foreach (const QString &s, parts) {
            appName.prepend(QLatin1Char('.'));
            appName.prepend(s);
        }
    }

    // Check the D-Bus connection health
    QDBusConnectionInterface* dbusService = nullptr;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.isConnected() || !(dbusService = sessionBus.interface())) {
        kError() << "KUniqueApplication: Cannot find the D-Bus session server: " << sessionBus.lastError().message();
        ::exit(255);
    }

    if (Private::s_multipleInstances || flags & KUniqueApplication::NonUniqueInstance) {
        appName = appName + '-' + QString::number(::getpid());
    }

    // Check to make sure that we're actually able to register with the D-Bus session
    // server.
    bool registered = dbusService->registerService(appName) == QDBusConnectionInterface::ServiceRegistered;
    if (!registered) {
        kError() << "KUniqueApplication: Can't setup D-Bus service. Probably already running.";
        ::exit(255);
    }

    // We'll call newInstance in the constructor. Do nothing here.
    return true;
}


KUniqueApplication::KUniqueApplication(bool configUnique)
    : KApplication(Private::initHack(configUnique)),
    d(new Private(this))
{
    d->firstInstance = true;

    // the sanity checking happened in initHack
    new KUniqueApplicationAdaptor(this);

    // Can't call newInstance directly from the constructor since it's virtual...
    QTimer::singleShot(0, this, SLOT(_k_newInstance()));
}


#ifdef Q_WS_X11
KUniqueApplication::KUniqueApplication(Display *display, Qt::HANDLE visual,
                                       Qt::HANDLE colormap, bool configUnique)
    : KApplication(display, visual, colormap, Private::initHack(configUnique)),
    d(new Private(this))
{
    d->firstInstance = true;

    // the sanity checking happened in initHack
    new KUniqueApplicationAdaptor(this);

    // Can't call newInstance directly from the constructor since it's virtual...
    QTimer::singleShot(0, this, SLOT(_k_newInstance()));
}
#endif

KUniqueApplication::~KUniqueApplication()
{
    delete d;
}

// this gets called before even entering QApplication::QApplication()
KComponentData KUniqueApplication::Private::initHack(bool configUnique)
{
    KComponentData cData(KCmdLineArgs::aboutData());
    if (configUnique) {
        KConfigGroup cg(cData.config(), "KDE");
        s_multipleInstances = cg.readEntry("MultipleInstances", false);
    }
    if (!KUniqueApplication::start()) {
        // Already running
        ::exit(0);
    }
    return cData;
}

void KUniqueApplication::Private::_k_newInstance()
{
    q->newInstance();
    firstInstance = false;
}

bool KUniqueApplication::restoringSession()
{
    return d->firstInstance && isSessionRestored();
}

int KUniqueApplication::newInstance()
{
    if (!d->firstInstance) {
        QList<KMainWindow*> allWindows = KMainWindow::memberList();
        if (!allWindows.isEmpty()) {
            // This method is documented to only work for applications
            // with only one mainwindow.
            KMainWindow* mainWindow = allWindows.first();
            if (mainWindow) {
                mainWindow->show();
#ifdef Q_WS_X11
                // This is the line that handles window activation if necessary,
                // and what's important, it does it properly. If you reimplement newInstance(),
                // and don't call the inherited one, use this (but NOT when newInstance()
                // is called for the first time, like here).
                KStartupInfo::setNewStartupId(mainWindow, startupId());
#endif

            }
        }
    }
    // do nothing in default implementation
    return 0;
}

////
int KUniqueApplicationAdaptor::newInstance(const QByteArray &asn_id, const QByteArray &args)
{
    if (!asn_id.isEmpty()) {
        parent()->setStartupId(asn_id);
    }

    QDataStream ds(args);
    KCmdLineArgs::loadAppArgs(ds);

    int ret = parent()->newInstance();
    // Must be done out of the newInstance code, in case it is overloaded
    parent()->d->firstInstance = false;
    return ret;
}

#include "moc_kuniqueapplication.cpp"
#include "moc_kuniqueapplication_p.cpp"
