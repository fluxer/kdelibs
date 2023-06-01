/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "service.h"
#include "servicejob.h"
#include "private/service_p.h"

#include "config-plasma.h"

#include <QFile>
#include <QGraphicsWidget>
#include <QTimer>

#include <kdebug.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <ksharedconfig.h>
#include <kstandarddirs.h>

#include "configloader.h"
#include "private/configloader_p.h"
#include "pluginloader.h"

namespace Plasma
{

Service::Service(QObject *parent)
    : QObject(parent),
      d(new ServicePrivate(this))
{
}

Service::Service(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(new ServicePrivate(this))
{
    Q_UNUSED(args)
}

Service::~Service()
{
    delete d;
}

Service *Service::load(const QString &name, QObject *parent)
{
    QVariantList args;
    return load(name, args, parent);
}

Service *Service::load(const QString &name, const QVariantList &args, QObject *parent)
{
    return PluginLoader::pluginLoader()->loadService(name, args, parent);
}

void ServicePrivate::jobFinished(KJob *job)
{
    emit q->finished(static_cast<ServiceJob*>(job));
}

void ServicePrivate::associatedWidgetDestroyed(QObject *obj)
{
    associatedWidgets.remove(static_cast<QWidget*>(obj));
}

void ServicePrivate::associatedGraphicsWidgetDestroyed(QObject *obj)
{
    associatedGraphicsWidgets.remove(static_cast<QGraphicsObject*>(obj));
}

KConfigGroup ServicePrivate::dummyGroup()
{
    if (!dummyConfig) {
        dummyConfig = new KConfig(QString(), KConfig::SimpleConfig);
    }

    return KConfigGroup(dummyConfig, "DummyGroup");
}

void Service::setDestination(const QString &destination)
{
    d->destination = destination;
}

QString Service::destination() const
{
    return d->destination;
}

QStringList Service::operationNames() const
{
    if (!d->config) {
        kDebug() << "No valid operations scheme has been registered";
        return QStringList();
    }

    return d->config->groupList();
}

KConfigGroup Service::operationDescription(const QString &operationName)
{
    if (!d->config) {
        kDebug() << "No valid operations scheme has been registered";
        return d->dummyGroup();
    }

    d->config->writeConfig();
    KConfigGroup params(d->config->config(), operationName);
    //kDebug() << "operation" << operationName
    //         << "requested, has keys" << params.keyList() << "from"
    //         << d->config->config()->name();
    return params;
}

QMap<QString, QVariant> Service::parametersFromDescription(const KConfigGroup &description)
{
    QMap<QString, QVariant> params;

    if (!d->config || !description.isValid()) {
        return params;
    }

    const QString op = description.name();
    foreach (const QString &key, description.keyList()) {
        KConfigSkeletonItem *item = d->config->findItemByGroup(op, key);
        if (item) {
            params.insert(key, description.readEntry(key, item->property()));
        }
    }

    return params;
}

ServiceJob *Service::startOperationCall(const KConfigGroup &description, QObject *parent)
{
    // TODO: nested groups?
    ServiceJob *job = 0;
    const QString op = description.isValid() ? description.name() : QString();

    if (!d->config) {
        kDebug() << "No valid operations scheme has been registered";
    } else if (!op.isEmpty() && d->config->hasGroup(op)) {
        if (d->disabledOperations.contains(op)) {
            kDebug() << "Operation" << op << "is disabled";
        } else {
            job = createJob(op, parametersFromDescription(description));
        }
    } else {
        kDebug() << op << "is not a valid group; valid groups are:" << d->config->groupList();
    }

    if (!job) {
        job = new NullServiceJob(destination(), op, this);
    }

    job->setParent(parent ? parent : this);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(jobFinished(KJob*)));
    QTimer::singleShot(0, job, SLOT(autoStart()));
    return job;
}

void Service::associateWidget(QWidget *widget, const QString &operation)
{
    if (!widget) {
        return;
    }

    disassociateWidget(widget);
    d->associatedWidgets.insert(widget, operation);
    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(associatedWidgetDestroyed(QObject*)));

