/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>
    Copyright 2013 Lukas Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "powermanagement.h"
#include "powermanagement_p.h"

#include "soliddefs_p.h"

#include <QtCore/QCoreApplication>
#include <QtDBus/QDBusPendingCall>
#include <QtDBus/QDBusConnectionInterface>

Q_GLOBAL_STATIC(Solid::PowerManagementPrivate, globalPowerManager)

Solid::PowerManagementPrivate::PowerManagementPrivate()
    : managerIface(new QDBusInterface(QLatin1String("org.freedesktop.PowerManagement"),
                   QLatin1String("/org/freedesktop/PowerManagement"),
                   QLatin1String("org.freedesktop.PowerManagement"),
                   QDBusConnection::sessionBus())),
    inhibitIface(QLatin1String("org.freedesktop.PowerManagement.Inhibit"),
                 QLatin1String("/org/freedesktop/PowerManagement/Inhibit"),
                 QLatin1String("org.freedesktop.PowerManagement.Inhibit"),
                 QDBusConnection::sessionBus()),
    saverIface(QLatin1String("org.freedesktop.ScreenSaver"),
               QLatin1String("/ScreenSaver"),
               QLatin1String("org.freedesktop.ScreenSaver"),
               QDBusConnection::sessionBus()),
    serviceWatcher(QLatin1String("org.freedesktop.PowerManagement"),
                   QDBusConnection::sessionBus(),
                   QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration),
    powerSaveStatus(false)
{
    if (managerIface->isValid()) {
        connect(managerIface, SIGNAL(CanSuspendChanged(bool)),
                this, SLOT(slotCanSuspendChanged(bool)));
        connect(managerIface, SIGNAL(CanHibernateChanged(bool)),
                this, SLOT(slotCanHibernateChanged(bool)));
        connect(managerIface, SIGNAL(CanHybridSuspendChanged(bool)),
                this, SLOT(slotCanHybridSuspendChanged(bool)));
        connect(managerIface, SIGNAL(PowerSaveStatusChanged(bool)),
                this, SLOT(slotPowerSaveStatusChanged(bool)));
    }
    connect(&serviceWatcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(slotServiceRegistered(QString)));
    connect(&serviceWatcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(slotServiceUnregistered(QString)));

    // If the service is registered, trigger the connection immediately
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.freedesktop.PowerManagement"))) {
        slotServiceRegistered(QLatin1String("org.freedesktop.PowerManagement"));
    }
}

Solid::PowerManagementPrivate::~PowerManagementPrivate()
{
    if (managerIface) {
        managerIface->deleteLater();
    }
}

Solid::PowerManagement::Notifier::Notifier()
{
}

bool Solid::PowerManagement::appShouldConserveResources()
{
    return globalPowerManager()->powerSaveStatus;
}

QSet<Solid::PowerManagement::SleepState> Solid::PowerManagement::supportedSleepStates()
{
    return globalPowerManager()->supportedSleepStates;
}

void Solid::PowerManagement::requestSleep(SleepState state)
{
    if (!globalPowerManager()->supportedSleepStates.contains(state)) {
        return;
    }

    Q_ASSERT(managerIface);
    switch (state) {
        case SuspendState: {
            globalPowerManager()->managerIface->asyncCall("Suspend");
            break;
        }
        case HibernateState: {
            globalPowerManager()->managerIface->asyncCall("Hibernate");
            break;
        }
        case HybridSuspendState: {
            globalPowerManager()->managerIface->asyncCall("HybridSuspend");
            break;
        }
    }
}

uint Solid::PowerManagement::beginSuppressingSleep(const QString &reason)
{
    QDBusReply<uint> reply = globalPowerManager()->inhibitIface.call("Inhibit", QCoreApplication::applicationName(), reason);
    if (reply.isValid()) {
        return reply.value();
    }
    return 0;
}

bool Solid::PowerManagement::stopSuppressingSleep(uint cookie)
{
    if (globalPowerManager()->inhibitIface.isValid()) {
        globalPowerManager()->inhibitIface.asyncCall("UnInhibit", cookie);
        return true;
    }
    return false;
}

uint Solid::PowerManagement::beginSuppressingScreenPowerManagement(const QString& reason)
{
    if (globalPowerManager()->saverIface.isValid()) {
        QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.ScreenSaver"),
                                                              QLatin1String("/ScreenSaver"),
                                                              QLatin1String("org.freedesktop.ScreenSaver"),
                                                              QLatin1String("Inhibit"));
        message << QCoreApplication::applicationName();
        message << reason;

        QDBusReply<uint> ssReply = QDBusConnection::sessionBus().call(message);
        if (ssReply.isValid()) {
            globalPowerManager()->screensaverCookies.append(ssReply.value());
        }

        return ssReply.value();
    }
    // No way to fallback on something, hence return failure (0 is invalid cookie)
    return 0;
}

bool Solid::PowerManagement::stopSuppressingScreenPowerManagement(uint cookie)
{
    if (globalPowerManager()->saverIface.isValid()) {
        if (globalPowerManager()->screensaverCookies.contains(cookie)) {
            QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.ScreenSaver"),
                                                                  QLatin1String("/ScreenSaver"),
                                                                  QLatin1String("org.freedesktop.ScreenSaver"),
                                                                  QLatin1String("UnInhibit"));
            message << cookie;
            QDBusReply<void> ssReply = QDBusConnection::sessionBus().call(message);
            if (ssReply.isValid()) {
                globalPowerManager()->screensaverCookies.removeAll(cookie);
            }
        }

        return true;
    }
    // No way to fallback on something, hence return failure
    return false;
}

