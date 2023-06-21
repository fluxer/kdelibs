/*
   Copyright (C) 2000-2002 Stephan Kulow <coolo@kde.org>
   Copyright (C) 2000-2002 David Faure <faure@kde.org>
   Copyright (C) 2000-2002 Waldo Bastian <bastian@kde.org>

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

#ifndef __file_h__
#define __file_h__

#include <kio/global.h>
#include <kio/slavebase.h>

#include <QtCore/QObject>
#include <QtCore/QHash>

#include <sys/types.h>

class FileProtocol : public QObject, public KIO::SlaveBase
{
    Q_OBJECT
public:
    FileProtocol(const QByteArray &app);
    ~FileProtocol();

    void get(const KUrl &url) final;
    void put(const KUrl &url, int _mode, KIO::JobFlags _flags) final;
    void copy(const KUrl &src, const KUrl &dest, int mode, KIO::JobFlags flags) final;
    void rename(const KUrl &src, const KUrl &dest, KIO::JobFlags flags) final;
    void symlink(const QString &target, const KUrl &dest, KIO::JobFlags flags) final;

    void stat(const KUrl &url) final;
    void listDir(const KUrl &url) final;
    void mkdir(const KUrl &url, int permissions) final;
    void chmod(const KUrl &url, int permissions) final;
    void chown(const KUrl &url, const QString &owner, const QString &group) final;
    void setModificationTime(const KUrl &url, const QDateTime &mtime) final;
    void del(const KUrl &url, bool isfile) final;

private:
    bool createUDSEntry(const QString &filename, const QByteArray &path, KIO::UDSEntry &entry,
                        short int details, bool withACL);
    int setACL(const char *path, mode_t perm, bool _directoryDefault);

    QString getUserName(uid_t uid) const;
    QString getGroupName(gid_t gid) const;

    bool deleteRecursive(const QString &path);

private:
    mutable QHash<uid_t, QString> mUsercache;
    mutable QHash<gid_t, QString> mGroupcache;
};

#endif
