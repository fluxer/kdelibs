/*
   This file is part of the KDE libraries
   Copyright (c) 2004 Waldo Bastian <bastian@kde.org>
   Copyright (c) 2011 David Faure <faure@kde.org>

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

#include "klockfile.h"

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QHostInfo>

#include "kdebug.h"
#include "krandom.h"
#include "kglobal.h"
#include "kcomponentdata.h"
#include "kde_file.h"

// Related reading:
// http://www.spinnaker.de/linux/nfs-locking.html
// http://en.wikipedia.org/wiki/File_locking
// http://apenwarr.ca/log/?m=201012

// Related source code:
// * the flock program, which uses flock(LOCK_EX), works on local filesystems (including FAT32),
//    but not NFS.
//  Note about flock: don't unlink, it creates a race. http://world.std.com/~swmcd/steven/tech/flock.html

// fcntl(F_SETLK) is not a good solution.
// It locks other processes but locking out other threads must be done by hand,
// and worse, it unlocks when just reading the file in the same process (!).
// See the apenwarr.ca article above.

// open(O_EXCL) seems to be the best solution for local files (on all filesystems),
// it only fails over NFS (at least with old NFS servers, v3 or older).
// See http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=144

// Conclusion: we use O_EXCL regardless.

class KLockFile::Private
{
public:
    Private(const KComponentData &c)
        : staleTime(30), // 30 seconds
          isLocked(false),
          mustCloseFd(false),
          m_pid(-1),
          m_componentData(c)
    {
    }

    KLockFile::LockResult lockFile(KDE_struct_stat &st_buf);
    KLockFile::LockResult deleteStaleLock();

    void writeIntoLockFile(QFile& file, const KComponentData& componentData);
    void readLockFile();

    QFile m_file;
    QString m_fileName;
    int staleTime;
    bool isLocked;
    bool mustCloseFd;
    QTime staleTimer;
    KDE_struct_stat statBuf;
    int m_pid;
    QString m_hostname;
    QString m_componentName;
    KComponentData m_componentData;
};


KLockFile::KLockFile(const QString &file, const KComponentData &componentData)
    : d(new Private(componentData))
{
    d->m_fileName = file;
}

KLockFile::~KLockFile()
{
    unlock();
    delete d;
}

int KLockFile::staleTime() const
{
    return d->staleTime;
}

void KLockFile::setStaleTime(int _staleTime)
{
    d->staleTime = _staleTime;
}

static bool operator==( const KDE_struct_stat &st_buf1,
            const KDE_struct_stat &st_buf2)
{
#define FIELD_EQ(what)       (st_buf1.what == st_buf2.what)
    return FIELD_EQ(st_dev) && FIELD_EQ(st_ino) &&
           FIELD_EQ(st_uid) && FIELD_EQ(st_gid) && FIELD_EQ(st_nlink);
#undef FIELD_EQ
}

static bool operator!=( const KDE_struct_stat& st_buf1,
            const KDE_struct_stat& st_buf2 )
{
    return !(st_buf1 == st_buf2);
}

void KLockFile::Private::writeIntoLockFile(QFile& file, const KComponentData& componentData)
{
    file.setPermissions(QFile::ReadUser|QFile::WriteUser|QFile::ReadGroup|QFile::ReadOther);

    m_hostname = QHostInfo::localHostName();
    m_componentName = componentData.componentName();

    QTextStream stream(&file);
    m_pid = getpid();

    stream << QString::number(m_pid) << endl
           << m_componentName << endl
           << m_hostname << endl;
    stream.flush();
}

void KLockFile::Private::readLockFile()
{
    m_pid = -1;
    m_hostname.clear();
    m_componentName.clear();

    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream ts(&file);
        if (!ts.atEnd())
            m_pid = ts.readLine().toInt();
        if (!ts.atEnd())
            m_componentName = ts.readLine();
        if (!ts.atEnd())
            m_hostname = ts.readLine();
    }
}

KLockFile::LockResult KLockFile::Private::lockFile(KDE_struct_stat &st_buf)
{
    const QByteArray lockFileName = QFile::encodeName( m_fileName );

    int fd = KDE_open(lockFileName.constData(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd < 0) {
        if (errno == EEXIST) {
            // File already exists
            if (KDE_lstat(lockFileName, &st_buf) != 0) { // caller wants stat buf details
                // File got deleted meanwhile! Clear struct rather than leaving it unset.
                st_buf.st_dev = 0;
                st_buf.st_ino = 0;
                st_buf.st_uid = 0;
                st_buf.st_gid = 0;
                st_buf.st_nlink = 0;
            }
            return LockFail;
        } else {
            return LockError;
        }
    }
    // We hold the lock, continue.
    if (!m_file.open(fd, QIODevice::WriteOnly)) {
        return LockError;
    }
    mustCloseFd = true;
    writeIntoLockFile(m_file, m_componentData);

    // stat to get the modification time
    const int result = KDE_lstat(QFile::encodeName(m_fileName), &st_buf);
    if (result != 0) {
        return KLockFile::LockError;
    }
    return KLockFile::LockOK;
}

KLockFile::LockResult KLockFile::Private::deleteStaleLock()
{
    // I see no way to prevent the race condition here, where we could
    // delete a new lock file that another process just got after we
    // decided the old one was too stale for us too.
    kWarning() << "Deleting stale lockfile" << qPrintable(m_fileName);
    QFile::remove(m_fileName);
    return LockOK;
}

KLockFile::LockResult KLockFile::lock(LockFlags options)
{
    if (d->isLocked) {
        return KLockFile::LockOK;
    }

    KLockFile::LockResult result;
    int hardErrors = 5;
    while(true) {
        KDE_struct_stat st_buf;
        // Try to create the lock file
        result = d->lockFile(st_buf);

        if (result == KLockFile::LockOK) {
            d->staleTimer = QTime();
            break;
        } else if (result == KLockFile::LockError) {
            d->staleTimer = QTime();
            if (--hardErrors == 0) {
                break;
            }
        } else {
            // KLockFile::Fail -- there is already such a file present (e.g. left by a crashed app)
            if (!d->staleTimer.isNull() && d->statBuf != st_buf) {
                d->staleTimer = QTime();
            }

            if (d->staleTimer.isNull()) {
                memcpy(&(d->statBuf), &st_buf, sizeof(KDE_struct_stat));
                d->staleTimer.start();

                d->readLockFile();
            }

            bool isStale = false;
            if ((d->m_pid > 0) && !d->m_hostname.isEmpty()) {
                // Check if hostname is us
                if (d->m_hostname == QHostInfo::localHostName()) {
                    // Check if pid still exists
                    int res = ::kill(d->m_pid, 0);
                    if ((res == -1) && (errno == ESRCH)) {
                        isStale = true; // pid does not exist
                    }
                }
            }
            if (d->staleTimer.elapsed() > (d->staleTime*1000)) {
                isStale = true;
            }

            if (isStale) {
                if ((options & ForceFlag) == 0) {
                    return KLockFile::LockStale;
                }

                result = d->deleteStaleLock();

                if (result == KLockFile::LockOK) {
                    // Lock deletion successful
                    d->staleTimer = QTime();
                    continue; // Now try to get the new lock
                } else if (result != KLockFile::LockFail) {
                    return result;
                }
            }
        }

        if (options & NoBlockFlag) {
            break;
        }

        ::usleep(KRandom::randomMax(1000) + 200);
    }
    if (result == LockOK) {
        d->isLocked = true;
    }
    return result;
}

bool KLockFile::isLocked() const
{
    return d->isLocked;
}

void KLockFile::unlock()
{
    if (d->isLocked) {
        ::unlink(QFile::encodeName(d->m_fileName));
        if (d->mustCloseFd) {
            close(d->m_file.handle());
            d->mustCloseFd = false;
        }
        d->m_file.close();
        d->m_pid = -1;
        d->isLocked = false;
    }
}

bool KLockFile::getLockInfo(int &pid, QString &hostname, QString &appname)
{
    if (d->m_pid == -1) {
        return false;
    }
    pid = d->m_pid;
    hostname = d->m_hostname;
    appname = d->m_componentName;
    return true;
}
