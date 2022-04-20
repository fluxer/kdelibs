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

#include "kpowermanager.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kauthaction.h"
#include "kdebug.h"

#include <QFile>

KPowerManager::KPowerManager(QObject *parent)
    : QObject(parent)
{
}

QString KPowerManager::profile() const
{
    KConfig kconfig("kpowermanagerrc", KConfig::SimpleConfig);
    KConfigGroup kconfiggeneral = kconfig.group("General");
    return kconfiggeneral.readEntry("Profile", QString::fromLatin1("Performance"));
}

QStringList KPowerManager::profiles() const
{
    QStringList result = QStringList()
        << QString::fromLatin1("Performance")
        << QString::fromLatin1("PowerSave");
    return result;
}

bool KPowerManager::setProfile(const QString &profile)
{
    if (!profiles().contains(profile)) {
        kWarning() << "Invalid profile" << profile;
        return false;
    }

    KConfig kconfig("kpowermanagerrc", KConfig::SimpleConfig);
    KConfigGroup kconfiggeneral = kconfig.group("General");
    const bool enable = kconfiggeneral.readEntry("Enable", true);
    if (enable) {
        KConfigGroup kconfigprofile = kconfig.group(profile);
        QString defaultcpugovernor;
        if (profile == QLatin1String("Performance")) {
            defaultcpugovernor = QString::fromLatin1("performance");
        } else {
            defaultcpugovernor = QString::fromLatin1("powersave");
        }
        const QString cpugovernor = kconfigprofile.readEntry("CPUGovernor", defaultcpugovernor);
        kDebug() << "Power manager CPU governor" << cpugovernor;
        const bool result = setCPUGovernor(cpugovernor);
        if (result) {
            kconfiggeneral.writeEntry("Profile", profile);
            emit profileChanged(profile);
        }
        return result;
    }
    kDebug() << "Power manager disabled";
    return true;
}

QString KPowerManager::CPUGovernor() const
{
    QString result;
    // this assumes all CPU devices use the same governor
    QFile governorsfile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    if (!governorsfile.open(QFile::ReadOnly)) {
        kDebug() << "Could not open CPU governor file";
        return result;
    }
    const QByteArray trimmedgovernor = governorsfile.readAll().trimmed();
    result = QString::fromLatin1(trimmedgovernor.constData(), trimmedgovernor.size());
    return result;
}

QStringList KPowerManager::CPUGovernors() const
{
    QStringList result;
    QFile governorsfile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors");
    if (!governorsfile.open(QFile::ReadOnly)) {
        kDebug() << "Could not open CPU governors file";
        return result;
    }
    const QByteArray governorsdata = governorsfile.readAll();
    foreach (const QByteArray &governor, governorsdata.split(' ')) {
        const QByteArray trimmedgovernor = governor.trimmed();
        if (trimmedgovernor.isEmpty()) {
            continue;
        }
        result.append(QString::fromLatin1(trimmedgovernor.constData(), trimmedgovernor.size()));
    }
    return result;
}

bool KPowerManager::setCPUGovernor(const QString &governor)
{
    if (!CPUGovernors().contains(governor)) {
        kWarning() << "Invalid CPU governor" << governor;
        return false;
    }

    KAuth::Action helperaction("org.kde.kpowermanager.helper.setgovernor");
    helperaction.setHelperID("org.kde.kpowermanager.helper");
    helperaction.addArgument("governor", governor);
    KAuth::ActionReply helperreply = helperaction.execute();
    // qDebug() << helperreply.errorCode() << helperreply.errorDescription();
    const bool result = (helperreply == KAuth::ActionReply::SuccessReply);
    if (result) {
        emit CPUGovernorChanged(governor);
    }
    return result;
}

#include "moc_kpowermanager.cpp"
