/* This file is part of the KDE libraries
    Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include "kautostart.h"

#include "kaboutdata.h"
#include "kglobal.h"
#include "kcomponentdata.h"
#include "kdesktopfile.h"
#include "kstandarddirs.h"
#include "kconfiggroup.h"

#include <QtCore/QFile>
#include <QStringList>

static const int s_defaultphase = static_cast<int>(KAutostart::Applications);

class KAutostart::Private
{
    public:
        Private()
            : df(0),
              copyIfNeededChecked(false)
        {
        }

        ~Private()
        {
            delete df;
        }

        void copyIfNeeded();

        QString name;
        KDesktopFile *df;
        bool copyIfNeededChecked;
};

void KAutostart::Private::copyIfNeeded()
{
    if (copyIfNeededChecked) {
        return;
    }

    const QString local = KGlobal::dirs()->locateLocal("autostart", name);

    if (!QFile::exists(local)) {
        const QString global = KGlobal::dirs()->locate("autostart", name);
        if (!global.isEmpty()) {
            KDesktopFile *newDf = df->copyTo(local); Q_UNUSED(newDf)
            delete df;
            delete newDf; //Force sync-to-disk
            df = new KDesktopFile("autostart", name); //Recreate from disk
        }
    }

    copyIfNeededChecked = true;
}

KAutostart::KAutostart(const QString& entryName, QObject* parent)
    : QObject(parent),
      d(new Private)
{
    if (entryName.isEmpty()) {
        d->name = KGlobal::mainComponent().aboutData()->appName();
    } else {
        d->name = entryName;
    }

    if (!d->name.endsWith(QLatin1String(".desktop"))) {
        d->name.append(QString::fromLatin1(".desktop"));
    }

    const QString path = KGlobal::dirs()->locate("autostart", d->name);
    if (path.isEmpty()) {
        // just a new KDesktopFile, since we have nothing to use
        d->df = new KDesktopFile("autostart", d->name);
        d->copyIfNeededChecked = true;
    } else {
        d->df = new KDesktopFile("autostart", path);
    }
}

KAutostart::~KAutostart()
{
    delete d;
}

void KAutostart::setAutostarts(bool autostart)
{
    bool currentAutostartState = !d->df->desktopGroup().readEntry("Hidden", false);
    if (currentAutostartState == autostart) {
        return;
    }

    d->copyIfNeeded();
    d->df->desktopGroup().writeEntry("Hidden", !autostart);
}

bool KAutostart::autostarts(const QString& environment, Conditions check) const
{
    // check if this is actually a .desktop file
    bool starts = d->df->desktopGroup().exists();

    // check the hidden field
    starts = starts && !d->df->desktopGroup().readEntry("Hidden", false);

    if (!environment.isEmpty()) {
        starts = starts && checkAllowedEnvironment(environment);
    }

    if (check & CheckCommand) {
        starts = starts && d->df->tryExec();
    }

    if (check & CheckCondition) {
        starts = starts && checkStartCondition();
    }

    return starts;
}

bool KAutostart::checkStartCondition() const
{
    QString condition = d->df->desktopGroup().readEntry("X-KDE-autostart-condition");
    if (condition.isEmpty())
        return true;

    const QStringList list = condition.split(QLatin1Char(':'));
    if (list.count() < 4) {
        return true;
    }

    if (list[0].isEmpty() || list[2].isEmpty()) {
        return true;
    }

    KConfig config(list[0], KConfig::NoGlobals);
    KConfigGroup cg(&config, list[1]);

    const bool defaultValue = (list[3].toLower() == QLatin1String("true"));
    return cg.readEntry(list[2], defaultValue);
}

bool KAutostart::checkAllowedEnvironment(const QString& environment) const
{
    const QStringList allowed = allowedEnvironments();
    if (!allowed.isEmpty()) {
        return allowed.contains(environment);
    }

    const QStringList excluded = excludedEnvironments();
    if (!excluded.isEmpty()) {
        return !excluded.contains( environment );
    }

    return true;
}

QString KAutostart::command() const
{
    return d->df->desktopGroup().readEntry("Exec", QString());
}

void KAutostart::setCommand(const QString &command)
{
    if (d->df->desktopGroup().readEntry("Exec", QString()) == command) {
        return;
    }

    d->copyIfNeeded();
    d->df->desktopGroup().writeEntry("Exec", command);
}

QString KAutostart::visibleName() const
{
    return d->df->readName();
}

void KAutostart::setVisibleName(const QString &name)
{
    if (d->df->desktopGroup().readEntry("Name", QString()) == name) {
        return;
    }

    d->copyIfNeeded();
    d->df->desktopGroup().writeEntry("Name", name);
}

bool KAutostart::isServiceRegistered(const QString& entryName)
{
    return !KStandardDirs::locate("autostart", entryName + QString::fromLatin1(".desktop")).isEmpty();
}

QString KAutostart::commandToCheck() const
{
    return d->df->desktopGroup().readPathEntry("TryExec", QString());
}

void KAutostart::setCommandToCheck(const QString &exec)
{
    if (d->df->desktopGroup().readEntry("TryExec", QString()) == exec) {
        return;
    }

    d->copyIfNeeded();
    d->df->desktopGroup().writePathEntry("TryExec", exec);
}

KAutostart::StartPhase KAutostart::startPhase() const
{
    return static_cast<KAutostart::StartPhase>(d->df->desktopGroup().readEntry("X-KDE-autostart-phase", s_defaultphase));
}

void KAutostart::setStartPhase(KAutostart::StartPhase phase)
{
    if (d->df->desktopGroup().readEntry("X-KDE-autostart-phase", s_defaultphase) == static_cast<int>(phase)) {
        return;
    }

    d->copyIfNeeded();
    d->df->desktopGroup().writeEntry("X-KDE-autostart-phase", static_cast<int>(phase));
}

QStringList KAutostart::allowedEnvironments() const
{
    return d->df->desktopGroup().readXdgListEntry("OnlyShowIn");
}

void KAutostart::setAllowedEnvironments(const QStringList& environments)
{
    if (d->df->desktopGroup().readEntry("OnlyShowIn", QStringList()) == environments) {
        return;
    }

    d->copyIfNeeded();
    d->df->desktopGroup().writeXdgListEntry("OnlyShowIn", environments);
}

void KAutostart::addToAllowedEnvironments(const QString& environment)
{
    QStringList envs = allowedEnvironments();

    if (envs.contains(environment)) {
        return;
    }

    envs.append(environment);
    setAllowedEnvironments(envs);
}

void KAutostart::removeFromAllowedEnvironments(const QString& environment)
{
    QStringList envs = allowedEnvironments();
    int index = envs.indexOf(environment);

    if (index < 0) {
        return;
    }

    envs.removeAt(index);
    setAllowedEnvironments(envs);
}

QStringList KAutostart::excludedEnvironments() const
{
    return d->df->desktopGroup().readXdgListEntry("NotShowIn");
}

void KAutostart::setExcludedEnvironments(const QStringList& environments)
{
    if (d->df->desktopGroup().readEntry("NotShowIn", QStringList()) == environments) {
        return;
    }

    d->copyIfNeeded();
    d->df->desktopGroup().writeXdgListEntry("NotShowIn", environments);
}

void KAutostart::addToExcludedEnvironments(const QString& environment)
{
    QStringList envs = excludedEnvironments();

    if (envs.contains(environment)) {
        return;
    }

    envs.append(environment);
    setExcludedEnvironments(envs);
}

void KAutostart::removeFromExcludedEnvironments(const QString& environment)
{
    QStringList envs = excludedEnvironments();
    int index = envs.indexOf(environment);

    if (index < 0) {
        return;
    }

    envs.removeAt(index);
    setExcludedEnvironments(envs);
}

#include "moc_kautostart.cpp"
