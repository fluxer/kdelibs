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
    : m_udev(Q_NULLPTR)
{
}

Device::Device(struct udev_device *udev_, bool ref)
    : m_udev(udev_)
{
    if (m_udev && ref)
        udev_device_ref(m_udev);
}

Device::Device(const Device &other)
    : m_udev(other.m_udev)
{
    if (other.m_udev) {
        udev_device_ref(other.m_udev);
    }
}

Device::~Device()
{
    if (m_udev) {
        udev_device_unref(m_udev);
    }
}

Device &Device::operator=(const Device &other)
{
    if (m_udev) {
        udev_device_unref(m_udev);
    }
    if (other.m_udev) {
        m_udev = udev_device_ref(other.m_udev);
    } else {
        m_udev = Q_NULLPTR;
    }
    return *this;
}

bool Device::isValid() const
{
    return (m_udev != Q_NULLPTR);
}

QString Device::subsystem() const
{
    if (!m_udev) {
        return QString();
    }
    return QString::fromLatin1(udev_device_get_subsystem(m_udev));
}

QString Device::devType() const
{
    if (!m_udev) {
        return QString();
    }
    return QString::fromLatin1(udev_device_get_devtype(m_udev));
}

QString Device::name() const
{
    if (!m_udev) {
        return QString();
    }
    return QString::fromLatin1(udev_device_get_sysname(m_udev));
}

QString Device::sysfsPath() const
{
    if (!m_udev) {
        return QString();
    }
    return QString::fromLatin1(udev_device_get_syspath(m_udev));
}

int Device::sysfsNumber() const
{
    if (!m_udev) {
        return -1;
    }
    QString value = QString::fromLatin1(udev_device_get_sysnum(m_udev));
    bool success = false;
    int number = value.toInt(&success);
    if (success) {
        return number;
    }
    return -1;
}

QString Device::driver() const
{
    if (!m_udev) {
        return QString();
    }
    return QString::fromLatin1(udev_device_get_driver(m_udev));
}

QStringList Device::alternateDeviceSymlinks() const
{
    if (!m_udev) {
        return QStringList();
    }
    return listFromListEntry(udev_device_get_devlinks_list_entry(m_udev));
}

QStringList Device::deviceProperties() const
{
    if (!m_udev) {
        return QStringList();
    }
    return listFromListEntry(udev_device_get_properties_list_entry(m_udev));
}

Device Device::parent() const
{
    if (!m_udev) {
        return Device();
    }
    struct udev_device *p = udev_device_get_parent(m_udev);
    if (!p) {
        return Device();
    }
    return Device(p);
}

QString Device::deviceProperty(const QString &name) const
{
    if (!m_udev) {
        return QString();
    }
    const QByteArray propName(name.toLatin1());
    return QString::fromLatin1(udev_device_get_property_value(m_udev, propName.constData()));
}

QString Device::sysfsProperty(const QString &name) const
{
    if (!m_udev) {
        return QString();
    }
    const QByteArray propName(name.toLatin1());
    return QString::fromLatin1(udev_device_get_sysattr_value(m_udev, propName.constData()));
}

}
