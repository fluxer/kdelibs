/*
    Copyright 2009 Benjamin K. Stuhl <bks24@cornell.edu>

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

#include "udevqt.h"

#include <QtCore/QByteArray>

namespace UdevQt {

Device::Device()
    : m_device(nullptr)
{
}

Device::Device(struct udev_device *device, bool ref)
    : m_device(device)
{
    if (m_device && ref)
        udev_device_ref(m_device);
}

Device::Device(const Device &other)
    : m_device(other.m_device)
{
    if (other.m_device) {
        udev_device_ref(other.m_device);
    }
}

Device::~Device()
{
    if (m_device) {
        udev_device_unref(m_device);
    }
}

Device &Device::operator=(const Device &other)
{
    if (m_device) {
        udev_device_unref(m_device);
    }
    if (other.m_device) {
        m_device = udev_device_ref(other.m_device);
    } else {
        m_device = nullptr;
    }
    return *this;
}

bool Device::isValid() const
{
    return (m_device != nullptr);
}

QByteArray Device::subsystem() const
{
    if (!m_device) {
        return QByteArray();
    }
    return QByteArray(udev_device_get_subsystem(m_device));
}

QString Device::devType() const
{
    if (!m_device) {
        return QString();
    }
    return QString::fromUtf8(udev_device_get_devtype(m_device));
}

QString Device::name() const
{
    if (!m_device) {
        return QString();
    }
    return QString::fromUtf8(udev_device_get_sysname(m_device));
}

QString Device::sysfsPath() const
{
    if (!m_device) {
        return QString();
    }
    return QString::fromUtf8(udev_device_get_syspath(m_device));
}

int Device::sysfsNumber() const
{
    if (!m_device) {
        return -1;
    }
    QByteArray value = udev_device_get_sysnum(m_device);
    bool success = false;
    int number = value.toInt(&success);
    if (success) {
        return number;
    }
    return -1;
}

QByteArray Device::driver() const
{
    if (!m_device) {
        return QByteArray();
    }
    return QByteArray(udev_device_get_driver(m_device));
}

QStringList Device::alternateDeviceSymlinks() const
{
    if (!m_device) {
        return QStringList();
    }
    return listFromListEntry(udev_device_get_devlinks_list_entry(m_device));
}

QStringList Device::deviceProperties() const
{
    if (!m_device) {
        return QStringList();
    }
    return listFromListEntry(udev_device_get_properties_list_entry(m_device));
}

Device Device::parent() const
{
    if (!m_device) {
        return Device();
    }
    struct udev_device *p = udev_device_get_parent(m_device);
    if (!p) {
        return Device();
    }
    return Device(p);
}

QString Device::deviceProperty(const QByteArray &name) const
{
    if (!m_device) {
        return QString();
    }
    return QString::fromUtf8(udev_device_get_property_value(m_device, name.constData()));
}

QString Device::sysfsProperty(const QByteArray &name) const
{
    if (!m_device) {
        return QString();
    }
    return QString::fromUtf8(udev_device_get_sysattr_value(m_device, name.constData()));
}

}
