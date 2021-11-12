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
    Private(DevinfoManager *parent);
    ~Private();

    bool isOfInterest(const DevdQt::Device &device);

    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
    DevdQt::Client *m_client;
    DevinfoManager *m_parent;
};

DevinfoManager::Private::Private(DevinfoManager *parent)
    : m_client(new DevdQt::Client()),
    m_parent(parent)
{
}

DevinfoManager::Private::~Private()
{
    delete m_client;
}

bool DevinfoManager::Private::isOfInterest(const DevdQt::Device &device)
{
    const QByteArray devicename(device.device());
    const QString devicestring = QString::fromLatin1(devicename.constData(), devicename.size());
    const QString deviceudi = QString::fromLatin1("%1/%2").arg(DEVINFO_UDI_PREFIX, devicestring);
    const QStringList allDev = m_parent->allDevices();
    // qDebug() << Q_FUNC_INFO << deviceudi << allDev;
    return (allDev.contains(deviceudi));
}

DevinfoManager::DevinfoManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent),
      d(new Private(this))
{
    connect(d->m_client, SIGNAL(deviceAdded(DevdQt::Device)), this, SLOT(slotDeviceAdded(DevdQt::Device)));
    connect(d->m_client, SIGNAL(deviceRemoved(DevdQt::Device)), this, SLOT(slotDeviceRemoved(DevdQt::Device)));
    // deviceChanged() is relevant only for block devices

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

void DevinfoManager::slotDeviceAdded(const DevdQt::Device &device)
{
    if (d->isOfInterest(device)) {
        emit deviceAdded(udiPrefix() + device.device());
    }
}

void DevinfoManager::slotDeviceRemoved(const DevdQt::Device &device)
{
    if (d->isOfInterest(device)) {
        emit deviceRemoved(udiPrefix() + device.device());
    }
}
