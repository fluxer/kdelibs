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

#ifndef SOLID_BACKENDS_GEOM_GEOMDEVICE_H
#define SOLID_BACKENDS_GEOM_GEOMDEVICE_H

#include <solid/ifaces/device.h>
#include <QtCore/QStringList>

namespace Solid
{
namespace Backends
{
namespace Geom
{

class GeomDevice : public Solid::Ifaces::Device
{
    Q_OBJECT

public:
    GeomDevice(const QString &device);
    virtual ~GeomDevice();

    virtual QString udi() const;
    virtual QString parentUdi() const;
    virtual QString vendor() const;
    virtual QString product() const;
    virtual QString icon() const;
    virtual QStringList emblems() const;
    virtual QString description() const;
    virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const;
    virtual QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type);

public:
    QString m_device;
    // synthetized
    QByteArray m_realdevice;
    QByteArray m_parent;
    // mostly populated from geom providers
    qulonglong m_size;
    QByteArray m_type;
    QByteArray m_label;
    QByteArray m_uuid;
    QByteArray m_class;
    int m_major; // this one is not
    int m_minor;
};

}
}
}
#endif // SOLID_BACKENDS_GEOM_GEOMDEVICE_H
