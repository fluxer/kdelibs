/* kgrantpty - helper program for KPty. */

/* This program is based on the glibc2.1 pt_chmod.
 * It was pulled out from there since both Linux
 * distributors and other OSes are not able to make
 * use of the glibc for different reasons.
 *
 * THIS IS A ROOT SUID PROGRAM
 *
 * Things work as following:
 *
 * In konsole we open a master pty. This can be opened
 * done by at most one process. Prior to opening the
 * master pty, the slave pty cannot be opened. Then, in
 * grantpty, we fork to this program. The trick is, that
 * the parameter is passes as a file handle, which cannot
 * be faked, so that we get a secure setuid root chmod/chown
 * with this program.
 *
 * We have to chown/chmod the slave pty to prevent eavesdroping.
 *
 * Contributed by Zack Weinberg <zack@rabi.phys.columbia.edu>, 1998.
 * Copyright (c) 1999 by Lars Doelle <lars.doelle@on-line.de>.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA
 */

#include <config.h>
#include <config-pty.h>

#include <sys/types.h>
#include <errno.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#ifdef HAVE_PTY_H
#  include <pty.h>
#endif

#include <sys/param.h>

#define TTY_GROUP "tty"

int main (int argc, char *argv[])
{
  struct stat   st;
  gid_t         gid;
  uid_t         uid;
  mode_t        mod;
  char*         tty = 0;
  int           fd;
#if defined(HAVE_PTSNAME_R)
  char          ttyb[32];
#endif

  /* check preconditions **************************************************/
  if (argc != 3 || (strcmp(argv[1],"--grant") && strcmp(argv[1],"--revoke")))
  {
    printf("usage: %s (--grant|--revoke) <file descriptor>\n"
           "%s is a helper for the KDE core libraries.\n"
           "It is not intended to be called from the command line.\n"
           "It needs to be installed setuid root to function.\n",
           argv[0], argv[0]);
    return 1; /* FAIL */
  }

  if (geteuid () != 0)
  {
    fprintf(stderr, "%s not installed setuid root\n", argv[0]);
    return 1; /* FAIL */
  }

  fd = atoi(argv[2]);

  /* get slave pty name from master pty file handle *********/
#ifdef HAVE_PTSNAME_R
  ::memset(ttyb, '\0', sizeof(ttyb) * sizeof(char));
  if (ptsname_r(fd, ttyb, sizeof(ttyb)) == 0) {
     tty = ttyb;
  if (!tty)
#else
  tty = ptsname(fd);
  if (!tty)
#endif
  {
    /* Check that fd is a valid master pseudo terminal.  */
    char *pty = ttyname(fd);

    if (pty == NULL)
    {
      fprintf(stderr,"%s: cannot determine pty name.\n",argv[0]);
      return 1; /* FAIL */
    }

    /* matches /dev/pty?? */
    if (memcmp(pty,"/dev/pty",8))
    {
      fprintf(stderr,"%s: determined a strange pty name '%s'.\n",argv[0],pty);
      free(pty);
      return 1; /* FAIL */
    }

    tty = malloc(strlen(pty) + 1);
    strcpy(tty,"/dev/tty");
    strcat(tty,pty+8);
    free(pty);
  }

  /* Check that the returned slave pseudo terminal is a character device.  */
  if (stat(tty, &st) < 0 || !S_ISCHR(st.st_mode))
  {
    fprintf(stderr,"%s: found '%s' not to be a character device.\n",argv[0],tty);
    free(tty);
    return 1; /* FAIL */
  }

  /* setup parameters for the operation ***********************************/

  if (!strcmp(argv[1],"--grant"))
  {
    uid = getuid();
    struct group* p = getgrnam(TTY_GROUP);
    if (!p)
      p = getgrnam("wheel");
    gid = p ? p->gr_gid : getgid ();
    mod = S_IRUSR | S_IWUSR | S_IWGRP;
  }
  else
  {
    uid = 0;
    gid = st.st_gid == getgid () ? 0 : -1;
    mod = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
  }

  /* Perform the actual chown/chmod ************************************/

  if (chown(tty, uid, gid) < 0)
  {
    fprintf(stderr,"%s: cannot chown %s: %s\n",argv[0],tty,strerror(errno));
    free(tty);
    return 1; /* FAIL */
  }

  if (chmod(tty, mod) < 0)
  {
    fprintf(stderr,"%s: cannot chmod %s: %s\n",argv[0],tty,strerror(errno));
    free(tty);
    return 1; /* FAIL */
  }

  free(tty);
  return 0; /* OK */
}
