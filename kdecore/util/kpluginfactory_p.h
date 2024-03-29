/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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

#ifndef KDECORE_KPLUGINFACTORY_P_H
#define KDECORE_KPLUGINFACTORY_P_H

#include "kpluginfactory.h"

#include <QtCore/QHash>

#include <kaboutdata.h>

class KAboutData;

class KPluginFactoryPrivate
{
    Q_DECLARE_PUBLIC(KPluginFactory)
protected:
    typedef QPair<const QMetaObject *, KPluginFactory::CreateInstanceFunction> Plugin;

    QHash<QString, Plugin> createInstanceHash;
    KComponentData componentData;

    KPluginFactory *q_ptr;
};

#endif // KDECORE_KPLUGINFACTORY_P_H
