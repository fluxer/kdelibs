/* This file is part of the KDE libraries
   Copyright (C) 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

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
#ifndef KDIRWATCH_H
#define KDIRWATCH_H

#include <QtCore/qdatetime.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

#include <kdecore_export.h>

class KDirWatchPrivate;

 /**
  * @short Class for watching directory and file changes.
  *
  * Watch directories and files for changes.
  * The watched directories or files has to exist.
  *
  * When a watched directory is changed, i.e. when files therein are
  * created or deleted, KDirWatch will emit the signal dirty().
  *
  * When a watched, but previously not existing directory gets created,
  * KDirWatch will emit the signal created().
  *
  * When a watched directory gets deleted, KDirWatch will emit the
  * signal deleted(). The directory is still watched for new
  * creation.
  *
  * Both created() and deleted() are kept for compatibility and you
  * should not rely on them as they are used as proxy for other
  * classes. Instead you can use KDirNotify or KDirLister which
  * provide a lot more functionality than KDirWatch.
  *
  * When a watched file is changed, i.e. attributes changed or written
  * to, KDirWatch will emit the signal dirty().
  *
  * The implementation uses the QFileSystemWatch functionality which may
  * uses platform dependant notification API and as last resort
  * stat-based poller.
  *
  * @see self()
  * @author Sven Radej (in 1998)
  */
class KDECORE_EXPORT KDirWatch : public QObject
{
  Q_OBJECT

  public:

   /**
   * Available watch modes for directory monitoring
   **/
    enum WatchMode {
      WatchDirOnly = 0,  ///< Watch just the specified directory
      WatchFiles = 0x01, ///< Watch also all files contained by the directory
      WatchSubDirs = 0x02 ///< Watch also all the subdirs contained by the directory
    };
    Q_DECLARE_FLAGS(WatchModes, WatchMode)

   /**
    * Constructor.
    *
    * Scanning begins immediately when a dir/file watch
    * is added.
    * @param parent the parent of the QObject (or 0 for parent-less KDirWatch)
    */
   KDirWatch(QObject* parent = 0);

   /**
    * Destructor.
    *
    * Stops scanning and cleans up.
    */
   ~KDirWatch();

   /**
    * Adds a directory to be watched.
    *
    * The directory does not have to exist. When @p watchModes is set to
    * WatchDirOnly (the default), the signals dirty(), created(), deleted()
    * can be emitted, all for the watched directory.
    * When @p watchModes is set to WatchFiles, all files in the watched
    * directory are watched for changes, too. Thus, the signals dirty(),
    * created(), deleted() can be emitted.
    * When @p watchModes is set to WatchSubDirs, all subdirs are watched using
    * the same flags specified in @p watchModes (symlinks aren't followed).
    * If the @p path points to a symlink to a directory, the target directory
    * is watched instead. If you want to watch the link, use @p addFile().
    *
    * @param path the path to watch
    * @param watchModes watch modes
    *
    * @sa  KDirWatch::WatchMode
    */
   void addDir(const QString& path, WatchModes watchModes = WatchDirOnly);

   /**
    * Adds a file to be watched.
    * If it's a symlink to a directory, it watches the symlink itself.
    * @param file the file to watch
    */
   void addFile(const QString& file);

   /**
    * Removes a directory from the list of scanned directories.
    *
    * If specified path is not in the list this does nothing.
    * @param path the path of the dir to be removed from the list
    */
   void removeDir(const QString& path);

   /**
    * Removes a file from the list of watched files.
    *
    * If specified path is not in the list this does nothing.
    * @param file the file to be removed from the list
    */
   void removeFile(const QString& file);

   /**
    * Check if a directory is being watched by this KDirWatch instance
    * @param path the directory to check
    * @return true if the directory is being watched
    */
   bool contains( const QString& path) const;

   /**
    * The KDirWatch instance usually globally used in an application.
    * It is automatically deleted when the application exits.
    *
    * However, you can create an arbitrary number of KDirWatch instances
    * aside from this one - for those you have to take care of memory management.
    *
    * This function returns an instance of KDirWatch. If there is none, it
    * will be created.
    *
    * @return a KDirWatch instance
    */
   static KDirWatch* self();

public Q_SLOTS:

   /**
    * Emits created().
    * @param path the path of the file or directory
    */
   void setCreated( const QString &path );

   /**
    * Emits dirty().
    * @param path the path of the file or directory
    */
   void setDirty( const QString &path );

   /**
    * Emits deleted().
    * @param path the path of the file or directory
    */
   void setDeleted( const QString &path );

 Q_SIGNALS:

   /**
    * Emitted when a watched object is changed.
    * For a directory this signal is emitted when files
    * therein are created or deleted.
    * For a file this signal is emitted when its size or attributes change.
    *
    * When you watch a directory, changes in the size or attributes of
    * contained files may or may not trigger this signal to be emitted
    * depending on which backend is used by KDirWatch.
    *
    * The new ctime is set before the signal is emitted.
    * @param path the path of the file or directory
    */
   void dirty(const QString &path);

   /**
    * Emitted when a file or directory is created.
    * @param path the path of the file or directory
    */
   void created(const QString &path);

   /**
    * Emitted when a file or directory is deleted.
    *
    * The object is still watched for new creation.
    * @param path the path of the file or directory
    */
   void deleted(const QString &path);

 private:

    KDirWatchPrivate* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDirWatch::WatchModes)

#endif

// vim: sw=3 et
