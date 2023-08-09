/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2001 Dawit Alemayehu <adawit@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */

#include "authinfo.h"

#include <config.h>

#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <kde_file.h>

#include <kdebug.h>
#include <kstandarddirs.h>

#define NETRC_READ_BUF_SIZE 4096

using namespace KIO;

//////

class KIO::AuthInfoPrivate  
{
public:
    QMap<QString, QVariant> extraFields;
};


//////

AuthInfo::AuthInfo()
    : d(new AuthInfoPrivate())
{
    readOnly = false;
    keepPassword = false;
}

AuthInfo::AuthInfo(const AuthInfo &info)
    : d(new AuthInfoPrivate())
{
    (*this) = info;
}

AuthInfo::~AuthInfo()
{
    delete d;
}

AuthInfo& AuthInfo::operator=(const AuthInfo &info)
{
    url = info.url;
    username = info.username;
    password = info.password;
    prompt = info.prompt;
    caption = info.caption;
    comment = info.comment;
    commentLabel = info.commentLabel;
    readOnly = info.readOnly;
    keepPassword = info.keepPassword;
    d->extraFields = info.d->extraFields;
    return *this;
}

/////

void AuthInfo::setExtraField(const QString &fieldName, const QVariant &value)
{
    d->extraFields[fieldName] = value;
}

QVariant AuthInfo::getExtraField(const QString &fieldName) const
{
    if (!d->extraFields.contains(fieldName)) {
        return QVariant();
    }
    return d->extraFields[fieldName];
}

/////

QDataStream& KIO::operator<<(QDataStream &s, const AuthInfo &a)
{
    s << a.url << a.username << a.password << a.prompt << a.caption
      << a.comment << a.commentLabel << a.readOnly << a.keepPassword
      << a.d->extraFields;
    return s;
}

QDataStream& KIO::operator>>(QDataStream &s, AuthInfo &a)
{
    s >> a.url >> a.username >> a.password >> a.prompt >> a.caption
      >> a.comment >> a.commentLabel >> a.readOnly >> a.keepPassword
      >> a.d->extraFields;
    return s;
}
