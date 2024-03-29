/*
    Copyright 2005-2007 Kevin Ottens <ervin@kde.org>

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

#include "devicenotifier.h"
#include "devicemanager_p.h" //krazy:exclude=includes (devicenotifier.h is the header file for this class)

#include "device.h"
#include "device_p.h"
#include "predicate.h"

#include "ifaces/devicemanager.h"
#include "ifaces/device.h"

#include "soliddefs_p.h"

Q_GLOBAL_STATIC(Solid::DeviceManagerStorage, globalDeviceStorage)

thread_local Solid::DeviceManagerPrivate* Solid::DeviceManagerStorage::m_storage = 0;

Solid::DeviceManagerPrivate::DeviceManagerPrivate()
    : m_nullDevice(new DevicePrivate(QString()))
{
    loadBackends();

    foreach (QObject *backend, managerBackends()) {
        connect(backend, SIGNAL(deviceAdded(QString)),
                this, SLOT(_k_deviceAdded(QString)));
        connect(backend, SIGNAL(deviceRemoved(QString)),
                this, SLOT(_k_deviceRemoved(QString)));
        connect(backend, SIGNAL(contentChanged(QString,bool)),
                this, SLOT(_k_contentChanged(QString,bool)));
    }
}

Solid::DeviceManagerPrivate::~DeviceManagerPrivate()
{
    foreach (QObject *backend, managerBackends()) {
        disconnect(backend, 0, this, 0);
    }

    foreach (QWeakPointer<DevicePrivate> dev, m_devicesMap) {
        DevicePrivate *devData = dev.data();
        disconnect(devData, 0, this, 0);
        if (!devData->ref.deref()) {
            delete devData;
        }
    }

    m_devicesMap.clear();
}

QList<Solid::Device> Solid::Device::allDevices()
{
    QList<Device> list;

    foreach (QObject *backendObj, globalDeviceStorage()->managerBackends()) {
        Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager *>(backendObj);

        if (backend == 0) {
            continue;
        }

        foreach (const QString &udi, backend->allDevices()) {
            list.append(Device(udi));
        }
    }

    return list;
}

QList<Solid::Device> Solid::Device::listFromQuery(const QString &predicate,
                                                  const QString &parentUdi)
{
    Predicate p = Predicate::fromString(predicate);

    if (p.isValid())
    {
        return listFromQuery(p, parentUdi);
    }
    else
    {
        return QList<Device>();
    }
}

QList<Solid::Device> Solid::Device::listFromType(const DeviceInterface::Type &type,
                                                 const QString &parentUdi)
{
    QList<Device> list;

    foreach (QObject *backendObj, globalDeviceStorage()->managerBackends()) {
        Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager *>(backendObj);

        if (backend == 0) {
            continue;
        }
        if (!backend->supportedInterfaces().contains(type)) {
            continue;
        }

        foreach (const QString &udi, backend->devicesFromQuery(parentUdi, type)) {
            list.append(Device(udi));
        }
    }

    return list;
}

QList<Solid::Device> Solid::Device::listFromQuery(const Predicate &predicate,
                                                  const QString &parentUdi)
{
    QList<Device> list;
    QSet<DeviceInterface::Type> usedTypes = predicate.usedTypes();

    foreach (QObject *backendObj, globalDeviceStorage()->managerBackends()) {
        Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager *>(backendObj);

        if (backend == 0) {
            continue;
        }

        QStringList udis;
        if (predicate.isValid()) {
            QSet<DeviceInterface::Type> supportedTypes = backend->supportedInterfaces();
            if (supportedTypes.intersect(usedTypes).isEmpty()) {
                continue;
            }

            foreach (DeviceInterface::Type type, supportedTypes) {
                udis += backend->devicesFromQuery(parentUdi, type);
            }
        } else {
            udis += backend->allDevices();
        }

        foreach (const QString &udi, udis) {
            Device dev(udi);

            bool matches = false;

            if (!predicate.isValid()) {
                matches = true;
            } else {
                matches = predicate.matches(dev);
            }

            if (matches) {
                list.append(dev);
            }
        }
    }

    return list;
}

Solid::DeviceNotifier *Solid::DeviceNotifier::instance()
{
    return globalDeviceStorage()->notifier();
}

void Solid::DeviceManagerPrivate::_k_deviceAdded(const QString &udi)
{
    if (m_devicesMap.contains(udi)) {
        DevicePrivate *dev = m_devicesMap[udi].data();

        // Ok, this one was requested somewhere was invalid
        // and now becomes magically valid!

        if (dev && dev->backendObject() == 0) {
            dev->setBackendObject(createBackendObject(udi));
            Q_ASSERT(dev->backendObject()!=0);
        }
    }

    emit deviceAdded(udi);
}

void Solid::DeviceManagerPrivate::_k_deviceRemoved(const QString &udi)
{
    if (m_devicesMap.contains(udi)) {
        DevicePrivate *dev = m_devicesMap[udi].data();

        // Ok, this one was requested somewhere was valid
        // and now becomes magically invalid!

        if (dev) {
            Q_ASSERT(dev->backendObject()!=0);
            dev->setBackendObject(0);
            Q_ASSERT(dev->backendObject()==0);
        }
    }

    emit deviceRemoved(udi);
}

void Solid::DeviceManagerPrivate::_k_contentChanged(const QString &udi, const bool hascontent)
{
    emit contentChanged(udi, hascontent);
}

void Solid::DeviceManagerPrivate::_k_destroyed(QObject *object)
{
    QString udi = m_reverseMap.take(object);

    if (!udi.isEmpty()) {
        m_devicesMap.remove(udi);
    }
}

Solid::DevicePrivate *Solid::DeviceManagerPrivate::findRegisteredDevice(const QString &udi)
{
    if (udi.isEmpty()) {
        return m_nullDevice.data();
    } else if (m_devicesMap.contains(udi)) {
        return m_devicesMap[udi].data();
    } else {
        Ifaces::Device *iface = createBackendObject(udi);

        DevicePrivate *devData = new DevicePrivate(udi);
        devData->setBackendObject(iface);

        QWeakPointer<DevicePrivate> ptr(devData);
        m_devicesMap[udi] = ptr;
        m_reverseMap[devData] = udi;

        connect(devData, SIGNAL(destroyed(QObject*)),
                this, SLOT(_k_destroyed(QObject*)));

        return devData;
    }
}

Solid::Ifaces::Device *Solid::DeviceManagerPrivate::createBackendObject(const QString &udi)
{
    foreach (QObject *backendObj, globalDeviceStorage()->managerBackends()) {
        Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager *>(backendObj);

        if (backend == 0) {
            continue;
        }
        if (!udi.startsWith(backend->udiPrefix())) {
            continue;
        }

        QObject *object = backend->createDevice(udi);
        Ifaces::Device *iface = qobject_cast<Ifaces::Device *>(object);

        if (iface == 0) {
            delete object;
        }

        return iface;
    }

    return 0;
}

Solid::DeviceManagerStorage::DeviceManagerStorage()
{

}

Solid::DeviceManagerStorage::~DeviceManagerStorage()
{
    if (m_storage) {
        delete m_storage;
        m_storage = 0;
    }
}

QList<QObject*> Solid::DeviceManagerStorage::managerBackends()
{
    ensureManagerCreated();
    return m_storage->managerBackends();
}

Solid::DeviceNotifier *Solid::DeviceManagerStorage::notifier()
{
    ensureManagerCreated();
    return m_storage;
}

void Solid::DeviceManagerStorage::ensureManagerCreated()
{
    if (!m_storage) {
        m_storage = new DeviceManagerPrivate();
    }
}

#include "moc_devicenotifier.cpp"
#include "moc_devicemanager_p.cpp"

