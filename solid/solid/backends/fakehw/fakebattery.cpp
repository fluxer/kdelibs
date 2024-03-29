/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>

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

#include "fakebattery.h"

using namespace Solid::Backends::Fake;

FakeBattery::FakeBattery(FakeDevice *device)
    : FakeDeviceInterface(device)
{
}

FakeBattery::~FakeBattery()
{
}


bool FakeBattery::isPlugged() const
{
    return fakeDevice()->property("isPlugged").toBool();
}

Solid::Battery::BatteryType FakeBattery::type() const
{
    QString name = fakeDevice()->property("batteryType").toString();

    if (name == "primary")
    {
        return Solid::Battery::PrimaryBattery;
    }
    else if (name == "ups")
    {
        return Solid::Battery::UpsBattery;
    }
    else if (name == "usb")
    {
        return Solid::Battery::UsbBattery;
    }
    else
    {
        return Solid::Battery::UnknownBattery;
    }
}

int FakeBattery::chargePercent() const
{
    int last_full = fakeDevice()->property("lastFullLevel").toInt();
    int current = fakeDevice()->property("currentLevel").toInt();

    int percent = (100 * current) / last_full;

    return percent;
}

int FakeBattery::capacity() const
{
    int last_full = fakeDevice()->property("lastFullLevel").toInt();
    int max = fakeDevice()->property("maxLevel").toInt();

    int percent = (100 * last_full) / max;

    return percent;
}

bool FakeBattery::isRechargeable() const
{
    return fakeDevice()->property("isRechargeable").toBool();
}

bool FakeBattery::isPowerSupply() const
{
    return fakeDevice()->property("isPowerSupply").toBool();
}

Solid::Battery::ChargeState FakeBattery::chargeState() const
{
    QString state = fakeDevice()->property("chargeState").toString();

    if (state == "charging")
    {
        return Solid::Battery::Charging;
    }
    else if (state == "discharging")
    {
        return Solid::Battery::Discharging;
    }
    else if (state == "full")
    {
        return Solid::Battery::FullyCharged;
    }
    else
    {
        return Solid::Battery::UnknownCharge;
    }
}

void FakeBattery::setChargeState(Solid::Battery::ChargeState newState)
{
    QString name;

    switch(newState)
    {
    case Solid::Battery::Charging:
        name = "charging";
        break;
    case Solid::Battery::Discharging:
        name = "discharging";
        break;
    case Solid::Battery::FullyCharged:
        name = "full";
        break;
    case Solid::Battery::UnknownCharge:
        name = "unknown";
        break;
    }

    fakeDevice()->setProperty("chargeState", name);
    emit chargeStateChanged(newState, fakeDevice()->udi());
}

void FakeBattery::setChargeLevel(int newLevel)
{
    fakeDevice()->setProperty("currentLevel", newLevel);
    emit chargePercentChanged(chargePercent(), fakeDevice()->udi());
}

#include "backends/fakehw/moc_fakebattery.cpp"
