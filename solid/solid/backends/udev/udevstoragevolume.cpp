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

#include "udevstoragevolume.h"
#include "udevdevice.h"
#include "qdebug.h"

using namespace Solid::Backends::UDev;

StorageVolume::StorageVolume(UDevDevice *device)
    : Block(device)
{
}

StorageVolume::~StorageVolume()
{
}

QString StorageVolume::encryptedContainerUdi() const
{
    // encrypted devices are not support
    return QString();
}

qulonglong StorageVolume::size() const
{
    return (m_device->property("ID_PART_ENTRY_SIZE").toULongLong() / 2 * 1024);
}

QString StorageVolume::uuid() const
{
    return m_device->property("ID_FS_UUID").toString();
}

QString StorageVolume::label() const
{
    return m_device->property("ID_FS_LABEL").toString();
}

QString StorageVolume::fsType() const
{
    return m_device->property("ID_FS_TYPE").toString();
}

Solid::StorageVolume::UsageType StorageVolume::usage() const
{
    const QString devtype = m_device->property("DEVTYPE").toString();
    const QString idfsusage = m_device->property("ID_FS_USAGE").toString();

    if (idfsusage == "crypto") {
        return Solid::StorageVolume::Encrypted;
    } else if (idfsusage == "swap") {
        return Solid::StorageVolume::Other;
    } else if (devtype == "partition") {
        return Solid::StorageVolume::FileSystem;
    } else if (devtype == "disk") {
        return Solid::StorageVolume::PartitionTable;
    // TODO: how to detect it?
#if 0
    } else if (devtype == "raid") {
        return Solid::StorageVolume::Raid;
#endif
    } else {
        return Solid::StorageVolume::Unused;
    }
}

bool StorageVolume::isIgnored() const
{
    const Solid::StorageVolume::UsageType usg = usage();
    return (usg != Solid::StorageVolume::FileSystem);
}
