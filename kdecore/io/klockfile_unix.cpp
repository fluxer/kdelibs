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
#include <errno.h>

#define KLOCKFILE_TIMEOUT 150
#define KLOCKFILE_SLEEPTIME 50

class KLockFilePrivate
{
public:
    KLockFilePrivate();

    QByteArray m_lockfile;
    int m_lockfd;
};

KLockFilePrivate::KLockFilePrivate()
    : m_lockfd(-1)
{
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

bool KLockFile::tryLock()
{
    if (d->m_lockfd != -1) {
        return true;
    }

#ifdef O_CLOEXEC
    d->m_lockfd = KDE_open(d->m_lockfile.constData(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
#else
    d->m_lockfd = KDE_open(d->m_lockfile.constData(), O_WRONLY | O_CREAT | O_EXCL, 0644);
#endif
    return (d->m_lockfd != -1);
}

void KLockFile::lock()
{
    while (!tryLock()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KLOCKFILE_TIMEOUT);
        QThread::msleep(KLOCKFILE_SLEEPTIME);
    }
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
