/*
   Copyright (C) 2000-2002 Stephan Kulow <coolo@kde.org>
   Copyright (C) 2000-2002 David Faure <faure@kde.org>
   Copyright (C) 2000-2002 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2006 Allan Sandfeld Jensen <sandfeld@kde.org>
   Copyright (C) 2007 Thiago Macieira <thiago@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#define QT_NO_CAST_FROM_ASCII

#include "file.h"
#include <QDirIterator>

#include <config.h>
#include <config-kioslave-file.h>


#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <utime.h>
#include <unistd.h>
#include <string.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qdatetime.h>
#include <QtCore/QVarLengthArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QRegExp>
#include <QtCore/QFile>
#include <QtCore/QProcess>

#include <kdebug.h>
#include <kurl.h>
#include <kcomponentdata.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ktemporaryfile.h>
#include <klocale.h>
#include <limits.h>
#include <kshell.h>
#include <kmountpoint.h>
#include <kstandarddirs.h>
#include <kdirnotify.h>
#include <kio/ioslave_defaults.h>
#include <kde_file.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <kuser.h>

using namespace KIO;

#define MAX_IPC_SIZE (1024*32)

#ifdef HAVE_POSIX_ACL
static void appendACLAtoms( const QByteArray & path, UDSEntry& entry,
                            mode_t type, bool withACL );
#endif

int main( int argc, char **argv )
{
  QCoreApplication app( argc, argv ); // needed for QSocketNotifier
  KComponentData componentData( "kio_file", "kdelibs4" );
  ( void ) KGlobal::locale();

  kDebug(7101) << "Starting" << getpid();

  if (argc != 2)
  {
     fprintf(stderr, "Usage: kio_file app-socket\n");
     exit(-1);
  }

  FileProtocol slave(argv[1]);
  slave.dispatchLoop();

  kDebug(7101) << "Done";
  return 0;
}

FileProtocol::FileProtocol( const QByteArray &app )
    : SlaveBase( "file", app ), openFd(-1)
{
}

FileProtocol::~FileProtocol()
{
}

#ifdef HAVE_POSIX_ACL
static QString aclToText(acl_t acl) {
    ssize_t size = 0;
    char* txt = acl_to_text(acl, &size);
    const QString ret = QString::fromLatin1(txt, size);
    acl_free(txt);
    return ret;
}
#endif

int FileProtocol::setACL( const char *path, mode_t perm, bool directoryDefault )
{
    int ret = 0;
#ifdef HAVE_POSIX_ACL

    const QString ACLString = metaData(QLatin1String("ACL_STRING"));
    const QString defaultACLString = metaData(QLatin1String("DEFAULT_ACL_STRING"));
    // Empty strings mean leave as is
    if ( !ACLString.isEmpty() ) {
        acl_t acl = 0;
        if (ACLString == QLatin1String("ACL_DELETE")) {
            // user told us to delete the extended ACL, so let's write only
            // the minimal (UNIX permission bits) part
            acl = acl_from_mode( perm );
        }
        acl = acl_from_text( ACLString.toLatin1() );
        if ( acl_valid( acl ) == 0 ) { // let's be safe
            ret = acl_set_file( path, ACL_TYPE_ACCESS, acl );
            kDebug(7101) << "Set ACL on:" << path << "to:" << aclToText(acl);
        }
        acl_free( acl );
        if ( ret != 0 ) return ret; // better stop trying right away
    }

    if ( directoryDefault && !defaultACLString.isEmpty() ) {
        if ( defaultACLString == QLatin1String("ACL_DELETE") ) {
            // user told us to delete the default ACL, do so
            ret += acl_delete_def_file( path );
        } else {
            acl_t acl = acl_from_text( defaultACLString.toLatin1() );
            if ( acl_valid( acl ) == 0 ) { // let's be safe
                ret += acl_set_file( path, ACL_TYPE_DEFAULT, acl );
                kDebug(7101) << "Set Default ACL on:" << path << "to:" << aclToText(acl);
            }
            acl_free( acl );
        }
    }
#else
    Q_UNUSED(path);
    Q_UNUSED(perm);
    Q_UNUSED(directoryDefault);
#endif
    return ret;
}

void FileProtocol::chmod( const KUrl& url, int permissions )
{
    const QString path(url.toLocalFile());
    const QByteArray _path( QFile::encodeName(path) );
    /* FIXME: Should be atomic */
    if ( KDE::chmod( path, permissions ) == -1 ||
        ( setACL( _path.data(), permissions, false ) == -1 ) ||
        /* if not a directory, cannot set default ACLs */
        ( setACL( _path.data(), permissions, true ) == -1 && errno != ENOTDIR ) ) {

        switch (errno) {
            case EPERM:
            case EACCES:
                error(KIO::ERR_ACCESS_DENIED, path);
                break;
#if defined(ENOTSUP)
            case ENOTSUP: // from setACL since chmod can't return ENOTSUP
                error(KIO::ERR_UNSUPPORTED_ACTION, i18n("Setting ACL for %1", path));
                break;
#endif
            case ENOSPC:
                error(KIO::ERR_DISK_FULL, path);
                break;
            default:
                error(KIO::ERR_CANNOT_CHMOD, path);
        }
    } else
        finished();
}

