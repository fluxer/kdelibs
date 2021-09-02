/*
    Copyright 2009 Harald Fernengel <harry@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which sudevl
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "udevserialinterface.h"
#include "udevdevice.h"

#include <QString>

#include <stdio.h>

using namespace Solid::Backends::UDev;

SerialInterface::SerialInterface(UDevDevice *device)
    : DeviceInterface(device),
    m_portnum(-1),
    m_type(Solid::SerialInterface::Unknown)
{
    const QString path = m_device->deviceName();
    const int lastSlash = path.length() - path.lastIndexOf(QLatin1String("/")) -1;
    const QByteArray lastElement = path.right(lastSlash).toLatin1();

    if (::sscanf(lastElement.constData(), "ttyS%d", &m_portnum) == 1) {
        ;
    } else if (::sscanf(lastElement.constData(), "ttyUSB%d", &m_portnum) == 1) {
        ;
    }

    const QString idbus(m_device->deviceProperty("ID_BUS"));
    if (idbus == "pci") {
        m_type = Solid::SerialInterface::Pci;
    } else if (idbus == "usb" || path.contains("ttyUSB")) {
        m_type = Solid::SerialInterface::Usb;
    } else {
        m_type = Solid::SerialInterface::Platform;
    }
}

SerialInterface::~SerialInterface()
{
}

QVariant SerialInterface::driverHandle() const
{
    return m_device->deviceProperty("DEVNAME");
}

Solid::SerialInterface::SerialType SerialInterface::serialType() const
{
    return m_type;
}

int SerialInterface::port() const
{
    return m_portnum;
}

#include "backends/udev/moc_udevserialinterface.cpp"
