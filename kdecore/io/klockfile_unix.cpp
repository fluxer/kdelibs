/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "klockfile.h"
#include "kdebug.h"
#include "kde_file.h"

#include <QCoreApplication>
#include <QThread>

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define KLOCKFILE_TIMEOUT 150
#define KLOCKFILE_SLEEPTIME 50

class KLockFilePrivate
{
public:
    KLockFilePrivate();

    KLockFile::LockResult tryLock();

    QByteArray m_lockfile;
    int m_lockfd;
    qint64 m_pid;
};

KLockFilePrivate::KLockFilePrivate()
    : m_lockfd(-1),
    m_pid(-1)
{
    m_pid = static_cast<qint64>(::getpid());
}

KLockFile::LockResult KLockFilePrivate::tryLock()
{
    if (m_lockfd != -1) {
        return KLockFile::LockOK;
    }

    char lockbuffer[256];
    ::memset(lockbuffer, 0, sizeof(lockbuffer) * sizeof(char));
    m_lockfd = KDE_open(m_lockfile.constData(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (m_lockfd == -1) {
        const int savederrno = errno;
        if (savederrno == EEXIST) {
            const int infofile = KDE_open(m_lockfile.constData(), O_RDONLY);
            if (Q_UNLIKELY(infofile == -1)) {
                kWarning() << "Could not open lock file";
                return KLockFile::LockFail;
            }
            const int inforesult = QT_READ(infofile, lockbuffer, sizeof(lockbuffer));
            QT_CLOSE(infofile);
            if (Q_UNLIKELY(inforesult <= 0)) {
                kWarning() << "Could not read lock file";
                return KLockFile::LockFail;
            }
            qint64 lockpid = 0;
            char lockpidbuffer[256];
            ::memset(lockpidbuffer, 0, sizeof(lockpidbuffer) * sizeof(char));
            ::sscanf(lockbuffer, "%lld", &lockpid);
            if (Q_UNLIKELY(lockpid <= 0)) {
                kWarning() << "Invalid lock information";
                return KLockFile::LockFail;
            }
            kDebug() << "Lock" << m_lockfile << "held by" << lockpid;

            if (::kill(lockpid, 0) == -1 && errno == ESRCH) {
                kWarning() << "Stale lock" << m_lockfile << "held by" << lockpid;
                return KLockFile::LockStale;
            }

            return KLockFile::LockFail;
        }

        kWarning() << "Could not create lock file" << qt_error_string(savederrno);
        return KLockFile::LockError;
    }

    ::memset(lockbuffer, 0, sizeof(lockbuffer) * sizeof(char));
    ::snprintf(lockbuffer, sizeof(lockbuffer), "%lld", m_pid);
    const int lockbufferlen = qstrlen(lockbuffer);
    if (Q_UNLIKELY(QT_WRITE(m_lockfd, lockbuffer, lockbufferlen) != lockbufferlen)) {
        const int savederrno = errno;
        kWarning() << "Could not write lock information" << qt_error_string(savederrno);
        return KLockFile::LockError;
    }

    return KLockFile::LockOK;
}


KLockFile::KLockFile(const QString &file)
    : d(new KLockFilePrivate())
{
    d->m_lockfile = QFile::encodeName(file);
    d->m_lockfile.append(".klockfile");
}

KLockFile::~KLockFile()
{
    unlock();
    delete d;
}

KLockFile::LockResult KLockFile::lock(LockFlags options)
{
tryagain:
    KLockFile::LockResult result = d->tryLock();
    if (result == KLockFile::LockStale && options & KLockFile::ForceFlag) {
        kWarning() << "Deleting stale lock file" << d->m_lockfile;
        if (Q_UNLIKELY(::unlink(d->m_lockfile.constData()) == -1)) {
            const int savederrno = errno;
            kWarning() << "Could not remove lock file" << qt_error_string(savederrno);
        }
        result = d->tryLock();
    }
    if (!(options & KLockFile::NoBlockFlag) && result == KLockFile::LockFail) {
        kDebug() << "Retrying to lock after" << (KLOCKFILE_TIMEOUT + KLOCKFILE_SLEEPTIME);
        QCoreApplication::processEvents(QEventLoop::AllEvents, KLOCKFILE_TIMEOUT);
        QThread::msleep(KLOCKFILE_SLEEPTIME);
        goto tryagain;
    }
    return result;
}

bool KLockFile::isLocked() const
{
    return (d->m_lockfd != -1);
}

void KLockFile::unlock()
{
    if (d->m_lockfd != -1) {
        QT_CLOSE(d->m_lockfd);
        if (Q_UNLIKELY(::unlink(d->m_lockfile.constData()) == -1)) {
            const int savederrno = errno;
            kWarning() << "Could not remove lock file" << qt_error_string(savederrno);
        }
        d->m_lockfd = -1;
    }
}

bool KLockFile::getLockInfo(qint64 &pid)
{
    if (d->m_lockfd == -1) {
        return false;
    }
    pid = d->m_pid;
    return true;
}