void FileProtocol::setModificationTime( const KUrl& url, const QDateTime& mtime )
{
    const QString path(url.toLocalFile());
    KDE_struct_stat statbuf;
    if (KDE::lstat(path, &statbuf) == 0) {
        struct utimbuf utbuf;
        utbuf.actime = statbuf.st_atime; // access time, unchanged
        utbuf.modtime = mtime.toTime_t(); // modification time
        if (KDE::utime(path, &utbuf) != 0) {
            // TODO: errno could be EACCES, EPERM, EROFS
            error(KIO::ERR_CANNOT_SETTIME, path);
        } else {
            finished();
        }
    } else {
        error(KIO::ERR_DOES_NOT_EXIST, path);
    }
}

void FileProtocol::mkdir( const KUrl& url, int permissions )
{
    const QString path(url.toLocalFile());

    kDebug(7101) << path << "permission=" << permissions;

    // Remove existing file or symlink, if requested (#151851)
    if (metaData(QLatin1String("overwrite")) == QLatin1String("true"))
        QFile::remove(path);

    KDE_struct_stat buff;
    if ( KDE::lstat( path, &buff ) == -1 ) {
        if ( KDE::mkdir( path, 0777 /*umask will be applied*/ ) != 0 ) {
            if ( errno == EACCES ) {
                error(KIO::ERR_ACCESS_DENIED, path);
                return;
            } else if ( errno == ENOSPC ) {
                error(KIO::ERR_DISK_FULL, path);
                return;
            } else {
                error(KIO::ERR_COULD_NOT_MKDIR, path);
                return;
            }
        } else {
            if ( permissions != -1 )
                chmod( url, permissions );
            else
                finished();
            return;
        }
    }

    if ( S_ISDIR( buff.st_mode ) ) {
        kDebug(7101) << "ERR_DIR_ALREADY_EXIST";
        error(KIO::ERR_DIR_ALREADY_EXIST, path);
        return;
    }
    error(KIO::ERR_FILE_ALREADY_EXIST, path);
    return;
}

void FileProtocol::get( const KUrl& url )
{
    if (!url.isLocalFile()) {
        KUrl redir(url);
        redir.setProtocol(config()->readEntry("DefaultRemoteProtocol", "sftp"));
        redirection(redir);
        finished();
        return;
    }

    const QString path(url.toLocalFile());
    KDE_struct_stat buff;
    if ( KDE::stat( path, &buff ) == -1 ) {
        if ( errno == EACCES )
           error(KIO::ERR_ACCESS_DENIED, path);
        else
           error(KIO::ERR_DOES_NOT_EXIST, path);
        return;
    }

    if ( S_ISDIR( buff.st_mode ) ) {
        error(KIO::ERR_IS_DIRECTORY, path);
        return;
    }
    if ( !S_ISREG( buff.st_mode ) ) {
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, path);
        return;
    }

    int fd = KDE::open( path, O_RDONLY);
    if ( fd < 0 ) {
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, path);
        return;
    }

