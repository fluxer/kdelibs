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

#include <QGraphicsWidget>
#include <QTimer>

#include <kdebug.h>

namespace Plasma
{

Service::Service(QObject *parent)
    : QObject(parent),
      d(new ServicePrivate(this))
{
}

Service::~Service()
{
    delete d;
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
    if (d->operationNames.isEmpty()) {
        kDebug() << "No operations are set";
    }
    return d->operationNames;
}

QMap<QString, QVariant> Service::operationParameters(const QString &operation)
{
    if (!d->operationNames.contains(operation)) {
        kDebug() << operation << "is not valid operations name";
    }
    // NOTE: default implementation returns nothing on purpose, here for future
    // expansion and binding the parameters type to variable in plasmoids easier
    return QMap<QString, QVariant>();
}

ServiceJob *Service::startOperationCall(const QString &operation, const QMap<QString, QVariant> &parameters, QObject *parent)
{
    ServiceJob *job = 0;
    if (!operation.isEmpty() && d->operationNames.contains(operation)) {
        if (d->disabledOperations.contains(operation)) {
            kDebug() << "Operation" << operation << "is disabled";
        } else {
            job = createJob(operation, parameters);
        }
    } else {
        kDebug() << operation << "is not valid, valid are:" << d->operationNames;
    }

    if (!job) {
        job = new NullServiceJob(destination(), operation, this);
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
    connect(
        widget, SIGNAL(destroyed(QObject*)),
        this, SLOT(associatedWidgetDestroyed(QObject*))
    );
    widget->setEnabled(!d->disabledOperations.contains(operation));
}

void Service::disassociateWidget(QWidget *widget)
{
    if (!widget) {
        return;
    }

    disconnect(
        widget, SIGNAL(destroyed(QObject*)),
        this, SLOT(associatedWidgetDestroyed(QObject*))
    );
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
    connect(
        widget, SIGNAL(destroyed(QObject*)),
        this, SLOT(associatedGraphicsWidgetDestroyed(QObject*))
    );
    widget->setEnabled(!d->disabledOperations.contains(operation));
}

void Service::disassociateItem(QGraphicsObject *widget)
{
    if (!widget) {
        return;
    }

    disconnect(
        widget, SIGNAL(destroyed(QObject*)),
        this, SLOT(associatedGraphicsWidgetDestroyed(QObject*))
    );
    d->associatedGraphicsWidgets.remove(widget);
}

QString Service::name() const
{
    return d->name;
}

void Service::setName(const QString &name)
{
    d->name = name;

    if (d->name.isEmpty()) {
        kDebug() << "Name is set to empty";
        emit serviceReady(this);
        return;
    }

    emit operationsChanged();

    {
        QHashIterator<QWidget *, QString> it(d->associatedWidgets);
        while (it.hasNext()) {
            it.next();
            it.key()->setEnabled(isOperationEnabled(it.value()));
        }
    }

    {
        QHashIterator<QGraphicsObject *, QString> it(d->associatedGraphicsWidgets);
        while (it.hasNext()) {
            it.next();
            it.key()->setEnabled(isOperationEnabled(it.value()));
        }
    }

    emit serviceReady(this);
}

void Service::setOperationNames(const QStringList &operations)
{
    d->operationNames = operations;

    if (d->operationNames.isEmpty()) {
        kDebug() << "Operation names is set to empty";
        return;
    }

    emit operationsChanged();
}

void Service::setOperationEnabled(const QString &operation, bool enable)
{
    if (!d->operationNames.contains(operation)) {
        kDebug() << operation << "is not valid operations name";
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
    return d->operationNames.contains(operation) && !d->disabledOperations.contains(operation);
}

} // namespace Plasma

#include "moc_service.cpp"

