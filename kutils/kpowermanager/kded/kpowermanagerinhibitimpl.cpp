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

// for refrence:
// https://www.freedesktop.org/wiki/Software/systemd/inhibit/
// https://consolekit2.github.io/ConsoleKit2/#Manager

struct KInhibitor {
    QString what;
    QString who;
    QString why;
    QString mode;
    uint uid;
    uint pid;
};
Q_DECLARE_METATYPE(KInhibitor);
Q_DECLARE_METATYPE(QList<KInhibitor>);

QDBusArgument& operator<<(QDBusArgument &argument, const KInhibitor &kinhibitor)
{
    argument.beginStructure();
    argument << kinhibitor.what << kinhibitor.who << kinhibitor.why << kinhibitor.mode << kinhibitor.uid << kinhibitor.pid;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument &argument, KInhibitor &kinhibitor)
{
    argument.beginStructure();
    argument >> kinhibitor.what >> kinhibitor.who >> kinhibitor.why >> kinhibitor.mode >> kinhibitor.uid >> kinhibitor.pid;
    argument.endStructure();
    return argument;
}

KPowerManagerInhibitImpl::KPowerManagerInhibitImpl(QObject *parent)
    : QObject(parent),
    m_objectregistered(false),
    m_serviceregistered(false),
    m_login1("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus()),
    m_consolekit("org.freedesktop.ConsoleKit", "/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", QDBusConnection::systemBus()),
    m_consolekittimerid(0),
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
    } else if (m_consolekit.isValid()) {
        qDBusRegisterMetaType<KInhibitor>();
        qDBusRegisterMetaType<QList<KInhibitor>>();
        m_consolekittimerid = startTimer(2000);
    }
}

KPowerManagerInhibitImpl::~KPowerManagerInhibitImpl()
{
    if (m_consolekittimerid > 0) {
        killTimer(m_consolekittimerid);
    }

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
    if (m_login1.isValid()) {
        return (m_login1.property("NCurrentInhibitors").toULongLong() > 0);
    }
    if (m_consolekit.isValid()) {
        QDBusReply<QList<KInhibitor>> reply = m_consolekit.call("ListInhibitors");
        return (reply.isValid() && reply.value().size() > 0);
    }
    return false;
}

uint KPowerManagerInhibitImpl::Inhibit(const QString &application, const QString &reason)
{
    qulonglong maxinhibitors = 0;
    if (m_login1.isValid()) {
        maxinhibitors = m_login1.property("InhibitorsMax").toULongLong();
    } else if (m_consolekit.isValid()) {
        maxinhibitors = INT_MAX;
    } else {
        return 0;
    }

    if (maxinhibitors == 0) {
        kWarning() << "Inhibit limit is zero";
        return 0;
    } else if (maxinhibitors > INT_MAX) {
        kWarning() << "Inhibit limit greater than INT_MAX";
        return 0;
    }
    uint cookiecounter = 0;
    while (m_cookies.contains(cookiecounter)) {
        if (cookiecounter >= maxinhibitors) {
            kWarning() << "Inhibit limit reached";
            return 0;
        }
        cookiecounter++;
    }

    QDBusReply<QDBusUnixFileDescriptor> reply;
    if (m_login1.isValid()) {
        reply = m_login1.call(
            "Inhibit",
            "sleep",
            application, reason,
            "block"
        );
    } else {
        reply = m_consolekit.call(
            "Inhibit",
            "sleep",
            application, reason,
            "block"
        );
    }
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
    if (!m_login1.isValid() && !m_consolekit.isValid()) {
        return;
    }
    if (!m_cookies.contains(cookie)) {
        kWarning() << "Attempt to UnInhibit with invalid cookie";
        return;
    }
    // qDebug() << Q_FUNC_INFO << cookie;
    ::close(m_cookies.value(cookie));
    m_cookies.remove(cookie);
}

void KPowerManagerInhibitImpl::slotPropertiesChanged(QString interface, QVariantMap changed_properties, QStringList invalidated_properties)
{
    Q_UNUSED(interface);
    Q_UNUSED(changed_properties);
    Q_UNUSED(invalidated_properties);

    emitSignals();
}

void KPowerManagerInhibitImpl::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_consolekittimerid) {
        emitSignals();
    }
}

void KPowerManagerInhibitImpl::emitSignals()
{
    const bool oldhasinhibit = m_hasinhibit;
    m_hasinhibit = HasInhibit();

    if (oldhasinhibit != m_hasinhibit) {
        emit HasInhibitChanged(m_hasinhibit);
    }
}

#include "moc_kpowermanagerinhibitimpl.cpp"
