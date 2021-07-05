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

#include "devinfoprocessor.h"
#include "devinfodevice.h"

#include <QtCore/QDebug>

using namespace Solid::Backends::Devinfo;

// for reference:
// freebsd-src/usr.sbin/powerd/powerd.c

Processor::Processor(DevinfoDevice *device)
    : DeviceInterface(device)
{
}

Processor::~Processor()
{
}

int Processor::number() const
{
    return m_device->deviceProperty(DevinfoDevice::DeviceName).right(1).toInt();
}

int Processor::maxSpeed() const
{
    // TODO: parse freq_levels instead
    return m_device->deviceCtl("freq").toInt();
}

bool Processor::canChangeFrequency() const
{
    return !m_device->deviceCtl("freq").isEmpty();
}

Solid::Processor::InstructionSets Processor::instructionSets() const
{
    // TODO:
    Solid::Processor::InstructionSets cpuinstructions = Solid::Processor::NoExtensions;
    return cpuinstructions;
}

#include "backends/devinfo/moc_devinfoprocessor.cpp"
