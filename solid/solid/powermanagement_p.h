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

#ifndef SOLID_POWERMANAGEMENT_P_H
#define SOLID_POWERMANAGEMENT_P_H

#include "powermanagement.h"

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusServiceWatcher>

namespace Solid
{
    class PowerManagementPrivate : public PowerManagement::Notifier
    {
        Q_OBJECT
    public:
        PowerManagementPrivate();
        ~PowerManagementPrivate();

    public Q_SLOTS:
        void slotCanSuspendChanged(bool newState);
        void slotCanHibernateChanged(bool newState);
        void slotCanHybridSuspendChanged(bool newState);
        void slotPowerSaveStatusChanged(bool newState);
        void slotResumeFromSuspend();
        void slotServiceRegistered(const QString &serviceName);
        void slotServiceUnregistered(const QString &serviceName);

    public:
        QDBusInterface* managerIface;
        QDBusInterface inhibitIface;
        QDBusInterface saverIface;
        QDBusServiceWatcher serviceWatcher;

        bool powerSaveStatus;
        QSet<Solid::PowerManagement::SleepState> supportedSleepStates;
        QList<uint> screensaverCookies;
    };
}

#endif
