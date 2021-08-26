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
#include "geomstorageaccess.h"
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
    // devices which are named like path should not be listed by manager, e.g.
    // gpt/swapfs or gptid/9d7008c3-990e-11eb-bf4c-002590ec5bf2
    Q_ASSERT(!m_realdevice.contains('/'));
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

    struct gmesh tree;
    ::memset(&tree, 0, sizeof(gmesh));
    const int geomresult = geom_gettree(&tree);
    struct gclass* geomclass = nullptr;
    struct ggeom* geomgeom = nullptr;
    struct gprovider* geomprovider = nullptr;
    struct gconfig* geomconfig = nullptr;
    LIST_FOREACH(geomclass, &tree.lg_class, lg_class) {
        LIST_FOREACH(geomgeom, &geomclass->lg_geom, lg_geom) {
            LIST_FOREACH(geomprovider, &geomgeom->lg_provider, lg_provider) {
                // qDebug() << geomgeom->lg_name << m_realdevice;
                if (qstrcmp(geomprovider->lg_name, m_realdevice.constData()) != 0) {
                    continue;
                }
                LIST_FOREACH(geomconfig, &geomprovider->lg_config, lg_config) {
                    // qDebug() << geomprovider->lg_name << geomconfig->lg_name << geomconfig->lg_val;
                    if (qstrcmp(geomconfig->lg_name, "index") == 0) {
                        m_minor = QByteArray(geomconfig->lg_val).toInt();
                    } else if (qstrcmp(geomconfig->lg_name, "length") == 0) {
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
    // NOTE: keep in sync with kdelibs/solid/solid/backends/geom/geommanager.cpp
    LIST_FOREACH(geomclass, &tree.lg_class, lg_class) {
        if (qstrcmp(geomclass->lg_name, "DISK") != 0 && qstrcmp(geomclass->lg_name, "PART") != 0
            && qstrcmp(geomclass->lg_name, "SWAP") != 0) {
            continue;
        }
        LIST_FOREACH(geomgeom, &geomclass->lg_geom, lg_geom) {
            LIST_FOREACH(geomprovider, &geomgeom->lg_provider, lg_provider) {
                if (qstrcmp(geomprovider->lg_name, m_realdevice.constData()) != 0) {
                    continue;
                }
                m_class = QByteArray(geomclass->lg_name).toLower();
                // qDebug() << m_realdevice << m_class;
                break;
            }
        }
    }
    geom_deletetree(&tree);

    // qDebug() << Q_FUNC_INFO << m_device << m_realdevice << m_parent << m_major << m_minor;
    // qDebug() << Q_FUNC_INFO << m_size << m_type << m_label << m_uuid << m_class;
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
#warning TODO: compatibility bits, see warning in kdelibs/solid/solid/backends/udev/udevdevice.cpp
#if 0
    if (m_parent.isEmpty()) {
        return QString(GEOM_ROOT_UDI);
    }
    return QString::fromLatin1("%1/%2").arg(GEOM_UDI_PREFIX, m_parent.constData());
#else
    return QString::fromLatin1("%1/%2").arg(GEOM_UDI_PREFIX, m_realdevice.constData());
#endif
}

QString GeomDevice::vendor() const
{
    // no information related to vendor/product from geom
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
    QStringList res;
    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess)) {
        const StorageAccess accessIface(const_cast<GeomDevice *>(this));
        if (accessIface.isAccessible()) {
            res << "emblem-mounted";
        } else {
            res << "emblem-unmounted";
        }
    }
    return res;
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
            desc = m_realdevice;
        }
        return desc;
    }
    return QString();
}

bool GeomDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
        case Solid::DeviceInterface::Block:
        case Solid::DeviceInterface::StorageDrive:
        case Solid::DeviceInterface::StorageVolume:
        case Solid::DeviceInterface::StorageAccess: {
            return true;
        }
        default: {
            return false;
        }
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
        case Solid::DeviceInterface::StorageAccess: {
            return new StorageAccess(this);
        }
        default: {
            Q_ASSERT(false);
            return 0;
        }
    }
}