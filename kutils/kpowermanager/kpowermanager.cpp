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
#include "kauthorization.h"
#include "kstandarddirs.h"
#include "kdebug.h"

#include <QFile>
#include <QFileSystemWatcher>

// for reference:
// https://www.kernel.org/doc/Documentation/cpu-freq/governors.txt
static QString governorForProfile(const QString &profile, const QStringList &governors)
{
    const QString lowerprofile = profile.toLower();
    if (governors.contains(lowerprofile)) {
        // exact match
        return lowerprofile;
    }
    // governors that do the same essentially - preserve power one way or the other. because
    // governors are option some may not be aviable thus when the profile is "PowerSave" any
    // governor that will save (some) power is used if the powersave governor is not aviable
    if (lowerprofile == QLatin1String("powersave")) {
        if (governors.contains(QString::fromLatin1("conservative"))) {
            return QString::fromLatin1("conservative");
        }
        if (governors.contains(QString::fromLatin1("ondemand"))) {
            return QString::fromLatin1("ondemand");
        }
        if (governors.contains(QString::fromLatin1("schedutil"))) {
            return QString::fromLatin1("schedutil");
        }
    }
    // will trigger error
    return lowerprofile;
}

class KPowerManagerPrivate
 {
public:
    KPowerManagerPrivate();

    QString profile;
    QFileSystemWatcher kconfigwatch;
    QFileSystemWatcher cpuwatch;
};

KPowerManagerPrivate::KPowerManagerPrivate()
{
    kconfigwatch.addPath(KStandardDirs::locateLocal("config", "kpowermanagerrc"));
    cpuwatch.addPath("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
}

KPowerManager::KPowerManager(QObject *parent)
    : QObject(parent),
    d(new KPowerManagerPrivate())
{
    d->profile = profile();
    connect(&d->kconfigwatch, SIGNAL(fileChanged(QString)), this, SLOT(_configDirty(QString)));
    connect(&d->cpuwatch, SIGNAL(fileChanged(QString)), this, SLOT(_CPUGovernorDirty(QString)));
}

KPowerManager::~KPowerManager()
{
    delete d;
}

QString KPowerManager::profile() const
{
    KConfig kconfig("kpowermanagerrc", KConfig::SimpleConfig);
    KConfigGroup kconfiggeneral = kconfig.group("General");
    return kconfiggeneral.readEntry("Profile", QString::fromLatin1("Performance"));
}

QStringList KPowerManager::profiles() const
{
    static const QStringList result = QStringList()
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
    KConfigGroup kconfigprofile = kconfig.group(profile);
    const QString cpugovernor = kconfigprofile.readEntry("CPUGovernor", governorForProfile(profile, CPUGovernors()));
    kDebug() << "Power manager CPU governor" << cpugovernor;
    return setCPUGovernor(cpugovernor);
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

    QVariantMap helperargs;
    helperargs.insert("governor", governor);
    int helperreply = KAuthorization::execute(
        QString::fromLatin1("org.kde.kpowermanager.helper"),
        QString::fromLatin1("setgovernor"), helperargs
    );
    return (helperreply == KAuthorization::NoError);
}

bool KPowerManager::isSupported()
{
    KPowerManager kpowermanager;
    const QStringList cpugovernors = kpowermanager.CPUGovernors();
    bool result = true;
    KConfig kconfig("kpowermanagerrc", KConfig::SimpleConfig);
    foreach (const QString &profile, kpowermanager.profiles()) {
        KConfigGroup kconfigprofile = kconfig.group(profile);
        const QString cpugovernor = kconfigprofile.readEntry("CPUGovernor", governorForProfile(profile, cpugovernors));
        if (!cpugovernors.contains(cpugovernor)) {
            result = false;
            break;
        }
    }
    return result;
}

void KPowerManager::_configDirty(const QString &path)
{
    Q_UNUSED(path);
    const QString oldprofile = d->profile;
    d->profile = profile();
    if (oldprofile != d->profile) {
        emit profileChanged(d->profile);
    }
}

void KPowerManager::_CPUGovernorDirty(const QString &path)
{
    Q_UNUSED(path);
    emit CPUGovernorChanged(CPUGovernor());
}

#include "moc_kpowermanager.cpp"
