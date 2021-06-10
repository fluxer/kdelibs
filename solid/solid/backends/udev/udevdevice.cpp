/*
    Copyright 2010 Rafael Fernández López <ereslibre@kde.org>

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

#include "udevdevice.h"

#include "udevgenericinterface.h"
#include "udevstoragedrive.h"
#include "udevstoragevolume.h"
#include "udevstorageaccess.h"
#include "udevprocessor.h"
#include "udevacadapter.h"
#include "udevbattery.h"
#include "udevcamera.h"
#include "udevvideo.h"
#include "udevportablemediaplayer.h"
#include "udevdvbinterface.h"
#include "udevblock.h"
#include "udevaudiointerface.h"
#include "udevserialinterface.h"
#include "udevnetworkinterface.h"
#include "udevbutton.h"
#include "cpuinfo.h"
#include "kglobal.h"
#include "klocale.h"

#ifdef UDEV_CDIO
#include "udevopticaldisc.h"
#include "udevopticaldrive.h"
#endif

#include <sys/socket.h>
#include <linux/if_arp.h>

#include <QFile>
#include <QDebug>

using namespace Solid::Backends::UDev;

UDevDevice::UDevDevice(const UdevQt::Device device)
    : Solid::Ifaces::Device()
    , m_device(device)
{
}

UDevDevice::~UDevDevice()
{
}

QString UDevDevice::udi() const
{
    return devicePath();
}

QString UDevDevice::parentUdi() const
{
#warning FIXME: block devices workaround for optical and storage drives
    // code in e.g. dolphin and plasma casts the parent instead of the actual device to either
    // Solid::StorageDrive or Solid::OpticalDrive which is wrong but was expected to work with the
    // UDisks backends, has to be fixed and verified to work in several places at some point
    const QString subsystem = m_device.deviceProperty("SUBSYSTEM").toString();
    if (subsystem == QLatin1String("block")) {
        return devicePath();
    }
    const int idcdrom = m_device.deviceProperty("ID_CDROM").toInt();
    if (idcdrom == 1) {
        return devicePath();
    }

    return UDEV_UDI_PREFIX;
}

QString UDevDevice::vendor() const
{
    QString vendor = m_device.sysfsProperty("manufacturer").toString();
    if (vendor.isEmpty()) {
         if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
             // sysfs doesn't have anything useful here
            vendor = extractCpuInfoLine(deviceNumber(), "vendor_id\\s+:\\s+(\\S.+)");
         } else if (queryDeviceInterface(Solid::DeviceInterface::Video)) {
             vendor = m_device.deviceProperty("ID_VENDOR").toString().replace('_', " ");
         }  else if (queryDeviceInterface(Solid::DeviceInterface::NetworkInterface)) {
             vendor = m_device.deviceProperty("ID_VENDOR_FROM_DATABASE").toString();
         } else if (queryDeviceInterface(Solid::DeviceInterface::AudioInterface)) {
             if (m_device.parent().isValid()) {
                 vendor = m_device.parent().deviceProperty("ID_VENDOR_FROM_DATABASE").toString();
             }
         }

         if (vendor.isEmpty()) {
             vendor = m_device.deviceProperty("ID_VENDOR").toString().replace('_', ' ');
         }
    }
    return vendor;
}

QString UDevDevice::product() const
{
    QString product = m_device.sysfsProperty("product").toString();
    if (product.isEmpty()) {
        if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
            // sysfs doesn't have anything useful here
            product = extractCpuInfoLine(deviceNumber(), "model name\\s+:\\s+(\\S.+)");
        } else if(queryDeviceInterface(Solid::DeviceInterface::Video)) {
            product = m_device.deviceProperty("ID_V4L_PRODUCT").toString();
        } else if(queryDeviceInterface(Solid::DeviceInterface::AudioInterface)) {
            const AudioInterface audioIface(const_cast<UDevDevice *>(this));
            product = audioIface.name();
        }  else if(queryDeviceInterface(Solid::DeviceInterface::NetworkInterface)) {
            QFile typeFile(deviceName() + "/type");
            if (typeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                int mediaType = typeFile.readAll().trimmed().toInt();
                if (mediaType == ARPHRD_LOOPBACK) {
                    product = QLatin1String("Loopback device Interface");
                } else  {
                    product = m_device.deviceProperty("ID_MODEL_FROM_DATABASE").toString();
                }
            }
        } else if(queryDeviceInterface(Solid::DeviceInterface::SerialInterface)) {
            const SerialInterface serialIface(const_cast<UDevDevice *>(this));
            if (serialIface.serialType() == Solid::SerialInterface::Platform) {
                product.append(QLatin1String("Platform serial"));
            } else if (serialIface.serialType() == Solid::SerialInterface::Usb) {
                product.append(QLatin1String("USB Serial Port"));
            }
        }

        if (product.isEmpty()) {
            product = m_device.deviceProperty("ID_MODEL").toString().replace('_', ' ');
        }
    }
    return product;
}

QString UDevDevice::icon() const
{
    if (parentUdi().isEmpty()) {
        return QLatin1String("computer");
#ifdef UDEV_CDIO
    // prioritize since it is a storage drive/disc too
    } else if (queryDeviceInterface(Solid::DeviceInterface::OpticalDrive)) {
        const OpticalDrive drive(const_cast<UDevDevice*>(this));
        if (drive.isHotpluggable()) {
            return QLatin1String("drive-removable-media-usb");
        }
        return QLatin1String("drive-removable-media");
    } else if (queryDeviceInterface(Solid::DeviceInterface::OpticalDisc)) {
        const OpticalDisc disc(const_cast<UDevDevice*>(this));
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
        }

        // fallback for every other optical disc
        return QLatin1String("media-optical");
#endif
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
        const StorageDrive storageIface(const_cast<UDevDevice *>(this));
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
    } if (queryDeviceInterface(Solid::DeviceInterface::AcAdapter)) {
        return QLatin1String("preferences-system-power-management");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Battery)) {
        return QLatin1String("battery");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return QLatin1String("cpu");
    } else if (queryDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer)) {
        // TODO: check out special cases like iPod
        return QLatin1String("multimedia-player");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return QLatin1String("camera-photo");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Video)) {
        return QLatin1String("camera-web");
    } else if (queryDeviceInterface(Solid::DeviceInterface::AudioInterface)) {
        const AudioInterface audioIface(const_cast<UDevDevice *>(this));
        switch (audioIface.soundcardType()) {
        case Solid::AudioInterface::InternalSoundcard:
            return QLatin1String("audio-card");
        case Solid::AudioInterface::UsbSoundcard:
            return QLatin1String("audio-card-usb");
        case Solid::AudioInterface::FirewireSoundcard:
            return QLatin1String("audio-card-firewire");
        case Solid::AudioInterface::Headset:
            if (udi().contains("usb", Qt::CaseInsensitive) ||
                    audioIface.name().contains("usb", Qt::CaseInsensitive)) {
                return QLatin1String("audio-headset-usb");
            } else {
                return QLatin1String("audio-headset");
            }
        case Solid::AudioInterface::Modem:
            return QLatin1String("modem");
        }
    } else if (queryDeviceInterface(Solid::DeviceInterface::SerialInterface)) {
        // TODO - a serial device can be a modem, or just
        // a COM port - need a new icon?
        return QLatin1String("modem");
    }

    return QString();
}

QStringList UDevDevice::emblems() const
{
    QStringList res;

    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess)) {
        const StorageAccess accessIface(const_cast<UDevDevice *>(this));
        if (accessIface.isAccessible()) {
            res << "emblem-mounted";
        } else {
            res << "emblem-unmounted";
        }
    }

    return res;
}

QString UDevDevice::description() const
{
    if (parentUdi().isEmpty()) {
        return QObject::tr("Computer");
    }

#ifdef UDEV_CDIO
    // prioritize since it is a storage drive/disc too
    if (queryDeviceInterface(Solid::DeviceInterface::OpticalDrive)) {
        const OpticalDrive opticalDrive(const_cast<UDevDevice*>(this));
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
#endif // UDEV_CDIO
    if (queryDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
        const StorageDrive storageIface(const_cast<UDevDevice *>(this));
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
        const StorageVolume storageIface(const_cast<UDevDevice *>(this));
        QString desc = storageIface.label();
        if (desc.isEmpty()) {
            desc = storageIface.uuid();
        }
        if (desc.isEmpty()) {
            desc = storageIface.property("DEVNAME").toString();
        }
        return desc;
    } else if (queryDeviceInterface(Solid::DeviceInterface::AcAdapter)) {
        return QObject::tr("A/C Adapter");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Battery)) {
        const QString powersupplytechnology = property("POWER_SUPPLY_TECHNOLOGY").toString();
        if (powersupplytechnology == QLatin1String("NiMH")) {
            return QObject::tr("Nickel Metal Hydride Battery");
        } else if (powersupplytechnology == QLatin1String("Li-ion")) {
            return QObject::tr("Lithium Ion Battery");
        } else if (powersupplytechnology == QLatin1String("Li-poly")) {
            return QObject::tr("Lithium Polymer Battery");
        } else if (powersupplytechnology == QLatin1String("LiFe")) {
            return QObject::tr("Lithium Iron Disulfide Battery");
        } else if (powersupplytechnology == QLatin1String("NiCd")) {
            return QObject::tr("Nickel Cadmium Battery");
        } else if (powersupplytechnology == QLatin1String("LiMn")) {
            return QObject::tr("Lithium Manganese Dioxide Battery");
        }
        return QObject::tr("Unknown Battery");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return QObject::tr("Processor");
    } else if (queryDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer)) {
        /*
         * HACK: As Media player is very generic return the device product instead
         *       until we can return the Name.
         */
        const PortableMediaPlayer player(const_cast<UDevDevice *>(this));
        if (player.supportedProtocols().contains("mtp")) {
            return product();
        } else {
            // TODO: check out special cases like iPod
            return QObject::tr("Portable Media Player");
        }
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return QObject::tr("Camera");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Video)) {
        return product();
    } else if (queryDeviceInterface(Solid::DeviceInterface::AudioInterface)) {
        return product();
    } else if (queryDeviceInterface(Solid::DeviceInterface::NetworkInterface)) {
        const NetworkInterface networkIface(const_cast<UDevDevice *>(this));
        if (networkIface.isWireless()) {
            return QObject::tr("WLAN Interface");
        }
        return QObject::tr("Networking Interface");
    }

    return QString();
}

