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

#include "blkidstoragevolume.h"
#include "blkidmanager.h"
#include "blkiddevice.h"

#include <QDir>
#include <QDebug>

using namespace Solid::Backends::Blkid;

StorageVolume::StorageVolume(BlkidDevice *device)
    : Block(device)
{
}

StorageVolume::~StorageVolume()
{
}

QString StorageVolume::encryptedContainerUdi() const
{
#warning TODO: implement
#if 0
    const QString dmname(m_device->deviceProperty("DM_NAME"));
    if (!dmname.isEmpty()) {
        const QDir slavesdir(m_device->deviceName() + "/slaves");
        const QFileInfoList slavelinks(slavesdir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries));
        if (!slavelinks.isEmpty()) {
            return QString::fromLatin1("%1%2").arg(BLKID_UDI_PREFIX, slavelinks.first().canonicalFilePath());
        }
    }
#endif
    return QString();
}

qulonglong StorageVolume::size() const
{
#warning TODO: implement
#if 0
    return m_device->deviceProperty("ID_PART_ENTRY_SIZE").toULongLong() / 2 * 1024;
#endif
    return 0;
}

QString StorageVolume::uuid() const
{
    return m_device->deviceProperty(BlkidDevice::DeviceUUID);
}

QString StorageVolume::label() const
{
    return m_device->deviceProperty(BlkidDevice::DeviceLabel);
}

QString StorageVolume::fsType() const
{
    return m_device->deviceProperty(BlkidDevice::DeviceType);
}

Solid::StorageVolume::UsageType StorageVolume::usage() const
{
    // TODO: Encrypted, Other, Raid
    if (!m_device->deviceProperty(BlkidDevice::DeviceType).isEmpty()) {
        return Solid::StorageVolume::FileSystem;
    } else if (m_device->m_minor != 0) {
        return Solid::StorageVolume::PartitionTable;
    }
    return Solid::StorageVolume::Unused;
}

bool StorageVolume::isIgnored() const
{
    const Solid::StorageVolume::UsageType volumeusage(usage());
    return (volumeusage != Solid::StorageVolume::FileSystem && volumeusage != Solid::StorageVolume::Encrypted);
}
