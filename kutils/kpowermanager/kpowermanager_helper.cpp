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

#include "kpowermanager_helper.h"
#include "kauthhelpersupport.h"

#include <QFile>
#include <QDir>

ActionReply KPowerManagerHelper::setgovernor(const QVariantMap &parameters)
{
    if (!parameters.contains("governor")) {
        return KAuth::ActionReply::HelperErrorReply;
    }

    const QByteArray governorbytes = parameters.value("governor").toByteArray();
    QDir cpudir("/sys/devices/system/cpu");
    foreach (const QFileInfo &cpuinfo, cpudir.entryInfoList()) {
        QFile cpufile(cpuinfo.filePath() + QLatin1String("/cpufreq/scaling_governor"));
        if (!cpufile.exists()) {
            continue;
        }
        if (!cpufile.open(QFile::WriteOnly)) {
            KAuth::ActionReply errorreply(KAuth::ActionReply::HelperError);
            errorreply.setErrorDescription(
                QString::fromLatin1("Could not open: %1 (%2)")
                    .arg(cpufile.fileName())
                    .arg(cpufile.errorString()
                )
            );
            errorreply.setErrorCode(1);
            return errorreply;
        }
        if (cpufile.write(governorbytes) != governorbytes.size()) {
            KAuth::ActionReply errorreply(KAuth::ActionReply::HelperError);
            errorreply.setErrorDescription(
                QString::fromLatin1("Could not write to: %1 (%2)")
                    .arg(cpufile.fileName())
                    .arg(cpufile.errorString()
                )
            );
            errorreply.setErrorCode(2);
            return errorreply;
        }
    }

    return KAuth::ActionReply::SuccessReply;
}

KDE4_AUTH_HELPER_MAIN("org.kde.kpowermanager.helper", KPowerManagerHelper)