#if HAVE_FADVISE
    posix_fadvise( fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

    // Determine the mimetype of the file to be retrieved, and emit it.
    // This is mandatory in all slaves (for KRun/BrowserRun to work)
    // In real "remote" slaves, this is usually done using findByNameAndContent
    // after receiving some data. But we don't know how much data the mimemagic rules
    // need, so for local files, better use findByUrl with localUrl=true.
    KMimeType::Ptr mt = KMimeType::findByUrl( url, buff.st_mode, true /* local URL */ );
    emit mimeType( mt->name() );
    // Emit total size AFTER mimetype
    totalSize( buff.st_size );

    KIO::filesize_t processed_size = 0;

    const QString resumeOffset = metaData(QLatin1String("resume"));
    if ( !resumeOffset.isEmpty() )
    {
        bool ok;
        KIO::fileoffset_t offset = resumeOffset.toLongLong(&ok);
        if (ok && (offset > 0) && (offset < buff.st_size))
        {
            if (KDE_lseek(fd, offset, SEEK_SET) == offset)
            {
                canResume ();
                processed_size = offset;
                kDebug(7101) << "Resume offset:" << KIO::number(offset);
            }
        }
    }

    char buffer[ MAX_IPC_SIZE ];
    while( 1 )
    {
       int n = ::read( fd, buffer, MAX_IPC_SIZE );
       if (n == -1)
       {
          if (errno == EINTR)
              continue;
          error(KIO::ERR_COULD_NOT_READ, path);
          ::close(fd);
          return;
       }
       if (n == 0)
          break; // Finished

       data( QByteArray::fromRawData(buffer, n) );

       processed_size += n;
       processedSize( processed_size );

       //kDebug(7101) << "Processed: " << KIO::number (processed_size);
    }

    data( QByteArray() );

    ::close( fd );

    processedSize( buff.st_size );
    finished();
}

int write_all(int fd, const char *buf, size_t len)
{
   while (len > 0)
   {
      ssize_t written = write(fd, buf, len);
      if (written < 0)
      {
          if (errno == EINTR)
             continue;
          return -1;
      }
      buf += written;
      len -= written;
   }
   return 0;
}

void FileProtocol::open(const KUrl &url, QIODevice::OpenMode mode)
{
    kDebug(7101) << url;

    openPath = url.toLocalFile();
    KDE_struct_stat buff;
    if (KDE::stat(openPath, &buff) == -1) {
        if ( errno == EACCES )
           error(KIO::ERR_ACCESS_DENIED, openPath);
        else
           error(KIO::ERR_DOES_NOT_EXIST, openPath);
        return;
    }

    if ( S_ISDIR( buff.st_mode ) ) {
        error(KIO::ERR_IS_DIRECTORY, openPath);
        return;
    }
    if ( !S_ISREG( buff.st_mode ) ) {
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, openPath);
        return;
    }

    int flags = 0;
    if (mode & QIODevice::ReadOnly) {
        if (mode & QIODevice::WriteOnly) {
            flags = O_RDWR | O_CREAT;
        } else {
            flags = O_RDONLY;
        }
    } else if (mode & QIODevice::WriteOnly) {
        flags = O_WRONLY | O_CREAT;
    }

    if (mode & QIODevice::Append) {
        flags |= O_APPEND;
    } else if (mode & QIODevice::Truncate) {
        flags |= O_TRUNC;
    }

    int fd = -1;
    if ( flags & O_CREAT)
        fd = KDE::open( openPath, flags, 0666);
    else
        fd = KDE::open( openPath, flags);
    if ( fd < 0 ) {
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, openPath);
        return;
    }
    // Determine the mimetype of the file to be retrieved, and emit it.
    // This is mandatory in all slaves (for KRun/BrowserRun to work).
    // If we're not opening the file ReadOnly or ReadWrite, don't attempt to
    // read the file and send the mimetype.
    if (mode & QIODevice::ReadOnly){
        KMimeType::Ptr mt = KMimeType::findByUrl( url, buff.st_mode, true /* local URL */ );
        emit mimeType( mt->name() );
   }

    totalSize( buff.st_size );
    position( 0 );

    emit opened();
    openFd = fd;
}

