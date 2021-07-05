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

#include "geommanager.h"
#include "geomdevice.h"
#include "geomblock.h"
#include "geomstoragedrive.h"
#include "geomstoragevolume.h"
#include "kglobal.h"
#include "klocale.h"

#include <QDebug>

#include <libgeom.h>
#include <stdlib.h>

using namespace Solid::Backends::Geom;

GeomDevice::GeomDevice(const QString &device)
    : Solid::Ifaces::Device()
    , m_size(0)
    , m_device(device)
    , m_major(0)
    , m_minor(0)
{
    m_realdevice = m_device.right(m_device.size() - qstrlen(GEOM_UDI_PREFIX) - 1).toLatin1();
    // assume devices which are named like path do not have major/minor (e.g. gpt/swapfs or
    // gptid/9d7008c3-990e-11eb-bf4c-002590ec5bf2)
    if (!m_realdevice.contains('/')) {
        int datapos = 0;
        int majornumberpos = 0;
        int minornumberpos = 0;
        const char* devicedata = m_realdevice.constData();
        while (*devicedata) {
            if (::isdigit(*devicedata)) {
                if (!majornumberpos) {
                    majornumberpos = datapos;
                } else {
                    minornumberpos = datapos;
                    break;
                }
            }
            devicedata++;
            datapos++;
        }
        m_parent = QByteArray(m_realdevice.constData(), majornumberpos);
        m_major = QByteArray(m_realdevice.constData() + majornumberpos, minornumberpos - majornumberpos).toInt();
        m_minor = QByteArray(m_realdevice.constData() + minornumberpos, m_realdevice.size() - majornumberpos).toInt();
    }

    struct gmesh tree;
    ::memset(&tree, 0, sizeof(gmesh));
    const int geomresult = geom_gettree(&tree);
    struct gclass* geomclass = Q_NULLPTR;
    struct ggeom* geomgeom = Q_NULLPTR;
    struct gprovider* geomprovider = Q_NULLPTR;
    struct gconfig* geomconfig = Q_NULLPTR;
    LIST_FOREACH(geomclass, &tree.lg_class, lg_class) {
        LIST_FOREACH(geomgeom, &geomclass->lg_geom, lg_geom) {
            LIST_FOREACH(geomprovider, &geomgeom->lg_provider, lg_provider) {
                // qDebug() << geomgeom->lg_name << m_realdevice;
                if (qstrcmp(geomprovider->lg_name, m_realdevice.constData()) != 0) {
                    continue;
                }
                m_class = QByteArray(geomclass->lg_name).toLower();
                LIST_FOREACH(geomconfig, &geomprovider->lg_config, lg_config) {
                    // qDebug() << geomprovider->lg_name << geomconfig->lg_name << geomconfig->lg_val;
                    if (qstrcmp(geomconfig->lg_name, "length") == 0) {
                        m_size = QByteArray(geomconfig->lg_val).toULongLong();
                    } else if (qstrcmp(geomconfig->lg_name, "type") == 0) {
                        m_type = geomconfig->lg_val;
                    } else if (qstrcmp(geomconfig->lg_name, "label") == 0) {
                        m_label = geomconfig->lg_val;
                    } else if (qstrcmp(geomconfig->lg_name, "rawuuid") == 0) {
                        m_uuid = geomconfig->lg_val;
                    }
                }
            }
        }
    }
    // re-iterate to assign class other than DEV if possible
    LIST_FOREACH(geomclass, &tree.lg_class, lg_class) {
        if (qstrcmp(geomclass->lg_name, "PART") != 0 && qstrcmp(geomclass->lg_name, "DISK") != 0
            && qstrcmp(geomclass->lg_name, "SWAP") != 0) {
            continue;
        }
        LIST_FOREACH(geomgeom, &geomclass->lg_geom, lg_geom) {
            LIST_FOREACH(geomprovider, &geomgeom->lg_provider, lg_provider) {
                if (qstrcmp(geomprovider->lg_name, m_realdevice.constData()) != 0) {
                    continue;
                }
                m_class = QByteArray(geomclass->lg_name).toLower();
                // qDebug() << geomgeom->lg_name << m_class;
            }
        }
    }
    geom_deletetree(&tree);

    // qDebug() << Q_FUNC_INFO << m_device << m_realdevice << m_parent << m_major << m_minor;
}