Solid::PowerManagement::Notifier *Solid::PowerManagement::notifier()
{
    return globalPowerManager();
}

void Solid::PowerManagementPrivate::slotCanSuspendChanged(bool newState)
{
    if (supportedSleepStates.contains(Solid::PowerManagement::SuspendState) == newState) {
        return;
    }

    if (newState) {
        supportedSleepStates += Solid::PowerManagement::SuspendState;
    } else {
        supportedSleepStates -= Solid::PowerManagement::SuspendState;
    }

    emit supportedSleepStatesChanged();
}

void Solid::PowerManagementPrivate::slotCanHibernateChanged(bool newState)
{
    if (supportedSleepStates.contains(Solid::PowerManagement::HibernateState) == newState) {
        return;
    }

    if (newState) {
        supportedSleepStates += Solid::PowerManagement::HibernateState;
    } else {
        supportedSleepStates -= Solid::PowerManagement::HibernateState;
    }

    emit supportedSleepStatesChanged();
}

void Solid::PowerManagementPrivate::slotCanHybridSuspendChanged(bool newState)
{
    if (supportedSleepStates.contains(Solid::PowerManagement::HybridSuspendState) == newState) {
        return;
    }

    if (newState) {
        supportedSleepStates += Solid::PowerManagement::HybridSuspendState;
    } else {
        supportedSleepStates -= Solid::PowerManagement::HybridSuspendState;
    }

    emit supportedSleepStatesChanged();
}

void Solid::PowerManagementPrivate::slotPowerSaveStatusChanged(bool newState)
{
    if (powerSaveStatus == newState) {
        return;
    }

    powerSaveStatus = newState;
    emit appShouldConserveResourcesChanged(powerSaveStatus);
}

void Solid::PowerManagementPrivate::slotResumeFromSuspend()
{
    emit resumingFromSuspend();
}

void Solid::PowerManagementPrivate::slotServiceRegistered(const QString &serviceName)
{
    if (serviceName == QLatin1String("org.freedesktop.PowerManagement")) {
        Q_ASSERT(!managerIface);
        managerIface = new QDBusInterface(
            QLatin1String("org.freedesktop.PowerManagement"),
            QLatin1String("/org/freedesktop/PowerManagement"),
            QLatin1String("org.freedesktop.PowerManagement"),
            QDBusConnection::sessionBus()
        );

        // Load all the properties
        QDBusReply<bool> suspendReply = managerIface->call("CanSuspend");
        slotCanSuspendChanged(suspendReply.isValid() ? suspendReply.value() : false);

        QDBusReply<bool> hibernateReply = managerIface->call("CanHibernate");
        slotCanHibernateChanged(hibernateReply.isValid() ? hibernateReply.value() : false);

        QDBusReply<bool> hybridSuspendReply = managerIface->call("CanHybridSuspend");
        slotCanHybridSuspendChanged(hybridSuspendReply.isValid() ? hybridSuspendReply.value() : false);

        QDBusReply<bool> saveStatusReply = managerIface->call("GetPowerSaveStatus");
        slotPowerSaveStatusChanged(saveStatusReply.isValid() ? saveStatusReply.value() : false);

        connect(managerIface, SIGNAL(CanSuspendChanged(bool)),
                this, SLOT(slotCanSuspendChanged(bool)));
        connect(managerIface, SIGNAL(CanHibernateChanged(bool)),
                this, SLOT(slotCanHibernateChanged(bool)));
        connect(managerIface, SIGNAL(CanHybridSuspendChanged(bool)),
                this, SLOT(slotCanHybridSuspendChanged(bool)));
        connect(managerIface, SIGNAL(PowerSaveStatusChanged(bool)),
                this, SLOT(slotPowerSaveStatusChanged(bool)));
        connect(managerIface, SIGNAL(ResumeFromSuspend()),
                this, SLOT(slotResumeFromSuspend()));
    }
}

void Solid::PowerManagementPrivate::slotServiceUnregistered(const QString &serviceName)
{
    if (serviceName == QLatin1String("org.freedesktop.PowerManagement")) {
        disconnect(managerIface, SIGNAL(CanSuspendChanged(bool)),
                   this, SLOT(slotCanSuspendChanged(bool)));
        disconnect(managerIface, SIGNAL(CanHibernateChanged(bool)),
                   this, SLOT(slotCanHibernateChanged(bool)));
        disconnect(managerIface, SIGNAL(CanHybridSuspendChanged(bool)),
                   this, SLOT(slotCanHybridSuspendChanged(bool)));
        disconnect(managerIface, SIGNAL(PowerSaveStatusChanged(bool)),
                   this, SLOT(slotPowerSaveStatusChanged(bool)));
        disconnect(managerIface, SIGNAL(ResumeFromSuspend()),
                   this, SLOT(slotResumeFromSuspend()));
        managerIface->deleteLater();
        managerIface = nullptr;

        // Reset the values
        slotCanSuspendChanged(false);
        slotCanHibernateChanged(false);
        slotCanHybridSuspendChanged(false);
        slotPowerSaveStatusChanged(false);
    }
}

#include "moc_powermanagement_p.cpp"
#include "moc_powermanagement.cpp"

