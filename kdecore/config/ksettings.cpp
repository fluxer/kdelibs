/*
   This file is part of the KDE libraries
   Copyright (C) 2019 Ivailo Monev <xakepa10@gmail.com>

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

#include "ksettings.h"
#include "kstandarddirs.h"

#include <QApplication>
#include <QFile>

class KSettingsPrivate {
public:
    KSettings::OpenFlags m_mode;
};

static QString getSettingsPath(const QString &filename)
{
    if (filename.isEmpty()) {
        return KStandardDirs::locateLocal("config", QApplication::applicationName());
    } else if (QFile::exists(filename)) {
        return filename;
    }
    return KStandardDirs::locateLocal("config", filename);
}

KSettings::KSettings(const QString& file, const OpenFlags mode)
    : d_ptr(new KSettingsPrivate()), QSettings(getSettingsPath(file), QSettings::NativeFormat)
{
    d_ptr->m_mode = mode;
    if ((mode & IncludeGlobals) != mode) {
        addSource(KStandardDirs::locateLocal("config", QLatin1String("kdeglobals")));
    }
}

KSettings::~KSettings()
{
    delete d_ptr;
}

void KSettings::addSource(const QString &source)
{
    QSettings settings(source, QSettings::NativeFormat);
    foreach (const QString &key, settings.allKeys()) {
        if (!QSettings::contains(key)) {
            QSettings::setValue(key, settings.value(key));
        }
    }
}

