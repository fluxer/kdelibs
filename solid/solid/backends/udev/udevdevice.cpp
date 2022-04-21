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

#include "config-solid.h"
#include "udevdevice.h"
#include "udevstoragedrive.h"
#include "udevstoragevolume.h"
#include "udevstorageaccess.h"
#include "udevprocessor.h"
#include "udevacadapter.h"
#include "udevbattery.h"
#include "udevcamera.h"
#include "udevvideo.h"
#include "udevportablemediaplayer.h"
#include "udevblock.h"
#include "udevaudiointerface.h"
#include "udevnetworkinterface.h"
#include "udevbutton.h"
#include "udevgraphic.h"
#include "udevmanager.h"
#include "cpuinfo.h"
#include "../shared/pciidstables.h"
#include "../shared/usbidstables.h"

#include "kglobal.h"
#include "klocale.h"

#if defined(LIBCDIO_FOUND)
#include "udevopticaldisc.h"
#include "udevopticaldrive.h"
#endif

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
    const int idcdrom = m_device.deviceProperty("ID_CDROM").toInt();
#warning FIXME: block devices workaround for optical and storage drives
    // code in several places expects the parent to NOT be the actual parent (disk) device UDI even
    // for partitions but another device UDI related to this device, has to be fixed and verified
    // to work at some point
    if (m_device.subsystem() == QLatin1String("block") || idcdrom == 1) {
        return devicePath();
    }

    return UDEV_UDI_PREFIX;
}

QString UDevDevice::vendor() const
{
    QString vendor = m_device.sysfsProperty("manufacturer");
    if (vendor.isEmpty()) {
        if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
            // sysfs doesn't have anything useful here
            vendor = extractCpuInfoLine(deviceNumber(), "vendor_id\\s+:\\s+(\\S.+)");
        } else if (queryDeviceInterface(Solid::DeviceInterface::AudioInterface)) {
            const UdevQt::Device deviceparent(m_device.parent());
            if (deviceparent.isValid()) {
                vendor = deviceparent.deviceProperty("ID_VENDOR_FROM_DATABASE");
            }
        }

        if (vendor.isEmpty()) {
            vendor = m_device.deviceProperty("ID_VENDOR_FROM_DATABASE");;
        }

        if (vendor.isEmpty()) {
            QByteArray idvendorid(m_device.deviceProperty("ID_VENDOR_ID").toLatin1());
            idvendorid = normalizeID(idvendorid);
            if (!idvendorid.isEmpty()) {
                const QString idbus(m_device.deviceProperty("ID_BUS"));
                if (idbus == QLatin1String("pci")) {
                    vendor = lookupPCIVendor(idvendorid.constData());
                } else if (idbus == QLatin1String("usb")) {
                    vendor = lookupUSBVendor(idvendorid.constData());
                }
            }
        }

        if (vendor.isEmpty()) {
            // may not be visible to `udevadm info /dev/sdX` query but it may be there
            vendor = m_device.deviceProperty("ID_VENDOR");
        }
    }
    return vendor;
}

QString UDevDevice::product() const
{
    QString product = m_device.sysfsProperty("product");
    if (product.isEmpty()) {
        if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
            // sysfs doesn't have anything useful here
            product = extractCpuInfoLine(deviceNumber(), "model name\\s+:\\s+(\\S.+)");
        } else if(queryDeviceInterface(Solid::DeviceInterface::Video)) {
            product = m_device.deviceProperty("ID_V4L_PRODUCT");
        } else if(queryDeviceInterface(Solid::DeviceInterface::AudioInterface)) {
            const AudioInterface audioIface(const_cast<UDevDevice *>(this));
            product = audioIface.name();
        }  else if(queryDeviceInterface(Solid::DeviceInterface::NetworkInterface)) {
            const NetworkInterface netIface(const_cast<UDevDevice *>(this));
            if (netIface.isLoopback()) {
                product = QLatin1String("Loopback device Interface");
            }
        }

        if (product.isEmpty()) {
            product = m_device.deviceProperty("ID_MODEL_FROM_DATABASE");
        }

        if (product.isEmpty()) {
            QByteArray idvendorid(m_device.deviceProperty("ID_VENDOR_ID").toLatin1());
            idvendorid = normalizeID(idvendorid);
            QByteArray idmodelid(m_device.deviceProperty("ID_MODEL_ID").toLatin1());
            idmodelid = normalizeID(idmodelid);
            if (!idvendorid.isEmpty() && !idmodelid.isEmpty()) {
                const QString idbus(m_device.deviceProperty("ID_BUS"));
                if (idbus == QLatin1String("pci")) {
                    product = lookupPCIDevice(idvendorid.constData(), idmodelid.constData());
                } else if (idbus == QLatin1String("usb")) {
                    product = lookupUSBDevice(idvendorid.constData(), idmodelid.constData());
                }
            }
        }

        if (product.isEmpty()) {
            // may not be visible to `udevadm info /dev/sdX` query but it may be there
            product = m_device.deviceProperty("ID_MODEL");
        }
    }
    return product;
}

