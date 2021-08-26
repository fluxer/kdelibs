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

#include "geommanager.h"
#include "geomdevice.h"

#include <QtCore/QSet>
#include <QtCore/QDebug>

#include <libgeom.h>

using namespace Solid::Backends::Geom;

class GeomManager::Private
{
public:
    Private();
    ~Private();

    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
};

GeomManager::Private::Private()
{
}

GeomManager::Private::~Private()
{
}

GeomManager::GeomManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent),
      d(new Private)
{
    d->m_supportedInterfaces
        << Solid::DeviceInterface::Block
        << Solid::DeviceInterface::StorageDrive
        << Solid::DeviceInterface::StorageVolume
        << Solid::DeviceInterface::StorageAccess;
}

GeomManager::~GeomManager()
{
    delete d;
}

QString GeomManager::udiPrefix() const
{
    return QString(GEOM_UDI_PREFIX);
}

QSet<Solid::DeviceInterface::Type> GeomManager::supportedInterfaces() const
{
    return d->m_supportedInterfaces;
}

QStringList GeomManager::allDevices()
{
    QStringList result;

    struct gmesh tree;
    ::memset(&tree, 0, sizeof(gmesh));
    const int geomresult = geom_gettree(&tree);
    struct gclass* geomclass = nullptr;
    struct ggeom* geomgeom = nullptr;
    struct gprovider* geomprovider = nullptr;
    // NOTE: keep in sync with kdelibs/solid/solid/backends/geom/geomdevice.cpp
    LIST_FOREACH(geomclass, &tree.lg_class, lg_class) {
        // not interested in devices made up by providers such as labels
        if (qstrcmp(geomclass->lg_name, "DISK") != 0 && qstrcmp(geomclass->lg_name, "PART") != 0
            && qstrcmp(geomclass->lg_name, "SWAP") != 0) {
            continue;
        }
        LIST_FOREACH(geomgeom, &geomclass->lg_geom, lg_geom) {
            LIST_FOREACH(geomprovider, &geomgeom->lg_provider, lg_provider) {
                // qDebug() << geomclass->lg_name << geomgeom->lg_name << geomprovider->lg_name;
                const QString devudi = QString::fromLatin1("%1/%2").arg(GEOM_UDI_PREFIX, geomprovider->lg_name);
                result << devudi;
            }
        }
    }
    geom_deletetree(&tree);

    // qDebug() << Q_FUNC_INFO << result;
    return result;
}

QStringList GeomManager::devicesFromQuery(const QString &parentUdi,
                                          Solid::DeviceInterface::Type type)
{
    QStringList allDev = allDevices();
    QStringList result;

    if (!parentUdi.isEmpty()) {
        foreach (const QString &udi, allDev) {
            GeomDevice device(udi);
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi) {
                result << udi;
            }
        }
        return result;
    } else if (type != Solid::DeviceInterface::Unknown) {
        foreach (const QString &udi, allDev) {
            GeomDevice device(udi);
            if (device.queryDeviceInterface(type)) {
                result << udi;
            }
        }
        return result;
    } else {
        return allDev;
    }
}

QObject *GeomManager::createDevice(const QString &udi)
{
    if (udi == udiPrefix()) {
        return new GeomDevice(GEOM_ROOT_UDI);
    }

    if (!udi.isEmpty()) {
        return new GeomDevice(udi);
    }

    qWarning() << "cannot create device for UDI" << udi;
    return 0;
}