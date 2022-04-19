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

#include "kpowermanagerinhibitimpl.h"
#include "inhibitadaptor.h"
#include "kdebug.h"

#include <unistd.h>
#include <limits.h>

// TODO: fallback to ConsoleKit

// for refrence:
// https://www.freedesktop.org/wiki/Software/systemd/inhibit/
// https://consolekit2.github.io/ConsoleKit2/#Manager

KPowerManagerInhibitImpl::KPowerManagerInhibitImpl(QObject *parent)
    : QObject(parent),
    m_objectregistered(false),
    m_serviceregistered(false),
    m_login1("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus()),
    m_hasinhibit(false)
{
    (void)new InhibitAdaptor(this);

    QDBusConnection connection = QDBusConnection::sessionBus();

    const bool object = connection.registerObject("/org/freedesktop/PowerManagement/Inhibit", this);
    if (!object) {
        kWarning() << "Could not register object" << connection.lastError().message();
        return;
    }
    m_objectregistered = true;

    const bool service = connection.registerService("org.freedesktop.PowerManagement.Inhibit");
    if (!service) {
        kWarning() << "Could not register service" << connection.lastError().message();
        connection.unregisterObject("/org/freedesktop/PowerManagement/Inhibit");
        return;
    }
    m_serviceregistered = true;

    m_hasinhibit = HasInhibit();
    if (m_login1.isValid()) {
        connection = QDBusConnection::systemBus();
        connection.connect(
            "org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.DBus.Properties", "PropertiesChanged",
            this, SLOT(slotPropertiesChanged(QString,QVariantMap,QStringList))
        );
    }
}

KPowerManagerInhibitImpl::~KPowerManagerInhibitImpl()
{
    if (m_serviceregistered) {
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.unregisterService("org.freedesktop.PowerManagement.Inhibit");
    }

    if (m_objectregistered) {
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.unregisterObject("/org/freedesktop/PowerManagement/Inhibit");
    }
}

bool KPowerManagerInhibitImpl::HasInhibit()
{
    if (!m_login1.isValid()) {
        return false;
    }
    return (m_login1.property("NCurrentInhibitors").toULongLong() > 0);
}

uint KPowerManagerInhibitImpl::Inhibit(const QString &application, const QString &reason)
{
    if (!m_login1.isValid()) {
        return 0;
    }
    uint cookiecounter = 0;
    const qulonglong maxinhibitors = m_login1.property("InhibitorsMax").toULongLong();
    if (maxinhibitors == 0) {
        kWarning() << "Inhibit limit is zero";
        return 0;
    }
    while (m_cookies.contains(cookiecounter)) {
        if (cookiecounter >= maxinhibitors) {
            kWarning() << "Inhibit limit reached";
            return 0;
        }
        cookiecounter++;
    }

    QDBusReply<QDBusUnixFileDescriptor> reply = m_login1.call(
        "Inhibit",
        "sleep",
        application, reason,
        "block"
    );
    if (reply.isValid()) {
        // qDebug() << Q_FUNC_INFO << cookiecounter;
        const int inhibitfd = reply.value().takeFileDescriptor();
        m_cookies.insert(cookiecounter, inhibitfd);
        return cookiecounter;
    }
    kWarning() << "Invalid reply";
    return 0;
}

void KPowerManagerInhibitImpl::UnInhibit(uint cookie)
{
    if (!m_login1.isValid()) {
        return;
    }
    if (!m_cookies.contains(cookie)) {
        kWarning() << "Attempt to UnInhibit with invalid cookie";
        return;
    }
    m_cookies.remove(cookie);
    // qDebug() << Q_FUNC_INFO << cookie;
    ::close(m_cookies.value(cookie));
}

void KPowerManagerInhibitImpl::slotPropertiesChanged(QString interface, QVariantMap changed_properties, QStringList invalidated_properties)
{
    Q_UNUSED(interface);
    Q_UNUSED(changed_properties);
    Q_UNUSED(invalidated_properties);

    bool oldhasinhibit = HasInhibit();

    m_hasinhibit = HasInhibit();

    if (oldhasinhibit != m_hasinhibit) {
        emit HasInhibitChanged(m_hasinhibit);
    }
}
#include "moc_kpowermanagerinhibitimpl.cpp"
