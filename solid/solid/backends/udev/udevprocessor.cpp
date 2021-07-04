/*
    Copyright 2010 Rafael Fernández López <ereslibre@kde.org>

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

#include "udevprocessor.h"
#include "udevdevice.h"
#include "cpuinfo.h"

#include <QtCore/QFile>

using namespace Solid::Backends::UDev;

Processor::Processor(UDevDevice *device)
    : DeviceInterface(device),
      m_canChangeFrequency(NotChecked),
      m_maxSpeed(-1)
{
}

Processor::~Processor()
{
}

int Processor::number() const
{
    // There's a subtle assumption here: suppose the system's ACPI
    // supports more processors/cores than are installed, and so udev reports
    // 4 cores when there are 2, say.  Will the processor numbers (in
    // /proc/cpuinfo, in particular) always match the sysfs device numbers?
    return m_device->deviceNumber();
}

// NOTE: do not parse /proc/cpuinfo for "cpu MHz", that may be current not maximum speed
int Processor::maxSpeed() const
{
    if (m_maxSpeed == -1) {
        QFile cpuMaxFreqFile(m_device->deviceName() + prefix() + "/cpufreq/cpuinfo_max_freq");
        if (cpuMaxFreqFile.open(QIODevice::ReadOnly)) {
            const QString value(cpuMaxFreqFile.readAll().trimmed());
            // cpuinfo_max_freq is in kHz
            m_maxSpeed = static_cast<int>(value.toLongLong() / 1000);
        }
    }
    return m_maxSpeed;
}

bool Processor::canChangeFrequency() const
{
    if (m_canChangeFrequency == NotChecked) {
        /* Note that cpufreq is the right information source here, rather than
         * anything to do with throttling (ACPI T-states).  */

        m_canChangeFrequency = CannotChangeFreq;

        QFile cpuMinFreqFile(m_device->deviceName() + prefix() + "/cpufreq/cpuinfo_min_freq");
        QFile cpuMaxFreqFile(m_device->deviceName() + prefix() + "/cpufreq/cpuinfo_max_freq");
        if (cpuMinFreqFile.open(QIODevice::ReadOnly) && cpuMaxFreqFile.open(QIODevice::ReadOnly)) {
            qlonglong minFreq = cpuMinFreqFile.readAll().trimmed().toLongLong();
            qlonglong maxFreq = cpuMaxFreqFile.readAll().trimmed().toLongLong();
            if (minFreq > 0 && maxFreq > minFreq) {
                m_canChangeFrequency = CanChangeFreq;
            }
        }
    }

    return m_canChangeFrequency == CanChangeFreq;
}

Solid::Processor::InstructionSets Processor::instructionSets() const
{
    static QStringList cpuflags = extractCpuInfoLine(m_device->deviceNumber(), "flags\\s+:\\s+(\\S.+)").split(" ");

    // for reference:
    // arch/x86/include/asm/cpufeatures.h
    // arch/powerpc/kernel/prom.c
    Solid::Processor::InstructionSets cpuinstructions = Solid::Processor::NoExtensions;
    if (cpuflags.contains("mmx")) {
        cpuinstructions |= Solid::Processor::IntelMmx;
    }
    if (cpuflags.contains("sse")) {
        cpuinstructions |= Solid::Processor::IntelSse;
    }
    if (cpuflags.contains("sse2")) {
        cpuinstructions |= Solid::Processor::IntelSse2;
    }
    if (cpuflags.contains("pni") || cpuflags.contains("ssse3")) {
        cpuinstructions |= Solid::Processor::IntelSse3;
    }
    if (cpuflags.contains("sse4") || cpuflags.contains("sse4_1") || cpuflags.contains("sse4_2")) {
        cpuinstructions |= Solid::Processor::IntelSse4;
    }
    if (cpuflags.contains("3dnow") || cpuflags.contains("3dnowext")) {
        cpuinstructions |= Solid::Processor::Amd3DNow;
    }
    if (cpuflags.contains("altivec")) {
        cpuinstructions |= Solid::Processor::AltiVec;
    }

    return cpuinstructions;
}

QString Processor::prefix() const
{
    const QLatin1String sysPrefix("/sysdev");
    if (QFile::exists(m_device->deviceName() + sysPrefix)) {
        return sysPrefix;
    }

    return QString();
}

#include "backends/udev/moc_udevprocessor.cpp"
