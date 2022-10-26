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
#include "udevmanager.h"
#include "udevqt.h"
#include "udevdevice.h"
#include "../shared/rootdevice.h"

#include <QtCore/QSet>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDebug>

using namespace Solid::Backends::UDev;
using namespace Solid::Backends::Shared;

class UDevManager::Private
{
public:
    Private();
    ~Private();


    bool isOfInterest(const QString &udi, const UdevQt::Device &device);
    bool checkOfInterest(const UdevQt::Device &device);

    UdevQt::Client *m_client;
    QStringList m_devicesOfInterest;
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
};

UDevManager::Private::Private()
{
    static const QList<QByteArray> subsystems = QList<QByteArray>()
        << "block"
        << "power_supply"
        << "processor"
        << "cpu"
        << "sound"
        << "video4linux"
        << "net"
        << "usb"
        << "input"
        << "pci";
    m_client = new UdevQt::Client(subsystems);
}

UDevManager::Private::~Private()
{
    delete m_client;
}

bool UDevManager::Private::isOfInterest(const QString &udi, const UdevQt::Device &device)
{
    if (m_devicesOfInterest.contains(udi)) {
        return true;
    }

    bool isOfInterest = checkOfInterest(device);
    if (isOfInterest) {
        m_devicesOfInterest.append(udi);
    }

    return isOfInterest;
}

bool UDevManager::Private::checkOfInterest(const UdevQt::Device &device)
{
#ifdef UDEV_DETAILED_OUTPUT
    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    qDebug() << "Path:" << device.sysfsPath();
    qDebug() << "Properties:" << device.deviceProperties();
    Q_FOREACH (const QString &key, device.deviceProperties()) {
        qDebug() << "\t" << key << ":" << device.deviceProperty(key.toLatin1());
    }
    qDebug() << "Driver:" << device.driver();
    qDebug() << "Subsystem:" << device.subsystem();
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
#endif
    if (device.subsystem() == QLatin1String("block")) {
        return !device.deviceProperty("ID_FS_TYPE").isEmpty();
    }

    if (device.subsystem() == QLatin1String("power_supply")) {
        return true;
    }

    if (device.driver() == QLatin1String("processor")) {
        // Linux ACPI reports processor slots, rather than processors.
        // Empty slots will not have a system device associated with them.
        QDir sysfsDir(device.sysfsPath());
        return (sysfsDir.exists("sysdev") || sysfsDir.exists("cpufreq") || sysfsDir.exists("topology/core_id"));
    }
    if (device.subsystem() == QLatin1String("sound") &&
        device.deviceProperty("SOUND_FORM_FACTOR") != "internal") {
        return true;
    }

    if (device.subsystem() == QLatin1String("pci")) {
        const QString pciclass = device.deviceProperty("PCI_CLASS");
        return (pciclass == QLatin1String("30000")); // VGA controller
    }

    if (device.subsystem() == QLatin1String("input")) {
        const QStringList deviceProperties = device.deviceProperties();
        return (device.deviceProperty("ID_INPUT_KEY").toInt() == 1 && (deviceProperties.contains("KEY") || deviceProperties.contains("SW")));
    }

    return device.subsystem() == QLatin1String("video4linux") ||
           device.subsystem() == QLatin1String("net") ||
           device.deviceProperty("ID_MEDIA_PLAYER").toInt() == 1 || // media-player-info recognized devices
           (device.deviceProperty("ID_GPHOTO2").toInt() == 1 && device.parent().deviceProperty("ID_GPHOTO2").toInt() != 1); // GPhoto2 cameras
}

