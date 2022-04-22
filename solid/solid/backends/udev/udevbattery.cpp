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

using namespace Solid::Backends::UDev;

// for reference:
// https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-power
// linux/drivers/power/supply/power_supply_sysfs.c
// include/linux/power_supply.h

static const QStringList powersupplysubsystems = QStringList() << QLatin1String("power_supply");

Battery::Battery(UDevDevice *device)
    : DeviceInterface(device),
    m_client(new UdevQt::Client(powersupplysubsystems)),
    m_chargepercent(0),
    m_capacity(0),
    m_chargestate(Solid::Battery::UnknownCharge),
    m_ispowersupply(false),
    m_isplugged(false)
{
    m_chargepercent = chargePercent();
    m_capacity = capacity();
    m_chargestate = chargeState();
    m_ispowersupply = isPowerSupply();
    m_isplugged = isPlugged();

    QObject::connect(m_client, SIGNAL(deviceChanged(UdevQt::Device)),
        this, SLOT(slotEmitSignals(UdevQt::Device)));
}

Battery::~Battery()
{
    m_client->deleteLater();
}

bool Battery::isPlugged() const
{
    return (m_device->deviceProperty("POWER_SUPPLY_ONLINE").toInt() == 1);
}

Solid::Battery::BatteryType Battery::type() const
{
    const QString powersupplytype(m_device->deviceProperty("POWER_SUPPLY_TYPE").toLower());
    // some of these are not even documented, wild-guessing
    if (powersupplytype == QLatin1String("battery") || powersupplytype == QLatin1String("mains")) {
        return Solid::Battery::PrimaryBattery;
    } else if (powersupplytype == QLatin1String("ups")) {
        return Solid::Battery::UpsBattery;
    } else if (powersupplytype.contains(QLatin1String("usb"))) {
        return Solid::Battery::UsbBattery;
    }
    return Solid::Battery::UnknownBattery;
}

int Battery::chargePercent() const
{
    // yes, it is POWER_SUPPLY_CAPACITY
    return m_device->deviceProperty("POWER_SUPPLY_CAPACITY").toInt();
}

int Battery::capacity() const
{
    const int powersupplyvoltagemax = m_device->deviceProperty("POWER_SUPPLY_VOLTAGE_MAX_DESIGN").toInt();
    const int powersupplyvoltagenow = m_device->deviceProperty("POWER_SUPPLY_VOLTAGE_NOW").toInt();
    if (powersupplyvoltagemax <= 0 || powersupplyvoltagenow <= 0) {
        return 0;
    }
    return (powersupplyvoltagemax / powersupplyvoltagenow);
}

bool Battery::isRechargeable() const
{
    const QString powersupplytechnology(m_device->deviceProperty("POWER_SUPPLY_TECHNOLOGY"));
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
    return (chargeState() == Solid::Battery::Discharging);
}

Solid::Battery::ChargeState Battery::chargeState() const
{
    const QString powersupplystatus(m_device->deviceProperty("POWER_SUPPLY_STATUS").toLower());
    if (powersupplystatus == QLatin1String("charging")) {
        return Solid::Battery::Charging;
    } else if (powersupplystatus == QLatin1String("discharging")) {
        return Solid::Battery::Discharging;
    } else if (powersupplystatus == QLatin1String("full")) {
        return Solid::Battery::FullyCharged;
    }

    return Solid::Battery::UnknownCharge;
}

void Battery::slotEmitSignals(const UdevQt::Device &device)
{
    if (device.sysfsPath() == m_device->deviceName()) {
        const int previouschargepercent = m_chargepercent;
        m_chargepercent = chargePercent();
        if (previouschargepercent != m_chargepercent) {
            emit chargePercentChanged(m_chargepercent, m_device->udi());
        }

        const int previouscapacity = m_capacity;
        m_capacity = capacity();
        if (previouscapacity != m_capacity) {
            emit capacityChanged(m_capacity, m_device->udi());
        }

        const Solid::Battery::ChargeState previouschargestate = m_chargestate;
        m_chargestate = chargeState();
        if (previouschargestate != m_chargestate) {
            emit chargeStateChanged(m_chargestate, m_device->udi());
        }

        const bool previousispowersupply = m_ispowersupply;
        m_ispowersupply = isPowerSupply();
        if (previousispowersupply != m_ispowersupply) {
            emit powerSupplyStateChanged(m_ispowersupply, m_device->udi());
        }

        const bool previousplugged = m_isplugged;
        m_isplugged = isPlugged();
        if (previousplugged != m_isplugged) {
            emit plugStateChanged(m_isplugged, m_device->udi());
        }
    }
}

#include "moc_udevbattery.cpp"
