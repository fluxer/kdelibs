/*
    Copyright 2021 Ivailo Monev <xakepa10@gmail.com>

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

#include "udevbattery.h"
#include <qmath.h>

using namespace Solid::Backends::UDev;

// for reference:
// linux/drivers/power/supply/power_supply_sysfs.c
// include/linux/power_supply.h

static const QStringList powerSupplySubSystems = QStringList() << QLatin1String("power_suply");

Battery::Battery(UDevDevice *device)
    : DeviceInterface(device),
    m_client(new UdevQt::Client(powerSupplySubSystems)),
    m_chargePercent(0),
    m_capacity(0),
    m_chargeState(Solid::Battery::NoCharge),
    m_isPowerSupply(false),
    m_isPlugged(false)
{
    m_chargePercent = chargePercent();
    m_capacity = capacity();
    m_chargeState = chargeState();
    m_isPlugged = isPlugged();
    m_isPowerSupply = isPowerSupply();

    QObject::connect(m_client, SIGNAL(deviceChanged(UdevQt::Device)),
        this, SLOT(slotEmitSignals(UdevQt::Device)));
}

Battery::~Battery()
{
    m_client->deleteLater();
}

bool Battery::isPlugged() const
{
    return (m_device->property("POWER_SUPPLY_ONLINE").toInt() == 1);
}

Solid::Battery::BatteryType Battery::type() const
{
    const QString powersupplytype = m_device->property("POWER_SUPPLY_TYPE").toString();
    // some of these are not even documented, wild-guessing
    if (powersupplytype == QLatin1String("battery") || powersupplytype == QLatin1String("mains")) {
        return Solid::Battery::PrimaryBattery;
    } else if (powersupplytype == QLatin1String("ups")) {
        return Solid::Battery::UpsBattery;
    } else if (powersupplytype == QLatin1String("usb_aca")) {
        // one of:
        // Solid::Battery::MouseBattery
        // Solid::Battery::KeyboardBattery
        // Solid::Battery::KeyboardMouseBattery
        // Solid::Battery::CameraBattery
        return Solid::Battery::KeyboardMouseBattery;
    } else if (powersupplytype == QLatin1String("usb_pd")) {
        return Solid::Battery::MonitorBattery;
    } else if (powersupplytype == QLatin1String("usb_pd_drp")) {
        // one of:
        // Solid::Battery::PdaBattery
        return Solid::Battery::PhoneBattery;
    }
    return Solid::Battery::UnknownBattery;
}

int Battery::chargePercent() const
{
    return m_device->property("POWER_SUPPLY_CAPACITY").toInt();
}

int Battery::capacity() const
{
    const int powersupplyvoltagemax = m_device->property("POWER_SUPPLY_VOLTAGE_MAX_DESIGN").toInt();
    const int powersupplyvoltagenow = m_device->property("POWER_SUPPLY_VOLTAGE_NOW").toInt();
    return (powersupplyvoltagemax / powersupplyvoltagenow);
}

bool Battery::isRechargeable() const
{
    const QString powersupplytechnology = m_device->property("POWER_SUPPLY_TECHNOLOGY").toString();
    if (powersupplytechnology == QLatin1String("NiMH")
        || powersupplytechnology == QLatin1String("Li-ion")
        || powersupplytechnology == QLatin1String("Li-poly")
        || powersupplytechnology == QLatin1String("LiFe")
        || powersupplytechnology == QLatin1String("NiCd")) {
        return true;
    }
    return false;
}

bool Battery::isPowerSupply() const
{
    return (m_device->property("POWER_SUPPLY_CURRENT_NOW").toInt() == 1);
}

Solid::Battery::ChargeState Battery::chargeState() const
{
    const QString powersupplystatus = m_device->property("POWER_SUPPLY_STATUS").toString();
    if (powersupplystatus == QLatin1String("charging")) {
        return Solid::Battery::Charging;
    } else if (powersupplystatus == QLatin1String("discharging")) {
        return Solid::Battery::Discharging;
    }

    return Solid::Battery::NoCharge; // stable or unknown
}

void Battery::slotEmitSignals(const UdevQt::Device &device)
{
    if (device.sysfsPath() == m_device->deviceName()) {
        const int previousChargePercent = m_chargePercent;
        m_chargePercent = chargePercent();
        if (previousChargePercent != m_chargePercent) {
            emit chargePercentChanged(m_chargePercent, m_device->udi());
        }

        const int previousCapacity = m_capacity;
        m_capacity = capacity();
        if (previousCapacity != m_capacity) {
            emit capacityChanged(m_capacity, m_device->udi());
        }

        const Solid::Battery::ChargeState previousChargeState = m_chargeState;
        m_chargeState = chargeState();
        if (previousChargeState != m_chargeState) {
            emit chargeStateChanged(m_chargeState, m_device->udi());
        }

        const bool previousPlugged = m_isPlugged;
        m_isPlugged = isPlugged();
        if (previousPlugged != m_isPlugged) {
            emit plugStateChanged(m_isPlugged, m_device->udi());
        }

        const bool previousPowerSupply = m_isPowerSupply;
        m_isPowerSupply = isPowerSupply();
        if (previousPowerSupply != m_isPowerSupply) {
            emit powerSupplyStateChanged(m_isPowerSupply, m_device->udi());
        }
    }
}
#include "moc_udevbattery.cpp"
