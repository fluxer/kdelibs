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
#include "kstandarddirs.h"
#include "kmountpoint.h"

#include <QProcess>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QDebug>

using namespace Solid::Backends::UDev;

StorageAccess::StorageAccess(UDevDevice *device)
    : DeviceInterface(device)
{
    m_device->registerAction("setup", this,
                             SLOT(slotSetupRequested()),
                             SLOT(slotSetupDone(int,QString)));

    m_device->registerAction("teardown", this,
                             SLOT(slotTeardownRequested()),
                             SLOT(slotTeardownDone(int,QString)));
}

StorageAccess::~StorageAccess()
{
}

bool StorageAccess::isAccessible() const
{
    return !filePath().isEmpty();
}

QString StorageAccess::filePath() const
{
    const QString devpath = m_device->property("DEVNAME").toString();
    const KMountPoint::List mountpoints = KMountPoint::currentMountPoints();

    foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
        if (mountpoint->mountedFrom() == devpath || mountpoint->realDeviceName() == devpath) {
            return mountpoint->mountPoint();
        }
    }

    const QStringList devlinks = m_device->property("DEVLINKS").toString().split(" ");
    foreach (const QString &link, devlinks) {
        foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
            if (mountpoint->mountedFrom() == link || mountpoint->realDeviceName() == link) {
                return mountpoint->mountPoint();
            }
        }
    }

    return QString();
}

bool StorageAccess::isIgnored() const
{
    const QString devtype = m_device->property("DEVTYPE").toString();
    return (devtype != "partition");
}

bool StorageAccess::setup()
{
    QString mountpoint = filePath();
    if (!mountpoint.isEmpty()) {
        return true;
    }

    // permission denied on /run/mount so.. using base directory that is writable
    const QString mountbase = KGlobal::dirs()->saveLocation("tmp");
    const QString devuuid = m_device->property("ID_FS_UUID").toString();
    mountpoint = mountbase + QLatin1Char('/') + devuuid;
    QDir mountdir(mountbase);
    if (!mountdir.exists(devuuid) && !mountdir.mkdir(devuuid)) {
        qWarning() << "could not create" << mountpoint;
        return false;
    }

    m_device->broadcastActionRequested("setup");

    const QStringList mountargs = QStringList() << "mount" << m_device->property("DEVNAME").toString() << mountpoint;
    QProcess mountproc;
    mountproc.start("kdesudo", mountargs);
    mountproc.waitForStarted();
    while (mountproc.state() == QProcess::Running) {
        QCoreApplication::processEvents();
    }

    if (mountproc.exitCode() == 0) {
        m_device->broadcastActionDone("setup", Solid::NoError, QString());
        emit accessibilityChanged(true, m_device->udi());
    } else {
        QString mounterror = mountproc.readAllStandardError();
        if (mounterror.isEmpty()) {
            mounterror = mountproc.readAllStandardOutput();
        }
        qWarning() << mounterror;
        m_device->broadcastActionDone("setup", Solid::UnauthorizedOperation, mounterror);
    }

    return (mountproc.exitCode() == 0);
}

bool StorageAccess::teardown()
{
    const QString mountpoint = filePath();
    if (mountpoint.isEmpty()) {
        return false;
    }

    m_device->broadcastActionRequested("teardown");

    const QStringList umountargs = QStringList() << "umount" << mountpoint;
    QProcess umountproc;
    umountproc.start("kdesudo", umountargs);
    umountproc.waitForStarted();
    while (umountproc.state() == QProcess::Running) {
        QCoreApplication::processEvents();
    }

    if (umountproc.exitCode() == 0) {
        m_device->broadcastActionDone("teardown", Solid::NoError, QString());
        emit accessibilityChanged(false, m_device->udi());
    } else {
        QString umounterror = umountproc.readAllStandardError();
        if (umounterror.isEmpty()) {
            umounterror = umountproc.readAllStandardOutput();
        }
        qWarning() << umounterror;
        m_device->broadcastActionDone("teardown", Solid::UnauthorizedOperation, umounterror);
    }

    return (umountproc.exitCode() == 0);
}

void StorageAccess::slotSetupRequested()
{
    emit setupRequested(m_device->udi());
}

void StorageAccess::slotSetupDone(int error, const QString &errorString)
{
    emit setupDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}

void StorageAccess::slotTeardownRequested()
{
    emit teardownRequested(m_device->udi());
}

void StorageAccess::slotTeardownDone(int error, const QString &errorString)
{
    emit teardownDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}
