/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 Joseph Wenninger <jowenn@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "ktempdir.h"

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#ifdef HAVE_TEST
#include <test.h>
#endif

#include <QtCore/QDir>

#include "kglobal.h"
#include "kcomponentdata.h"
#include "kstandarddirs.h"
#include <kdebug.h>
#include "kde_file.h"


class KTempDir::Private
{
public:
    int error;
    QString tmpName;
    bool exists;
    bool autoRemove;

    Private()
        : error(0),
        exists(false),
        autoRemove(true)
    {
    }
};

KTempDir::KTempDir(const QString &directoryPrefix, int mode)
    : d(new Private)
{
    (void) create( directoryPrefix.isEmpty() ? KStandardDirs::locateLocal("tmp", KGlobal::mainComponent().componentName()) : directoryPrefix , mode);
}

bool KTempDir::create(const QString &directoryPrefix, int mode)
{
   QByteArray nme = QFile::encodeName(directoryPrefix) + "XXXXXX";
   char *realName = ::mkdtemp(nme.data());
   if (realName == 0) {
       // Recreate it for the warning, mkdtemps emptied it
       nme = QFile::encodeName(directoryPrefix) + "XXXXXX";
       kWarning(180) << "KTempDir: Error trying to create " << nme.data()
                     << ": " << ::strerror(errno) << endl;
       d->error = errno;
       d->tmpName.clear();
       return false;
   }

   // got a return value != 0
   QByteArray realNameStr(realName);
   d->tmpName = QFile::decodeName(realNameStr)+QLatin1Char('/');
   kDebug(180) << "KTempDir: Temporary directory created :" << d->tmpName << endl;

   mode_t umsk = KGlobal::umask();
   if (::chmod(nme, mode & (~umsk)) < 0) {
       kWarning(180) << "KTempDir: Unable to change permissions on" << d->tmpName
                     << ":" << ::strerror(errno);
       d->error = errno;
       d->tmpName.clear();
       (void) ::rmdir(realName); // Cleanup created directory
       return false;
   }

   // Success!
   d->exists = true;

   // Set uid/gid (necessary for SUID programs)
   if (::chown(nme, ::getuid(), ::getgid()) < 0) {
       // Just warn, but don't failover yet
       kWarning(180) << "KTempDir: Unable to change owner on" << d->tmpName
                     << ":" << ::strerror(errno);
   }

   return true;
}

KTempDir::~KTempDir()
{
    if (d->autoRemove) {
        unlink();
    }

    delete d;
}

int KTempDir::status() const
{
    return d->error;
}

QString KTempDir::name() const
{
    return d->tmpName;
}

bool KTempDir::exists() const
{
    return d->exists;
}

void KTempDir::setAutoRemove(bool autoRemove)
{
    d->autoRemove = autoRemove;
}

bool KTempDir::autoRemove() const
{
    return d->autoRemove;
}

void KTempDir::unlink()
{
    if (!d->exists) {
        return;
    }
    if (KTempDir::removeDir(d->tmpName)) {
        d->error = 0;
    } else {
        d->error = errno;
    }
    d->exists = false;
}

// Auxiliary recursive function for removeDirs
static bool rmtree(const QByteArray& name)
{
    //kDebug(180) << "Checking directory for remove" << name;
    KDE_struct_stat st;
    if (KDE_lstat(name.data(), &st) == -1) { // Do not dereference symlink!
        return false;
    }
    if (S_ISDIR(st.st_mode)) {
        // This is a directory, so process it
        //kDebug(180) << "File" << name << "is DIRECTORY!";
        KDE_struct_dirent* ep;
        DIR* dp = ::opendir(name.data());
        if (!dp) {
            return false;
        }
        while ((ep = KDE_readdir(dp))) {
            //kDebug(180) << "CHECKING" << name << "/" << ep->d_name;
            if (qstrcmp(ep->d_name, "." ) == 0 || qstrcmp(ep->d_name, "..") == 0) {
                continue;
            }
            QByteArray newName(name);
            newName += '/';
            newName += ep->d_name;
            /*
             * Be defensive and close the directory.
             *
             * Potential problems:
             * - opendir/readdir/closedir is not re-entrant
             * - unlink and rmdir invalidates a opendir/readdir/closedir
             * - limited number of file descriptors for opendir/readdir/closedir
             */
            if (::closedir(dp)) {
                kDebug(180) << "Error closing" << name;
                return false;
            }
            // Recurse!
            //kDebug(180) << "RECURSE: " << newName;
            if (!rmtree(newName)) {
                return false;
            }
            // We have to re-open the directory before continuing
            dp = ::opendir(name.data());
            if (!dp)
                return false;
        }
        if (::closedir(dp)) {
            kDebug(180) << "Error closing" << name;
            return false;
        }
        //kDebug(180) << "RMDIR dir " << name;
        return (::rmdir(name) == 0);
    }
    // This is a non-directory file, so remove it
    //kDebug(180) << "KTempDir: unlinking file" << name;
    return (::unlink(name) == 0);
}

bool KTempDir::removeDir(const QString &path)
{
    //kDebug(180) << path;
    if (!QDir(path).exists()) {
        return true; // The goal is that there is no directory
    }

    const QByteArray cstr(QFile::encodeName(path));
    return rmtree(cstr);
}

