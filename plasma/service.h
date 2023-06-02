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

#ifndef PLASMA_SERVICE_H
#define PLASMA_SERVICE_H

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <kconfiggroup.h>

#include <plasma/plasma_export.h>
#include <plasma/plasma.h>
#include "packagemetadata.h"

#include <QGraphicsObject>
#include <QIODevice>
#include <QWidget>

namespace Plasma
{

class ServiceJob;
class ServicePrivate;

/**
 * @class Service plasma/service.h <Plasma/Service>
 *
 * @short This class provides a generic API for write access services.
 *
 * Plasma::Service allows interaction with a "destination", the definition of which
 * depends on the Service itself. For a network settings Service this might be a
 * profile name ("Home", "Office", "Road Warrior") while a web based Service this
 * might be a username ("aseigo", "stranger65").
 *
 * A Service provides one or more operations, each of which provides some sort
 * of interaction with the destination. Operations are set by the service itself
 * and their availability can be changed at any time.
 *
 * A service is started with a QVariantMap representing the parameters, after
 * completion a signal is emitted. The service job is automatically deleted,
 * see KJob for more information on this part of the process.
 *
 * Services may be loaded from a DataEngine by passing in a source name to be used
 * as the destination.
 *
 * Sample use might look like:
 *
 * @code
 * Plasma::DataEngine *twitter = dataEngine("twitter");
 * Plasma::Service *service = twitter.serviceForSource("aseigo");
 * QVariantMap args = service->operationParameters("update");
 * args["tweet"] = "Hacking on plasma!";
 * Plasma::ServiceJob *job = service->startOperationCall("update", args);
 * connect(job, SIGNAL(finished(KJob*)), this, SLOT(jobCompeted()));
 * @endcode
 *
 * Please remember, the service needs to be deleted when it will no longer be
 * used. This can be done manually or by these (perhaps easier) alternatives:
 *
 * If it is needed throughout the lifetime of the object:
 * @code
 * service->setParent(this);
 * @endcode
 *
 * If the service will not be used after just one operation call, use:
 * @code
 * connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
 * @endcode
 *
 */
class PLASMA_EXPORT Service : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Service)
    Q_PROPERTY(QString destination READ destination WRITE setDestination)
    Q_PROPERTY(QStringList operationNames READ operationNames)
    Q_PROPERTY(QString name READ name)

public:
    /**
     * Destructor
     */
    ~Service();

    /**
     * Sets the destination for this Service to operate on
     *
     * @param destination specific to each Service, this sets which
     *                  target or address for ServiceJobs to operate on
     */
    Q_INVOKABLE void setDestination(const QString &destination);

    /**
     * @return the target destination, if any, that this service is associated with
     */
    Q_INVOKABLE QString destination() const;

    /**
     * @return the possible operations for this Service
     */
    Q_INVOKABLE QStringList operationNames() const;

    /**
     * Retrieves the parameters for a given operation
     *
     * @param operation the operation to retrieve parameters for
     * @return QVariantMap containing the default parameters
     */
    Q_INVOKABLE QMap<QString, QVariant> operationParameters(const QString &operation);

    /**
     * Called to create a ServiceJob which is associated with a given
     * operation and parameter set.
     *
     * @return a started ServiceJob; the consumer may connect to relevant
     *         signals before returning to the event loop
     */
    Q_INVOKABLE ServiceJob *startOperationCall(const QString &operation, const QMap<QString, QVariant> &parameters,
                                               QObject *parent = 0);

    /**
     * Query to find if an operation is enabled or not.
     *
     * @param operation the name of the operation to check
     * @return true if the operation is enabled, false otherwise
     */
    Q_INVOKABLE bool isOperationEnabled(const QString &operation) const;

    /**
     * The name of this service
     */
    Q_INVOKABLE QString name() const;

    /**
     * Assoicates a widget with an operation, which allows the service to
     * automatically manage, for example, the enabled state of a widget.
     *
     * This will remove any previous associations the widget had with
     * operations on this engine.
     *
     * @param widget the QWidget to associate with the service
     * @param operation the operation to associate the widget with
     */
    Q_INVOKABLE void associateWidget(QWidget *widget, const QString &operation);

    /**
     * Disassociates a widget if it has been associated with an operation
     * on this service.
     *
     * This will not change the enabled state of the widget.
     *
     * @param widget the QWidget to disassociate.
     */
    Q_INVOKABLE void disassociateWidget(QWidget *widget);

    /**
     * This method only exists to maintain binary compatibility.
     *
     * @see associateItem
     */
    Q_INVOKABLE void associateWidget(QGraphicsWidget *widget, const QString &operation);

    /**
     * This method only exists to maintain binary compatibility.
     *
     * @see disassociateItem
     */
    Q_INVOKABLE void disassociateWidget(QGraphicsWidget *widget);

    /**
     * Associates a graphics item with an operation, which allows the service to
     * automatically manage, for example, the enabled state of the item.
     *
     * This will remove any previous associations the item had with
     * operations on this engine.
     *
     * @param item the QGraphicsObject to associate with the service
     * @param operation the operation to associate the item with
     */
    Q_INVOKABLE void associateItem(QGraphicsObject *item, const QString &operation);

    /**
     * Disassociates a graphics item if it has been associated with an operation
     * on this service.
     *
     * This will not change the enabled state of the item.
     *
     * @param widget the QGraphicsItem to disassociate.
     */
    Q_INVOKABLE void disassociateItem(QGraphicsObject *widget);

Q_SIGNALS:
    /**
     * Emitted when a job associated with this Service completes its task
     */
    void finished(Plasma::ServiceJob *job);

    /**
     * Emitted when the Service's operations change. For example, a
     * media player service may change what operations are available
     * in response to the state of the player.
     */
    void operationsChanged();

    /**
     * Emitted when this service is ready for use
     */
    void serviceReady(Plasma::Service *service);

protected:
    /**
     * Default constructor
     *
     * @param parent the parent object for this service
     */
    explicit Service(QObject *parent = 0);

    /**
     * Called when a job should be created by the Service.
     *
     * @param operation which operation to work on
     * @param parameters the parameters set by the user for the operation
     * @return a ServiceJob that can be started and monitored by the consumer
     */
    virtual ServiceJob *createJob(const QString &operation,
                                  const QMap<QString, QVariant> &parameters) = 0;

    /**
     * Sets the name of the Service.
     *
     * @param name the name to use for this service
     */
    void setName(const QString &name);

    /**
     * Sets the operations of the Service.
     *
     * @param operations the operations this service supports
     */
    void setOperationNames(const QStringList &operations);

    /**
     * Enables or disables given operation
     *
     * @param operation the name of the operation to enable or disable
     * @param enable true if the operation should be enabld, false if disabled
     */
    void setOperationEnabled(const QString &operation, bool enable);

private:
    Q_PRIVATE_SLOT(d, void jobFinished(KJob *))
    Q_PRIVATE_SLOT(d, void associatedWidgetDestroyed(QObject *))
    Q_PRIVATE_SLOT(d, void associatedGraphicsWidgetDestroyed(QObject *))

    ServicePrivate * const d;

    friend class Applet;
    friend class DataEnginePrivate;
    friend class GetSource;
    friend class PackagePrivate;
};

} // namespace Plasma

Q_DECLARE_METATYPE(Plasma::Service *)

#endif // multiple inclusion guard

