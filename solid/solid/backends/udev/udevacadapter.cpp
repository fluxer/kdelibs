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

#include "udevacadapter.h"

using namespace Solid::Backends::UDev;

static const QStringList powerSupplySubSystems = QStringList() << QLatin1String("power_suply");

AcAdapter::AcAdapter(UDevDevice *device)
    : DeviceInterface(device),
    m_client(new UdevQt::Client(powerSupplySubSystems)),
    m_isPlugged(false)
{
    QObject::connect(m_client, SIGNAL(deviceChanged(UdevQt::Device)),
        this, SLOT(slotEmitSignals(UdevQt::Device)));
}

AcAdapter::~AcAdapter()
{
    m_client->deleteLater();
}

bool AcAdapter::isPlugged() const
{
    return (m_device->property("POWER_SUPPLY_ONLINE").toInt() == 1);
}

void AcAdapter::slotEmitSignals(const UdevQt::Device &device)
{
    if (device.sysfsPath() == m_device->deviceName()) {
        bool wasPlugged = m_isPlugged;
        m_isPlugged = isPlugged();
        if (wasPlugged != m_isPlugged) {
            emit plugStateChanged(m_isPlugged, m_device->udi());
        }
    }
}

#include "moc_udevacadapter.cpp"
