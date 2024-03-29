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
#include "kdebug.h"

#include <QFileInfo>
#include <QStringList>
#include <QMetaProperty>
#include <QWidget>

static QString getSettingsPath(const QString &filename)
{
    const QFileInfo info(filename);
    if (info.isAbsolute()) {
        return filename;
    }
    return KStandardDirs::locateLocal("config", filename);
}

KSettings::KSettings(const QString& file, const OpenFlags mode)
    : QSettings(getSettingsPath(file))
{
    if ((mode & IncludeGlobals) != mode) {
        addSource(KStandardDirs::locateLocal("config", QLatin1String("kdeglobals")));
    }
}

void KSettings::addSource(const QString &source)
{
    QSettings settings(source);
    foreach (const QString &key, settings.keys()) {
        if (!QSettings::contains(key)) {
            QSettings::setString(key, settings.string(key));
        }
    }
}


bool KSettings::save(const QObject *object)
{
    const QWidget* widget = qobject_cast<const QWidget*>(object);
    if (!widget) {
        kWarning() << "object is not widget";
        return false;
    }

    const QString objectname = widget->objectName();
    if (objectname.isEmpty()) {
        kWarning() << "object name of widget is empty";
        return false;
    }

    QSettings::sync();

    const QMetaObject* metaobject = widget->metaObject();
    for (int i = 0; i < metaobject->propertyCount(); i++) {
        const QMetaProperty property = metaobject->property(i);
        const QString propertyname = QString::fromLatin1(property.name());
        if (!property.isWritable()) {
            kDebug() << "skipping non-writable property" << propertyname << "of" << objectname;
            continue;
        }
        const QVariant propertyvalue = property.read(widget);
        if (!propertyvalue.canConvert<QString>()) {
            kWarning() << "property value not QString-convertable" << propertyname << propertyvalue;
        } else if (!propertyvalue.isValid()) {
            kWarning() << "invalid property value" << propertyname << propertyvalue;
        } else {
            kDebug() << "saving property" << propertyname << "with value" << propertyvalue << "of" << objectname;
            QSettings::setString(objectname + QLatin1Char('/') + propertyname, propertyvalue.toString());
        }
    }

    QSettings::sync();

    return (QSettings::status() == QSettings::NoError);
}

bool KSettings::restore(QObject *object)
{
    QWidget* widget = qobject_cast<QWidget*>(object);
    if (!widget) {
        kWarning() << "object is not widget";
        return false;
    }

    const QString objectname = widget->objectName();
    if (objectname.isEmpty()) {
        kWarning() << "object name of widget is empty";
        return false;
    }

    QSettings::sync();

    const QMetaObject* metaobject = widget->metaObject();
    for (int i = 0; i < metaobject->propertyCount(); i++) {
        const QMetaProperty property = metaobject->property(i);
        const QString propertyname = QString::fromLatin1(property.name());
        if (!property.isWritable()) {
            kDebug() << "skipping non-writable property" << propertyname << "of" << objectname;
            continue;
        }
        const QVariant propertyvalue = QSettings::string(objectname + QLatin1Char('/') + propertyname, property.read(widget).toString());
        if (propertyvalue.isValid()) {
            kDebug() << "restoring property" << propertyname << "with value" << propertyvalue << "of" << objectname;
            bool success = false;
            if (property.isEnumType() || property.isFlagType()) {
                success = property.write(widget, propertyvalue.toInt());
            } else {
                success = property.write(widget, propertyvalue);
            }
            if (!success) {
                kWarning() << "could not set property" << propertyname << propertyvalue;
            }
        } else {
            kWarning() << "invalid property value" << propertyname << propertyvalue;
        }
    }

    QSettings::sync();

    return (QSettings::status() == QSettings::NoError);
}
