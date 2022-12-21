/*  This file is part of the KDE libraries
    Copyright (C) 2019 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KDIRWATCH_H
#define KDIRWATCH_H

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

#include <kdecore_export.h>

class KDirWatchPrivate;

 /**
  * @short Class for watching directory and file changes.
  *
  * The watched directories or files do not have to exist. When a watched
  * directory is changed, i.e. when files therein are created or deleted,
  * KDirWatch will emit the signal dirty(). Scanning begins immediately when a
  * dir or file watch is added.
  *
  * @see self()
  */
class KDECORE_EXPORT KDirWatch : public QObject
{
    Q_OBJECT
public:
   /**
    * Constructor.
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
    * @param path the path to watch
    * @param recurse add sub-directories aswell
    */
   void addDir(const QString &path, bool recurse = false);

   /**
    * Adds a file to be watched.
    * @param file the file to watch
    */
   void addFile(const QString &file);

   /**
    * Removes a directory from the list of scanned directories.
    * @param path the path of the dir to be removed from the list
    */
   void removeDir(const QString &path);

   /**
    * Removes a file from the list of watched files.
    * @param file the file to be removed from the list
    */
   void removeFile(const QString &file);

   /**
    * Check if a file or directory is being watched by this KDirWatch instance
    * @param path the file or directory to check
    * @return true if the path is being watched
    */
   bool contains(const QString &path) const;

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
    * Emits dirty().
    * @param path the path of the file or directory
    */
   void setDirty(const QString &path);

Q_SIGNALS:
   /**
    * Emitted when a watched object is changed.
    *
    * For a directory this signal is emitted when files therein are created or
    * deleted. For a file this signal is emitted when its size or attributes
    * change.
    *
    * @param path the path of the file or directory
    */
   void dirty(const QString &path);

private:
    KDirWatchPrivate* d;
};

#endif // KDIRWATCH_H
