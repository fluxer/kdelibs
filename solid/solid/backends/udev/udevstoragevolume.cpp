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
#include "udevmanager.h"
#include "udevdevice.h"

#include <QDir>
#include <QDebug>

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
    const QString dmname(m_device->deviceProperty("DM_NAME"));
    if (!dmname.isEmpty()) {
        const QDir slavesdir(m_device->deviceName() + "/slaves");
        const QFileInfoList slavelinks(slavesdir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries));
        if (!slavelinks.isEmpty()) {
            return QString::fromLatin1("%1%2").arg(UDEV_UDI_PREFIX, slavelinks.first().canonicalFilePath());
        }
    }
    return QString();
}

qulonglong StorageVolume::size() const
{
    return (m_device->deviceProperty("ID_PART_ENTRY_SIZE").toULongLong() / 2 * 1024);
}

QString StorageVolume::uuid() const
{
    return m_device->deviceProperty("ID_FS_UUID");
}

QString StorageVolume::label() const
{
    return m_device->deviceProperty("ID_FS_LABEL");
}

QString StorageVolume::fsType() const
{
    return m_device->deviceProperty("ID_FS_TYPE");
}

Solid::StorageVolume::UsageType StorageVolume::usage() const
{
    const QString devtype(m_device->deviceProperty("DEVTYPE"));
    const QString idfsusage(m_device->deviceProperty("ID_FS_USAGE"));
    if (idfsusage == "crypto") {
        return Solid::StorageVolume::Encrypted;
    } else if (idfsusage == "other") {
        return Solid::StorageVolume::Other;
    } else if (idfsusage == "raid") {
        return Solid::StorageVolume::Raid;
    } else if (idfsusage == "filesystem") {
        return Solid::StorageVolume::FileSystem;
    } else if (devtype == "partition") {
        return Solid::StorageVolume::PartitionTable;
    } else {
        return Solid::StorageVolume::Unused;
    }
}

bool StorageVolume::isIgnored() const
{
    const QString idfsusage(m_device->deviceProperty("ID_FS_USAGE"));
    const QString devtype(m_device->deviceProperty("DEVTYPE"));
    const int idcdrom = m_device->deviceProperty("ID_CDROM").toInt();
    return ((idfsusage != "filesystem" && idfsusage != "crypto") || (devtype == "disk" && idcdrom != 1));
}
