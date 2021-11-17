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

#include "config-solid.h"
#include "blkiddevice.h"
#include "blkidstoragedrive.h"
#include "blkidstoragevolume.h"
#include "blkidstorageaccess.h"
#include "blkidblock.h"
#include "blkidmanager.h"
#include "../shared/pciidstables.h"
#include "../shared/usbidstables.h"

#include "kglobal.h"
#include "klocale.h"

#if defined(LIBCDIO_FOUND)
#include "blkidopticaldisc.h"
#include "blkidopticaldrive.h"
#endif

#include <QDebug>

#include <blkid/blkid.h>

using namespace Solid::Backends::Blkid;

BlkidDevice::BlkidDevice(const QString &device)
    : Solid::Ifaces::Device()
    , m_device(device)
    , m_major(0)
    , m_minor(0)
{
    if (m_device.startsWith(BLKID_UDI_PREFIX)) {
        m_device = m_device.right(m_device.size() - qstrlen(BLKID_UDI_PREFIX));
    }
    const QByteArray devname(m_device.toLatin1());
    Q_ASSERT(!devname.contains('/'));
    int datapos = 0;
    int majornumberpos = 0;
    int minornumberpos = 0;
    const char* devicedata = devname.constData();
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
    m_parent = QByteArray(devname.constData(), majornumberpos);
    m_major = QByteArray(devname.constData() + majornumberpos, minornumberpos - majornumberpos).toInt();
    m_minor = QByteArray(devname.constData() + minornumberpos, devname.size() - majornumberpos).toInt();

    // qDebug() << Q_FUNC_INFO << m_device << m_parent << m_major << m_minor;
}

BlkidDevice::~BlkidDevice()
{
}

QString BlkidDevice::udi() const
{
    return QString::fromLatin1("%1/%2").arg(BLKID_UDI_PREFIX, m_device);
}

QString BlkidDevice::parentUdi() const
{
#warning FIXME: block devices workaround for optical and storage drives
    // code in several places expects the parent to NOT be the actual parent (disk) device UDI even
    // for partitions but another device UDI related to this device, has to be fixed and verified
    // to work at some point
    return udi();
}

QString BlkidDevice::vendor() const
{
    QString vendor;
#warning TODO: implement
    return vendor;
}

QString BlkidDevice::product() const
{
    QString product;
#warning TODO: implement
    return product;
}