bool UDevDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return true;

    case Solid::DeviceInterface::StorageAccess:
    case Solid::DeviceInterface::StorageDrive:
    case Solid::DeviceInterface::StorageVolume: {
        return m_device.subsystem() == QLatin1String("block");
    }

    case Solid::DeviceInterface::AcAdapter:
    case Solid::DeviceInterface::Battery:
        return m_device.subsystem() == QLatin1String("power_supply");

    case Solid::DeviceInterface::Processor:
        return property("DRIVER").toString() == "processor";

#ifdef UDEV_CDIO
    case Solid::DeviceInterface::OpticalDrive:
    case Solid::DeviceInterface::OpticalDisc:
        return (property("ID_CDROM").toInt() == 1);
#endif

    case Solid::DeviceInterface::Camera:
        return property("ID_GPHOTO2").toInt() == 1;

    case Solid::DeviceInterface::PortableMediaPlayer:
        return !property("ID_MEDIA_PLAYER").toString().isEmpty();

    case Solid::DeviceInterface::DvbInterface:
        return m_device.subsystem() ==  QLatin1String("dvb");

    case Solid::DeviceInterface::Block:
        return !property("MAJOR").toString().isEmpty();

    case Solid::DeviceInterface::Video:
        return m_device.subsystem() == QLatin1String("video4linux");

    case Solid::DeviceInterface::AudioInterface:
        return m_device.subsystem() == QLatin1String("sound");

    case Solid::DeviceInterface::NetworkInterface:
        return m_device.subsystem() == QLatin1String("net");

    case Solid::DeviceInterface::SerialInterface:
        return m_device.subsystem() == QLatin1String("tty");

    case Solid::DeviceInterface::Button:
        return m_device.subsystem() == QLatin1String("input");

    default:
        return false;
    }
}

