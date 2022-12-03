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

#include "klauncher.h"
#include "klauncher_adaptor.h"
#include "kaboutdata.h"
#include "kdeversion.h"
#include "kapplication.h"
#include "kcmdlineargs.h"
#include "kdebug.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>

KLauncher::KLauncher(QObject *parent)
    : QObject(parent)
{
    m_adaptor = new KLauncherAdaptor(this);

    QDBusConnection session = QDBusConnection::sessionBus();
    session.registerObject("/KLauncher", this);
    session.registerService("org.kde.klauncher");
}

KLauncher::~KLauncher()
{
    QDBusConnection session = QDBusConnection::sessionBus();
    session.unregisterObject("/KLauncher");
    session.unregisterService("org.kde.klauncher");
}

void KLauncher::setSessionManager(const QByteArray &sessionmanager)
{
    const int equalindex = sessionmanager.indexOf('=');
    kDebug() << "SESSION_MANAGER" << sessionmanager;
    if (equalindex > 0) {
        const QByteArray sessionmanagervalue = sessionmanager.mid(equalindex + 1, sessionmanager.size() - equalindex - 1);
        kDebug() << "SESSION_MANAGER" << sessionmanager << sessionmanagervalue;
        m_adaptor->setLaunchEnv(QString::fromLatin1("SESSION_MANAGER"), sessionmanagervalue);
    }
}

int main(int argc, char *argv[])
{
    KAboutData aboutData(
        "klauncher", "kdelibs4", ki18n("KDE launcher"),
        KDE_VERSION_STRING,
        ki18n("KDE launcher - launches and autostarts applications")
    );

    KCmdLineArgs::init(argc, argv, &aboutData);

    const QByteArray sessionmanager = qgetenv("SESSION_MANAGER");

    // NOTE: disables session manager entirely, for reference:
    // https://www.x.org/releases/X11R7.7/doc/libSM/xsmp.html
    ::unsetenv("SESSION_MANAGER");

    KApplication app;
    app.setQuitOnLastWindowClosed(false);
    app.disableSessionManagement();
    app.quitOnSignal();
    
    QDBusConnection session = QDBusConnection::sessionBus();
    if (!session.isConnected()) {
        kWarning() << "No DBUS session-bus found. Check if you have started the DBUS server.";
        return 1;
    }
    QDBusReply<bool> sessionReply = session.interface()->isServiceRegistered("org.kde.klauncher");
    if (sessionReply.isValid() && sessionReply.value() == true) {
        kWarning() << "Another instance of klauncher is already running!";
        return 2;
    }

    KLauncher klauncher(&app);
    klauncher.setSessionManager(sessionmanager);

    return app.exec(); // keep running
}

#include "moc_klauncher.cpp"
