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

#include "udevstorageaccess.h"
#include "kmountpoint.h"
#include "kde_file.h"

#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

using namespace Solid::Backends::UDev;

#define SOLID_TIMEOUT 250

static Solid::ErrorType kSolidUiCall(const QString &method, const QString &udi)
{
    QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
    QDBusPendingReply<int> reply = soliduiserver.asyncCall(method, udi);
    while (!reply.isFinished()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, SOLID_TIMEOUT);
    }
    return static_cast<Solid::ErrorType>(reply.value());
}

StorageAccess::StorageAccess(UDevDevice *device)
    : DeviceInterface(device),
    m_mtabfd(0),
    m_isaccessible(false),
    m_mtabnotifier(nullptr)
{
    m_isaccessible = isAccessible();

    m_mtabfd = KDE_open("/etc/mtab", O_RDONLY);
    if (m_mtabfd <= 0) {
        qWarning("StorageAccess: unable to open /etc/mtab");
        return;
    }
    m_mtabnotifier = new QSocketNotifier(m_mtabfd, QSocketNotifier::Exception);
    QObject::connect(m_mtabnotifier, SIGNAL(activated(int)), this, SLOT(slotEmitSignals()));
}

StorageAccess::~StorageAccess()
{
    if (m_mtabnotifier) {
        delete m_mtabnotifier;
    }
    if (m_mtabfd > 0) {
        QT_CLOSE(m_mtabfd);
    }
}

bool StorageAccess::isAccessible() const
{
    return !filePath().isEmpty();
}

QString StorageAccess::filePath() const
{
    const KMountPoint::List mountpoints = KMountPoint::currentMountPoints();

    const QString devname(m_device->deviceProperty("DEVNAME"));
    foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
        if (mountpoint->mountedFrom() == devname || mountpoint->realDeviceName() == devname) {
            return mountpoint->mountPoint();
        }
    }

    const QStringList devlinks = m_device->udevDevice().alternateDeviceSymlinks();
    foreach (const QString &link, devlinks) {
        foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
            if (mountpoint->mountedFrom() == link || mountpoint->realDeviceName() == link) {
                return mountpoint->mountPoint();
            }
        }
    }

    const QString idfsusage(m_device->deviceProperty("ID_FS_USAGE"));
    if (idfsusage == "crypto") {
        // NOTE: keep in sync with kde-workspace/soliduiserver/soliduiserver.cpp
        const QString idfsuuid(m_device->deviceProperty("ID_FS_UUID"));
        const QString dmdevice = QLatin1String("/dev/mapper/") + idfsuuid;
        foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
            if (mountpoint->mountedFrom() == dmdevice || mountpoint->realDeviceName() == dmdevice) {
                return mountpoint->mountPoint();
            }
        }
    }

    return QString();
}

bool StorageAccess::isIgnored() const
{
    const QString idfsusage(m_device->deviceProperty("ID_FS_USAGE"));
    const QString devtype(m_device->deviceProperty("DEVTYPE"));
    const int idcdrom = m_device->deviceProperty("ID_CDROM").toInt();
    return ((idfsusage != "filesystem" && idfsusage != "crypto") || (devtype == "disk" && idcdrom != 1));
}

bool StorageAccess::setup()
{
    const QString mountpoint = filePath();
    if (!mountpoint.isEmpty()) {
        return true;
    }

    emit setupRequested(m_device->udi());

    const Solid::ErrorType replyvalue = kSolidUiCall("mountUdi", m_device->udi());
    if (replyvalue == Solid::NoError) {
        emit accessibilityChanged(true, m_device->udi());
    }

    emit setupDone(replyvalue, Solid::errorString(replyvalue), m_device->udi());
    return (replyvalue == Solid::NoError);
}

bool StorageAccess::teardown()
{
    const QString mountpoint = filePath();
    if (mountpoint.isEmpty()) {
        return false;
    }

    emit teardownRequested(m_device->udi());

    const Solid::ErrorType replyvalue = kSolidUiCall("unmountUdi", m_device->udi());
    if (replyvalue == Solid::NoError) {
        emit accessibilityChanged(false, m_device->udi());
    }

    emit teardownDone(replyvalue, Solid::errorString(replyvalue), m_device->udi());
    return (replyvalue == Solid::NoError);
}

void StorageAccess::slotEmitSignals()
{
    const bool previousisaccessible = m_isaccessible;
    m_isaccessible = isAccessible();
    // qDebug() << Q_FUNC_INFO << m_mtabfd << previousisaccessible << m_isaccessible;
    if (previousisaccessible != m_isaccessible) {
        emit accessibilityChanged(m_isaccessible, m_device->udi());
    }
}