void FileProtocol::read(KIO::filesize_t bytes)
{
    kDebug(7101) << "File::open -- read";
    Q_ASSERT(openFd != -1);

    QVarLengthArray<char> buffer(bytes);
    while (true) {
        int res;
        do {
            res = ::read(openFd, buffer.data(), bytes);
        } while (res == -1 && errno == EINTR);

        if (res > 0) {
            data( QByteArray::fromRawData(buffer.data(), res) );
            bytes -= res;
        } else {
            // empty array designates eof
            data(QByteArray());
            if (res != 0) {
                error(KIO::ERR_COULD_NOT_READ, openPath);
                close();
            }
            break;
        }
        if (bytes <= 0) break;
    }
}

void FileProtocol::write(const QByteArray &data)
{
    kDebug(7101) << "File::open -- write";
    Q_ASSERT(openFd != -1);

    if (write_all(openFd, data.constData(), data.size())) {
        if (errno == ENOSPC) { // disk full
            error(KIO::ERR_DISK_FULL, openPath);
            close();
        } else {
            kWarning(7101) << "Couldn't write. Error:" << strerror(errno);
            error(KIO::ERR_COULD_NOT_WRITE, openPath);
            close();
        }
    } else {
        written(data.size());
    }
}

void FileProtocol::seek(KIO::filesize_t offset)
{
    kDebug(7101) << "File::open -- seek";
    Q_ASSERT(openFd != -1);

    int res = KDE_lseek(openFd, offset, SEEK_SET);
    if (res != -1) {
        position( offset );
    } else {
        error(KIO::ERR_COULD_NOT_SEEK, openPath);
        close();
    }
}

void FileProtocol::close()
{
    kDebug(7101) << "File::open -- close ";
    Q_ASSERT(openFd != -1);

    ::close( openFd );
    openFd = -1;
    openPath.clear();

    finished();
}