QString BlkidDevice::icon() const
{
    if (parentUdi().isEmpty()) {
        return QLatin1String("computer");
#if defined(LIBCDIO_FOUND)
    // prioritize since it is a storage drive/disc too
    } else if (queryDeviceInterface(Solid::DeviceInterface::OpticalDrive)) {
        const OpticalDrive drive(const_cast<BlkidDevice*>(this));
        if (drive.isHotpluggable()) {
            return QLatin1String("drive-removable-media-usb");
        }
        return QLatin1String("drive-removable-media");
    } else if (queryDeviceInterface(Solid::DeviceInterface::OpticalDisc)) {
        const OpticalDisc disc(const_cast<BlkidDevice*>(this));
        Solid::OpticalDisc::ContentTypes availcontent = disc.availableContent();

        if (availcontent & Solid::OpticalDisc::VideoDvd) { // Video DVD
            return QLatin1String("media-optical-dvd-video");
        } else if ((availcontent & Solid::OpticalDisc::VideoCd) || (availcontent & Solid::OpticalDisc::SuperVideoCd)) { // Video CD
            return QLatin1String("media-optical-video");
        } else if ((availcontent & Solid::OpticalDisc::Data) && (availcontent & Solid::OpticalDisc::Audio)) { // Mixed CD
            return QLatin1String("media-optical-mixed-cd");
        } else if (availcontent & Solid::OpticalDisc::Audio) { // Audio CD
            return QLatin1String("media-optical-audio");
        } else if (availcontent & Solid::OpticalDisc::Data) { // Data CD
            return QLatin1String("media-optical-data");
        } else if ( disc.isRewritable() ) {
            return QLatin1String("media-optical-recordable");
        }

        switch (disc.discType()) {
            // DVD
            case Solid::OpticalDisc::DiscType::DvdRom:
            case Solid::OpticalDisc::DiscType::DvdRam:
            case Solid::OpticalDisc::DiscType::DvdRecordable:
            case Solid::OpticalDisc::DiscType::DvdRewritable:
            case Solid::OpticalDisc::DiscType::DvdPlusRecordable:
            case Solid::OpticalDisc::DiscType::DvdPlusRewritable:
            case Solid::OpticalDisc::DiscType::DvdPlusRecordableDuallayer:
            case Solid::OpticalDisc::DiscType::DvdPlusRewritableDuallayer:
            case Solid::OpticalDisc::DiscType::HdDvdRom:
            case Solid::OpticalDisc::DiscType::HdDvdRecordable:
            case Solid::OpticalDisc::DiscType::HdDvdRewritable:
                return QLatin1String("media-optical-dvd");
             // BluRay
            case Solid::OpticalDisc::DiscType::BluRayRom:
            case Solid::OpticalDisc::DiscType::BluRayRecordable:
            case Solid::OpticalDisc::DiscType::BluRayRewritable:
                return QLatin1String("media-optical-blu-ray");
            default:
                break;
        }

        // fallback for every other optical disc
        return QLatin1String("media-optical");
#endif // LIBCDIO_FOUND
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
        const StorageDrive storageIface(const_cast<BlkidDevice *>(this));
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

QStringList BlkidDevice::emblems() const
{
    QStringList res;

    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess)) {
        const StorageAccess accessIface(const_cast<BlkidDevice *>(this));
        if (accessIface.isAccessible()) {
            res << "emblem-mounted";
        } else {
            res << "emblem-unmounted";
        }
    }

    return res;
}

QString BlkidDevice::description() const
{
    if (parentUdi().isEmpty()) {
        return QObject::tr("Computer");
    }

#if defined(LIBCDIO_FOUND)
    // prioritize since it is a storage drive/disc too
    if (queryDeviceInterface(Solid::DeviceInterface::OpticalDrive)) {
        const OpticalDrive opticalDrive(const_cast<BlkidDevice*>(this));
        Solid::OpticalDrive::MediumTypes mediumTypes = opticalDrive.supportedMedia();
        QString first;
        QString second;

        first = QObject::tr("CD-ROM");
        if (mediumTypes & Solid::OpticalDrive::Cdr)
            first = QObject::tr("CD-R");
        if (mediumTypes & Solid::OpticalDrive::Cdrw)
            first = QObject::tr("CD-RW");

        if (mediumTypes & Solid::OpticalDrive::Dvd)
            second = QObject::tr("/DVD-ROM");
        if (mediumTypes & Solid::OpticalDrive::Dvdplusr)
            second = QObject::tr("/DVD+R");
        if (mediumTypes & Solid::OpticalDrive::Dvdplusrw)
            second = QObject::tr("/DVD+RW");
        if (mediumTypes & Solid::OpticalDrive::Dvdr)
            second = QObject::tr("/DVD-R");
        if (mediumTypes & Solid::OpticalDrive::Dvdrw)
            second = QObject::tr("/DVD-RW");
        if (mediumTypes & Solid::OpticalDrive::Dvdram)
            second = QObject::tr("/DVD-RAM");
        if ((mediumTypes & Solid::OpticalDrive::Dvdr) && (mediumTypes & Solid::OpticalDrive::Dvdplusr)) {
            if (mediumTypes & Solid::OpticalDrive::Dvdplusdl) {
                second = QObject::tr("/DVD±R DL");
            } else {
                second = QObject::tr("/DVD±R");
            }
        }
        if ((mediumTypes & Solid::OpticalDrive::Dvdrw) && (mediumTypes & Solid::OpticalDrive::Dvdplusrw)) {
            if((mediumTypes & Solid::OpticalDrive::Dvdplusdl) || (mediumTypes & Solid::OpticalDrive::Dvdplusdlrw)) {
                second = QObject::tr("/DVD±RW DL");
            } else {
                second = QObject::tr("/DVD±RW");
            }
        }
        if (mediumTypes & Solid::OpticalDrive::Bd)
            second = QObject::tr("/BD-ROM");
        if (mediumTypes & Solid::OpticalDrive::Bdr)
            second = QObject::tr("/BD-R");
        if (mediumTypes & Solid::OpticalDrive::Bdre)
            second = QObject::tr("/BD-RE");
        if (mediumTypes & Solid::OpticalDrive::HdDvd)
            second = QObject::tr("/HD DVD-ROM");
        if (mediumTypes & Solid::OpticalDrive::HdDvdr)
            second = QObject::tr("/HD DVD-R");
        if (mediumTypes & Solid::OpticalDrive::HdDvdrw)
            second = QObject::tr("/HD DVD-RW");

        QString description;
        if (opticalDrive.isHotpluggable()) {
            description = QObject::tr("External %1%2 Drive");
        } else {
            description = QObject::tr("%1%2 Drive");
        }
        description = description.arg(first, second);

        return description;
    }
#endif // LIBCDIO_FOUND
    if (queryDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
        const StorageDrive storageIface(const_cast<BlkidDevice *>(this));
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
        const StorageVolume storageIface(const_cast<BlkidDevice *>(this));
        QString desc = storageIface.label();
        if (desc.isEmpty()) {
            desc = storageIface.uuid();
        }
        if (desc.isEmpty()) {
            desc = deviceProperty(BlkidDevice::DeviceName);
        }
        return desc;
    }

    return QString();
}

