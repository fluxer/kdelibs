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

#include "kdbusmenu_p.h"

QDBusArgument& operator<<(QDBusArgument &argument, const KDBusMenu &kdbusmenu)
{
    argument.beginStructure();
    argument << kdbusmenu.icon;
    argument << kdbusmenu.title;
    argument << kdbusmenu.icondata;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument &argument, KDBusMenu &kdbusmenu)
{
    argument.beginStructure();
    argument >> kdbusmenu.icon;
    argument >> kdbusmenu.title;
    argument >> kdbusmenu.icondata;
    argument.endStructure();
    return argument;
}


QDBusArgument& operator<<(QDBusArgument &argument, const KDBusMenuAction &kdbusmenuaction)
{
    argument.beginStructure();
    argument << kdbusmenuaction.id;
    argument << kdbusmenuaction.text << kdbusmenuaction.icon;
    argument << kdbusmenuaction.tooltip << kdbusmenuaction.statustip << kdbusmenuaction.whatsthis;
    argument << kdbusmenuaction.separator << kdbusmenuaction.checkable << kdbusmenuaction.checked << kdbusmenuaction.enabled << kdbusmenuaction.visible;
    argument << kdbusmenuaction.shortcuts;
    argument << kdbusmenuaction.icondata;
    argument << kdbusmenuaction.title;
    argument << kdbusmenuaction.exclusive;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument &argument, KDBusMenuAction &kdbusmenuaction)
{
    argument.beginStructure();
    argument >> kdbusmenuaction.id;
    argument >> kdbusmenuaction.text >> kdbusmenuaction.icon;
    argument >> kdbusmenuaction.tooltip >> kdbusmenuaction.statustip >> kdbusmenuaction.whatsthis;
    argument >> kdbusmenuaction.separator >> kdbusmenuaction.checkable >> kdbusmenuaction.checked >> kdbusmenuaction.enabled >> kdbusmenuaction.visible;
    argument >> kdbusmenuaction.shortcuts;
    argument >> kdbusmenuaction.icondata;
    argument >> kdbusmenuaction.title;
    argument >> kdbusmenuaction.exclusive;
    argument.endStructure();
    return argument;
}
