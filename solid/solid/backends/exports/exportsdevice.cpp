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
#include "exportsnetworkshare.h"
#include "exportsstorageaccess.h"

#include <QFile>
#include <QDir>
#include <QDebug>

using namespace Solid::Backends::Exports;

static inline QString dehex(const QString &hexstring)
{
    const QByteArray devicehex = hexstring.toLatin1();
    if (devicehex.size() < 17) {
        return QString();
    }
    return QString::fromLocal8Bit(QByteArray::fromHex(devicehex.constData() + 17));
}

static inline QString normalizedAddress(const QByteArray &exportaddress)
{
    QByteArray normalized = exportaddress;
    const int bracketindex = normalized.indexOf('(');
    if (bracketindex > 0) {
        normalized = normalized.left(bracketindex);
    }
    const int slashindex = normalized.indexOf('/');
    if (slashindex > 0) {
        normalized = normalized.left(slashindex);
    }
    return QString::fromLatin1(normalized.constData(), normalized.size());
}

ExportsDevice::ExportsDevice(const QString &device)
    : Solid::Ifaces::Device()
    , m_device(device)
    , m_dir(dehex(m_device))
{
    // qDebug() << Q_FUNC_INFO << m_device;
    foreach (const QString &exportsfilepath, ExportsDevice::exportsFiles()) {
        QFile exportsfile(exportsfilepath);
        if (!exportsfile.open(QFile::ReadOnly)) {
            continue;
        }
        while (!exportsfile.atEnd()) {
            const QByteArray exportsline = exportsfile.readLine().trimmed();
            const QList<QByteArray> exportparts = ExportsDevice::exportParts(exportsline);
            if (exportparts.size() < 2) {
                continue;
            }
            const QByteArray exportdir = exportparts.at(0);
            // qDebug() << Q_FUNC_INFO << m_device << m_dir;
            if (exportdir == m_dir) {
                m_address = normalizedAddress(exportparts.at(1));
            }
        }
    }
}

ExportsDevice::~ExportsDevice()
{
}

QString ExportsDevice::udi() const
{
    return m_device;
}

QString ExportsDevice::parentUdi() const
{
    if (m_device == EXPORTS_ROOT_UDI) {
        // root0
        return QString();
    }
    return QString(EXPORTS_ROOT_UDI);
}

QString ExportsDevice::vendor() const
{
    return m_dir;
}

QString ExportsDevice::product() const
{
    if (m_device == EXPORTS_ROOT_UDI) {
        return QString::fromLatin1("Devices");
    }  else if(queryDeviceInterface(Solid::DeviceInterface::NetworkShare)) {
        return m_address;
    }

    return QString();
}

QString ExportsDevice::icon() const
{
    if (m_device == EXPORTS_ROOT_UDI) {
        return QString::fromLatin1("computer");
    } else if (queryDeviceInterface(Solid::DeviceInterface::NetworkShare)) {
        return QString::fromLatin1("network-server");
    }
    return QString();
}

QStringList ExportsDevice::emblems() const
{
    QStringList result;
    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess)) {
        const StorageAccess accessIface(const_cast<ExportsDevice*>(this));
        if (accessIface.isAccessible()) {
            result << QString::fromLatin1("emblem-mounted");
        } else {
            result << QString::fromLatin1("emblem-unmounted");
        }
    }
    return result;
}

QString ExportsDevice::description() const
{
    if (m_device == EXPORTS_ROOT_UDI || parentUdi().isEmpty()) {
        return QObject::tr("Computer");
    } else if (queryDeviceInterface(Solid::DeviceInterface::NetworkShare)) {
        return QString::fromLatin1("%1 on %2").arg(QDir(m_dir).dirName()).arg(m_address);
    }
    return QString();
}

bool ExportsDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
        case Solid::DeviceInterface::NetworkShare:
        case Solid::DeviceInterface::StorageAccess: {
            return true;
        }
        default: {
            return false;
        }
    }
    Q_UNREACHABLE();
}

QObject *ExportsDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return nullptr;
    }
    switch (type) {
        case Solid::DeviceInterface::NetworkShare: {
            return new NetworkShare(this);
        }
        case Solid::DeviceInterface::StorageAccess: {
            return new StorageAccess(this);
        }
        default: {
            Q_ASSERT(false);
            return nullptr;
        }
    }
    Q_UNREACHABLE();
}

// for reference:
// https://linux.die.net/man/8/exportfs
QStringList ExportsDevice::exportsFiles()
{
    QStringList result;
    result.append(QString::fromLatin1("/etc/exports"));
    QDir exportsdir(QString::fromLatin1("/etc/exports.d"));
    foreach (const QFileInfo &exportsinfo, exportsdir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
        if (!exportsinfo.isFile()) {
            continue;
        }
        const QString exportsfilepath = exportsinfo.filePath();
        if (!exportsfilepath.endsWith(QLatin1String(".exports"))) {
            continue;
        }
        result.append(exportsfilepath);
    }
    return result;
}

QList<QByteArray> ExportsDevice::exportParts(const QByteArray &exportline)
{
    QList<QByteArray> result;
    if (exportline.isEmpty() || exportline.startsWith('#')) {
        return result;
    }
    int partstart = 0;
    for (int i = 0; i < exportline.size(); i++) {
        if (exportline.at(i) == ' ' || exportline.at(i) == '\t') {
            // qDebug() << Q_FUNC_INFO << i << partstart;
            result.append(exportline.mid(partstart, i - partstart));
            partstart = i + 1;
        }
        if (partstart && i == (exportline.size() - 1)) {
            result.append(exportline.mid(partstart, i - partstart + 1));
        }
    }
    // qDebug() << Q_FUNC_INFO << result;
    return result;
}