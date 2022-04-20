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

#include "kded_kpowermanager.h"
#include "kpluginfactory.h"
#include "kpowermanager.h"
#include "kdebug.h"

K_PLUGIN_FACTORY(KPowerManagerModuleFactory, registerPlugin<KPowerManagerModule>();)
K_EXPORT_PLUGIN(KPowerManagerModuleFactory("kpowermanager"))

KPowerManagerModule::KPowerManagerModule(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent),
    m_powermanagerimpl(nullptr),
    m_powermanagerinhibitimpl(nullptr)
{
    m_powermanagerimpl = new KPowerManagerImpl(this);
    connect(
        m_powermanagerimpl, SIGNAL(PowerSaveStatusChanged(bool)),
        this, SLOT(slotPowerSaveStatusChanged(bool))
    );

    m_powermanagerinhibitimpl = new KPowerManagerInhibitImpl(this);

    setPowerProfile(m_powermanagerimpl->GetPowerSaveStatus());
}

KPowerManagerModule::~KPowerManagerModule()
{
    m_powermanagerimpl->deleteLater();
    m_powermanagerinhibitimpl->deleteLater();
}

void KPowerManagerModule::slotPowerSaveStatusChanged(bool save_power)
{
    setPowerProfile(save_power);
}

void KPowerManagerModule::setPowerProfile(bool save_power)
{
    if (KPowerManager::isEnabled() && KPowerManager::isSupported()) {
        KPowerManager kpowermanager;
        if (save_power) {
            kpowermanager.setProfile(QString::fromLatin1("PowerSave"));
        } else {
            kpowermanager.setProfile(QString::fromLatin1("Performance"));
        }
    }
}

#include "moc_kded_kpowermanager.cpp"
