/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KPOWERMANAGERIMPL_H
#define KPOWERMANAGERIMPL_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QDBusInterface>
#include <QTimerEvent>

class KPowerManagerImpl : public QObject
{
    Q_OBJECT
public:
    KPowerManagerImpl(QObject *parent = nullptr);
    ~KPowerManagerImpl();

public Q_SLOTS:
    bool CanHibernate();
    bool CanHybridSuspend();
    bool CanSuspend();
    bool GetPowerSaveStatus();
    void Hibernate();
    void Suspend();

    // extension
    bool isLidClosed();

Q_SIGNALS:
    void CanHibernateChanged(bool can_hibernate);
    void CanHybridSuspendChanged(bool can_hybrid_suspend);
    void CanSuspendChanged(bool can_suspend);
    void PowerSaveStatusChanged(bool save_power);

    // extensions
    void BatteryRemainingTimeChanged(qulonglong time);
    void ResumeFromSuspend();

private Q_SLOTS:
    void slotPropertiesChanged(QString interface, QVariantMap changed_properties, QStringList invalidated_properties);
    void slotPrepareForSleep(bool start);

private:
    void timerEvent(QTimerEvent *event) final;

private:
    void emitSignals();

    bool m_objectregistered;
    bool m_serviceregistered;
    QDBusInterface m_login1;
    QDBusInterface m_consolekit;
    int m_consolekittimerid;
    bool m_canhibernate;
    bool m_canhybridsuspend;
    bool m_cansuspend;
    bool m_powersavestatus;
};

#endif // KPOWERMANAGERIMPL_H
