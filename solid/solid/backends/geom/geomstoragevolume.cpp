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

#include "geomstoragevolume.h"
#include "geomdevice.h"

#include <QDebug>

using namespace Solid::Backends::Geom;

StorageVolume::StorageVolume(GeomDevice *device)
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
    return m_device->m_size;
}

QString StorageVolume::uuid() const
{
    return m_device->m_uuid;
}

QString StorageVolume::label() const
{
    return m_device->m_label;
}

QString StorageVolume::fsType() const
{
    return m_device->m_type;
}

Solid::StorageVolume::UsageType StorageVolume::usage() const
{
    // TODO: not implemented: Encrypted, Raid
    if (m_device->m_class == "swap") {
        return Solid::StorageVolume::Other;
    } else if (!m_device->m_type.isEmpty()) {
        return Solid::StorageVolume::FileSystem;
    } else if (m_device->m_class == "disk" || m_device->m_class == "part") {
        return Solid::StorageVolume::PartitionTable;
    } else {
        return Solid::StorageVolume::Unused;
    }
}

bool StorageVolume::isIgnored() const
{
    const Solid::StorageVolume::UsageType volumeusage = usage();
    return (volumeusage != Solid::StorageVolume::FileSystem);
}
