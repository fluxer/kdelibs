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

#include "devinfomanager.h"
#include "devinfodevice.h"

#include <QtCore/QSet>
#include <QtCore/QDebug>

#include <devinfo.h>

using namespace Solid::Backends::Devinfo;

int getDeviceList(struct devinfo_dev *dev, void *arg)
{
    const QString devname = QString::fromLatin1(dev->dd_name);
    if (!devname.isEmpty()) {
        const QString devudi = QString::fromLatin1("%1/%2").arg(DEVINFO_UDI_PREFIX, devname);
        reinterpret_cast<QStringList*>(arg)->append(devudi);
    }
    return devinfo_foreach_device_child(dev, getDeviceList, arg);
}

class DevinfoManager::Private
{
public:
    Private();
    ~Private();

    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
};

DevinfoManager::Private::Private()
{
}

DevinfoManager::Private::~Private()
{
}

DevinfoManager::DevinfoManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent),
      d(new Private)
{
    d->m_supportedInterfaces
        << Solid::DeviceInterface::Processor
        << Solid::DeviceInterface::NetworkInterface
        << Solid::DeviceInterface::Graphic;
}

DevinfoManager::~DevinfoManager()
{
    delete d;
}

QString DevinfoManager::udiPrefix() const
{
    return QString(DEVINFO_UDI_PREFIX);
}

QSet<Solid::DeviceInterface::Type> DevinfoManager::supportedInterfaces() const
{
    return d->m_supportedInterfaces;
}

QStringList DevinfoManager::allDevices()
{
    QStringList result;
    if (devinfo_init() != 0) {
        qWarning() << "could not initialize devinfo";
        return result;
    }
    struct devinfo_dev *root = devinfo_handle_to_device(DEVINFO_ROOT_DEVICE);
    if (root) {
        devinfo_foreach_device_child(root, getDeviceList, &result);
    } else {
        qWarning() << "no root device";
    }
    devinfo_free();
    return result;
}

QStringList DevinfoManager::devicesFromQuery(const QString &parentUdi,
                                          Solid::DeviceInterface::Type type)
{
    QStringList allDev = allDevices();
    QStringList result;

    if (!parentUdi.isEmpty()) {
        foreach (const QString &udi, allDev) {
            DevinfoDevice device(udi);
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi) {
                result << udi;
            }
        }
        return result;
    } else if (type != Solid::DeviceInterface::Unknown) {
        foreach (const QString &udi, allDev) {
            DevinfoDevice device(udi);
            if (device.queryDeviceInterface(type)) {
                result << udi;
            }
        }
        return result;
    } else {
        return allDev;
    }
}

QObject *DevinfoManager::createDevice(const QString &udi)
{
    if (udi == udiPrefix()) {
        return new DevinfoDevice(DEVINFO_ROOT_UDI);
    }

    if (!udi.isEmpty()) {
        return new DevinfoDevice(udi);
    }

    qWarning() << "cannot create device for UDI" << udi;
    return 0;
}
