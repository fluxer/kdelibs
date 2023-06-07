/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "abstractrunner.h"

#include <QAction>
#include <QHash>
#include <QMenu>
#include <QMimeData>
#include <QMutex>
#include <QElapsedTimer>

#include <kdebug.h>
#include <kicon.h>
#include <kplugininfo.h>
#include <kstandarddirs.h>

#include <plasma/querymatch.h>

#include "private/abstractrunner_p.h"
#include "runnercontext.h"

namespace Plasma
{

AbstractRunner::AbstractRunner(const KService::Ptr service, QObject *parent)
    : QObject(parent),
      d(new AbstractRunnerPrivate(this))
{
    d->init(service);
}

AbstractRunner::AbstractRunner(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(new AbstractRunnerPrivate(this))
{
    if (args.count() > 0) {
        KService::Ptr service = KService::serviceByStorageId(args[0].toString());
        if (service) {
            d->init(service);
        }
    }
}

AbstractRunner::~AbstractRunner()
{
    delete d;
}

KConfigGroup AbstractRunner::config() const
{
    QString group = id();
    if (group.isEmpty()) {
        group = "UnnamedRunner";
    }

    KConfigGroup runners(KGlobal::config(), "Runners");
    return KConfigGroup(&runners, group);
}

void AbstractRunner::reloadConfiguration()
{
}

void AbstractRunner::addSyntax(const RunnerSyntax &syntax)
{
    d->syntaxes.append(syntax);
}

void AbstractRunner::setDefaultSyntax(const RunnerSyntax &syntax)
{
    d->syntaxes.append(syntax);
    d->defaultSyntax = &(d->syntaxes.last());
}

void AbstractRunner::setSyntaxes(const QList<RunnerSyntax> &syntaxes)
{
    d->syntaxes = syntaxes;
}

QList<RunnerSyntax> AbstractRunner::syntaxes() const
{
    return d->syntaxes;
}

RunnerSyntax *AbstractRunner::defaultSyntax() const
{
    return d->defaultSyntax;
}

void AbstractRunner::performMatch(Plasma::RunnerContext &localContext)
{
    static const int reasonableRunTime = 1500;
    static const int fastEnoughTime = 250;

    if (d->suspendMatching) {
        return;
    }

    QElapsedTimer time;
    time.restart();

    //The local copy is already obtained in the job
    match(localContext);

    // automatically rate limit runners that become slooow
    const qint64 runtime = time.elapsed();
    bool slowed = speed() == SlowSpeed;

    if (!slowed && runtime > reasonableRunTime) {
        // we punish runners that return too slowly, even if they don't bring
        // back matches
        kDebug() << id() << "runner is too slow, putting it on the back burner.";
        d->fastRuns = 0;
        setSpeed(SlowSpeed);
    }

    if (slowed && runtime < fastEnoughTime && localContext.query().size() > 2) {
        ++d->fastRuns;

        if (d->fastRuns > 2) {
            // we reward slowed runners who bring back matches fast enough
            // 3 times in a row
            kDebug() << id() << "runner is faster than we thought, kicking it up a notch";
            setSpeed(NormalSpeed);
        }
    }
}

QList<QAction*> AbstractRunner::actionsForMatch(const Plasma::QueryMatch &match)
{
    Q_UNUSED(match)
    QList<QAction*> ret;
    return ret;
}

QAction* AbstractRunner::addAction(const QString &id, const QIcon &icon, const QString &text)
{
    QAction *a = new QAction(icon, text, this);
    d->actions.insert(id, a);
    return a;
}

void AbstractRunner::addAction(const QString &id, QAction *action)
{
    d->actions.insert(id, action);
}

void AbstractRunner::removeAction(const QString &id)
{
    QAction *a = d->actions.take(id);
    delete a;
}

QAction* AbstractRunner::action(const QString &id) const
{
    return d->actions.value(id);
}

QHash<QString, QAction*> AbstractRunner::actions() const
{
    return d->actions;
}

void AbstractRunner::clearActions()
{
    qDeleteAll(d->actions);
    d->actions.clear();
}

QMimeData * AbstractRunner::mimeDataForMatch(const QueryMatch *match)
{
    Q_UNUSED(match)
    return 0;
}

bool AbstractRunner::hasRunOptions()
{
    return d->hasRunOptions;
}

void AbstractRunner::setHasRunOptions(bool hasRunOptions)
{
    d->hasRunOptions = hasRunOptions;
}

void AbstractRunner::createRunOptions(QWidget *parent)
{
    Q_UNUSED(parent)
}

AbstractRunner::Speed AbstractRunner::speed() const
{
    return d->speed;
}

void AbstractRunner::setSpeed(Speed speed)
{
    QMutexLocker locker(&d->speedMutex);
    d->speed = speed;
}

AbstractRunner::Priority AbstractRunner::priority() const
{
    return d->priority;
}

void AbstractRunner::setPriority(Priority priority)
{
    d->priority = priority;
}

RunnerContext::Types AbstractRunner::ignoredTypes() const
{
    return d->blackListed;
}

void AbstractRunner::setIgnoredTypes(RunnerContext::Types types)
{
    d->blackListed = types;
}

void AbstractRunner::run(const Plasma::RunnerContext &search, const Plasma::QueryMatch &action)
{
    Q_UNUSED(search)
    Q_UNUSED(action)
}

void AbstractRunner::match(Plasma::RunnerContext &search)
{
    Q_UNUSED(search)
}

QString AbstractRunner::name() const
{
    if (d->runnerDescription.isValid()) {
        return d->runnerDescription.name();
    }

    return objectName();
}

QIcon AbstractRunner::icon() const
{
    if (d->runnerDescription.isValid()) {
        return KIcon(d->runnerDescription.icon());
    }

    return QIcon();
}

QString AbstractRunner::id() const
{
    if (d->runnerDescription.isValid()) {
        return d->runnerDescription.pluginName();
    }

    return objectName();
}

QString AbstractRunner::description() const
{
    if (d->runnerDescription.isValid()) {
        return d->runnerDescription.property("Comment").toString();
    }

    return objectName();
}

void AbstractRunner::init()
{
    reloadConfiguration();
}

DataEngine *AbstractRunner::dataEngine(const QString &name) const
{
    return d->dataEngine(name);
}

bool AbstractRunner::isMatchingSuspended() const
{
    return d->suspendMatching;
}

void AbstractRunner::suspendMatching(bool suspend)
{
    if (d->suspendMatching == suspend) {
        return;
    }

    d->suspendMatching = suspend;
    emit matchingSuspended(suspend);
}

AbstractRunnerPrivate::AbstractRunnerPrivate(AbstractRunner *r)
    : priority(AbstractRunner::NormalPriority),
      speed(AbstractRunner::NormalSpeed),
      blackListed(0),
      runner(r),
      fastRuns(0),
      defaultSyntax(0),
      hasRunOptions(false),
      suspendMatching(false)
{
}

AbstractRunnerPrivate::~AbstractRunnerPrivate()
{
}

void AbstractRunnerPrivate::init(const KService::Ptr service)
{
    runnerDescription = KPluginInfo(service);
}

} // Plasma namespace

#include "moc_abstractrunner.cpp"
