/*
    Copyright 2006-2010 Kevin Ottens <ervin@kde.org>
    Copyright 2010 Mario Bensi <mbensi@ipsquad.net>

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

#include "fstabhandling.h"

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include <kstandarddirs.h>
#include <kmountpoint.h>

#include <solid/soliddefs_p.h>

#include <config.h>

Q_GLOBAL_STATIC(Solid::Backends::Fstab::FstabHandling, globalFstabCache)

Solid::Backends::Fstab::FstabHandling::FstabHandling()
    : m_fstabCacheValid(false),
      m_mtabCacheValid(false)
{ }

bool _k_isFstabNetworkFileSystem(const QString &fstype, const QString &devName)
{
    if (fstype == "nfs"
     || fstype == "nfs4"
     || fstype == "smbfs"
     || fstype == "cifs"
     || devName.startsWith(QLatin1String("//"))) {
        return true;
    }
    return false;
}

void Solid::Backends::Fstab::FstabHandling::_k_updateFstabMountPointsCache()
{
    if (globalFstabCache()->m_fstabCacheValid)
        return;

    globalFstabCache()->m_fstabCache.clear();

    const KMountPoint::List mountpoints = KMountPoint::possibleMountPoints();
    foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
        const QString device = mountpoint->mountedFrom();
        if (_k_isFstabNetworkFileSystem(mountpoint->mountType(), device)) {
            globalFstabCache()->m_fstabCache.insert(device, mountpoint->mountPoint());
        }
    }

    globalFstabCache()->m_fstabCacheValid = true;
}

QStringList Solid::Backends::Fstab::FstabHandling::deviceList()
{
    _k_updateFstabMountPointsCache();
    _k_updateMtabMountPointsCache();

    QStringList devices = globalFstabCache()->m_fstabCache.keys();
    devices += globalFstabCache()->m_mtabCache.keys();
    devices.removeDuplicates();
    return devices;
}

QStringList Solid::Backends::Fstab::FstabHandling::mountPoints(const QString &device)
{
    _k_updateFstabMountPointsCache();
    _k_updateMtabMountPointsCache();

    QStringList mountpoints = globalFstabCache()->m_fstabCache.values(device);
    mountpoints += globalFstabCache()->m_mtabCache.values(device);
    mountpoints.removeDuplicates();
    return mountpoints;
}

QProcess *Solid::Backends::Fstab::FstabHandling::callSystemCommand(const QString &commandName,
                                                                 const QStringList &args,
                                                                 QObject *obj, const char *slot)
{
    QString commandExe = KStandardDirs::findRootExe(commandName);
    if (commandExe.isEmpty()) {
        return 0;
    }

    QProcess *process = new QProcess(obj);

    QObject::connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),
                     obj, slot);

    process->start(commandExe, args);

    if (process->waitForStarted()) {
        return process;
    } else {
        delete process;
        return 0;
    }
}

QProcess *Solid::Backends::Fstab::FstabHandling::callSystemCommand(const QString &commandName,
                                                                 const QString &device,
                                                                 QObject *obj, const char *slot)
{
    return callSystemCommand(commandName, QStringList() << device, obj, slot);
}

void Solid::Backends::Fstab::FstabHandling::_k_updateMtabMountPointsCache()
{
    if (globalFstabCache()->m_mtabCacheValid)
        return;

    globalFstabCache()->m_mtabCache.clear();

    const KMountPoint::List mountpoints = KMountPoint::currentMountPoints();
    foreach (const KMountPoint::Ptr mountpoint, mountpoints) {
        const QString device = mountpoint->mountedFrom();
        if (_k_isFstabNetworkFileSystem(mountpoint->mountType(), QString())) {
            globalFstabCache()->m_fstabCache.insert(device, mountpoint->mountPoint());
        }
    }

    globalFstabCache()->m_mtabCacheValid = true;
}

QStringList Solid::Backends::Fstab::FstabHandling::currentMountPoints(const QString &device)
{
    _k_updateMtabMountPointsCache();
    return globalFstabCache()->m_mtabCache.values(device);
}

void Solid::Backends::Fstab::FstabHandling::flushMtabCache()
{
    globalFstabCache()->m_mtabCacheValid = false;
}

void Solid::Backends::Fstab::FstabHandling::flushFstabCache()
{
    globalFstabCache()->m_fstabCacheValid = false;
}
