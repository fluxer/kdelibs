/* This file is part of the KDE libraries
   Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KDBUSMENU_H
#define KDBUSMENU_H

#include "kiconloader.h"

#include <QString>
#include <QStringList>
#include <QDBusArgument>
#include <QAction>
#include <QImageWriter>

static const int s_kdbusmenuiconsize = KIconLoader::SizeSmall;
static const QByteArray s_kdbusmenuiconformat = QImageWriter::defaultImageFormat();
// see kdebug.areas
static const int s_kdbusmenuarea = 301;

struct KDBusMenu
{
    QString icon;
    QString title;
    QByteArray icondata;
};
Q_DECLARE_METATYPE(KDBusMenu);

QDBusArgument& operator<<(QDBusArgument &argument, const KDBusMenu &kdbusmenu);
const QDBusArgument& operator>>(const QDBusArgument &argument, KDBusMenu &kdbusmenu);

struct KDBusMenuAction
{
    quint64 id;
    QString text;
    QString icon;
    QString tooltip;
    QString statustip;
    QString whatsthis;
    bool separator;
    bool checkable;
    bool checked;
    bool enabled;
    bool visible;
    QStringList shortcuts;
    QByteArray icondata;
    bool title;
    bool exclusive;
};
Q_DECLARE_METATYPE(KDBusMenuAction);
Q_DECLARE_METATYPE(QList<KDBusMenuAction>);

QDBusArgument& operator<<(QDBusArgument &argument, const KDBusMenuAction &kdbusmenuaction);
const QDBusArgument& operator>>(const QDBusArgument &argument, KDBusMenuAction &kdbusmenuaction);

static quint64 kDBusMenuActionID(const QAction *action)
{
    return quint64(quintptr(action));
}

#endif // KDBUSMENU_H
