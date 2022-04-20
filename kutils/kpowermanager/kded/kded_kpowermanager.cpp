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
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kpowermanager.h"

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
}

KPowerManagerModule::~KPowerManagerModule()
{
    m_powermanagerimpl->deleteLater();
    m_powermanagerinhibitimpl->deleteLater();
}

void KPowerManagerModule::slotPowerSaveStatusChanged(bool save_power)
{
    KConfig kconfig("kpowermanagerrc", KConfig::SimpleConfig);
    KConfigGroup kconfiggroup;
    QString defaultcpugovernor;
    int defaultscreenbrightness = 0;
    int defaultkeyboardbrightness = 0;
    if (save_power) {
        kconfiggroup = kconfig.group("PowerSave");
        defaultcpugovernor = QString::fromLatin1("powersave");
        defaultscreenbrightness = 70;
        defaultkeyboardbrightness = 70;
    } else {
        kconfiggroup = kconfig.group("External");
        defaultcpugovernor = QString::fromLatin1("performance");
        defaultscreenbrightness = 100;
        defaultkeyboardbrightness = 100;
    }
    const QString cpugovernor = kconfiggroup.readEntry("CPUGovernor", defaultcpugovernor);
    const int screenbrightness = kconfiggroup.readEntry("ScreenBrightness", defaultscreenbrightness);
    const int keyboardbrightness = kconfiggroup.readEntry("KeyboardBrightness", defaultkeyboardbrightness);
    KPowerManager kpowermanager;
    kpowermanager.setCPUGovernor(cpugovernor);
    kpowermanager.setScreenBrightness(screenbrightness);
    kpowermanager.setKeyboardBrightness(keyboardbrightness);
}

#include "moc_kded_kpowermanager.cpp"
