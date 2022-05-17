/*
    Copyright 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "exportsstorageaccess.h"
#include "kmountpoint.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

using namespace Solid::Backends::Exports;

StorageAccess::StorageAccess(ExportsDevice *device)
    : DeviceInterface(device)
{
}

StorageAccess::~StorageAccess()
{
}

bool StorageAccess::isAccessible() const
{
    return !filePath().isEmpty();
}

QString StorageAccess::filePath() const
{
    const KMountPoint::List mountpoints = KMountPoint::currentMountPoints();

    const QString exportdir(m_device->exportDir());
    const QString exportdirandaddress = QString::fromLatin1("%1:%2").arg(m_device->exportAddress()).arg(m_device->exportDir());
    foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
        // qDebug() << Q_FUNC_INFO << mountpoint->mountedFrom() << mountpoint->realDeviceName() << mountpoint->mountPoint() << exportdir;
        if (mountpoint->mountedFrom() == exportdir || mountpoint->mountedFrom() == exportdirandaddress) {
            return mountpoint->mountPoint();
        }
    }

    return QString();
}

bool StorageAccess::isIgnored() const
{
    return false;
}

bool StorageAccess::setup()
{
    const QString mountpoint = filePath();
    if (!mountpoint.isEmpty()) {
        return true;
    }

    emit setupRequested(m_device->udi());

    QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
    QDBusReply<int> reply = soliduiserver.call("mountDevice", m_device->udi());

    const Solid::ErrorType replyvalue = static_cast<Solid::ErrorType>(reply.value());
    if (replyvalue == Solid::NoError) {
        emit accessibilityChanged(true, m_device->udi());
    }

    emit setupDone(replyvalue, Solid::errorString(replyvalue), m_device->udi());
    return (replyvalue == Solid::NoError);
}

bool StorageAccess::teardown()
{
    const QString mountpoint = filePath();
    if (mountpoint.isEmpty()) {
        return false;
    }

    emit teardownRequested(m_device->udi());

    QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
    QDBusReply<int> reply = soliduiserver.call("unmountDevice", m_device->udi());

    const Solid::ErrorType replyvalue = static_cast<Solid::ErrorType>(reply.value());
    if (replyvalue == Solid::NoError) {
        emit accessibilityChanged(false, m_device->udi());
    }

    emit teardownDone(replyvalue, Solid::errorString(replyvalue), m_device->udi());
    return (replyvalue == Solid::NoError);
}
