/*
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "abstractdialogmanager.h"

#include "corona.h"

namespace Plasma
{

class AbstractDialogManagerPrivate
{
public:
    AbstractDialogManagerPrivate()
    {
    }

};

AbstractDialogManager::AbstractDialogManager(Corona *parent)
    : QObject(parent),
      d(new AbstractDialogManagerPrivate)
{
}

AbstractDialogManager::~AbstractDialogManager()
{
    delete d;
}

void AbstractDialogManager::showDialog(QWidget *widget, Plasma::Applet *applet)
{
    Q_UNUSED(applet)
    widget->show();
}

}

#include "moc_abstractdialogmanager.cpp"
