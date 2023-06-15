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

#include "kautomount.h"

#include <krun.h>
#include <kdirnotify.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <QDBusInterface>
#include <QDBusReply>

/***********************************************************************
 *
 * Utility classes
 *
 ***********************************************************************/

class KAutoMountPrivate
{
public:
    KAutoMountPrivate(KAutoMount *qq, const QString &device, const QString &mountPoint,
                      const QString &desktopFile, bool showFileManagerWindow);

    KAutoMount *q;
    QString m_strDevice;
    QString m_desktopFile;
    QString m_mountPoint;
    bool m_bShowFilemanagerWindow;

    QDBusInterface m_solidInterface;
    QDBusPendingCallWatcher* m_callWatcher;

    void slotFinished(QDBusPendingCallWatcher *watcher);
};

KAutoMountPrivate::KAutoMountPrivate(KAutoMount *qq, const QString &device, const QString &mountPoint,
                                     const QString &desktopFile, bool showFileManagerWindow)
    : q(qq), m_strDevice(device), m_desktopFile(desktopFile), m_mountPoint(mountPoint),
    m_bShowFilemanagerWindow(showFileManagerWindow),
    m_solidInterface("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer"),
    m_callWatcher(nullptr)
{
}

KAutoMount::KAutoMount(bool readonly, const QString &device,
                       const QString &mountpoint, const QString &desktopFile,
                       bool show_filemanager_window)
    : d(new KAutoMountPrivate(this, device, mountpoint, desktopFile, show_filemanager_window))
{
    QDBusPendingCall pendingcall = d->m_solidInterface.asyncCall("mountDevice", device, mountpoint, readonly);
    d->m_callWatcher = new QDBusPendingCallWatcher(pendingcall, this);
    connect(
        d->m_callWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
        this, SLOT(slotFinished(QDBusPendingCallWatcher*))
    );
}

KAutoMount::~KAutoMount()
{
    delete d;
}

void KAutoMountPrivate::slotFinished(QDBusPendingCallWatcher *watcher)
{
    if (watcher->isError()) {
        emit q->error();
        KMessageBox::detailedError(
            nullptr /*TODO - window*/,
            i18n("Could not mount device."),
            watcher->error().message()
        );
        q->deleteLater();
        return;
    }

    const QList<QVariant> replyarguments = watcher->reply().arguments();
    const int replyvalue = replyarguments[0].toInt();
    // TODO: this should be using Solid::errorString() or org.kde.SolidUiServer getter for the error string
    if (replyvalue != 0) {
        emit q->error();
        KMessageBox::error(
            nullptr /*TODO - window*/,
            i18n("Could not mount device.")
        );
        q->deleteLater();
        return;
    }

    const KUrl url(m_mountPoint);
    // kDebug(7015) << "KAutoMount: m_strDevice=" << m_strDevice << " -> mountpoint=" << m_mountPoint;
    if (m_bShowFilemanagerWindow) {
        KRun::runUrl(url, "inode/directory", nullptr /*TODO - window*/);
    }
    // Notify about the new stuff in that dir, in case of opened windows showing it
    org::kde::KDirNotify::emitFilesAdded(url.url());

    // Update the desktop file which is used for mount/unmount (icon change)
    kDebug(7015) << " mount finished : updating " << m_desktopFile;
    KUrl dfURL;
    dfURL.setPath( m_desktopFile );
    org::kde::KDirNotify::emitFilesChanged(QStringList() << dfURL.url());

    emit q->finished();
    q->deleteLater();
}

class KAutoUnmountPrivate
{
public:
    KAutoUnmountPrivate(KAutoUnmount *qq, const QString &mountpoint, const QString &desktopFile);

    KAutoUnmount *q;
    QString m_desktopFile;
    QString m_mountpoint;

    QDBusInterface m_solidInterface;
    QDBusPendingCallWatcher* m_callWatcher;

    void slotFinished(QDBusPendingCallWatcher *watcher);
};

KAutoUnmountPrivate::KAutoUnmountPrivate(KAutoUnmount *qq, const QString &mountpoint, const QString &desktopFile)
    : q(qq), m_desktopFile(desktopFile), m_mountpoint(mountpoint),
    m_solidInterface("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer"),
    m_callWatcher(nullptr)
{
}

KAutoUnmount::KAutoUnmount(const QString &mountpoint, const QString &desktopFile)
    : d( new KAutoUnmountPrivate(this, mountpoint, desktopFile))
{
    QDBusPendingCall pendingcall = d->m_solidInterface.asyncCall("unmountDevice", mountpoint);
    d->m_callWatcher = new QDBusPendingCallWatcher(pendingcall, this);
    connect(
        d->m_callWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
        this, SLOT(slotFinished(QDBusPendingCallWatcher*))
    );
}

void KAutoUnmountPrivate::slotFinished(QDBusPendingCallWatcher *watcher)
{
    if (watcher->isError()) {
        emit q->error();
        KMessageBox::detailedError(
            nullptr /*TODO - window*/,
            i18n("Could not unmount device."),
            watcher->error().message()
        );
        q->deleteLater();
        return;
    }

    const QList<QVariant> replyarguments = watcher->reply().arguments();
    const int replyvalue = replyarguments[0].toInt();
    // TODO: this should be using Solid::errorString() or org.kde.SolidUiServer getter for the error string
    if (replyvalue != 0) {
        emit q->error();
        KMessageBox::error(
            nullptr /*TODO - window*/,
            i18n("Could not unmount device.")
        );
        q->deleteLater();
        return;
    }

    // Update the desktop file which is used for mount/unmount (icon change)
    kDebug(7015) << "unmount finished : updating " << m_desktopFile;
    KUrl dfURL;
    dfURL.setPath(m_desktopFile);
    org::kde::KDirNotify::emitFilesChanged(QStringList() << dfURL.url());

    // Notify about the new stuff in that dir, in case of opened windows showing it
    // You may think we removed files, but this may have also readded some
    // (if the mountpoint wasn't empty). The only possible behavior on FilesAdded
    // is to relist the directory anyway.
    KUrl mp(m_mountpoint);
    org::kde::KDirNotify::emitFilesAdded(mp.url());

    emit q->finished();
    q->deleteLater();
}

KAutoUnmount::~KAutoUnmount()
{
    delete d;
}

#include "moc_kautomount.cpp"
