/*  This file is part of the KDE libraries
    Copyright (c) 2004 Waldo Bastian <bastian@kde.org>
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
#include "krandom.h"
#include "kde_file.h"

#include <QHostInfo>
#include <QCoreApplication>
#include <QThread>

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define KLOCKFILE_TIMEOUT 250
#define KLOCKFILE_SLEEPTIME 250

class KLockFilePrivate
{
public:
    KLockFilePrivate();

    KLockFile::LockResult tryLock();

    QByteArray m_lockfile;
    int m_lockfd;
    qint64 m_pid;
    QByteArray m_hostname;
};

KLockFilePrivate::KLockFilePrivate()
    : m_lockfd(-1),
    m_pid(-1)
{
    m_pid = static_cast<qint64>(::getpid());
    m_hostname = QHostInfo::localHostName().toUtf8();
}

KLockFile::LockResult KLockFilePrivate::tryLock()
{
    if (m_lockfd != -1) {
        return KLockFile::LockOK;
    }

    m_lockfd = KDE_open(m_lockfile.constData(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (m_lockfd == -1) {
        const int savederrno = errno;
        if (savederrno == EEXIST) {
            QFile infofile(QFile::decodeName(m_lockfile));
            if (Q_UNLIKELY(!infofile.open(QFile::ReadOnly))) {
                kWarning() << infofile.errorString();
                return KLockFile::LockFail;
            }
            const QList<QByteArray> lockinfo = infofile.readAll().split('\t');
            if (Q_UNLIKELY(lockinfo.size() != 2)) {
                kWarning() << "Invalid lock information";
                return KLockFile::LockFail;
            }
            const qint64 lockpid = lockinfo.at(0).toLongLong();
            const QByteArray lockhost = lockinfo.at(1);
            kDebug() << "Lock" << m_lockfile << "held by" << lockpid << "on" << lockhost;

            if (lockhost == m_hostname && ::kill(lockpid, 0) == -1 && errno == ESRCH) {
                kWarning() << "Stale lock" << m_lockfile << "held by" << lockpid << "on" << lockhost;
                return KLockFile::LockStale;
            }

            return KLockFile::LockFail;
        }

        kWarning() << "Could not create lock file" << qt_error_string(savederrno);
        return KLockFile::LockError;
    }

    QByteArray infodata = QByteArray::number(m_pid);
    infodata.append('\t');
    infodata.append(m_hostname);
    if (Q_UNLIKELY(QT_WRITE(m_lockfd, infodata.constData(), infodata.size()) != infodata.size())) {
        const int savederrno = errno;
        kWarning() << "Could not write lock information" << qt_error_string(savederrno);
        return KLockFile::LockError;
    }

    return KLockFile::LockOK;
}


KLockFile::KLockFile(const QString &file)
    : d(new KLockFilePrivate())
{
    d->m_lockfile = QFile::encodeName(file + QLatin1String(".klockfile"));
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
        if (Q_UNLIKELY(::unlink(d->m_lockfile) == -1)) {
            const int savederrno = errno;
            kWarning() << "Could not remove lock file" << qt_error_string(savederrno);
        }
        result = d->tryLock();
    }
    if (!(options & KLockFile::NoBlockFlag) && result == KLockFile::LockFail) {
        const int randomtimeout = KRandom::randomMax(KLOCKFILE_TIMEOUT) + KLOCKFILE_TIMEOUT;
        kDebug() << "Retrying to lock after" << (randomtimeout + KLOCKFILE_SLEEPTIME);
        QCoreApplication::processEvents(QEventLoop::AllEvents, randomtimeout);
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
        if (Q_UNLIKELY(::unlink(d->m_lockfile) == -1)) {
            const int savederrno = errno;
            kWarning() << "Could not remove lock file" << qt_error_string(savederrno);
        }
        d->m_lockfd = -1;
    }
}

bool KLockFile::getLockInfo(qint64 &pid, QString &hostname)
{
    if (d->m_lockfd == -1) {
        return false;
    }
    pid = d->m_pid;
    hostname = QString::fromUtf8(d->m_hostname.constData(), d->m_hostname.size());
    return true;
}
