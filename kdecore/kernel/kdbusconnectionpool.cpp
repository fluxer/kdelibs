/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>
 * Copyright (C) 2010 David Faure <faure@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kdbusconnectionpool.h"
#include "kdebug.h"

#include <QCoreApplication>
#include <QThread>
#include <QDBusConnectionInterface>
#include <QDBusReply>

namespace {

QAtomicInt s_connectionCounter(0);

class KDBusConnectionPoolPrivate
{
public:
    KDBusConnectionPoolPrivate()
        : m_connection( QDBusConnection::connectToBus(
                            QDBusConnection::SessionBus,
                            QString::fromLatin1("KDBusConnection%1").arg(newNumber()) ) )
    {
    }

    ~KDBusConnectionPoolPrivate()
    {
        QDBusConnection::disconnectFromBus( m_connection.name() );
    }

    QDBusConnection connection() const
    {
        return m_connection;
    }

private:
    static int newNumber()
    {
        return s_connectionCounter.fetchAndAddAcquire(1);
    }

    QDBusConnection m_connection;
};
} // namespace

thread_local KDBusConnectionPoolPrivate* s_perThreadConnection = 0;

QDBusConnection KDBusConnectionPool::threadConnection()
{
    if (QCoreApplication::instance()->thread() == QThread::currentThread()) {
        return QDBusConnection::sessionBus();
    }
    if (!s_perThreadConnection) {
        s_perThreadConnection = new KDBusConnectionPoolPrivate;
    }

    return s_perThreadConnection->connection();
}

bool KDBusConnectionPool::isServiceRegistered(const QString &service, const QDBusConnection &connection)
{
    QDBusConnectionInterface* dbusinterface = connection.interface();
    if (!dbusinterface) {
        kDebug() << "Null D-Bus interface" << service;
        return false;
    }
    QDBusReply<bool> reply = dbusinterface->isServiceRegistered(service);
    if (reply.value() == false) {
        kDebug() << "Service not registered" << service;
        return false;
    }
    kDebug() << "Service is registered" << service;
    return true;
}