void FileProtocol::put( const KUrl& url, int _mode, KIO::JobFlags _flags )
{
    const QString dest_orig = url.toLocalFile();

    kDebug(7101) << dest_orig << "mode=" << _mode;

    QString dest_part(dest_orig + QLatin1String(".part"));

    KDE_struct_stat buff_orig;
    const bool bOrigExists = (KDE::lstat(dest_orig, &buff_orig) != -1);
    bool bPartExists = false;
    const bool bMarkPartial = config()->readEntry("MarkPartial", true);

    if (bMarkPartial)
    {
        KDE_struct_stat buff_part;
        bPartExists = (KDE::stat( dest_part, &buff_part ) != -1);

        if (bPartExists && !(_flags & KIO::Resume) && !(_flags & KIO::Overwrite) && buff_part.st_size > 0 && S_ISREG(buff_part.st_mode))
        {
            kDebug(7101) << "calling canResume with" << KIO::number(buff_part.st_size);

            // Maybe we can use this partial file for resuming
            // Tell about the size we have, and the app will tell us
            // if it's ok to resume or not.
            _flags |= canResume( buff_part.st_size ) ? KIO::Resume : KIO::DefaultFlags;

            kDebug(7101) << "got answer" << (_flags & KIO::Resume);
        }
    }

    if ( bOrigExists && !(_flags & KIO::Overwrite) && !(_flags & KIO::Resume))
    {
        if (S_ISDIR(buff_orig.st_mode))
            error( KIO::ERR_DIR_ALREADY_EXIST, dest_orig );
        else
            error( KIO::ERR_FILE_ALREADY_EXIST, dest_orig );
        return;
    }

    int result;
    QString dest;
    QByteArray _dest;

    int fd = -1;

    // Loop until we got 0 (end of data)
    do
    {
        QByteArray buffer;
        dataReq(); // Request for data
        result = readData( buffer );

        if (result >= 0)
        {
            if (dest.isEmpty())
            {
                if (bMarkPartial)
                {
                    kDebug(7101) << "Appending .part extension to" << dest_orig;
                    dest = dest_part;
                    if ( bPartExists && !(_flags & KIO::Resume) )
                    {
                        kDebug(7101) << "Deleting partial file" << dest_part;
                        QFile::remove( dest_part );
                        // Catch errors when we try to open the file.
                    }
                }
                else
                {
                    dest = dest_orig;
                    if ( bOrigExists && !(_flags & KIO::Resume) )
                    {
                        kDebug(7101) << "Deleting destination file" << dest_orig;
                        QFile::remove( dest_orig );
                        // Catch errors when we try to open the file.
                    }
                }

                if ( (_flags & KIO::Resume) )
                {
                    fd = KDE::open( dest, O_RDWR );  // append if resuming
                    if (fd != -1) {
                        KDE_lseek(fd, 0, SEEK_END); // Seek to end
                    }
                }
                else
                {
                    // WABA: Make sure that we keep writing permissions ourselves,
                    // otherwise we can be in for a surprise on NFS.
                    mode_t initialMode;
                    if (_mode != -1)
                        initialMode = _mode | S_IWUSR | S_IRUSR;
                    else
                        initialMode = 0666;

                    fd = KDE::open(dest, O_CREAT | O_TRUNC | O_WRONLY, initialMode);
                }

                if ( fd < 0 )
                {
                    kDebug(7101) << "####################### COULD NOT WRITE" << dest << "_mode=" << _mode;
                    kDebug(7101) << "errno==" << errno << "(" << strerror(errno) << ")";
                    if ( errno == EACCES )
                        error(KIO::ERR_WRITE_ACCESS_DENIED, dest);
                    else
                        error(KIO::ERR_CANNOT_OPEN_FOR_WRITING, dest);
                    return;
                }
            }

            if (write_all( fd, buffer.data(), buffer.size()))
            {
                if ( errno == ENOSPC ) // disk full
                {
                  error(KIO::ERR_DISK_FULL, dest_orig);
                  result = -2; // means: remove dest file
                }
                else
                {
                  kWarning(7101) << "Couldn't write. Error:" << strerror(errno);
                  error(KIO::ERR_COULD_NOT_WRITE, dest_orig);
                  result = -1;
                }
            }
        }
    }
    while ( result > 0 );

    // An error occurred deal with it.
    if (result < 0)
    {
        kDebug(7101) << "Error during 'put'. Aborting.";

        if (fd != -1)
        {
          ::close(fd);

          KDE_struct_stat buff;
          if (bMarkPartial && KDE::stat( dest, &buff ) == 0)
          {
            int size = config()->readEntry("MinimumKeepSize", DEFAULT_MINIMUM_KEEP_SIZE);
            if (buff.st_size <  size)
              remove(_dest.data());
          }
        }

        ::exit(255);
    }

    if ( fd == -1 ) // we got nothing to write out, so we never opened the file
    {
        finished();
        return;
    }

    if ( ::close(fd) )
    {
        kWarning(7101) << "Error when closing file descriptor:" << strerror(errno);
        error(KIO::ERR_COULD_NOT_WRITE, dest_orig);
        return;
    }

    // after full download rename the file back to original name
    if ( bMarkPartial )
    {
        // If the original URL is a symlink and we were asked to overwrite it,
        // remove the symlink first. This ensures that we do not overwrite the
        // current source if the symlink points to it.
        if( (_flags & KIO::Overwrite) && S_ISLNK( buff_orig.st_mode ) )
          QFile::remove( dest_orig );
        if ( KDE::rename( dest, dest_orig ) )
        {
            kWarning(7101) << " Couldn't rename " << _dest << " to " << dest_orig;
            error(KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig);
            return;
        }
        org::kde::KDirNotify::emitFileRenamed(dest, dest_orig);
    }

    // set final permissions
    if ( _mode != -1 && !(_flags & KIO::Resume) )
    {
        if (KDE::chmod(dest_orig, _mode) != 0)
        {
            // couldn't chmod. Eat the error if the filesystem apparently doesn't support it.
            KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath(dest_orig);
            if (mp && mp->testFileSystemFlag(KMountPoint::SupportsChmod))
                 warning( i18n( "Could not change permissions for\n%1" ,  dest_orig ) );
        }
    }

    // set modification time
    const QString mtimeStr = metaData(QLatin1String("modified"));
    if ( !mtimeStr.isEmpty() ) {
        QDateTime dt = QDateTime::fromString( mtimeStr, Qt::ISODate );
        if ( dt.isValid() ) {
            KDE_struct_stat dest_statbuf;
            if (KDE::stat( dest_orig, &dest_statbuf ) == 0) {
                struct timeval utbuf[2];
                // access time
                utbuf[0].tv_sec = dest_statbuf.st_atime; // access time, unchanged  ## TODO preserve msec
                utbuf[0].tv_usec = 0;
                // modification time
                utbuf[1].tv_sec = dt.toTime_t();
                utbuf[1].tv_usec = dt.time().msec() * 1000;
                utimes( QFile::encodeName(dest_orig), utbuf );
            }
        }

    }

    // We have done our job => finish
    finished();
}

