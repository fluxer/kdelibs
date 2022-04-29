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

#ifndef SOLID_BACKENDS_DEVINFO_DEVINFODEVICE_H
#define SOLID_BACKENDS_DEVINFO_DEVINFODEVICE_H

#include <solid/ifaces/device.h>
#include <QtCore/QStringList>

#include "kdevicedatabase.h"

namespace Solid
{
namespace Backends
{
namespace Devinfo
{

class DevinfoDevice : public Solid::Ifaces::Device
{
    Q_OBJECT

public:
    enum DeviceProperty {
        DeviceParent = 0,
        DeviceName = 1,
        DeviceDescription = 2,
        DeviceDriver = 3,
        DevicePnPInfo = 4,
        DeviceLocation = 5
    };

    enum PnPInfo {
        PnPVendor = 0,
        PnPDevice = 1,
        PnPSubVendor = 2,
        PnPSubDevice = 3,
        PnPClass = 4
    };

    DevinfoDevice(const QString &device);
    virtual ~DevinfoDevice();

    virtual QString udi() const;
    virtual QString parentUdi() const;
    virtual QString vendor() const;
    virtual QString product() const;
    virtual QString icon() const;
    virtual QStringList emblems() const;
    virtual QString description() const;
    virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const;
    virtual QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type);

    QByteArray deviceProperty(const DeviceProperty property) const;
    QByteArray devicePnP(const PnPInfo pnp) const;
    QByteArray deviceCtl(const char* field) const;

    static QByteArray stringByName(const char* sysctlname);
    static qlonglong integerByName(const char* sysctlname);

private:
    QString m_device;
    QMap<DeviceProperty, QByteArray> m_properties;
    QMap<PnPInfo, QByteArray> m_pnpinfo;
    mutable KDeviceDatabase m_devicedb;
};

}
}
}
#endif // SOLID_BACKENDS_DEVINFO_DEVINFODEVICE_H
