/*
    Copyright 2021 Ivailo Monev <xakepa10@gmail.com>

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

#include "blkidstorageaccess.h"
#include "blkidstoragevolume.h"
#include "kmountpoint.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

using namespace Solid::Backends::Blkid;

StorageAccess::StorageAccess(BlkidDevice *device)
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

    const QString devname(m_device->deviceProperty(BlkidDevice::DeviceName));
    const QString devlabel(m_device->deviceProperty(BlkidDevice::DeviceLabel));
    foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
        if (mountpoint->mountedFrom() == devname || mountpoint->realDeviceName() == devname
            || mountpoint->mountedFrom() == devlabel) {
            return mountpoint->mountPoint();
        }
    }

#warning TODO: additional checks
#if 0
    const QStringList devlinks = m_device->blkidDevice().alternateDeviceSymlinks();
    foreach (const QString &link, devlinks) {
        foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
            if (mountpoint->mountedFrom() == link || mountpoint->realDeviceName() == link) {
                return mountpoint->mountPoint();
            }
        }
    }

    const QString idfsusage(m_device->deviceProperty("ID_FS_USAGE"));
    if (idfsusage == "crypto") {
        // NOTE: keep in sync with kde-workspace/soliduiserver/soliduiserver.cpp
        const QString idfsuuid(m_device->deviceProperty("ID_FS_UUID"));
        const QString dmdevice = QLatin1String("/dev/mapper/") + idfsuuid;
        foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
            if (mountpoint->mountedFrom() == dmdevice || mountpoint->realDeviceName() == dmdevice) {
                return mountpoint->mountPoint();
            }
        }
    }
#endif

    return QString();
}

bool StorageAccess::isIgnored() const
{
    const StorageVolume* volume(dynamic_cast<const StorageVolume*>(this));
    if (volume) {
        return volume->isIgnored();
    }
    return true;
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
