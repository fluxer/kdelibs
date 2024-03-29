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
#include "udevinput.h"
#include "udevmanager.h"
#include "cpuinfo.h"

#include "kglobal.h"
#include "klocale.h"
#include "kdevicedatabase.h"

#if defined(LIBCDIO_FOUND)
#include "udevopticaldisc.h"
#include "udevopticaldrive.h"
#endif

#include <QDebug>

using namespace Solid::Backends::UDev;

static KDeviceDatabase s_devicedb;

UDevDevice::UDevDevice(const UdevQt::Device &device)
    : Solid::Ifaces::Device()
    , m_device(device)
{
}

UDevDevice::~UDevDevice()
{
}

QString UDevDevice::udi() const
{
    return QString::fromLatin1(UDEV_UDI_PREFIX) + deviceName();
}

QString UDevDevice::parentUdi() const
{
    const int idcdrom = m_device.deviceProperty("ID_CDROM").toInt();
#warning FIXME: block devices workaround for optical and storage drives
    // code in several places expects the parent to NOT be the actual parent (disk) device UDI even
    // for partitions but another device UDI related to this device, has to be fixed and verified
    // to work at some point
    if (m_device.subsystem() == "block" || idcdrom == 1) {
        return udi();
    }

    return QString::fromLatin1(UDEV_UDI_PREFIX);
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
            const QByteArray idvendorid(m_device.deviceProperty("ID_VENDOR_ID").toLatin1());
            if (!idvendorid.isEmpty()) {
                const QString idbus(m_device.deviceProperty("ID_BUS"));
                if (idbus == QLatin1String("pci")) {
                    vendor = s_devicedb.lookupPCIVendor(idvendorid);
                } else if (idbus == QLatin1String("usb")) {
                    vendor = s_devicedb.lookupUSBVendor(idvendorid);
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
            const QByteArray idvendorid(m_device.deviceProperty("ID_VENDOR_ID").toLatin1());
            const QByteArray idmodelid(m_device.deviceProperty("ID_MODEL_ID").toLatin1());
            if (!idvendorid.isEmpty() && !idmodelid.isEmpty()) {
                const QString idbus(m_device.deviceProperty("ID_BUS"));
                if (idbus == QLatin1String("pci")) {
                    product = s_devicedb.lookupPCIDevice(idvendorid, idmodelid);
                } else if (idbus == QLatin1String("usb")) {
                    product = s_devicedb.lookupUSBDevice(idvendorid, idmodelid);
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
        }
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageVolume)) {
        return QLatin1String("drive-harddisk");
    } if (queryDeviceInterface(Solid::DeviceInterface::AcAdapter)) {
        return QLatin1String("preferences-system-power-management");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Battery)) {
        const Battery batteryIface(const_cast<UDevDevice *>(this));
        const int batterypercent = batteryIface.chargePercent();
        if (batteryIface.chargeState() == Solid::Battery::Discharging) {
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
        } else if (batteryIface.chargeState() != Solid::Battery::UnknownCharge) {
            // either charging or fully charged
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
    } else if (queryDeviceInterface(Solid::DeviceInterface::Input)) {
        const Input inputIface(const_cast<UDevDevice *>(this));
        switch (inputIface.inputType()) {
        case Solid::Input::UnknownInput:
            return QString();
        case Solid::Input::Mouse:
            return QLatin1String("input-mouse");
        case Solid::Input::Keyboard:
            return QLatin1String("input-keyboard");
        case Solid::Input::Joystick:
            return QLatin1String("input-gaming");
        }
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
        return i18n("Computer");
    }

#if defined(LIBCDIO_FOUND)
    // prioritize since it is a storage drive/disc too
    if (queryDeviceInterface(Solid::DeviceInterface::OpticalDrive)) {
        const OpticalDrive opticalDrive(const_cast<UDevDevice*>(this));
        Solid::OpticalDrive::MediumTypes mediumTypes = opticalDrive.supportedMedia();
        QString first;
        QString second;

        first = i18n("CD-ROM");
        if (mediumTypes & Solid::OpticalDrive::Cdr)
            first = i18n("CD-R");
        if (mediumTypes & Solid::OpticalDrive::Cdrw)
            first = i18n("CD-RW");

        if (mediumTypes & Solid::OpticalDrive::Dvd)
            second = i18n("/DVD-ROM");
        if (mediumTypes & Solid::OpticalDrive::Dvdplusr)
            second = i18n("/DVD+R");
        if (mediumTypes & Solid::OpticalDrive::Dvdplusrw)
            second = i18n("/DVD+RW");
        if (mediumTypes & Solid::OpticalDrive::Dvdr)
            second = i18n("/DVD-R");
        if (mediumTypes & Solid::OpticalDrive::Dvdrw)
            second = i18n("/DVD-RW");
        if (mediumTypes & Solid::OpticalDrive::Dvdram)
            second = i18n("/DVD-RAM");
        if ((mediumTypes & Solid::OpticalDrive::Dvdr) && (mediumTypes & Solid::OpticalDrive::Dvdplusr)) {
            if (mediumTypes & Solid::OpticalDrive::Dvdplusdl) {
                second = i18n("/DVD±R DL");
            } else {
                second = i18n("/DVD±R");
            }
        }
        if ((mediumTypes & Solid::OpticalDrive::Dvdrw) && (mediumTypes & Solid::OpticalDrive::Dvdplusrw)) {
            if((mediumTypes & Solid::OpticalDrive::Dvdplusdl) || (mediumTypes & Solid::OpticalDrive::Dvdplusdlrw)) {
                second = i18n("/DVD±RW DL");
            } else {
                second = i18n("/DVD±RW");
            }
        }
        if (mediumTypes & Solid::OpticalDrive::Bd)
            second = i18n("/BD-ROM");
        if (mediumTypes & Solid::OpticalDrive::Bdr)
            second = i18n("/BD-R");
        if (mediumTypes & Solid::OpticalDrive::Bdre)
            second = i18n("/BD-RE");
        if (mediumTypes & Solid::OpticalDrive::HdDvd)
            second = i18n("/HD DVD-ROM");
        if (mediumTypes & Solid::OpticalDrive::HdDvdr)
            second = i18n("/HD DVD-R");
        if (mediumTypes & Solid::OpticalDrive::HdDvdrw)
            second = i18n("/HD DVD-RW");

        QString description;
        if (opticalDrive.isHotpluggable()) {
            description = i18n("External %1%2 Drive");
        } else {
            description = i18n("%1%2 Drive");
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
            return i18n("%1 Hard Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::CdromDrive) {
            return i18n("%1 CD-ROM Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::Floppy) {
            return i18n("%1 Floppy Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::Tape) {
            return i18n("%1 Tape Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::CompactFlash) {
            return i18n("%1 Compact Flash Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::MemoryStick) {
            return i18n("%1 Memory Stick Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::SmartMedia) {
            return i18n("%1 Smart Media Drive").arg(storagesize);
        } else if (drivetype == Solid::StorageDrive::SdMmc) {
            return i18n("%1 SD/MMC Drive").arg(storagesize);
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
        return i18n("A/C Adapter");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Battery)) {
        const QString powersupplytechnology(deviceProperty("POWER_SUPPLY_TECHNOLOGY"));
        if (powersupplytechnology == QLatin1String("NiMH")) {
            return i18n("Nickel Metal Hydride Battery");
        } else if (powersupplytechnology == QLatin1String("Li-ion")) {
            return i18n("Lithium Ion Battery");
        } else if (powersupplytechnology == QLatin1String("Li-poly")) {
            return i18n("Lithium Polymer Battery");
        } else if (powersupplytechnology == QLatin1String("LiFe")) {
            return i18n("Lithium Iron Disulfide Battery");
        } else if (powersupplytechnology == QLatin1String("NiCd")) {
            return i18n("Nickel Cadmium Battery");
        } else if (powersupplytechnology == QLatin1String("LiMn")) {
            return i18n("Lithium Manganese Dioxide Battery");
        }
        return i18n("Unknown Battery");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return i18n("Processor");
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
            return i18n("Portable Media Player");
        }
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return i18n("Camera");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Video)) {
        return product();
    } else if (queryDeviceInterface(Solid::DeviceInterface::AudioInterface)) {
        return product();
    } else if (queryDeviceInterface(Solid::DeviceInterface::NetworkInterface)) {
        const NetworkInterface networkIface(const_cast<UDevDevice *>(this));
        if (networkIface.isWireless()) {
            return i18n("WLAN Interface");
        }
        return i18n("Networking Interface");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Button)) {
        const Button buttonIface(const_cast<UDevDevice *>(this));
        switch (buttonIface.type()) {
            case Solid::Button::LidButton:
                return i18n("Lid Switch");
            case Solid::Button::PowerButton:
                return i18n("Power Button");
            case Solid::Button::SleepButton:
                return i18n("Sleep Button");
            case Solid::Button::TabletButton:
                return i18n("Tablet Button");
            case Solid::Button::UnknownButtonType:
                return i18n("Unknown Button");
        }
        return QString();
    } else if (queryDeviceInterface(Solid::DeviceInterface::Graphic)) {
        return i18n("Graphic display");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Input)) {
        const Input inputIface(const_cast<UDevDevice *>(this));
        switch (inputIface.inputType()) {
            case Solid::Input::UnknownInput:
                return i18n("Unknown Input");
            case Solid::Input::Mouse:
                return i18n("Mouse");
            case Solid::Input::Keyboard:
                return i18n("Keyboard");
            case Solid::Input::Joystick:
                return i18n("Joystick");
        }
        return QString();
    }

    return QString();
}

bool UDevDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
    case Solid::DeviceInterface::StorageAccess:
    case Solid::DeviceInterface::StorageDrive:
    case Solid::DeviceInterface::StorageVolume: {
        return m_device.subsystem() == "block";
    }

    case Solid::DeviceInterface::AcAdapter: {
        const QString powersupplytype = deviceProperty("POWER_SUPPLY_TYPE").toLower();
        return (
            m_device.subsystem() == "power_supply"
            && (powersupplytype == QLatin1String("mains")
            || powersupplytype.contains(QLatin1String("ups")))
        );
    }
    case Solid::DeviceInterface::Battery: {
        const QString powersupplytype = deviceProperty("POWER_SUPPLY_TYPE").toLower();
        return (
            m_device.subsystem() == "power_supply" &&
            (powersupplytype == QLatin1String("battery")
            || powersupplytype.contains(QLatin1String("usb")))
        );
    }

    case Solid::DeviceInterface::Processor:
        return m_device.driver() == "processor";

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
        return m_device.subsystem() == "video4linux";

    case Solid::DeviceInterface::AudioInterface:
        return m_device.subsystem() == "sound";

    case Solid::DeviceInterface::NetworkInterface:
        return m_device.subsystem() == "net";

    case Solid::DeviceInterface::Button:
        return deviceProperty("ID_INPUT_KEY").toInt() == 1;

    case Solid::DeviceInterface::Graphic:
        return deviceProperty("PCI_CLASS").toInt() == 30000;

    case Solid::DeviceInterface::Input:
        return (
            deviceProperty("ID_INPUT_MOUSE").toInt() == 1
            || deviceProperty("ID_INPUT_KEYBOARD").toInt() == 1
            || deviceProperty("ID_INPUT_JOYSTICK").toInt() == 1
        );

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

    case Solid::DeviceInterface::Input:
        return new Input(this);

    default:
        qFatal("Shouldn't happen");
        return 0;
    }
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

UdevQt::Device UDevDevice::udevDevice() const
{
    return m_device;
}