QString FileProtocol::getUserName( uid_t uid ) const
{
    if ( !mUsercache.contains( uid ) ) {
        const KUser kuser( uid );
        if ( !kuser.isValid() ) {
            return QString::number( uid );
        }
        mUsercache.insert( uid, kuser.loginName() );
    }
    return mUsercache[uid];
}

QString FileProtocol::getGroupName( gid_t gid ) const
{
    if ( !mGroupcache.contains( gid ) ) {
        const KUserGroup kusergroup( gid );
        if ( !kusergroup.isValid() ) {
            return QString::number( gid );
        }
        mGroupcache.insert( gid, kusergroup.name() );
    }
    return mGroupcache[gid];
}

bool FileProtocol::createUDSEntry( const QString & filename, const QByteArray & path, UDSEntry & entry,
                                   short int details, bool withACL )
{
#ifndef HAVE_POSIX_ACL
    Q_UNUSED(withACL);
#endif
    assert(entry.count() == 0); // by contract :-)
    // entry.reserve( 8 ); // speed up QHash insertion

    entry.insert( KIO::UDSEntry::UDS_NAME, filename );

    mode_t type;
    mode_t access;
    KDE_struct_stat buff;

    if ( KDE_lstat( path.data(), &buff ) == 0 )  {

        if (details > 2) {
            entry.insert( KIO::UDSEntry::UDS_DEVICE_ID, buff.st_dev );
            entry.insert( KIO::UDSEntry::UDS_INODE, buff.st_ino );
        }

        if (S_ISLNK(buff.st_mode)) {

            char buffer2[ 1000 ];
            ::memset(buffer2, 0, 1000 * sizeof(char));
            readlink( path.data(), buffer2, 999 );
            entry.insert( KIO::UDSEntry::UDS_LINK_DEST, QFile::decodeName( buffer2 ) );

            // A symlink -> follow it only if details>1
            if ( details > 1 && KDE_stat( path.data(), &buff ) == -1 ) {
                // It is a link pointing to nowhere
                type = S_IFMT - 1;
                access = S_IRWXU | S_IRWXG | S_IRWXO;

                entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, type );
                entry.insert( KIO::UDSEntry::UDS_ACCESS, access );
                entry.insert( KIO::UDSEntry::UDS_SIZE, 0LL );
                goto notype;

            }
        }
    } else {
        // kWarning() << "lstat didn't work on " << path.data();
        return false;
    }

    type = buff.st_mode & S_IFMT; // extract file type
    access = buff.st_mode & 07777; // extract permissions

    entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, type );
    entry.insert( KIO::UDSEntry::UDS_ACCESS, access );

    entry.insert( KIO::UDSEntry::UDS_SIZE, buff.st_size );

#ifdef HAVE_POSIX_ACL
    if (details > 0) {
        /* Append an atom indicating whether the file has extended acl information
         * and if withACL is specified also one with the acl itself. If it's a directory
         * and it has a default ACL, also append that. */
        appendACLAtoms( path, entry, type, withACL );
    }
