/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2007 The ProFTPD Project team           //krazy:exclude=copyright
 * Copyright (c) 2007 Alex Merry <alex.merry@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "proctitle.h"
#include <config.h>
#include <config-kdeinit.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef HAVE_SETPROCTITLE
#  define PF_ARGV_TYPE PF_ARGV_NONE
#  ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#  endif /* HAVE_SYS_TYPES_H */
#  ifdef HAVE_UNISTD_H
#    include <unistd.h>
#  endif /* HAVE_UNISTD_H */
#endif /* HAVE_SETPROCTITLE */

#ifdef HAVE___PROGNAME
extern char *__progname;
#endif /* HAVE___PROGNAME */
#ifdef HAVE___PROGNAME_FULL
extern char *__progname_full;
#endif /* HAVE___PROGNAME_FULL */
extern char **environ;

/**
 * Set up the memory space for setting the proctitle
 */
void proctitle_init(int argc, char *argv[]) {
# ifdef HAVE___PROGNAME
    /* Set the __progname variable so glibc and company
     * don't go nuts.
     */
    __progname = strdup("kdeinit4");
# endif /* HAVE___PROGNAME */
# ifdef HAVE___PROGNAME_FULL
    /* __progname_full too */
    __progname_full = strdup(argv[0]);
# endif /* HAVE___PROGNAME_FULL */
}

void proctitle_set(const char *fmt, ...) {
    va_list msg;

    if ( !fmt ) {
        return;
    }

    va_start(msg, fmt);

#ifdef HAVE_SETPROCTITLE
    char statbuf[BUFSIZ];
    memset(statbuf, 0, sizeof(statbuf));

# if __FreeBSD__ >= 4 && !defined(FREEBSD4_0) && !defined(FREEBSD4_1)
    /* FreeBSD's setproctitle() automatically prepends the process name. */
    vsnprintf(statbuf, sizeof(statbuf), fmt, msg);

# else /* FREEBSD4 */
    /* Manually append the process name for non-FreeBSD platforms. */
    snprintf(statbuf, sizeof(statbuf), "%s", "kdeinit4: ");
    vsnprintf(statbuf + strlen(statbuf),
              sizeof(statbuf) - strlen(statbuf),
              fmt,
              msg);

# endif /* FREEBSD4 */
    setproctitle("%s", statbuf);
#endif /* HAVE_SETPROCTITLE */

    va_end(msg);
}
