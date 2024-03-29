// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

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
#ifndef KIO_GLOBAL_H
#define KIO_GLOBAL_H

#include <kio/kio_export.h>

#include <QtCore/QMap>
#include <QtCore/QFile>  // for QFile::Permissions
#include <QtCore/QTime>
#include <QtGui/QPixmap> // for pixmapForUrl

#include <kiconloader.h>
#include <kjob.h>

#include <sys/types.h> // mode_t

class KUrl;
class KJobTrackerInterface;

/**
 * @short A namespace for KIO globals
 *
 */
namespace KIO
{
  /// 64-bit file offset
  typedef qlonglong fileoffset_t;
  /// 64-bit file size
  typedef qulonglong filesize_t;

  /**
   * Converts @p size from bytes to the string representation.
   *
   * @param  size  size in bytes
   * @return converted size as a string - e.g. 123.4 KiB , 12.0 MiB
   */
  KIO_EXPORT QString convertSize( KIO::filesize_t size );

  /**
   * Converts a size to a string representation
   * Not unlike QString::number(...)
   *
   * @param size size in bytes
   * @return  converted size as a string - e.g. 123456789
   */
  KIO_EXPORT QString number( KIO::filesize_t size );

  /**
   * Converts size from kibi-bytes (2^10) to the string representation.
   *
   * @param  kibSize  size in kibi-bytes (2^10)
   * @return converted size as a string - e.g. 123.4 KiB , 12.0 MiB
   */
   KIO_EXPORT QString convertSizeFromKiB( KIO::filesize_t kibSize );

  /**
   * Calculates remaining time in seconds from total size, processed size and speed.
   *
   * @param  totalSize      total size in bytes
   * @param  processedSize  processed size in bytes
   * @param  speed          speed in bytes per second
   * @return calculated remaining time in seconds
   */
  KIO_EXPORT unsigned int calculateRemainingSeconds( KIO::filesize_t totalSize,
                                                     KIO::filesize_t processedSize, KIO::filesize_t speed );

  /**
   * Convert @p seconds to a string representing number of days, hours, minutes and seconds
   *
   * @param  seconds number of seconds to convert
   * @return string representation in a locale depending format
   */
  KIO_EXPORT QString convertSeconds( unsigned int seconds );

  /**
   * Calculates remaining time from total size, processed size and speed.
   * Warning: As QTime is limited to 23:59:59, use calculateRemainingSeconds() instead
   *
   * @param  totalSize      total size in bytes
   * @param  processedSize  processed size in bytes
   * @param  speed          speed in bytes per second
   * @return calculated remaining time
   */

  /**
   * Helper for showing information about a set of files and directories
   * @param items the number of items (= @p files + @p dirs + number of symlinks :)
   * @param files the number of files
   * @param dirs the number of dirs
   * @param size the sum of the size of the @p files
   * @param showSize whether to show the size in the result
   * @return the summary string
   */
  KIO_EXPORT QString itemsSummaryString(uint items, uint files, uint dirs, KIO::filesize_t size, bool showSize);

  /**
   * Encodes (from the text displayed to the real filename)
   * This translates '/' into a "unicode fraction slash", QChar(0x2044).
   * Used by KIO::link, for instance.
   * @param str the file name to encode
   * @return the encoded file name
   */
  KIO_EXPORT QString encodeFileName( const QString & str );
  /**
   * Decodes (from the filename to the text displayed)
   * This doesn't do anything anymore, it used to do the opposite of encodeFileName
   * when encodeFileName was using %2F for '/'.
   * @param str the file name to decode
   * @return the decoded file name
   */
  KIO_EXPORT QString decodeFileName( const QString & str );

