/*
    This file is part of the KDE libraries

    Copyright (c) 2003,2007 Oswald Buddenhagen <ossi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kshell.h"
#include "kuser.h"
#include "kdebug.h"

#include <QtCore/QDir>

#include <stdlib.h>

namespace KShell {

QString homeDir( const QString &user )
{
    if (user.isEmpty())
        return QDir::homePath();
    return KUser(user).homeDir();
}

}

QString KShell::joinArgs( const QStringList &args )
{
    QString ret;
    for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it) {
        if (!ret.isEmpty())
            ret.append(QLatin1Char(' '));
        ret.append(quoteArg(*it));
    }
    return ret;
}

QString KShell::tildeExpand( const QString &fname )
{
    if (fname.length() && fname[0] == QLatin1Char('~')) {
        int pos = fname.indexOf( QLatin1Char('/') );
        if (pos < 0)
            return homeDir( fname.mid(1) );
        QString ret = homeDir( fname.mid(1, pos-1) );
        if (!ret.isNull())
            ret += fname.mid(pos);
        return ret;
    } else if (fname.length() > 1 && fname[0] == QLatin1Char('\\') && fname[1] == QLatin1Char('~')) {
        return fname.mid(1);
    }
    return fname;
}

static bool isVariableChar(const QChar &ch)
{
    const char c = ch.toLatin1();
    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_') {
        return true;
    }
    return false;
}

QString KShell::envExpand( const QString &fname )
{
    QString result = fname;
    int i = 0;
    while (i < result.size()) {
        if (result.at(i) == QLatin1Char('$')
            && result[i - 1] != QLatin1Char('\\')
            && result[i + 1] != QLatin1Char('(')) {
            int varstart = i + 1;
            int varend = varstart;
            int varlen = 0;
            while (varend < result.size()) {
                const QChar varchar = result.at(varend);
                if (varchar == QLatin1Char('{')) {
                    varstart++;
                    varend++;
                    varlen++;
                    continue;
                }
                if (varchar == QLatin1Char('}')) {
                    varlen++;
                    varend--;
                    break;
                }
                if (!isVariableChar(varchar)) {
                    break;
                }
                varlen++;
                varend++;
            }
            const QByteArray varname = result.mid(varstart, varend - i - 1).toLocal8Bit();
            const QString varvalue = QString::fromLocal8Bit(::getenv(varname.constData()));
            // kDebug() << "replacing" << varname << "with" << varvalue);
            result = result.replace(i, varlen + 1, varvalue);
            i = 0;
            continue;
        }
        i++;
    }
    return result;
}