    widget->setEnabled(!d->disabledOperations.contains(operation));
}

void Service::disassociateWidget(QWidget *widget)
{
    if (!widget) {
        return;
    }

    disconnect(widget, SIGNAL(destroyed(QObject*)),
               this, SLOT(associatedWidgetDestroyed(QObject*)));
    d->associatedWidgets.remove(widget);
}

void Service::associateWidget(QGraphicsWidget *widget, const QString &operation)
{
    associateItem(widget, operation);
}

void Service::disassociateWidget(QGraphicsWidget *widget)
{
    disassociateItem(widget);
}

void Service::associateItem(QGraphicsObject *widget, const QString &operation)
{
    if (!widget) {
        return;
    }

    disassociateItem(widget);
    d->associatedGraphicsWidgets.insert(widget, operation);
    connect(widget, SIGNAL(destroyed(QObject*)),
            this, SLOT(associatedGraphicsWidgetDestroyed(QObject*)));

    widget->setEnabled(!d->disabledOperations.contains(operation));
}

void Service::disassociateItem(QGraphicsObject *widget)
{
    if (!widget) {
        return;
    }

    disconnect(widget, SIGNAL(destroyed(QObject*)),
               this, SLOT(associatedGraphicsWidgetDestroyed(QObject*)));
    d->associatedGraphicsWidgets.remove(widget);
}

QString Service::name() const
{
    return d->name;
}

void Service::setName(const QString &name)
{
    d->name = name;

    // now reset the config, which may be based on our name
    delete d->config;
    d->config = 0;

    delete d->dummyConfig;
    d->dummyConfig = 0;

    registerOperationsScheme();

    emit serviceReady(this);
}

void Service::setOperationEnabled(const QString &operation, bool enable)
{
    if (!d->config || !d->config->hasGroup(operation)) {
        return;
    }

    if (enable) {
        d->disabledOperations.remove(operation);
    } else {
        d->disabledOperations.insert(operation);
    }

    {
        QHashIterator<QWidget *, QString> it(d->associatedWidgets);
        while (it.hasNext()) {
            it.next();
            if (it.value() == operation) {
                it.key()->setEnabled(enable);
            }
        }
    }

    {
        QHashIterator<QGraphicsObject *, QString> it(d->associatedGraphicsWidgets);
        while (it.hasNext()) {
            it.next();
            if (it.value() == operation) {
                it.key()->setEnabled(enable);
            }
        }
    }
}

bool Service::isOperationEnabled(const QString &operation) const
{
    return d->config && d->config->hasGroup(operation) && !d->disabledOperations.contains(operation);
}

void Service::setOperationsScheme(QIODevice *xml)
{
    delete d->config;

    delete d->dummyConfig;
    d->dummyConfig = 0;

    KSharedConfigPtr c = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    d->config = new ConfigLoader(c, xml, this);
    d->config->d->setWriteDefaults(true);

    emit operationsChanged();

    {
        QHashIterator<QWidget *, QString> it(d->associatedWidgets);
        while (it.hasNext()) {
            it.next();
            it.key()->setEnabled(d->config->hasGroup(it.value()));
        }
    }

    {
        QHashIterator<QGraphicsObject *, QString> it(d->associatedGraphicsWidgets);
        while (it.hasNext()) {
            it.next();
            it.key()->setEnabled(d->config->hasGroup(it.value()));
        }
    }
}

void Service::registerOperationsScheme()
{
    if (d->config) {
        // we've already done our job. let's go home.
        return;
    }

    if (d->name.isEmpty()) {
        kDebug() << "No name found";
        return;
    }

    const QString path = KStandardDirs::locate("data", "plasma/services/" + d->name + ".operations");

    if (path.isEmpty()) {
        kDebug() << "Cannot find operations description:" << d->name << ".operations";
        return;
    }

    QFile file(path);
    setOperationsScheme(&file);
}

} // namespace Plasma

#include "moc_service.cpp"