  /**
   * Error codes that can be emitted by KIO.
   */
  // TODO: s/COULD_NOT/CANNOT/ or the other way round
  enum Error {
    ERR_CANNOT_OPEN_FOR_READING = KJob::UserDefinedError + 1,
    ERR_CANNOT_OPEN_FOR_WRITING = KJob::UserDefinedError + 2,
    ERR_CANNOT_LAUNCH_PROCESS = KJob::UserDefinedError + 3,
    ERR_INTERNAL = KJob::UserDefinedError + 4,
    ERR_MALFORMED_URL = KJob::UserDefinedError + 5,
    ERR_UNSUPPORTED_ACTION = KJob::UserDefinedError + 6,
    // ... where a file was expected
    ERR_IS_DIRECTORY = KJob::UserDefinedError + 7,
    // ... where a directory was expected (e.g. listing)
    ERR_IS_FILE = KJob::UserDefinedError + 8,
    ERR_DOES_NOT_EXIST = KJob::UserDefinedError + 9,
    ERR_FILE_ALREADY_EXIST = KJob::UserDefinedError + 10,
    ERR_DIR_ALREADY_EXIST = KJob::UserDefinedError + 11,
    ERR_UNKNOWN_HOST = KJob::UserDefinedError + 12,
    ERR_ACCESS_DENIED = KJob::UserDefinedError + 13,
    ERR_WRITE_ACCESS_DENIED = KJob::UserDefinedError + 14,
    ERR_CANNOT_ENTER_DIRECTORY = KJob::UserDefinedError + 15,
    ERR_CYCLIC_LINK = KJob::UserDefinedError + 16,
    ERR_USER_CANCELED = KJob::KilledJobError,
    ERR_COULD_NOT_CONNECT = KJob::UserDefinedError + 17,
    ERR_CONNECTION_BROKEN = KJob::UserDefinedError + 18,
    ERR_COULD_NOT_READ = KJob::UserDefinedError + 19,
    ERR_COULD_NOT_WRITE = KJob::UserDefinedError + 20,
    ERR_COULD_NOT_BIND = KJob::UserDefinedError + 21,
    ERR_COULD_NOT_LISTEN = KJob::UserDefinedError + 22,
    ERR_COULD_NOT_ACCEPT = KJob::UserDefinedError + 23,
    ERR_COULD_NOT_LOGIN = KJob::UserDefinedError + 24,
    ERR_COULD_NOT_STAT = KJob::UserDefinedError + 25,
    ERR_COULD_NOT_CLOSEDIR = KJob::UserDefinedError + 26,
    ERR_COULD_NOT_MKDIR = KJob::UserDefinedError + 27,
    ERR_COULD_NOT_RMDIR = KJob::UserDefinedError + 28,
    ERR_COULD_NOT_SEEK = KJob::UserDefinedError + 29,
    ERR_CANNOT_RESUME = KJob::UserDefinedError + 30,
    ERR_CANNOT_RENAME = KJob::UserDefinedError + 31,
    ERR_CANNOT_CHMOD = KJob::UserDefinedError + 32,
    ERR_CANNOT_DELETE = KJob::UserDefinedError + 33,
    ERR_CANNOT_CHOWN = KJob::UserDefinedError + 34,
    // The text argument is the protocol that the dead slave supported.
    // This means for example: file, ftp, http, ...
    ERR_SLAVE_DIED = KJob::UserDefinedError + 35,
    ERR_OUT_OF_MEMORY = KJob::UserDefinedError + 36,
    ERR_UNKNOWN_PROXY_HOST = KJob::UserDefinedError + 37,
    ERR_COULD_NOT_AUTHENTICATE = KJob::UserDefinedError + 38,
    ERR_INTERNAL_SERVER = KJob::UserDefinedError + 39,
    ERR_SERVER_TIMEOUT = KJob::UserDefinedError + 40,
    ERR_SERVICE_NOT_AVAILABLE = KJob::UserDefinedError + 41,
    ERR_UNKNOWN = KJob::UserDefinedError + 42,
    ERR_CANNOT_DELETE_ORIGINAL = KJob::UserDefinedError + 43,
    ERR_CANNOT_DELETE_PARTIAL = KJob::UserDefinedError + 44,
    ERR_CANNOT_RENAME_ORIGINAL = KJob::UserDefinedError + 45,
    ERR_CANNOT_RENAME_PARTIAL = KJob::UserDefinedError + 46,
    ERR_CANNOT_SYMLINK = KJob::UserDefinedError + 47,
    // Action succeeded but no content will follow.
    ERR_NO_CONTENT = KJob::UserDefinedError + 48,
    ERR_DISK_FULL = KJob::UserDefinedError + 49,
    // src==dest when moving/copying
    ERR_IDENTICAL_FILES = KJob::UserDefinedError + 50,
    // for slave specified errors that can be
    // rich text.  Email links will be handled
    // by the standard email app and all hrefs
    // will be handled by the standard browser.
    // <a href="exec:/khelpcenter ?" will be
    // forked.
    ERR_SLAVE_DEFINED = KJob::UserDefinedError + 51,
    // Emitted by setModificationTime
    ERR_CANNOT_SETTIME = KJob::UserDefinedError + 52
  };

