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

#include "udevstoragedrive.h"

#include "../shared/udevqt.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>

using namespace Solid::Backends::UDev;

StorageDrive::StorageDrive(UDevDevice *device)
    : Block(device)
{
}

StorageDrive::~StorageDrive()
{
}

qulonglong StorageDrive::size() const
{
    return m_device->property("ID_PART_ENTRY_SIZE").toULongLong() / 2 * 1024;
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
    const QString idtype = m_device->property("ID_TYPE").toString();
    const QString idbus = m_device->property("ID_BUS").toString();

    if (idtype == "cd" || m_device->property("ID_CDROM_MEDIA_CD").toInt() == 1) {
        return Solid::StorageDrive::CdromDrive;
    } else if (m_device->property("ID_DRIVE_FLOPPY").toInt() == 1) {
        return Solid::StorageDrive::Floppy;
    // TODO: other types and remove this generic check
    } else if (idbus == "usb") {
        return Solid::StorageDrive::CompactFlash;
#if 0
    } else if (idtype == "tape") {
        return Solid::StorageDrive::Tape;
    } else if (idtype == "flash_cf") {
        return Solid::StorageDrive::CompactFlash;
    } else if (idtype == "flash_ms") {
        return Solid::StorageDrive::MemoryStick;
    } else if (idtype == "flash_sm") {
        return Solid::StorageDrive::SmartMedia;
    } else if (idtype == "flash_sd" || idtype == "flash_mmc") {
        return Solid::StorageDrive::SdMmc;
    } else if (idtype == "flash_xd") {
        return Solid::StorageDrive::Xd;
#endif
    } else {
        return Solid::StorageDrive::HardDisk;
    }
}

Solid::StorageDrive::Bus StorageDrive::bus() const
{
    const QString idbus = m_device->property("ID_BUS").toString();

    // qDebug() << "bus:" << idbus;

    if (idbus == "ata") {
        if (m_device->property("ID_ATA_SATA").toInt() == 1) {
            // serial ATA
            return Solid::StorageDrive::Sata;
        } else {
            // parallel (classical) ATA
            return Solid::StorageDrive::Ide;
        }
    } else if (idbus == "usb") {
        return Solid::StorageDrive::Usb;
    } else if (idbus == "ieee1394") {
        return Solid::StorageDrive::Ieee1394;
    } else if (idbus == "scsi") {
        return Solid::StorageDrive::Scsi;
    } else {
        return Solid::StorageDrive::Platform;
    }
}
