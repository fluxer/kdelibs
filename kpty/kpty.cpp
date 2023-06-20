/*

   This file is part of the KDE libraries
   Copyright (C) 2002 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2002-2003,2007-2008 Oswald Buddenhagen <ossi@kde.org>
   Copyright (C) 2010 KDE e.V. <kde-ev-board@kde.org>
     Author Adriaan de Groot <groot@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kpty_p.h"

#include <config.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if defined(HAVE_PTY_H)
# include <pty.h>
#endif

#if defined(HAVE_UTIL_LOGINX) || defined(HAVE_UTIL_LOGIN)
# include <util.h>
#endif

#if defined(HAVE_LIBUTIL_H)
# include <libutil.h>
#endif

#ifdef HAVE_UTMPX
# include <utmpx.h>
#else
# include <utmp.h>
#endif
#if !defined(_PATH_UTMPX) && defined(_UTMPX_FILE)
# define _PATH_UTMPX _UTMPX_FILE
#endif
#if !defined(_PATH_WTMPX) && defined(_WTMPX_FILE)
# define _PATH_WTMPX _WTMPX_FILE
#endif

/* for HP-UX (some versions) the extern C is needed, and for other
   platforms it doesn't hurt */
extern "C" {
#include <termios.h>
#if defined(HAVE_TERMIO_H)
# include <termio.h> // struct winsize on some systems
#endif
}

#ifdef HAVE_SYS_STROPTS_H
# include <sys/stropts.h> // Defines I_PUSH
# define _NEW_TTY_CTRL
#endif

#include <kdebug.h>
#include <kde_file.h>

#ifndef PATH_MAX
# define PATH_MAX _POSIX_PATH_MAX
#endif

///////////////////////
// private functions //
///////////////////////

//////////////////
// private data //
//////////////////

KPtyPrivate::KPtyPrivate(KPty* parent)
    : masterFd(-1), slaveFd(-1), ownMaster(true), q_ptr(parent)
{
}

KPtyPrivate::~KPtyPrivate()
{
}

/////////////////////////////
// public member functions //
/////////////////////////////

KPty::KPty()
    : d_ptr(new KPtyPrivate(this))
{
}

KPty::KPty(KPtyPrivate *d)
    : d_ptr(d)
{
    d_ptr->q_ptr = this;
}

KPty::~KPty()
{
    close();
    delete d_ptr;
}

bool KPty::open()
{
    Q_D(KPty);
    if (d->masterFd >= 0) {
        return true;
    }
    d->ownMaster = true;
    // Find a master pty that we can open
    char ptsn[PATH_MAX];
    ::memset(ptsn, '\0', sizeof(ptsn) * sizeof(char));
    if (::openpty( &d->masterFd, &d->slaveFd, ptsn, 0, 0)) {
        d->masterFd = -1;
        d->slaveFd = -1;
        kWarning(175) << "Can't open a pseudo teletype";
        return false;
    }
    d->ttyName = ptsn;
    fcntl(d->masterFd, F_SETFD, FD_CLOEXEC);
    fcntl(d->slaveFd, F_SETFD, FD_CLOEXEC);
    return true;
}