bool BlkidDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
    case Solid::DeviceInterface::StorageAccess:
    case Solid::DeviceInterface::StorageDrive:
    case Solid::DeviceInterface::StorageVolume: {
        return true;
    }

#if defined(LIBCDIO_FOUND)
#warning TODO: match more optical device names
    case Solid::DeviceInterface::OpticalDrive:
    case Solid::DeviceInterface::OpticalDisc:
        return (m_device.indexOf("cd") >= 0);
#endif

    case Solid::DeviceInterface::Block:
        return true;

    default:
        return false;
    }
}

QObject *BlkidDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return 0;
    }

    switch (type) {
    case Solid::DeviceInterface::StorageAccess:
        return new StorageAccess(this);

    case Solid::DeviceInterface::StorageDrive:
        return new StorageDrive(this);

    case Solid::DeviceInterface::StorageVolume:
        return new StorageVolume(this);

#if defined(LIBCDIO_FOUND)
    case Solid::DeviceInterface::OpticalDrive:
        return new OpticalDrive(this);
    case Solid::DeviceInterface::OpticalDisc:
        return new OpticalDisc(this);
#endif

    case Solid::DeviceInterface::Block:
        return new Block(this);

    default:
        qFatal("Shouldn't happen");
        return 0;
    }
}

static const QByteArray getBlkidValue(const char* const blkidtag, const QByteArray &devnode)
{
    blkid_cache blkidcache;
    if (blkid_get_cache(&blkidcache, NULL) != 0) {
        qWarning() << "could not open blkid cache";
        return QByteArray();
    }
    char* blkidvalue = blkid_get_tag_value(blkidcache, blkidtag, devnode.constData());
    if (!blkidvalue) {
        // could be empty
        // qDebug() << "could not get blkid value";
        return QByteArray();
    }
    const QByteArray bytevalue(blkidvalue);
    ::free(blkidvalue);
    return bytevalue;
}

QByteArray BlkidDevice::deviceProperty(const DeviceProperty property) const
{
    switch (property) {
        case BlkidDevice::DeviceName: {
            return m_device.toLatin1();
        }
        case BlkidDevice::DeviceType: {
            return getBlkidValue("TYPE", m_device.toLatin1());
        }
        case BlkidDevice::DeviceLabel: {
            return getBlkidValue("LABEL", m_device.toLatin1());
        }
        case BlkidDevice::DeviceUUID: {
            return getBlkidValue("UUID", m_device.toLatin1());
        }
        case BlkidDevice::DeviceParent: {
            return m_parent;
        }
    }
    return QByteArray();
}
