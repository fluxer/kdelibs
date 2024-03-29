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

#ifndef SOLID_BACKENDS_UDEV_UDEVMANAGER_H
#define SOLID_BACKENDS_UDEV_UDEVMANAGER_H

#define UDEV_UDI_PREFIX "/org/kde/solid/udev"

#include <solid/ifaces/devicemanager.h>

#include "udevqt.h"

namespace Solid
{
namespace Backends
{
namespace UDev
{
class UDevManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    UDevManager(QObject *parent);
    virtual ~UDevManager();

    virtual QString udiPrefix() const;
    virtual QSet<Solid::DeviceInterface::Type> supportedInterfaces() const;

    virtual QStringList allDevices();

    virtual QStringList devicesFromQuery(const QString &parentUdi,
                                         Solid::DeviceInterface::Type type);

    virtual QObject *createDevice(const QString &udi);

private Q_SLOTS:
    void slotDeviceAdded(const UdevQt::Device &device);
    void slotDeviceRemoved(const UdevQt::Device &device);
    void slotDeviceChanged(const UdevQt::Device &device);

private:
    class Private;
    Private *const d;
};
}
}
}

#endif // SOLID_BACKENDS_UDEV_UDEVMANAGER_H
