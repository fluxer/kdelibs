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

#include "geomstorageaccess.h"
#include "kstandarddirs.h"
#include "kmountpoint.h"

#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

using namespace Solid::Backends::Geom;

StorageAccess::StorageAccess(GeomDevice *device)
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
    const KMountPoint::List mountpoints = KMountPoint::currentMountPoints();

    const QString devname(QString::fromLatin1("/dev/%1").arg(m_device->m_realdevice.constData()));
    const QString devgptname(QString::fromLatin1("/dev/gpt/%1").arg(m_device->m_realdevice.constData()));
    const QString devlabel(QString::fromLatin1("/dev/%1").arg(m_device->m_label.constData()));
    const QString devgptlabel(QString::fromLatin1("/dev/gpt/%1").arg(m_device->m_label.constData()));
    foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
        // qDebug() << devname << devlabel << mountpoint->mountedFrom() << mountpoint->realDeviceName();
        if (mountpoint->mountedFrom() == devname || mountpoint->realDeviceName() == devname
            || mountpoint->mountedFrom() == devgptname || mountpoint->realDeviceName() == devgptname
            || mountpoint->mountedFrom() == devlabel || mountpoint->realDeviceName() == devlabel
            || mountpoint->mountedFrom() == devgptlabel || mountpoint->realDeviceName() == devgptlabel) {
            return mountpoint->mountPoint();
        }
    }

    return QString();
}

bool StorageAccess::isIgnored() const
{
    return (m_device->m_type.isEmpty());
}

bool StorageAccess::setup()
{
    QString mountpoint = filePath();
    if (!mountpoint.isEmpty()) {
        return true;
    }

    // permission denied on /run/mount so.. using base directory that is writable
    const QString mountbase = KGlobal::dirs()->saveLocation("tmp");
    const QString idfsuuid(m_device->m_uuid);
    mountpoint = mountbase + QLatin1Char('/') + idfsuuid;
    QDir mountdir(mountbase);
    if (!mountdir.exists(idfsuuid) && !mountdir.mkdir(idfsuuid)) {
        qWarning() << "could not create" << mountpoint;
        return false;
    }

    m_device->broadcastActionRequested("setup");

    const QString devname(QString::fromLatin1("/dev/%1").arg(m_device->m_realdevice.constData()));
    const QStringList mountargs = QStringList() << "mount" << devname << mountpoint;
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
        qWarning() << "mount error" << mounterror;
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
        qWarning() << "unmount error" << umounterror;
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
