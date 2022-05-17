/*
    Copyright 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "exportsmanager.h"
#include "exportsdevice.h"

#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QDebug>

using namespace Solid::Backends::Exports;

class ExportsManager::Private
{
public:
    Private(ExportsManager *parent);
    ~Private();

    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
};

ExportsManager::Private::Private(ExportsManager *parent)
{
}

ExportsManager::Private::~Private()
{
}

ExportsManager::ExportsManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent),
      d(new Private(this))
{
    // TODO: monitor /etc/exports and /etc/exports.d to emit:
    // deviceAdded(udi)
    // deviceRemoved(udi);
    // deviceChanged() does not apply for network share devices

    d->m_supportedInterfaces
        << Solid::DeviceInterface::NetworkShare
        << Solid::DeviceInterface::StorageAccess;
}

ExportsManager::~ExportsManager()
{
    delete d;
}

QString ExportsManager::udiPrefix() const
{
    return QString(EXPORTS_UDI_PREFIX);
}

QSet<Solid::DeviceInterface::Type> ExportsManager::supportedInterfaces() const
{
    return d->m_supportedInterfaces;
}

QStringList ExportsManager::allDevices()
{
    QStringList result;
    foreach (const QString &exportsfilepath, ExportsDevice::exportsFiles()) {
        QFile exportsfile(exportsfilepath);
        if (!exportsfile.open(QFile::ReadOnly)) {
            continue;
        }
        while (!exportsfile.atEnd()) {
            const QByteArray exportsline = exportsfile.readLine().trimmed();
            const QList<QByteArray> exportparts = ExportsDevice::exportParts(exportsline);
            if (exportparts.size() < 1) {
                continue;
            }
            const QByteArray devudihex = exportparts.at(0).toHex();
            const QString devudi = QString::fromLatin1("%1/%2").arg(EXPORTS_UDI_PREFIX, devudihex.constData());
            result.append(devudi);
        }
    }
    return result;
}

QStringList ExportsManager::devicesFromQuery(const QString &parentUdi,
                                          Solid::DeviceInterface::Type type)
{
    QStringList allDev = allDevices();
    QStringList result;

    if (!parentUdi.isEmpty()) {
        foreach (const QString &udi, allDev) {
            ExportsDevice device(udi);
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi) {
                result << udi;
            }
        }
        return result;
    } else if (type != Solid::DeviceInterface::Unknown) {
        foreach (const QString &udi, allDev) {
            ExportsDevice device(udi);
            if (device.queryDeviceInterface(type)) {
                result << udi;
            }
        }
        return result;
    } else {
        return allDev;
    }
}

QObject *ExportsManager::createDevice(const QString &udi)
{
    if (udi == udiPrefix()) {
        return new ExportsDevice(EXPORTS_ROOT_UDI);
    }

    if (!udi.isEmpty()) {
        return new ExportsDevice(udi);
    }

    qWarning() << "cannot create device for UDI" << udi;
    return 0;
}
