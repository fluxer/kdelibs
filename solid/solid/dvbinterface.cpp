/*
    Copyright 2007 Kevin Ottens <ervin@kde.org>

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

#include "dvbinterface.h"
#include "dvbinterface_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/dvbinterface.h>

Solid::DvbInterface::DvbInterface(QObject *backendObject)
    : DeviceInterface(*new DvbInterfacePrivate(), backendObject)
{
}

Solid::DvbInterface::~DvbInterface()
{

}

QString Solid::DvbInterface::device() const
{
    Q_D(const DvbInterface);
    return_SOLID_CALL(Ifaces::DvbInterface *, d->backendObject(), QString(), device());
}

int Solid::DvbInterface::deviceAdapter() const
{
    Q_D(const DvbInterface);
    return_SOLID_CALL(Ifaces::DvbInterface *, d->backendObject(), -1, deviceAdapter());
}

Solid::DvbInterface::DeviceType Solid::DvbInterface::deviceType() const
{
    Q_D(const DvbInterface);
    return_SOLID_CALL(Ifaces::DvbInterface *, d->backendObject(), DvbUnknown, deviceType());
}

int Solid::DvbInterface::deviceIndex() const
{
    Q_D(const DvbInterface);
    return_SOLID_CALL(Ifaces::DvbInterface *, d->backendObject(), -1, deviceIndex());
}

#include "moc_dvbinterface.cpp"
