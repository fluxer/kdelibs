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
#include "kauthaction.h"
#include "kdebug.h"

#include <QFile>

class KPowerManagerPrivate
{
public:
    KPowerManagerPrivate();
};

KPowerManagerPrivate::KPowerManagerPrivate()
{
}

KPowerManager::KPowerManager(QObject *parent)
    : QObject(parent),
    d(new KPowerManagerPrivate())
{
}

KPowerManager::~KPowerManager()
{
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
    return (helperreply == KAuth::ActionReply::SuccessReply);
}

int KPowerManager::screenBrightness() const
{
    // TODO:
    return 100;
}

bool KPowerManager::setScreenBrightness(const int brightness)
{
    if (brightness < 0 || brightness > 100) {
        kWarning() << "Screen brightness value out of range" << brightness;
        return false;
    }
    // TODO:
    return false;
}

int KPowerManager::keyboardBrightness() const
{
    // TODO:
    return 100;
}

bool KPowerManager::setKeyboardBrightness(const int brightness)
{
    if (brightness < 0 || brightness > 100) {
        kWarning() << "Keyboard brightness value out of range" << brightness;
        return false;
    }
    // TODO:
    return false;
}

#include "moc_kpowermanager.cpp"
