/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/*
  This file is part of the KDE libraries
  Copyright 1999 Waldo Bastian <bastian@kde.org>
  Copyright 2006 Allen Winter <winter@kde.org>
  Copyright 2006 Gregory S. Hayes <syncomm@kde.org>
  Copyright 2006 Jaison Lee <lee.jaison@gmail.com>

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

#include "ksavefile.h"

#include <config.h>

#include <QDir>
#include <QProcess>
#include <QTemporaryFile>

#include <kde_file.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Only used by the backup file feature:
#include <kconfig.h>
#include <kconfiggroup.h>

#include <stdlib.h>
#include <errno.h>

class KSaveFile::Private
{
public:
    QString realFileName; //The name of the end-result file
    QString tempFileName; //The name of the temp file we are using

    QFile::FileError error;
    QString errorString;
    bool needFinalize;
    bool directWriteFallback;

    Private()
        : error(QFile::NoError),
          needFinalize(false),
          directWriteFallback(false)
    {
    }
};

KSaveFile::KSaveFile()
 : d(new Private())
{
}

KSaveFile::KSaveFile(const QString &filename)
 : d(new Private())
{
    KSaveFile::setFileName(filename);
}

KSaveFile::~KSaveFile()
{
    finalize();

    delete d;
}

bool KSaveFile::open(OpenMode flags)
{
    if (isOpen()) {
        return false;
    }
    d->needFinalize = false;
    if ( d->realFileName.isNull() ) {
        d->error=QFile::OpenError;
        d->errorString=i18n("No target filename has been given.");
        return false;
    }

    if ( !d->tempFileName.isNull() ) {
#if 0 // do not set an error here, this open() fails, but the file itself is without errors
        d->error=QFile::OpenError;
        d->errorString=i18n("Already opened.");
#endif
        return false;
    }

    //Create our temporary file
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    tempFile.setFileTemplate(d->realFileName + QLatin1String("XXXXXX.new"));
    if (!tempFile.open()) {
#ifdef Q_OS_UNIX
        if (d->directWriteFallback && errno == EACCES) {
            QFile::setFileName(d->realFileName);
            if (QFile::open(flags)) {
                d->tempFileName.clear();
                d->error = QFile::NoError;
                d->needFinalize = true;
                return true;
            }
        }
#endif

        // we only check here if the directory can be written to
        // the actual filename isn't written to, but replaced later
        // with the contents of our tempfile
        const QFileInfo fileInfo(d->realFileName);
        QDir parentDir = fileInfo.dir();
        if (!QFileInfo(parentDir.absolutePath()).isWritable()) {
            d->error=QFile::PermissionsError;
            d->errorString=i18n("Insufficient permissions in target directory.");
            return false;
        }
        d->error=QFile::OpenError;
        d->errorString=i18n("Unable to open temporary file. Error was: %1.", tempFile.error());
        return false;
    }

    // if we're overwriting an existing file, ensure temp file's
    // permissions are the same as existing file so the existing
    // file's permissions are preserved. this will succeed completely
    // only if we are the same owner and group - or allmighty root.
    QFileInfo fi ( d->realFileName );
    if (fi.exists()) {
        //Qt apparently has no way to change owner/group of file :(
        if (::fchown(tempFile.handle(), fi.ownerId(), fi.groupId()) == -1) {
            // failed to set user and group => try to restore group only.
            ::fchown(tempFile.handle(), -1, fi.groupId());
        }

        tempFile.setPermissions(fi.permissions());
    }
    else {
        mode_t umsk = KGlobal::umask();
        fchmod(tempFile.handle(), 0666&(~umsk));
    }

    //Open oursleves with the temporary file
    QFile::setFileName(tempFile.fileName());
    if (!QFile::open(flags)) {
        tempFile.setAutoRemove(true);
        return false;
    }

    d->tempFileName = tempFile.fileName();
    d->error=QFile::NoError;
    d->errorString.clear();
    d->needFinalize = true;
    return true;
}

void KSaveFile::setFileName(const QString &filename)
{
    d->realFileName = filename;

    // make absolute if needed
    if ( QDir::isRelativePath( filename ) ) {
        d->realFileName = QDir::current().absoluteFilePath( filename );
    }

    // follow symbolic link, if any
    d->realFileName = KStandardDirs::realFilePath( d->realFileName );
}

QFile::FileError KSaveFile::error() const
{
    if ( d->error != QFile::NoError ) {
        return d->error;
    } else {
        return QFile::error();
    }
}

QString KSaveFile::errorString() const
{
    if ( !d->errorString.isEmpty() ) {
        return d->errorString;
    } else {
        return QFile::errorString();
    }
}

QString KSaveFile::fileName() const
{
    return d->realFileName;
}

void KSaveFile::abort()
{
    close();
    if (!d->tempFileName.isEmpty()) {
        QFile::remove(d->tempFileName); //non-static QFile::remove() does not work.
        d->needFinalize = false;
    }
}

bool KSaveFile::finalize()
{
    if (!d->needFinalize) {
        return false;
    }

    bool success = false;
#ifdef Q_OS_UNIX
    static const bool extraSync = (::getenv("KDE_EXTRA_FSYNC") != 0 ? true : false);
    if (extraSync) {
        if (flush()) {
            forever {
#ifdef HAVE_FDATASYNC
                if (::fdatasync(handle()) == 0)
#else
                if (::fsync(handle()) == 0)
#endif
                    break;
                if (errno != EINTR) {
                    d->error = QFile::WriteError;
                    d->errorString = i18n("Synchronization to disk failed");
                    break;
                }
            }
        }
    }
#endif

    close();

    if (!d->tempFileName.isEmpty()) {
        if (error() != NoError) {
            QFile::remove(d->tempFileName);
        }
        //Qt does not allow us to atomically overwrite an existing file,
        //so if the target file already exists, there is no way to change it
        //to the temp file without creating a small race condition. So we use
        //the standard rename call instead, which will do the copy without the
        //race condition.
        else if (0 == KDE::rename(d->tempFileName,d->realFileName)) {
            d->error=QFile::NoError;
            d->errorString.clear();
            success = true;
        } else {
            d->error=QFile::OpenError;
            d->errorString=i18n("Error during rename.");
            QFile::remove(d->tempFileName);
        }
    } else { // direct overwrite
        success = true;
    }
    d->needFinalize = false;

    return success;
}

void KSaveFile::setDirectWriteFallback(bool enabled)
{
    d->directWriteFallback = enabled;
}

bool KSaveFile::directWriteFallback() const
{
    return d->directWriteFallback;
}

bool KSaveFile::backupFile( const QString& qFilename, const QString& backupDir )
{
    // get extension from config, by default use "~"

    KConfigGroup g(KGlobal::config(), "Backups"); // look in the Backups section
    QString extension = g.readEntry( "Extension", "~" );

    QString backupFileName = qFilename + extension;

    if ( !backupDir.isEmpty() ) {
        QFileInfo fileInfo ( qFilename );
        backupFileName = backupDir + QLatin1Char('/') + fileInfo.fileName() + extension;
    }

    // kDebug() << "KSaveFile copying " << qFilename << " to " << backupFileName;
    QFile::remove(backupFileName);
    return QFile::copy(qFilename, backupFileName);
}
