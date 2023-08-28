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

#include "kdirwatch.h"
#include "kglobal.h"
#include "kdirwatch_p.h"
#include "kde_file.h"

#include <kdebug.h>
#include <QDir>

KDirWatchPrivate::KDirWatchPrivate()
    : watcher(new QFileSystemWatcher())
{
}

KDirWatchPrivate::~KDirWatchPrivate()
{
    watcher->deleteLater();
}

K_GLOBAL_STATIC(KDirWatch, globalWatch)

KDirWatch* KDirWatch::self()
{
    return globalWatch;
}

KDirWatch::KDirWatch(QObject* parent)
    : QObject(parent),
    d(new KDirWatchPrivate())
{
    connect(d->watcher, SIGNAL(directoryChanged(QString)), this, SLOT(setDirty(QString)));
    connect(d->watcher, SIGNAL(fileChanged(QString)), this, SLOT(setDirty(QString)));
}

KDirWatch::~KDirWatch()
{
    disconnect(d->watcher, SIGNAL(directoryChanged(QString)), this, SLOT(setDirty(QString)));
    disconnect(d->watcher, SIGNAL(fileChanged(QString)), this, SLOT(setDirty(QString)));
    delete d;
}

void KDirWatch::addDir(const QString &path, bool recurse)
{
    if (path.isEmpty() || path.startsWith(QLatin1String("/dev"))) {
        return; // Don't even go there.
    } else if (d->watcher->directories().contains(path)) {
        return;
    }

    kDebug(7001) << "watching directory" << path;
    QString dirpath = path;
    // watching non-existing directory requires a trailing slash
    if (dirpath != QDir::rootPath() && !dirpath.endsWith(QDir::separator())) {
        dirpath.append(QDir::separator());
    }
    d->watcheddirs.append(dirpath);
    d->watcher->addPath(dirpath);

    if (recurse) {
        QDir dir(dirpath);
        foreach(const QFileInfo &info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs)) {
            if (info.isDir()) {
                addDir(info.absoluteFilePath(), recurse);
            }
        }
    }
}

void KDirWatch::addFile(const QString &path)
{
    if (path.isEmpty() || path.startsWith(QLatin1String("/dev"))) {
        return; // Don't even go there.
    } else if (d->watcher->files().contains(path)) {
        return;
    }

    if (QDir(path).exists()) {
        // trying to add dir as file, huh?
        addDir(path);
        return;
    }

    kDebug(7001) << "watching file" << path;
    d->watchedfiles.append(path);
    d->watcher->addPath(path);
}

void KDirWatch::removeDir(const QString &path)
{
    d->watcheddirs.removeAll(path);
    d->watcher->removePath(path);
}

void KDirWatch::removeFile(const QString &path)
{
    d->watchedfiles.removeAll(path);
    d->watcher->removePath(path);
}

bool KDirWatch::contains(const QString &path) const
{
    return (d->watcher->files().contains(path) || d->watcher->directories().contains(path));
}

int KDirWatch::interval() const
{
#if QT_VERSION < 0x041400
    // same as the default of Katie
    return 1000;
#else
    return d->watcher->interval();
#endif
}

void KDirWatch::setInterval(int interval)
{
#if QT_VERSION < 0x041400
    Q_UNUSED(interval);
#else
    d->watcher->setInterval(interval);
#endif
}

void KDirWatch::setDirty(const QString &file)
{
    kDebug(7001) << "emitting dirty" << file;

    // QFileSystemWatcher removes the file/dir from the watched list when it is deleted, put it
    // back so that events are emited in case it is created
    if (d->watcheddirs.contains(file)) {
        addDir(file);
    } else {
        addFile(file);
    }

    emit dirty(file);
}

#include "moc_kdirwatch.cpp"