bool KPty::open(int fd)
{
    Q_D(KPty);
    if (d->masterFd >= 0) {
        kWarning(175) << "Attempting to open an already open pty";
        return false;
    }
    d->ownMaster = false;

#ifdef HAVE_PTSNAME_R
    char ptsn[32];
    ::memset(ptsn, '\0', sizeof(ptsn) * sizeof(char));
    if (ptsname_r(fd, ptsn, sizeof(ptsn)) == 0) {
        d->ttyName = ptsn;
#else
    char *ptsn = ptsname(fd);
    if (ptsn) {
        d->ttyName = ptsn;
#endif
    } else {
        kWarning(175) << "Failed to determine pty slave device for fd" << fd;
        return false;
    }
    d->masterFd = fd;
    if (!openSlave()) {
        d->masterFd = -1;
        return false;
    }
    return true;
}

void KPty::closeSlave()
{
    Q_D(KPty);
    if (d->slaveFd < 0) {
        return;
    }
    ::close(d->slaveFd);
    d->slaveFd = -1;
}

bool KPty::openSlave()
{
    Q_D(KPty);
    if (d->slaveFd >= 0) {
        return true;
    }
    if (d->masterFd < 0) {
        kWarning(175) << "Attempting to open pty slave while master is closed";
        return false;
    }
    d->slaveFd = KDE_open(d->ttyName.data(), O_RDWR | O_NOCTTY);
    if (d->slaveFd < 0) {
        kWarning(175) << "Can't open slave pseudo teletype";
        return false;
    }
    fcntl(d->slaveFd, F_SETFD, FD_CLOEXEC);
    return true;
}

void KPty::close()
{
    Q_D(KPty);
    if (d->masterFd < 0) {
        return;
    }
    closeSlave();
    if (d->ownMaster) {
        ::close(d->masterFd);
    }
    d->masterFd = -1;
}

void KPty::setCTty()
{
    Q_D(KPty);
    // Become session leader, process group leader,
    // and get rid of the old controlling terminal.
    setsid();

    // make our slave pty the new controlling terminal.
#ifdef TIOCSCTTY
    ioctl(d->slaveFd, TIOCSCTTY, 0);
#else
    // Q_OS_SOLARIS hack: the first tty opened after setsid() becomes controlling tty
    ::close(KDE_open(d->ttyName, O_WRONLY, 0));
#endif

    // make our new process group the foreground group on the pty
    ::tcsetpgrp(d->slaveFd, ::getpid());
}

void KPty::login(const char *user, const char *remotehost)
{
#ifdef HAVE_UTMPX
    struct utmpx l_struct;
#else
    struct utmp l_struct;
#endif
    ::memset(&l_struct, 0, sizeof(l_struct));
    // note: strncpy without terminators _is_ correct here. man 4 utmp

    if (user) {
#ifdef HAVE_STRUCT_UTMP_UT_USER
        strncpy(l_struct.ut_user, user, sizeof(l_struct.ut_user));
#else
        strncpy(l_struct.ut_name, user, sizeof(l_struct.ut_name));
#endif
    }

    if (remotehost) {
        strncpy(l_struct.ut_host, remotehost, sizeof(l_struct.ut_host));
#ifdef HAVE_STRUCT_UTMP_UT_SYSLEN
        l_struct.ut_syslen = qMin(strlen(remotehost), sizeof(l_struct.ut_host));
#endif
    }

#ifndef __GLIBC__
    Q_D(KPty);
    const char *str_ptr = d->ttyName.data();
    if (!memcmp(str_ptr, "/dev/", 5)) {
        str_ptr += 5;
    }
    strncpy(l_struct.ut_line, str_ptr, sizeof(l_struct.ut_line));
# ifdef HAVE_STRUCT_UTMP_UT_ID
    strncpy(l_struct.ut_id,
            str_ptr + strlen(str_ptr) - sizeof(l_struct.ut_id),
            sizeof(l_struct.ut_id));
# endif
#endif

#ifdef HAVE_UTMPX
    // due to binary hacks ut_tv members must be set explicitly
    struct timeval tod;
    gettimeofday(&tod, 0);
    l_struct.ut_tv.tv_sec = tod.tv_sec;
    l_struct.ut_tv.tv_usec = tod.tv_usec;
#else
    l_struct.ut_time = time(0);
#endif

    // on Linux login() fills these, atleast on NetBSD that is not the case and
    // the utmp/utmpx struct values must be filled before calling
    // loginx()/login()
#ifdef HAVE_STRUCT_UTMP_UT_TYPE
    l_struct.ut_type = USER_PROCESS;
#endif
#ifdef HAVE_STRUCT_UTMP_UT_PID
    l_struct.ut_pid = ::getpid();
#endif
#ifdef HAVE_STRUCT_UTMP_UT_SESSION
    l_struct.ut_session = getsid(0);
#endif

#if defined(HAVE_UTIL_LOGINX)
    ::loginx(&l_struct);
#elif defined(HAVE_LOGIN) || defined(HAVE_UTIL_LOGIN)
    ::login(&l_struct);
#elif defined(HAVE_UTMPX)
# ifdef _PATH_UTMPX
    utmpxname(_PATH_UTMPX);
# endif
    setutxent();
    pututxline(&l_struct);
    endutxent();
# ifdef _PATH_WTMPX
    updwtmpx(_PATH_WTMPX, &l_struct);
# endif
#else
# ifdef _PATH_UTMP
    utmpname(_PATH_UTMP);
# endif
    setutent();
    pututline(&l_struct);
    endutent();
# ifdef _PATH_WTMP
    updwtmp(_PATH_WTMP, &l_struct);
# endif
#endif
}

void KPty::logout()
{
    Q_D(KPty);

    const char *str_ptr = d->ttyName.data();
    if (!memcmp(str_ptr, "/dev/", 5))
        str_ptr += 5;
#ifdef __GLIBC__
    else {
        const char *sl_ptr = strrchr(str_ptr, '/');
        if (sl_ptr)
            str_ptr = sl_ptr + 1;
    }
#endif
#if defined(HAVE_UTIL_LOGINX)
    ::logoutx(str_ptr, 0, DEAD_PROCESS);
#elif defined(HAVE_LOGIN) || defined(HAVE_UTIL_LOGIN)
    ::logout(str_ptr);
#else
# ifdef HAVE_UTMPX
    struct utmpx l_struct, *ut;
# else
    struct utmp l_struct, *ut;
# endif
    memset(&l_struct, 0, sizeof(l_struct));

    strncpy(l_struct.ut_line, str_ptr, sizeof(l_struct.ut_line));

# ifdef HAVE_UTMPX
#  ifdef _PATH_UTMPX
    utmpxname(_PATH_UTMPX);
#  endif
    setutxent();
    if ((ut = getutxline(&l_struct))) {
# else
#  ifdef _PATH_UTMP
    utmpname(_PATH_UTMP);
#  endif
    setutent();
    if ((ut = getutline(&l_struct))) {
# endif
#  ifdef HAVE_STRUCT_UTMP_UT_USER
        memset(ut->ut_user, 0, sizeof(*ut->ut_user));
#  else
        memset(ut->ut_name, 0, sizeof(*ut->ut_name));
#  endif
        memset(ut->ut_host, 0, sizeof(*ut->ut_host));
# ifdef HAVE_STRUCT_UTMP_UT_SYSLEN
        ut->ut_syslen = 0;
# endif
# ifdef HAVE_STRUCT_UTMP_UT_TYPE
        ut->ut_type = DEAD_PROCESS;
# endif
# ifdef HAVE_UTMPX
        // due to binary hacks ut_tv members must be set explicitly
        struct timeval tod;
        gettimeofday(&tod, 0);
        ut->ut_tv.tv_sec = tod.tv_sec;
        ut->ut_tv.tv_usec = tod.tv_usec;
        pututxline(ut);
    }
    endutxent();
# else
        ut->ut_time = time(0);
        pututline(ut);
    }
    endutent();
# endif
#endif
}

bool KPty::tcGetAttr(struct ::termios *ttmode) const
{
    Q_D(const KPty);
#ifdef Q_OS_SOLARIS
    if (::tcgetattr(d->slaveFd, ttmode) == 0) {
        return true;
    }
#endif
    return ::tcgetattr(d->masterFd, ttmode) == 0;
}

bool KPty::tcSetAttr(struct ::termios *ttmode)
{
    Q_D(KPty);
#ifdef Q_OS_SOLARIS
    if (::tcsetattr(d->slaveFd, TCSANOW, ttmode) == 0) {
        return true;
    }
#endif
    return ::tcsetattr(d->masterFd, TCSANOW, ttmode) == 0;
}

bool KPty::setWinSize(int lines, int columns)
{
    Q_D(KPty);
    struct winsize winSize;
    ::memset(&winSize, 0, sizeof(winSize));
    winSize.ws_row = (unsigned short)lines;
    winSize.ws_col = (unsigned short)columns;
    return ioctl(d->masterFd, TIOCSWINSZ, (char *)&winSize) == 0;
}

bool KPty::setEcho(bool echo)
{
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        return false;
    }
    if (!echo) {
        ttmode.c_lflag &= ~ECHO;
    } else {
        ttmode.c_lflag |= ECHO;
    }
    return tcSetAttr(&ttmode);
}

const char *KPty::ttyName() const
{
    Q_D(const KPty);
    return d->ttyName.constData();
}

int KPty::masterFd() const
{
    Q_D(const KPty);
    return d->masterFd;
}

int KPty::slaveFd() const
{
    Q_D(const KPty);
    return d->slaveFd;
}
