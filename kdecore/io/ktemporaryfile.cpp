/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/* This file is part of the KDE libraries
 *  Copyright 2006 Jaison Lee <lee.jaison@gmail.com>
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

#include "ktemporaryfile.h"
#include "kcomponentdata.h"
#include "kstandarddirs.h"
#include "krandom.h"

#include <QDir>

KTemporaryFile::KTemporaryFile(const KComponentData &componentData)
{
    setFileTemplate(
        KStandardDirs::locateLocal("tmp", componentData.componentName(), componentData) + QLatin1String("XXXXXX.tmp")
    );
}

void KTemporaryFile::setPrefix(const QString &prefix)
{
    QString oldTemplate = fileTemplate();
    QString suffix = oldTemplate.mid(oldTemplate.lastIndexOf(QLatin1String("XXXXXX"))+6);
    QString newPrefix = prefix;

    if ( newPrefix.isEmpty() ) {
        newPrefix = KGlobal::dirs()->saveLocation("tmp");
    } else {
        if ( !QDir::isAbsolutePath(newPrefix) ) {
            newPrefix.prepend(KGlobal::dirs()->saveLocation("tmp"));
        }
    }

    setFileTemplate(newPrefix + QLatin1String("XXXXXX") + suffix);
}

void KTemporaryFile::setSuffix(const QString &suffix)
{
    QString oldTemplate = fileTemplate();
    QString prefix = oldTemplate.left(oldTemplate.indexOf(QLatin1String("XXXXXX")));

    setFileTemplate(prefix + QLatin1String("XXXXXX") + suffix);
}

QString KTemporaryFile::filePath(const QString &pathtemplate)
{
    static QChar underscorechar = QChar::fromLatin1('_');
    static const char tmpnamechars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    if (pathtemplate.isEmpty()) {
        QString result = KGlobal::dirs()->saveLocation("tmp");
        result.append(KGlobal::mainComponent().componentName());
        result.append(underscorechar);
        result.reserve(result.size() + 10);
        for (ushort i = 0; i < 10; i++) {
            result.append(QChar::fromLatin1(tmpnamechars[KRandom::randomMax(52)]));
        }
        return result;
    }

    QString result = pathtemplate;
    int counter = result.size();
    while (counter) {
        counter--;
        if (result.at(counter) == QLatin1Char('/')) {
            break;
        } else if (result.at(counter) == QLatin1Char('X')) {
            result.replace(counter, 1, QChar::fromLatin1(tmpnamechars[KRandom::randomMax(52)]));
        }
    }
    if (!QDir::isAbsolutePath(result)) {
        result.prepend(underscorechar);
        result.prepend(KGlobal::mainComponent().componentName());
        result.prepend(KGlobal::dirs()->saveLocation("tmp"));
    }
    return result;
}
