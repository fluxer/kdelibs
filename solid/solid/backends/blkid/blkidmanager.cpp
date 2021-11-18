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

#include "config-solid.h"
#include "blkidmanager.h"
#include "blkiddevice.h"
#include "../shared/rootdevice.h"

#include <QtCore/QSet>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDebug>

#include <blkid/blkid.h>

using namespace Solid::Backends::Blkid;
using namespace Solid::Backends::Shared;

class BlkidManager::Private
{
public:
    Private();
    ~Private();

    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
};

BlkidManager::Private::Private()
{
}

BlkidManager::Private::~Private()
{
}

BlkidManager::BlkidManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent),
      d(new Private)
{
    d->m_supportedInterfaces << Solid::DeviceInterface::StorageAccess
                             << Solid::DeviceInterface::StorageDrive
                             << Solid::DeviceInterface::StorageVolume
#if defined(LIBCDIO_FOUND)
                             << Solid::DeviceInterface::OpticalDrive
                             << Solid::DeviceInterface::OpticalDisc
#endif
                             << Solid::DeviceInterface::Block;
}

BlkidManager::~BlkidManager()
{
    delete d;
}

QString BlkidManager::udiPrefix() const
{
    return QString::fromLatin1(BLKID_UDI_PREFIX);
}

QSet<Solid::DeviceInterface::Type> BlkidManager::supportedInterfaces() const
{
    return d->m_supportedInterfaces;
}

QStringList BlkidManager::allDevices()
{
    QStringList res;

    blkid_cache blkidcache;
    if (blkid_get_cache(&blkidcache, NULL) != 0) {
        qWarning() << "could not get blkid cache";
        return res;
    }
    if (blkid_probe_all(blkidcache) != 0) {
        qWarning() << "could probe blkid cache";
    }
    blkid_dev_iterate blkiddevit = blkid_dev_iterate_begin(blkidcache);
    blkid_dev blkiddev;
    while (blkid_dev_next(blkiddevit, &blkiddev) == 0) {
        const char *blkiddevname = blkid_dev_devname(blkiddev);
        const QString devname = QString::fromLatin1("%1%2").arg(BLKID_UDI_PREFIX, QString::fromLatin1(blkiddevname));
        res.append(devname);
    }
    blkid_dev_iterate_end(blkiddevit);
    // qDebug() << Q_FUNC_INFO << res;

    return res;
}

QStringList BlkidManager::devicesFromQuery(const QString &parentUdi,
                                          Solid::DeviceInterface::Type type)
{
    QStringList allDev = allDevices();
    QStringList result;

    if (!parentUdi.isEmpty()) {
        foreach (const QString &udi, allDev) {
            BlkidDevice device(udi);
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi) {
                result << udi;
            }
        }
        return result;
    } else if (type != Solid::DeviceInterface::Unknown) {
        foreach (const QString &udi, allDev) {
            BlkidDevice device(udi);
            if (device.queryDeviceInterface(type)) {
                result << udi;
            }
        }
        return result;
    } else {
        return allDev;
    }
}

QObject *BlkidManager::createDevice(const QString &udi)
{
    if (udi == udiPrefix()) {
        return new BlkidDevice(BLKID_ROOT_UDI);
    }

    if (!udi.isEmpty()) {
        return new BlkidDevice(udi);
    }

    qWarning() << "cannot create device for UDI" << udi;
    return 0;
}