GeomDevice::~GeomDevice()
{
}

QString GeomDevice::udi() const
{
    return m_device;
}

QString GeomDevice::parentUdi() const
{
    if (m_parent.isEmpty()) {
        return QString(GEOM_ROOT_UDI);
    }
    return QString::fromLatin1("%1/%2").arg(GEOM_UDI_PREFIX, m_parent.constData());
}

QString GeomDevice::vendor() const
{
    return QString();
}

QString GeomDevice::product() const
{
    if (m_device == GEOM_ROOT_UDI) {
        return QLatin1String("Devices");
    }
    return QString();
}

QString GeomDevice::icon() const
{
    if (m_device == GEOM_ROOT_UDI) {
        return QString("computer");
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
        const StorageDrive storageIface(const_cast<GeomDevice *>(this));
        Solid::StorageDrive::DriveType drivetype = storageIface.driveType();

        if (drivetype == Solid::StorageDrive::HardDisk) {
            return QLatin1String("drive-harddisk");
        } else if (drivetype == Solid::StorageDrive::CdromDrive) {
            return QLatin1String("drive-optical");
        } else if (drivetype == Solid::StorageDrive::Floppy) {
            return QLatin1String("media-floppy");
        } else if (drivetype == Solid::StorageDrive::Tape) {
            return QLatin1String("media-tape");
        } else if (drivetype == Solid::StorageDrive::CompactFlash) {
            return QLatin1String("drive-removable-media");
        } else if (drivetype == Solid::StorageDrive::MemoryStick) {
            return QLatin1String("media-flash-memory-stick");
        } else if (drivetype == Solid::StorageDrive::SmartMedia) {
            return QLatin1String("media-flash-smart-media");
        } else if (drivetype == Solid::StorageDrive::SdMmc) {
            return QLatin1String("media-flash-sd-mmc");
        } else if (drivetype == Solid::StorageDrive::Xd) {
            return QLatin1String("drive-removable-media");
        }
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageVolume)) {
        return QLatin1String("drive-harddisk");
    }
    return QString();
}

QStringList GeomDevice::emblems() const
{
    return QStringList();
}

QString GeomDevice::description() const
{
    if (m_device == GEOM_ROOT_UDI || parentUdi().isEmpty()) {
        return QObject::tr("Computer");
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
        const StorageDrive storageIface(const_cast<GeomDevice *>(this));
        Solid::StorageDrive::DriveType drivetype = storageIface.driveType();
        const QString storagesize = KGlobal::locale()->formatByteSize(storageIface.size());

        if (drivetype == Solid::StorageDrive::HardDisk) {
            return QObject::tr("%1 Hard Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::CdromDrive) {
            return QObject::tr("%1 CD-ROM Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::Floppy) {
            return QObject::tr("%1 Floppy Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::Tape) {
            return QObject::tr("%1 Tape Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::CompactFlash) {
            return QObject::tr("%1 Compact Flash Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::MemoryStick) {
            return QObject::tr("%1 Memory Stick Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::SmartMedia) {
            return QObject::tr("%1 Smart Media Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::SdMmc) {
            return QObject::tr("%1 SD/MMC Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::Xd) {
            return QObject::tr("%1 Xd Drive").arg(storagesize);
        }
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageVolume)) {
        const StorageVolume storageIface(const_cast<GeomDevice *>(this));
        QString desc = storageIface.label();
        if (desc.isEmpty()) {
            desc = storageIface.uuid();
        }
        if (desc.isEmpty()) {
            desc = this->m_realdevice;
        }
        return desc;
    }
    return QString();
}

bool GeomDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
        case Solid::DeviceInterface::Block:
            return (m_major != 0);
        case Solid::DeviceInterface::StorageDrive:
        case Solid::DeviceInterface::StorageVolume:
            return true;
        default:
            return false;
    }
}

QObject *GeomDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return 0;
    }
    switch (type) {
        case Solid::DeviceInterface::Block: {
            return new Block(this);
        }
        case Solid::DeviceInterface::StorageDrive: {
            return new StorageDrive(this);
        }
        case Solid::DeviceInterface::StorageVolume: {
            return new StorageVolume(this);
        }
        default:
            Q_ASSERT(false);
            return 0;
    }
}
