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

#include "geomstoragedrive.h"

#include <QtCore/QDebug>

#include <libgeom.h>

using namespace Solid::Backends::Geom;

StorageDrive::StorageDrive(GeomDevice *device)
    : Block(device)
{
}

StorageDrive::~StorageDrive()
{
}

qulonglong StorageDrive::size() const
{
    return m_device->m_size;
}

bool StorageDrive::isHotpluggable() const
{
    const Solid::StorageDrive::Bus storagebus = bus();
    return (storagebus == Solid::StorageDrive::Usb || storagebus == Solid::StorageDrive::Ieee1394);
}

bool StorageDrive::isRemovable() const
{
    const Solid::StorageDrive::Bus storagebus = bus();
    return (storagebus == Solid::StorageDrive::Usb);
}

Solid::StorageDrive::DriveType StorageDrive::driveType() const
{
    // for reference:
    // https://docs.freebsd.org/doc/6.0-RELEASE/usr/share/doc/handbook/disks-naming.html
    // TODO: not implementd: MemoryStick, SmartMedia, SdMmc, Xd
    if (m_device->m_realdevice.startsWith("acd") || m_device->m_realdevice.startsWith("cd")
        || m_device->m_realdevice.startsWith("mcd")) {
        return Solid::StorageDrive::CdromDrive;
    } else if (m_device->m_realdevice.startsWith("fd")) {
        return Solid::StorageDrive::Floppy;
    } else if (m_device->m_realdevice.startsWith("sa") || m_device->m_realdevice.startsWith("ast")) {
        return Solid::StorageDrive::Tape;
    } else if (m_device->m_realdevice.startsWith("da") || m_device->m_realdevice.startsWith("fla")) {
        return Solid::StorageDrive::CompactFlash;
    } else {
        return Solid::StorageDrive::HardDisk;
    }
}

Solid::StorageDrive::Bus StorageDrive::bus() const
{
    // TODO: not implemented: Sata, Ieee1394
    if (m_device->m_realdevice.startsWith("ad") || m_device->m_realdevice.startsWith("acd")
        || m_device->m_realdevice.startsWith("ast")) {
        return Solid::StorageDrive::Ide;
    } else if (m_device->m_realdevice.startsWith("fla")) {
        return Solid::StorageDrive::Usb;
    } else if (m_device->m_realdevice.startsWith("da") || m_device->m_realdevice.startsWith("cd")
        || m_device->m_realdevice.startsWith("sa")) {
        return Solid::StorageDrive::Scsi;
    } else {
        return Solid::StorageDrive::Platform;
    }
}