UDevManager::UDevManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent),
      d(new Private)
{
    connect(d->m_client, SIGNAL(deviceAdded(UdevQt::Device)), this, SLOT(slotDeviceAdded(UdevQt::Device)));
    connect(d->m_client, SIGNAL(deviceRemoved(UdevQt::Device)), this, SLOT(slotDeviceRemoved(UdevQt::Device)));
    connect(d->m_client, SIGNAL(deviceChanged(UdevQt::Device)), this, SLOT(slotDeviceChanged(UdevQt::Device)));

    d->m_supportedInterfaces << Solid::DeviceInterface::StorageAccess
                             << Solid::DeviceInterface::StorageDrive
                             << Solid::DeviceInterface::StorageVolume
                             << Solid::DeviceInterface::AcAdapter
                             << Solid::DeviceInterface::Battery
                             << Solid::DeviceInterface::Processor
#if defined(LIBCDIO_FOUND)
                             << Solid::DeviceInterface::OpticalDrive
                             << Solid::DeviceInterface::OpticalDisc
#endif
                             << Solid::DeviceInterface::AudioInterface
                             << Solid::DeviceInterface::NetworkInterface
                             << Solid::DeviceInterface::Camera
                             << Solid::DeviceInterface::PortableMediaPlayer
                             << Solid::DeviceInterface::Block
                             << Solid::DeviceInterface::Video
                             << Solid::DeviceInterface::Button
                             << Solid::DeviceInterface::Graphic;
}

UDevManager::~UDevManager()
{
    delete d;
}

QString UDevManager::udiPrefix() const
{
    return QString::fromLatin1(UDEV_UDI_PREFIX);
}

QSet<Solid::DeviceInterface::Type> UDevManager::supportedInterfaces() const
{
    return d->m_supportedInterfaces;
}

QStringList UDevManager::allDevices()
{
    QStringList res;
    foreach (const UdevQt::Device &device, d->m_client->allDevices()) {
        if (d->isOfInterest(udiPrefix() + device.sysfsPath(), device)) {
            res << udiPrefix() + device.sysfsPath();
        }
    }
    return res;
}

QStringList UDevManager::devicesFromQuery(const QString &parentUdi,
                                          Solid::DeviceInterface::Type type)
{
    QStringList allDev = allDevices();
    QStringList result;

    if (!parentUdi.isEmpty()) {
        foreach (const QString &udi, allDev) {
            UDevDevice device(d->m_client->deviceBySysfsPath(udi.right(udi.size() - udiPrefix().size())));
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi) {
                result << udi;
            }
        }

        return result;
    } else if (type != Solid::DeviceInterface::Unknown) {
        foreach (const QString &udi, allDev) {
            UDevDevice device(d->m_client->deviceBySysfsPath(udi.right(udi.size() - udiPrefix().size())));
            if (device.queryDeviceInterface(type)) {
                result << udi;
            }
        }

        return result;
    } else {
        return allDev;
    }
}

QObject *UDevManager::createDevice(const QString &udi_)
{
    if (udi_ == udiPrefix()) {
        RootDevice *const device = new RootDevice(UDEV_UDI_PREFIX);
        device->setProduct(tr("Devices"));
        device->setDescription(tr("Devices declared in your system"));
        device->setIcon("computer");
        return device;
    }

    const QString udi = udi_.right(udi_.size() - udiPrefix().size());
    UdevQt::Device device = d->m_client->deviceBySysfsPath(udi);

    if (d->isOfInterest(udi_, device) || QFile::exists(udi)) {
        return new UDevDevice(device);
    }

    return 0;
}

void UDevManager::slotDeviceAdded(const UdevQt::Device &device)
{
    if (d->isOfInterest(udiPrefix() + device.sysfsPath(), device)) {
        emit deviceAdded(udiPrefix() + device.sysfsPath());
    }
}

void UDevManager::slotDeviceRemoved(const UdevQt::Device &device)
{
    if (d->isOfInterest(udiPrefix() + device.sysfsPath(), device)) {
        emit deviceRemoved(udiPrefix() + device.sysfsPath());
        d->m_devicesOfInterest.removeAll(udiPrefix() + device.sysfsPath());
    }
}

void UDevManager::slotDeviceChanged(const UdevQt::Device &device)
{
    if (d->isOfInterest(udiPrefix() + device.sysfsPath(), device)) {
        if (device.subsystem() == "block") {
            const QString idfsusage = device.deviceProperty("ID_FS_USAGE");
            const bool hascontent = (idfsusage == "filesystem" || idfsusage == "crypto");
            emit contentChanged(udiPrefix() + device.sysfsPath(), hascontent);
        }
    }
}
