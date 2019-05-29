/* This file is part of the KDE libraries
   Copyright (C) 1998 Sven Radej <sven@lisa.exp.univie.ac.at>
   Copyright (C) 2006 Dirk Mueller <mueller@kde.org>
   Copyright (C) 2007 Flavio Castelli <flavio.castelli@gmail.com>
   Copyright (C) 2008 Rafal Rzepecki <divided.mind@gmail.com>
   Copyright (C) 2010 David Faure <faure@kde.org>
   Copyright (C) 2019 Ivailo Monev <xakepa10@laimg.moc>

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

#include "kdirwatch.h"
#include "kdirwatch_p.h"

#include <kdebug.h>
#include <QStringList>
#include <QDir>

KDirWatchPrivate::KDirWatchPrivate()
    : watcher(new QFileSystemWatcher())
{
}

KDirWatchPrivate::~KDirWatchPrivate()
{
    watcher->deleteLater();
}

Q_GLOBAL_STATIC(KDirWatch, globalWatch)

KDirWatch* KDirWatch::self()
{
  return globalWatch();
}

KDirWatch::KDirWatch (QObject* parent)
  : QObject(parent), d(new KDirWatchPrivate())
{
    connect(d->watcher, SIGNAL(directoryChanged(QString)), this, SLOT(emitChanged(QString)));
    connect(d->watcher, SIGNAL(fileChanged(QString)), this, SLOT(emitChanged(QString)));
    connect(d->watcher, SIGNAL(fileChanged(QString)), this, SLOT(emitChanged(QString)));
    connect(d->watcher, SIGNAL(fileChanged(QString)), this, SLOT(emitChanged(QString)));
}

KDirWatch::~KDirWatch()
{
    disconnect(d->watcher, SIGNAL(directoryChanged(QString)), this, SLOT(emitChanged(QString)));
    disconnect(d->watcher, SIGNAL(fileChanged(QString)), this, SLOT(emitChanged(QString)));
    disconnect(d->watcher, SIGNAL(fileChanged(QString)), this, SLOT(emitChanged(QString)));
    disconnect(d->watcher, SIGNAL(fileChanged(QString)), this, SLOT(emitChanged(QString)));
    delete d;
}

void KDirWatch::addDir(const QString& path, WatchModes watchModes)
{
    if (path.isEmpty() || path.startsWith(QLatin1String("/dev")))
        return; // Don't even go there.
    if (watchModes & WatchDirOnly) {
        d->watcher->addPath(path);
        return;
    }
    QDir::Filters filters;
    if (watchModes & WatchFiles) {
        filters |= QDir::Files;
    }
    if (watchModes & WatchSubDirs) {
        filters |= QDir::Dirs;
    }
    QDir dir(path);
    d->watcher->addPaths(dir.entryList(filters));
}

void KDirWatch::addFile(const QString& path)
{
    if (path.isEmpty() || path.startsWith(QLatin1String("/dev")))
        return; // Don't even go there.
    d->watcher->addPath(path);
}

void KDirWatch::removeDir(const QString& path)
{
    d->watcher->removePath(path);
}

void KDirWatch::removeFile(const QString &path)
{
    d->watcher->removePath(path);
}

bool KDirWatch::contains(const QString &path) const
{
    return (d->watcher->files().contains(path) || d->watcher->directories().contains(path));
}

void KDirWatch::setCreated(const QString &file)
{
    kDebug(7001) << objectName() << "emitting created" << file;
    emit created(file);
}

void KDirWatch::setDirty(const QString &file)
{
    kDebug(7001) << objectName() << "emitting dirty" << file;
    emit dirty(file);
}

void KDirWatch::setDeleted(const QString &file)
{
    kDebug(7001) << objectName() << "emitting deleted" << file;
    emit deleted(file);
}

void KDirWatch::emitChanged(const QString &path)
{
    setDirty(path);
}

#include "moc_kdirwatch.cpp"
#include "moc_kdirwatch_p.cpp"

//sven
