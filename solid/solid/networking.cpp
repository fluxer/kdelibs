/*
    Copyright 2006-2007 Will Stephenson <wstephenson@kde.org>
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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

//#include <KDebug>

#include "networking.h"
#include "networking_p.h"

#include "soliddefs_p.h"
#include "org_kde_solid_networking_client.h"

Q_GLOBAL_STATIC(Solid::NetworkingPrivate, globalNetworkManager)

Solid::NetworkingPrivate::NetworkingPrivate()
    : netStatus(Solid::Networking::Unknown),
      iface(nullptr)
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(
        "org.kde.kded", QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForOwnerChange, this
    );
    connect(
        watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
        this, SLOT(serviceOwnerChanged(QString,QString,QString))
    );

    initialize();
}

Solid::Networking::Notifier::Notifier()
{
}

void Solid::NetworkingPrivate::initialize()
{
    delete iface;
    iface  = new OrgKdeSolidNetworkingClientInterface(
        "org.kde.kded",
        "/modules/networkstatus",
        QDBusConnection::sessionBus(),
        this
    );

    // connect(iface, SIGNAL(statusChanged(uint)), globalNetworkManager(), SIGNAL(statusChanged(Networking::Status)));
    connect(iface, SIGNAL(statusChanged(uint)), this, SLOT(serviceStatusChanged(uint)));

    QDBusReply<uint> reply = iface->status();
    if (reply.isValid()) {
        netStatus = static_cast<Solid::Networking::Status>(reply.value());
    } else {
        netStatus = Solid::Networking::Unknown;
    }
}

uint Solid::NetworkingPrivate::status() const
{
    return netStatus;
}

/*=========================================================================*/

Solid::Networking::Status Solid::Networking::status()
{
    return static_cast<Solid::Networking::Status>(globalNetworkManager()->status());
}

Solid::Networking::Notifier *Solid::Networking::notifier()
{
    return globalNetworkManager();
}

void Solid::NetworkingPrivate::serviceStatusChanged(uint status)
{
    // kDebug(921) ;
    netStatus = static_cast<Solid::Networking::Status>(status);
    switch (netStatus) {
        case Solid::Networking::Unknown:
            break;
        case Solid::Networking::Unconnected:
        case Solid::Networking::Disconnecting:
        case Solid::Networking::Connecting:
            emit globalNetworkManager()->shouldDisconnect();
            break;
        case Solid::Networking::Connected:
            emit globalNetworkManager()->shouldConnect();
            break;
        // default:
            // kDebug(921) <<  "Unrecognised status code!";
    }
    emit globalNetworkManager()->statusChanged(netStatus);
}

void Solid::NetworkingPrivate::serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(name)
    Q_UNUSED(oldOwner)
    if (newOwner.isEmpty()) {
        // kded quit on us
        netStatus = Solid::Networking::Unknown;
        emit globalNetworkManager()->statusChanged(netStatus);
    } else {
        // kded was replaced or started
        initialize();
        emit globalNetworkManager()->statusChanged(netStatus);
        serviceStatusChanged(netStatus);
    }
}

#include "moc_networking_p.cpp"
#include "moc_networking.cpp"
