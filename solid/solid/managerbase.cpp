/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"
#include "config-solid.h"
#include "managerbase_p.h"

#include <stdlib.h>

#include "backends/exports/exportsmanager.h"

#if defined(ENABLE_TESTING)
#include "backends/fakehw/fakemanager.h"
#endif

#if defined (DEVINFO_FOUND)
#include "backends/devinfo/devinfomanager.h"
#endif

#if defined(UDEV_FOUND)
#include "backends/udev/udevmanager.h"
#endif

Solid::ManagerBasePrivate::ManagerBasePrivate()
{
}

Solid::ManagerBasePrivate::~ManagerBasePrivate()
{
    qDeleteAll(m_backends);
}

void Solid::ManagerBasePrivate::loadBackends()
{
#if defined(ENABLE_TESTING)
    QString solidFakeXml(QString::fromLocal8Bit(qgetenv("SOLID_FAKEHW")));
    if (!solidFakeXml.isEmpty()) {
        m_backends << new Solid::Backends::Fake::FakeManager(0, solidFakeXml);
        return;
    }
#endif

    m_backends << new Solid::Backends::Exports::ExportsManager(0);

#if defined(UDEV_FOUND)
    m_backends << new Solid::Backends::UDev::UDevManager(0);
#endif

#if defined(DEVINFO_FOUND)
    m_backends << new Solid::Backends::Devinfo::DevinfoManager(0);
#endif
}

QList<QObject*> Solid::ManagerBasePrivate::managerBackends() const
{
    return m_backends;
}