#endif

 notype:
    if (details > 0) {
        entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime );
        entry.insert( KIO::UDSEntry::UDS_USER, getUserName( buff.st_uid ) );
        entry.insert( KIO::UDSEntry::UDS_GROUP, getGroupName( buff.st_gid ) );
        entry.insert( KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime );
    }

    // Note: buff.st_ctime isn't the creation time !
    // We made that mistake for KDE 2.0, but it's in fact the
    // "file status" change time, which we don't care about.

    return true;
}

void FileProtocol::special( const QByteArray &data)
{
    int tmp;
    QDataStream stream(data);

    stream >> tmp;
    switch (tmp) {
        case 1: {
            QString fstype, dev, point;
            qint8 iRo;

            stream >> iRo >> fstype >> dev >> point;

            bool ro = ( iRo != 0 );

            kDebug(7101) << "MOUNTING fstype=" << fstype << " dev=" << dev << " point=" << point << " ro=" << ro;
            mount( ro, fstype.toLatin1(), dev, point );
            break;
        }
        case 2: {
            QString point;
            stream >> point;

            kDebug(7101) << "UNMOUNTING point=" << point;
            unmount( point );
            break;
        }

        default:
            break;
    }
}


/*************************************
 *
 * mount/unmount handling
 *
 *************************************/

void FileProtocol::mount( bool _ro, const char *_fstype, const QString& _dev, const QString& _point )
{
    const QString kdesudoProg = KStandardDirs::findExe(QLatin1String("kdesudo"));
    if (kdesudoProg.isEmpty()) {
        error( KIO::ERR_COULD_NOT_MOUNT, i18n("Could not find program \"kdesudo\""));
        return;
    }

    const QString mountProg = KStandardDirs::findRootExe(QLatin1String("mount"));
    if (mountProg.isEmpty()) {
        error( KIO::ERR_COULD_NOT_MOUNT, i18n("Could not find program \"mount\""));
        return;
    }

    QProcess mountProc;
    QStringList mountArgs = QStringList() << mountProg;
    if (_ro) {
#if defined(Q_OS_SOLARIS)
        mountArgs << QLatin1String("-oro");
#else
        mountArgs << QLatin1String("-r");
#endif
    }
    if (_fstype && *_fstype) {
#if defined(Q_OS_SOLARIS)
        mountArgs << QLatin1String("-F");
#else
        mountArgs << QLatin1String("-t");
#endif
        mountArgs << KShell::quoteArg(QString::fromLatin1(_fstype));
    }
    if (_dev.startsWith(QLatin1String("LABEL="))) { // turn LABEL=foo into -L foo (#71430)
        QString labelName = _dev.mid( 6 );
        mountArgs << QLatin1String("-L");
        mountArgs << KShell::quoteArg( labelName );
    } else if (_dev.startsWith(QLatin1String("UUID="))) { // and UUID=bar into -U bar
        QString uuidName = _dev.mid( 5 );
        mountArgs << QLatin1String("-U");
        mountArgs << KShell::quoteArg( uuidName );
    } else {
        mountArgs << KShell::quoteArg(_dev);
    }
    if (!_point.isEmpty()) {
        mountArgs << KShell::quoteArg(_point);
    }
    mountProc.start(kdesudoProg, mountArgs);
    mountProc.waitForFinished();

    if ( mountProc.exitCode() == 0 ) {
        finished();
    } else {
        QByteArray erroroutput = mountProc.readAllStandardError();
        if (erroroutput.isEmpty()) {
            erroroutput = mountProc.readAllStandardOutput();
        }
        error( KIO::ERR_COULD_NOT_MOUNT, QString::fromLocal8Bit(erroroutput) );
    }
}


