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

#include "geomstorageaccess.h"
#include "kstandarddirs.h"
#include "kmountpoint.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

using namespace Solid::Backends::Geom;

StorageAccess::StorageAccess(GeomDevice *device)
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

    // TODO: use providers instead
    const QString devname(QString::fromLatin1("/dev/%1").arg(m_device->m_realdevice.constData()));
    const QString devgptname(QString::fromLatin1("/dev/gpt/%1").arg(m_device->m_realdevice.constData()));
    const QString devlabel(QString::fromLatin1("/dev/%1").arg(m_device->m_label.constData()));
    const QString devgptlabel(QString::fromLatin1("/dev/gpt/%1").arg(m_device->m_label.constData()));
    const QString devuuid(QString::fromLatin1("/dev/%1").arg(m_device->m_uuid.constData()));
    const QString devgptuuid(QString::fromLatin1("/dev/gpt/%1").arg(m_device->m_uuid.constData()));
    foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
        // qDebug() << devname << devlabel << mountpoint->mountedFrom() << mountpoint->realDeviceName();
        if (mountpoint->mountedFrom() == devname || mountpoint->realDeviceName() == devname
            || mountpoint->mountedFrom() == devgptname || mountpoint->realDeviceName() == devgptname
            || mountpoint->mountedFrom() == devlabel || mountpoint->realDeviceName() == devlabel
            || mountpoint->mountedFrom() == devgptlabel || mountpoint->realDeviceName() == devgptlabel
            || mountpoint->mountedFrom() == devuuid || mountpoint->realDeviceName() == devgptuuid) {
            return mountpoint->mountPoint();
        }
    }

    return QString();
}

bool StorageAccess::isIgnored() const
{
    // freebsd-swap and freebsd-boot type filesystems cannot be accessed
    return (m_device->m_type.isEmpty() || m_device->m_type == "freebsd-swap" || m_device->m_type == "freebsd-boot");
}

bool StorageAccess::setup()
{
    QString mountpoint = filePath();
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