QString UDevDevice::icon() const
{
    if (parentUdi().isEmpty()) {
        return QLatin1String("computer");
#if defined(LIBCDIO_FOUND)
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
            default:
                break;
        }

        // fallback for every other optical disc
        return QLatin1String("media-optical");
#endif // LIBCDIO_FOUND
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
        const Battery batteryIface(const_cast<UDevDevice *>(this));
        const int batterypercent = batteryIface.chargePercent();
        if (batteryIface.chargeState() == Solid::Battery::Charging) {
            if (batterypercent > 80) {
                return QLatin1String("battery-charging");
            } else if (batterypercent > 60) {
                return QLatin1String("battery-charging-080");
            } else if (batterypercent > 40) {
                return QLatin1String("battery-charging-060");
            } else if (batterypercent > 20) {
                return QLatin1String("battery-charging-040");
            } else if (batterypercent > 0) {
                return QLatin1String("battery-charging-low");
            } else {
                return QLatin1String("battery-charging-caution");
            }
        } else if (batteryIface.chargeState() == Solid::Battery::Discharging) {
            if (batterypercent > 80) {
                return QLatin1String("battery");
            } else if (batterypercent > 60) {
                return QLatin1String("battery-080");
            } else if (batterypercent > 40) {
                return QLatin1String("battery-060");
            } else if (batterypercent > 20) {
                return QLatin1String("battery-040");
            } else if (batterypercent > 0) {
                return QLatin1String("battery-low");
            } else {
                return QLatin1String("battery-caution");
            }
        }
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
    } else if (queryDeviceInterface(Solid::DeviceInterface::Button)) {
        return QLatin1String("insert-button");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Graphic)) {
        return QLatin1String("video-display");
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

#if defined(LIBCDIO_FOUND)
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
#endif // LIBCDIO_FOUND
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
            desc = deviceProperty("DEVNAME");
        }
        return desc;
    } else if (queryDeviceInterface(Solid::DeviceInterface::AcAdapter)) {
        return QObject::tr("A/C Adapter");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Battery)) {
        const QString powersupplytechnology(deviceProperty("POWER_SUPPLY_TECHNOLOGY"));
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
    } else if (queryDeviceInterface(Solid::DeviceInterface::Button)) {
        const Button buttonIface(const_cast<UDevDevice *>(this));
        switch (buttonIface.type()) {
            case Solid::Button::LidButton:
                return QObject::tr("Lid Switch");
            case Solid::Button::PowerButton:
                return QObject::tr("Power Button");
            case Solid::Button::SleepButton:
                return QObject::tr("Sleep Button");
            case Solid::Button::TabletButton:
                return QObject::tr("Tablet Button");
            case Solid::Button::UnknownButtonType:
                return QObject::tr("Unknown Button");
        }
        return QString();
    } else if (queryDeviceInterface(Solid::DeviceInterface::Graphic)) {
        return QObject::tr("Graphic display");
    }

    return QString();
}

bool UDevDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
    case Solid::DeviceInterface::StorageAccess:
    case Solid::DeviceInterface::StorageDrive:
    case Solid::DeviceInterface::StorageVolume: {
        return m_device.subsystem() == QLatin1String("block");
    }

    case Solid::DeviceInterface::AcAdapter: {
        return (
            m_device.subsystem() == QLatin1String("power_supply")
            && deviceName().contains(QLatin1String("/power_supply/AC"))
        );
    }
    case Solid::DeviceInterface::Battery: {
        return (
            m_device.subsystem() == QLatin1String("power_supply")
            && deviceName().contains(QLatin1String("/power_supply/BAT"))
        );
    }

    case Solid::DeviceInterface::Processor:
        return m_device.driver() == QLatin1String("processor");

#if defined(LIBCDIO_FOUND)
    case Solid::DeviceInterface::OpticalDrive:
    case Solid::DeviceInterface::OpticalDisc:
        return (deviceProperty("ID_CDROM").toInt() == 1);
#endif

    case Solid::DeviceInterface::Camera:
        return deviceProperty("ID_GPHOTO2").toInt() == 1;

    case Solid::DeviceInterface::PortableMediaPlayer:
        return !deviceProperty("ID_MEDIA_PLAYER").isEmpty();

    case Solid::DeviceInterface::Block:
        return !deviceProperty("MAJOR").isEmpty();

    case Solid::DeviceInterface::Video:
        return m_device.subsystem() == QLatin1String("video4linux");

    case Solid::DeviceInterface::AudioInterface:
        return m_device.subsystem() == QLatin1String("sound");

    case Solid::DeviceInterface::NetworkInterface:
        return m_device.subsystem() == QLatin1String("net");

    case Solid::DeviceInterface::Button:
        return deviceProperty("ID_INPUT_KEY").toInt() == 1;

    case Solid::DeviceInterface::Graphic:
        return deviceProperty("PCI_CLASS").toInt() == 30000;

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

#if defined(LIBCDIO_FOUND)
    case Solid::DeviceInterface::OpticalDrive:
        return new OpticalDrive(this);
    case Solid::DeviceInterface::OpticalDisc:
        return new OpticalDisc(this);
#endif

    case Solid::DeviceInterface::Camera:
        return new Camera(this);

    case Solid::DeviceInterface::PortableMediaPlayer:
        return new PortableMediaPlayer(this);

    case Solid::DeviceInterface::Block:
        return new Block(this);

    case Solid::DeviceInterface::Video:
        return new Video(this);

    case Solid::DeviceInterface::AudioInterface:
        return new AudioInterface(this);

    case Solid::DeviceInterface::NetworkInterface:
        return new NetworkInterface(this);

    case Solid::DeviceInterface::Button:
        return new Button(this);

    case Solid::DeviceInterface::Graphic:
        return new Graphic(this);

    default:
        qFatal("Shouldn't happen");
        return 0;
    }
}

QString UDevDevice::device() const
{
    return devicePath();
}

QString UDevDevice::deviceProperty(const QByteArray &key) const
{
    const QString res = m_device.deviceProperty(key);
    if (!res.isEmpty()) {
        return res;
    }
    return m_device.sysfsProperty(key);
}

bool UDevDevice::devicePropertyExists(const QByteArray &key) const
{
    return m_device.deviceProperties().contains(key);
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