  /**
   * Returns a translated error message for @p errorCode using the
   * additional error information provided by @p errorText.
   * @param errorCode the error code
   * @param errorText the additional error text
   * @return the created error string
   */
  KIO_EXPORT QString buildErrorString(int errorCode, const QString &errorText);

  /**
   * Returns translated error details for @p errorCode using the
   * additional error information provided by @p errorText and @p reqUrl
   * (the request URL).
   *
   * @param errorCode the error code
   * @param errorText the additional error text
   * @param reqUrl the request URL
   * @return the following data:
   * @li QString errorName - the name of the error
   * @li QString techName - if not null, the more technical name of the error
   * @li QString description - a description of the error
   * @li QStringList causes - a list of possible causes of the error
   * @li QStringList solutions - a liso of solutions for the error
   */
  KIO_EXPORT QByteArray rawErrorDetail(int errorCode, const QString &errorText,
                                       const KUrl *reqUrl = 0L);

  /**
   * Returns an appropriate error message if the given command @p cmd
   * is an unsupported action (ERR_UNSUPPORTED_ACTION).
   * @param protocol name of the protocol
   * @param cmd given command
   * @see enum Command
   */
  KIO_EXPORT QString unsupportedActionErrorString(const QString &protocol, int cmd);

  /**
   * Convenience method to find the pixmap for a URL.
   *
   * Call this one when you don't know the mimetype.
   *
   * @param _url URL for the file.
   * @param _mode the mode of the file. The mode may modify the icon
   *              with overlays that show special properties of the
   *              icon. Use 0 for default
   * @param _group The icon group where the icon is going to be used.
   * @param _force_size Override globally configured icon size.
   *        Use 0 for the default size
   * @param _state The icon state, one of: KIconLoader::DefaultState,
   * KIconLoader::ActiveState or KIconLoader::DisabledState.
   * @param _path Output parameter to get the full path. Seldom needed.
   *              Ignored if 0
   * @return the pixmap of the URL, can be a default icon if not found
   */
  KIO_EXPORT QPixmap pixmapForUrl( const KUrl & _url, mode_t _mode = 0, KIconLoader::Group _group = KIconLoader::Desktop,
                                   int _force_size = 0, int _state = 0, QString * _path = 0 );

  KIO_EXPORT KJobTrackerInterface *getJobTracker();

  /**
   * Converts KIO file permissions from mode_t to QFile::Permissions format.
   *
   * This is a convenience function for converting KIO permissions parameter from
   * mode_t to QFile::Permissions.
   *
   * @param permissions KIO file permissions.
   *
   * @return -1 if @p permissions is -1, otherwise its OR'ed QFile::Permission equivalent.
   * @since 4.12
   */
  KIO_EXPORT QFile::Permissions convertPermissions(int permissions);

/**
 * MetaData is a simple map of key/value strings.
 */
class KIO_EXPORT MetaData : public QMap<QString, QString>
{
public:
   /**
    * Adds the given meta data map to this map.
    * @param metaData the map to add
    * @return this map
    */
   MetaData & operator += ( const MetaData &metaData )
   {
      MetaData::ConstIterator it;
      for(it = metaData.constBegin(); it !=  metaData.constEnd(); ++it)
      {
         insert(it.key(), it.value());
      }
      return *this;
   }

   /**
    * @overload
    * Adds the given meta data map to this map.
    * @param metaData the map to add
    * @return this map
    */
   MetaData & operator += ( const QMap<QString,QString> &metaData )
   {
      MetaData::ConstIterator it;
      for(it = metaData.constBegin(); it !=  metaData.constEnd(); ++it)
      {
         insert(it.key(), it.value());
      }
      return *this;
   }
};

}
#endif
