/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KAUTOMOUNT_H
#define KAUTOMOUNT_H

#include <QtCore/QObject>
#include <QtDBus/QDBusPendingCallWatcher>

#include <kio/kio_export.h>

class KAutoMountPrivate;
/**
 * This class implements mounting of devices, as well as showing a file-manager
 * window after mounting a device, optionally. It is a wrapper around the Solid
 * UI and is used by KDesktopFileActions.
 *
 * @short This class implements mounting of devices.
 */
class KIO_EXPORT KAutoMount : public QObject
{
    Q_OBJECT
public:
    /**
     * Mounts a device.
     * @param readonly if true, the device is mounted read-only
     * @param device the path to the device (e.g. /dev/fd0)
     * @param mountpoint the directory where to mount the device [optional, fstab is used otherwise]
     * @param desktopFile the file the user clicked on - to notify KDirWatch of the fact that
     * it should emit fileDirty for it (to have the icon change)
     * @param show_filemanager_window if true, a file-manager window for that mountpoint is shown after
     * the mount, if successful.
     */
    KAutoMount(bool readonly, const QString &device, const QString &mountpoint,
               const QString &desktopFile, bool show_filemanager_window = true);

Q_SIGNALS:
    /** Emitted when the directory has been mounted */
    void finished();
    /** Emitted in case the directory could not been mounted */
    void error();

private:
    /** KAutoMount deletes itself. Don't delete it manually. */
    ~KAutoMount();
    Q_PRIVATE_SLOT(d, void slotFinished(QDBusPendingCallWatcher *))
    friend class KAutoMountPrivate;
    KAutoMountPrivate* const d;
};

class KAutoUnmountPrivate;
/**
 * This class implements unmounting of devices. It is a wrapper around Solid UI
 * and is used by KDesktopFileActions.
 *
 * @short This class implements unmounting of devices,
 */
class KIO_EXPORT KAutoUnmount : public QObject
{
    Q_OBJECT
public:
    /**
     * Unmounts a device.
     * @param mountpoint the mount point - KAutoUnmount finds the device from that
     * @param desktopFile the file the user clicked on - to notify KDirWatch of the fact that
     * it should emit fileDirty for it (to have the icon change)
     */
    KAutoUnmount(const QString &mountpoint, const QString &desktopFile);

Q_SIGNALS:
    /** Emitted when the directory has been unmounted */
    void finished();
    /** Emitted in case the directory could not been unmounted */
    void error();

private:
    /** KAutoUnmount deletes itself. Don't delete it manually. */
    ~KAutoUnmount();
    Q_PRIVATE_SLOT(d, void slotFinished(QDBusPendingCallWatcher *))
    friend class KAutoUnmountPrivate;
    KAutoUnmountPrivate* const d;
};

#endif // KAUTOMOUNT_H
