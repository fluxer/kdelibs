/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "deviceinterface.h"
#include "deviceinterface_p.h"
#include "solid/ifaces/deviceinterface.h"
#include "klocale.h"

#include <QMetaEnum>

Solid::DeviceInterface::DeviceInterface(DeviceInterfacePrivate &dd, QObject *backendObject)
    : d_ptr(&dd)
{
    Q_D(DeviceInterface);

    d->setBackendObject(backendObject);
}


Solid::DeviceInterface::~DeviceInterface()
{
    delete d_ptr;
    d_ptr = 0;
}

bool Solid::DeviceInterface::isValid() const
{
    Q_D(const DeviceInterface);
    return d->backendObject()!=0;
}

QString Solid::DeviceInterface::typeToString(Type type)
{
    int index = staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    return QString(metaEnum.valueToKey((int)type));
}

Solid::DeviceInterface::Type Solid::DeviceInterface::stringToType(const QString &type)
{
    int index = staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    return (Type)metaEnum.keyToValue(type.toUtf8());
}

QString Solid::DeviceInterface::typeDescription(Type type)
{
    switch (type) {
        case Solid::DeviceInterface::Unknown:
            return i18n("Unknown");
        case Solid::DeviceInterface::Processor:
            return i18n("Processor");
        case Solid::DeviceInterface::Block:
            return i18n("Block");
        case Solid::DeviceInterface::StorageAccess:
            return i18n("Storage Access");
        case Solid::DeviceInterface::StorageDrive:
            return i18n("Storage Drive");
        case Solid::DeviceInterface::OpticalDrive:
            return i18n("Optical Drive");
        case Solid::DeviceInterface::StorageVolume:
            return i18n("Storage Volume");
        case Solid::DeviceInterface::OpticalDisc:
            return i18n("Optical Disc");
        case Solid::DeviceInterface::Camera:
            return i18n("Camera");
        case Solid::DeviceInterface::PortableMediaPlayer:
            return i18n("Portable Media Player");
        case Solid::DeviceInterface::NetworkInterface:
            return i18n("Network Interface");
        case Solid::DeviceInterface::AcAdapter:
            return i18n("Ac Adapter");
        case Solid::DeviceInterface::Battery:
            return i18n("Battery");
        case Solid::DeviceInterface::Button:
            return i18n("Button");
        case Solid::DeviceInterface::AudioInterface:
            return i18n("Audio Interface");
        case Solid::DeviceInterface::Video:
            return i18n("Video");
        case Solid::DeviceInterface::Graphic:
            return i18n("Graphic");
        case Solid::DeviceInterface::Input:
            return i18n("Input");
        case Solid::DeviceInterface::Last:
            return QString();
    }
    return QString();
}

Solid::DeviceInterfacePrivate::DeviceInterfacePrivate()
    : m_devicePrivate(0)
{
}

Solid::DeviceInterfacePrivate::~DeviceInterfacePrivate()
{
}

QObject *Solid::DeviceInterfacePrivate::backendObject() const
{
    return m_backendObject.data();
}

void Solid::DeviceInterfacePrivate::setBackendObject(QObject *object)
{
    m_backendObject = object;
}

Solid::DevicePrivate* Solid::DeviceInterfacePrivate::devicePrivate() const
{
    return m_devicePrivate;
}

void Solid::DeviceInterfacePrivate::setDevicePrivate(DevicePrivate *devicePrivate)
{
    m_devicePrivate = devicePrivate;
}

#include "moc_deviceinterface.cpp"