void FileProtocol::unmount( const QString& point )
{
    const QString kdesudoProg = KStandardDirs::findExe(QLatin1String("kdesudo"));
    if (kdesudoProg.isEmpty()) {
        error( KIO::ERR_COULD_NOT_UNMOUNT, i18n("Could not find program \"kdesudo\""));
        return;
    }

    const QString umountProg = KStandardDirs::findRootExe(QLatin1String("umount"));
    if (umountProg.isEmpty()) {
        error( KIO::ERR_COULD_NOT_UNMOUNT, i18n("Could not find program \"umount\""));
        return;
    }

    QProcess umountProc;
    const QStringList umountArgs = QStringList() << umountProg << KShell::quoteArg(point);
    umountProc.start(kdesudoProg, umountArgs);
    umountProc.waitForFinished();

    if ( umountProc.exitCode() == 0 ) {
        finished();
    } else {
        QByteArray erroroutput = umountProc.readAllStandardError();
        if (erroroutput.isEmpty()) {
            erroroutput = umountProc.readAllStandardOutput();
        }
        error( KIO::ERR_COULD_NOT_UNMOUNT, QString::fromLocal8Bit(erroroutput) );
    }
}

/*************************************
 *
 * ACL handling helpers
 *
 *************************************/
#ifdef HAVE_POSIX_ACL

bool FileProtocol::isExtendedACL( acl_t acl )
{
    return ( acl_equiv_mode( acl, 0 ) != 0 );
}

static void appendACLAtoms( const QByteArray & path, UDSEntry& entry, mode_t type, bool withACL )
{
    // first check for a noop
    if ( acl_extended_file( path.data() ) == 0 ) return;

    acl_t acl = 0;
    acl_t defaultAcl = 0;
    bool isDir = S_ISDIR( type );
    // do we have an acl for the file, and/or a default acl for the dir, if it is one?
    acl = acl_get_file( path.data(), ACL_TYPE_ACCESS );
    /* Sadly libacl does not provided a means of checking for extended ACL and default
     * ACL separately. Since a directory can have both, we need to check again. */
    if ( isDir ) {
        if ( acl ) {
            if ( !FileProtocol::isExtendedACL( acl ) ) {
                acl_free( acl );
                acl = 0;
            }
        }
        defaultAcl = acl_get_file( path.data(), ACL_TYPE_DEFAULT );
    }
    if ( acl || defaultAcl ) {
      kDebug(7101) << path.constData() << "has extended ACL entries";
      entry.insert( KIO::UDSEntry::UDS_EXTENDED_ACL, 1 );
    }
    if ( withACL ) {
        if ( acl ) {
            const QString str = aclToText(acl);
            entry.insert( KIO::UDSEntry::UDS_ACL_STRING, str );
            kDebug(7101) << path.constData() << "ACL:" << str;
        }
        if ( defaultAcl ) {
            const QString str = aclToText(defaultAcl);
            entry.insert( KIO::UDSEntry::UDS_DEFAULT_ACL_STRING, str );
            kDebug(7101) << path.constData() << "DEFAULT ACL:" << str;
        }
    }
    if ( acl ) acl_free( acl );
    if ( defaultAcl ) acl_free( defaultAcl );
}
#endif

// We could port this to KTempDir::removeDir but then we wouldn't be able to tell the user
// where exactly the deletion failed, in case of errors.
bool FileProtocol::deleteRecursive(const QString& path)
{
    //kDebug() << path;
    QDirIterator it(path, QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden,
                    QDirIterator::Subdirectories);
    QStringList dirsToDelete;
    while ( it.hasNext() ) {
        const QString itemPath = it.next();
        //kDebug() << "itemPath=" << itemPath;
        const QFileInfo info = it.fileInfo();
        if (info.isDir() && !info.isSymLink())
            dirsToDelete.prepend(itemPath);
        else {
            //kDebug() << "QFile::remove" << itemPath;
            if (!QFile::remove(itemPath)) {
                error(KIO::ERR_CANNOT_DELETE, itemPath);
                return false;
            }
        }
    }
    QDir dir;
    Q_FOREACH(const QString& itemPath, dirsToDelete) {
        //kDebug() << "QDir::rmdir" << itemPath;
        if (!dir.rmdir(itemPath)) {
            error(KIO::ERR_CANNOT_DELETE, itemPath);
            return false;
        }
    }
    return true;
}

#include "moc_file.cpp"