QObject *UDevDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return 0;
    }

    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return new GenericInterface(this);

    case Solid::DeviceInterface::StorageAccess:
        return new StorageAccess(this);

    case Solid::DeviceInterface::StorageDrive:
        return new StorageDrive(this);

    case Solid::DeviceInterface::StorageVolume:
        return new StorageVolume(this);

    case Solid::DeviceInterface::AcAdapter:
        return new AcAdapter(this);

    case Solid::DeviceInterface::Battery:
        return new Battery(this);

    case Solid::DeviceInterface::Processor:
        return new Processor(this);

#ifdef UDEV_CDIO
    case Solid::DeviceInterface::OpticalDrive:
        return new OpticalDrive(this);
    case Solid::DeviceInterface::OpticalDisc:
        return new OpticalDisc(this);
#endif

    case Solid::DeviceInterface::Camera:
        return new Camera(this);

    case Solid::DeviceInterface::PortableMediaPlayer:
        return new PortableMediaPlayer(this);

    case Solid::DeviceInterface::DvbInterface:
        return new DvbInterface(this);

    case Solid::DeviceInterface::Block:
        return new Block(this);

    case Solid::DeviceInterface::Video:
        return new Video(this);

    case Solid::DeviceInterface::AudioInterface:
        return new AudioInterface(this);

    case Solid::DeviceInterface::NetworkInterface:
        return new NetworkInterface(this);

    case Solid::DeviceInterface::SerialInterface:
        return new SerialInterface(this);

    case Solid::DeviceInterface::Button:
        return new Button(this);

    default:
        qFatal("Shouldn't happen");
        return 0;
    }
}

QString UDevDevice::device() const
{
    return devicePath();
}

QVariant UDevDevice::property(const QString &key) const
{
    const QVariant res = m_device.deviceProperty(key);
    if (res.isValid()) {
        return res;
    }
    return m_device.sysfsProperty(key);
}

QMap<QString, QVariant> UDevDevice::allProperties() const
{
    QMap<QString, QVariant> res;
    foreach (const QString &prop, m_device.deviceProperties()) {
        res[prop] = property(prop);
    }
    return res;
}

bool UDevDevice::propertyExists(const QString &key) const
{
    return m_device.deviceProperties().contains(key);
}

QString UDevDevice::systemAttribute(const char *attribute) const
{
    return m_device.sysfsProperty(attribute).toString();
}

QString UDevDevice::deviceName() const
{
    return m_device.sysfsPath();
}

int UDevDevice::deviceNumber() const
{
    return m_device.sysfsNumber();
}

QString UDevDevice::devicePath() const
{
    return QString(UDEV_UDI_PREFIX) + deviceName();
}

UdevQt::Device UDevDevice::udevDevice()
{
    return m_device;
}
