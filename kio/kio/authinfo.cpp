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

typedef QList<NetRC::AutoLogin> LoginList;
typedef QMap<QString, LoginList> LoginMap;

class NetRC::NetRCPrivate
{
public:
    NetRCPrivate()
        : isDirty(false)
    {
    }
    bool isDirty;
    LoginMap loginMap;
};

NetRC* NetRC::instance = nullptr;

NetRC::NetRC()
    : d(new NetRCPrivate())
{
}

NetRC::~NetRC()
{
    delete instance;
    instance = 0L;
    delete d;
}

NetRC* NetRC::self()
{
    if (!instance) {
        instance = new NetRC();
    }
    return instance;
}

bool NetRC::lookup(const KUrl &url, AutoLogin &login, bool userealnetrc,
                   const QString &_type, LookUpMode mode)
{
    // kDebug() << "AutoLogin lookup for: " << url.host();
    if (!url.isValid()) {
        return false;
    }

    QString type = _type;
    if (type.isEmpty()) {
        type = url.protocol();
    }

  if (d->loginMap.isEmpty() || d->isDirty) {
        d->loginMap.clear();

        QString filename = KStandardDirs::locateLocal("config", QLatin1String("kionetrc"));
        bool status = parse (openf(filename));

        if (userealnetrc) {
            filename =  QDir::homePath() + QLatin1String("/.netrc");
            status |= parse (openf(filename));
        }

        if (!status) {
            return false;
        }
  }

    if (!d->loginMap.contains(type)) {
        return false;
    }

    const LoginList &l = d->loginMap[type];
    if (l.isEmpty()) {
        return false;
    }

    for (LoginList::ConstIterator it = l.begin(); it != l.end(); ++it) {
        const AutoLogin &log = *it;

        if ((mode & defaultOnly) == defaultOnly &&
            log.machine == QLatin1String("default") &&
            (login.login.isEmpty() || login.login == log.login) )
        {
            login.type = log.type;
            login.machine = log.machine;
            login.login = log.login;
            login.password = log.password;
            login.macdef = log.macdef;
        }

        if ((mode & presetOnly) == presetOnly &&
            log.machine == QLatin1String("preset") &&
            (login.login.isEmpty() || login.login == log.login))
        {
            login.type = log.type;
            login.machine = log.machine;
            login.login = log.login;
            login.password = log.password;
            login.macdef = log.macdef;
        }

        if ((mode & exactOnly) == exactOnly &&
            log.machine == url.host() &&
            (login.login.isEmpty() || login.login == log.login))
        {
            login.type = log.type;
            login.machine = log.machine;
            login.login = log.login;
            login.password = log.password;
            login.macdef = log.macdef;
            break;
        }
    }

    return true;
}

void NetRC::reload()
{
    d->isDirty = true;
}

int NetRC::openf(const QString &f)
{
    KDE_struct_stat sbuff;
    if (KDE::stat(f, &sbuff) != 0) {
        return -1;
    }

    // Security check!!
    if (sbuff.st_mode != (S_IFREG|S_IRUSR|S_IWUSR) || sbuff.st_uid != geteuid()) {
        return -1;
    }

    return KDE::open(f, O_RDONLY);
}

QString NetRC::extract(const char* buf, const char* key, int &pos)
{
    int idx = pos;
    int m_len = qstrlen(key);
    int b_len = qstrlen(buf);

    while( idx < b_len ) {
        while( buf[idx] == ' ' || buf[idx] == '\t' ) {
            idx++;
        }

        if (strncasecmp(buf+idx, key, m_len) != 0) {
            idx++;
        } else {
            idx += m_len;
            while (buf[idx] == ' ' || buf[idx] == '\t') {
                idx++;
            }

            int start = idx;
            while(buf[idx] != ' ' && buf[idx] != '\t' &&
                buf[idx] != '\n' && buf[idx] != '\r') {
                idx++;
            }

            if (idx > start) {
                pos = idx;
                return QString::fromLatin1( buf+start, idx-start);
            }
        }
    }

    return QString();
}

bool NetRC::parse(int fd)
{
    if (fd == -1) {
        return false;
    }

    QString type;
    QString macro;

    uint index = 0;
    bool isMacro = false;
    char* buf = new char[NETRC_READ_BUF_SIZE];
    FILE* fstream = KDE_fdopen(fd,"rb" );

  while ( fgets (buf, NETRC_READ_BUF_SIZE, fstream) != 0L )
  {
        int pos = 0;

        while (buf[pos] == ' ' || buf[pos] == '\t') {
            pos++;
        }

        if ( buf[pos] == '#' || buf[pos] == '\n' ||
            buf[pos] == '\r' || buf[pos] == '\0' )
        {
            if ( buf[pos] != '#' && isMacro) {
                isMacro = false;
            }

            continue;
        }

        if (isMacro) {
            int tail = qstrlen(buf);
            while (buf[tail-1] == '\n' || buf[tail-1] =='\r') {
                tail--;
            }

            QString mac = QString::fromLatin1(buf, tail).trimmed();
            if (!mac.isEmpty()) {
                d->loginMap[type][index].macdef[macro].append(mac);
            }

            continue;
        }

        AutoLogin l;
        l.machine = extract(buf, "machine", pos);
        if (l.machine.isEmpty()) {
            if (strncasecmp(buf+pos, "default", 7) == 0) {
                pos += 7;
                l.machine = QLatin1String("default");
            } else if (strncasecmp(buf+pos, "preset", 6) == 0) {
                pos += 6;
                l.machine = QLatin1String("preset");
            }
        }
        // kDebug() << "Machine: " << l.machine;

        l.login = extract(buf, "login", pos);
        // kDebug() << "Login: " << l.login;

        l.password = extract(buf, "password", pos);
        if (l.password.isEmpty()) {
            l.password = extract(buf, "account", pos);
        }
        // kDebug() << "Password: " << l.password;

        type = l.type = extract(buf, "type", pos);
        if (l.type.isEmpty() && !l.machine.isEmpty()) {
            type = l.type = QLatin1String("ftp");
        }
        // kDebug() << "Type: " << l.type;

        macro = extract(buf, "macdef", pos);
        isMacro = !macro.isEmpty();
        // kDebug() << "Macro: " << macro;

        d->loginMap[l.type].append(l);
        index = d->loginMap[l.type].count()-1;
    }

    delete[] buf;
    fclose(fstream);
    close(fd);
    return true;
}
