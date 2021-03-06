/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KSETTINGSWIDGETADAPTOR_H
#define KSETTINGSWIDGETADAPTOR_H

#include <QtCore/QObject>
#include <QString>

/*
 * Simple D-Bus object to return the KGlobal::caption()
 */
class KSettingsWidgetAdaptor: public QObject
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.internal.KSettingsWidget")
	public:
		KSettingsWidgetAdaptor(QObject *parent);

	public Q_SLOTS:
		QString applicationName();
};

#endif // KSETTINGSWIDGETADAPTOR_H
