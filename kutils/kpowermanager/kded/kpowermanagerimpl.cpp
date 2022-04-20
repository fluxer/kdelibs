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

#include "kpowermanagerimpl.h"
#include "powermanagementadaptor.h"
#include "kdebug.h"

// TODO: BatteryRemainingTimeChanged

// for reference:
// https://consolekit2.github.io/ConsoleKit2/#Manager

KPowerManagerImpl::KPowerManagerImpl(QObject *parent)
    : QObject(parent),
    m_objectregistered(false),
    m_serviceregistered(false),
    m_login1("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus()),
    m_consolekit("org.freedesktop.ConsoleKit", "/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", QDBusConnection::systemBus()),
    m_canhibernate(false),
    m_canhybridsuspend(false),
    m_cansuspend(false),
    m_powersavestatus(false)
{
    (void)new PowerManagementAdaptor(this);

    QDBusConnection connection = QDBusConnection::sessionBus();

    const bool object = connection.registerObject("/org/freedesktop/PowerManagement", this);
    if (!object) {
        kWarning() << "Could not register object" << connection.lastError().message();
        return;
    }
    m_objectregistered = true;

    const bool service = connection.registerService("org.freedesktop.PowerManagement");
    if (!service) {
        kWarning() << "Could not register service" << connection.lastError().message();
        connection.unregisterObject("/org/freedesktop/PowerManagement");
        return;
    }
    m_serviceregistered = true;

    m_canhibernate = CanHibernate();
    m_canhybridsuspend = CanHybridSuspend();
    m_cansuspend = CanSuspend();
    m_powersavestatus = GetPowerSaveStatus();
    if (m_login1.isValid()) {
        connection = QDBusConnection::systemBus();
        connection.connect(
            "org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.DBus.Properties", "PropertiesChanged",
            this, SLOT(slotPropertiesChanged(QString,QVariantMap,QStringList))
        );
        connect(&m_login1, SIGNAL(PrepareForSleep(bool)), this, SLOT(slotPrepareForSleep(bool)));
    } else if (m_consolekit.isValid()) {
        connect(&m_consolekit, SIGNAL(PrepareForSleep(bool)), this, SLOT(slotPrepareForSleep(bool)));
    }
}

KPowerManagerImpl::~KPowerManagerImpl()
{
    if (m_serviceregistered) {
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.unregisterService("org.freedesktop.PowerManagement");
    }

    if (m_objectregistered) {
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.unregisterObject("/org/freedesktop/PowerManagement");
    }
}

bool KPowerManagerImpl::CanHibernate()
{
    if (m_login1.isValid()) {
        QDBusReply<QString> reply = m_login1.call("CanHibernate");
        return (reply.value() == QLatin1String("yes"));
    }
    if (m_consolekit.isValid()) {
        QDBusReply<QString> reply = m_consolekit.call("CanHibernate");
        return (reply.value() == QLatin1String("yes"));
    }
    return false;
}

bool KPowerManagerImpl::CanHybridSuspend()
{
    if (m_login1.isValid()) {
        QDBusReply<QString> reply = m_login1.call("CanHybridSleep");
        return (reply.value() == QLatin1String("yes"));
    }
    if (m_consolekit.isValid()) {
        QDBusReply<QString> reply = m_consolekit.call("CanHybridSleep");
        return (reply.value() == QLatin1String("yes"));
    }
    return false;
}

bool KPowerManagerImpl::CanSuspend()
{
    if (m_login1.isValid()) {
        QDBusReply<QString> reply = m_login1.call("CanSuspend");
        return (reply.value() == QLatin1String("yes"));
    }
    if (m_consolekit.isValid()) {
        QDBusReply<QString> reply = m_consolekit.call("CanSuspend");
        return (reply.value() == QLatin1String("yes"));
    }
    return false;
}

bool KPowerManagerImpl::GetPowerSaveStatus()
{
    if (!m_login1.isValid()) {
        return false;
    }
    return !m_login1.property("OnExternalPower").toBool();
}

void KPowerManagerImpl::Hibernate()
{
    if (m_login1.isValid()) {
        m_login1.asyncCall("Hibernate", true);
        return;
    }
    if (m_consolekit.isValid()) {
        m_consolekit.asyncCall("Hibernate", true);
    }
}

void KPowerManagerImpl::Suspend()
{
    if (m_login1.isValid()) {
        m_login1.asyncCall("Suspend", true);
        return;
    }
    if (m_consolekit.isValid()) {
        m_consolekit.asyncCall("Suspend", true);
    }
}

bool KPowerManagerImpl::isLidClosed()
{
    if (!m_login1.isValid()) {
        return false;
    }
    return m_login1.property("LidClosed").toBool();
}

void KPowerManagerImpl::slotPropertiesChanged(QString interface, QVariantMap changed_properties, QStringList invalidated_properties)
{
    Q_UNUSED(interface);
    Q_UNUSED(changed_properties);
    Q_UNUSED(invalidated_properties);

    const bool oldcanhibernate = m_canhibernate;
    const bool oldcanhybridsuspend = m_canhybridsuspend;
    const bool oldcansuspend = m_cansuspend;
    const bool oldpowersavestatus = m_powersavestatus;
    m_canhibernate = CanHibernate();
    m_canhybridsuspend = CanHybridSuspend();
    m_cansuspend = CanSuspend();
    m_powersavestatus = GetPowerSaveStatus();

    if (oldcanhibernate != m_canhibernate) {
        emit CanHibernateChanged(m_canhibernate);
    }
    if (oldcanhybridsuspend != m_canhybridsuspend) {
        emit CanHybridSuspendChanged(m_canhybridsuspend);
    }
    if (oldcansuspend != m_cansuspend) {
        emit CanSuspendChanged(m_cansuspend);
    }
    if (oldpowersavestatus != m_powersavestatus) {
        emit PowerSaveStatusChanged(m_powersavestatus);
    }
}

void KPowerManagerImpl::slotPrepareForSleep(bool start)
{
    if (!start) {
        emit ResumeFromSuspend();
    }
}

#include "moc_kpowermanagerimpl.cpp"
