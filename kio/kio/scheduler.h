// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       Waldo Bastian <bastian@kde.org>

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

#ifndef KIO_SCHEDULER_H
#define KIO_SCHEDULER_H

#include "kio/job.h"
#include "kio/jobclasses.h"
#include <QtGui/qwindowdefs.h>

namespace KIO {

    class Slave;
    class SlaveConfig;

    class SchedulerPrivate;
    /**
     * The KIO::Scheduler manages io-slaves for the application.
     * It also queues jobs and assigns the job to a slave when one
     * becomes available.
     *
     * There are 3 possible ways for a job to get a slave:
     *
     * <h3>1. Direct</h3>
     * This is the default. When you create a job the
     * KIO::Scheduler will be notified and will find either an existing
     * slave that is idle or it will create a new slave for the job.
     *
     * Example:
     * \code
     *    TransferJob *job = KIO::get(KUrl("http://www.kde.org"));
     * \endcode
     *
     *
     * <h3>2. Scheduled</h3>
     * If you create a lot of jobs, you might not want to have a
     * slave for each job. If you schedule a job, a maximum number
     * of slaves will be created. When more jobs arrive, they will be
     * queued. When a slave is finished with a job, it will be assigned
     * a job from the queue.
     *
     * Example:
     * \code
     *    TransferJob *job = KIO::get(KUrl("http://www.kde.org"));
     *    KIO::Scheduler::setJobPriority(job, 1);
     * \endcode
     *
     * <h3>3. Connection Oriented</h3>
     * For some operations it is important that multiple jobs use
     * the same connection. This can only be ensured if all these jobs
     * use the same slave.
     *
     * You can then use the scheduler to assign jobs. The jobs will be
     * queued and the slave will handle these jobs one after the other.
     *
     * Example:
     * \code
     *    TransferJob *job1 = KIO::get(
     *            KUrl("pop3://bastian:password@mail.kde.org/msg1"));
     *    KIO::Scheduler::doJob(job1);
     *
     *    // ... Wait for jobs to finish...
     * \endcode
     *
     * @see KIO::Slave
     * @see KIO::Job
     **/

    class KIO_EXPORT Scheduler : public QObject
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.KIO.Scheduler")
    public:
        /**
         * Register @p job with the scheduler.
         * The default is to create a new slave for the job if no slave
         * is available. This can be changed by calling setJobPriority.
         * @param job the job to register
         */
        static void doJob(KIO::SimpleJob *job);

        /**
         * Changes the priority of @p job; jobs of the same priority run in the order in which
         * they were created. Jobs of lower numeric priority always run before any
         * waiting jobs of higher numeric priority. The range of priority is -10 to 10,
         * the default priority of jobs is 0.
         * @param job the job to change
         * @param priority new priority of @p job, lower runs earlier
         */
        static void setJobPriority(KIO::SimpleJob *job, int priority);

        /**
         * Stop the execution of a job.
         * @param job the job to cancel
         */
        static void cancelJob(KIO::SimpleJob *job);

        /**
         * Called when a job is done.
         * @param job the finished job
         * @param slave the slave that executed the @p job
         */
        static void jobFinished(KIO::SimpleJob *job, KIO::Slave *slave);

        /**
         * Puts a slave on notice. A next job may reuse this slave if it
         * requests the same URL.
         *
         * A job can be put on hold after it has emit'ed its mimetype.
         * Based on the mimetype, the program can give control to another
         * component in the same process which can then resume the job
         * by simply asking for the same URL again.
         * @param job the job that should be stopped
         * @param url the URL that is handled by the @p url
         */
        static void putSlaveOnHold(KIO::SimpleJob *job, const KUrl &url);

        /**
         * Removes any slave that might have been put on hold. If a slave
         * was put on hold it will be killed.
         */
        static void removeSlaveOnHold();

        /**
         * Send the slave that was put on hold back to KLauncher. This
         * allows another process to take over the slave and resume the job
         * that was started.
         */
        static void publishSlaveOnHold();

        /**
         * Register the mainwindow @p wid with the KIO subsystem
         * Do not call this, it is called automatically from
         * void KIO::Job::setWindow(QWidget*).
         * @param wid the window to register
         */
        static void registerWindow(QWidget *wid);

        /**
         * @internal
         * Unregisters the window registered by registerWindow().
         */
        static void unregisterWindow(QObject *wid);

        /**
         * When true, the next job will check whether KLauncher has a slave
         * on hold that is suitable for the job.
         * @param b true when KLauncher has a job on hold
         */
        static void checkSlaveOnHold(bool b);

        static void emitReparseSlaveConfiguration();

        /**
         * Returns true if there is a slave on hold for @p url.
         *
         * @since 4.7
         */
        static bool isSlaveOnHoldFor(const KUrl& url);

        /**
         * Updates the internal metadata from job.
         *
         * @since 4.6.5
         */
        static void updateInternalMetaData(SimpleJob* job);

        static Scheduler *self();

    Q_SIGNALS:
        // DBUS
        Q_SCRIPTABLE void reparseSlaveConfiguration(const QString &);
        Q_SCRIPTABLE void slaveOnHoldListChanged();

    private:
        Q_DISABLE_COPY(Scheduler)
        Scheduler();
        ~Scheduler();

        Q_PRIVATE_SLOT(d_func(), void slotSlaveDied(KIO::Slave *slave))

        // connected to D-Bus signal:
        Q_PRIVATE_SLOT(d_func(), void slotReparseSlaveConfiguration(const QString &, const QDBusMessage&))
        Q_PRIVATE_SLOT(d_func(), void slotSlaveOnHoldListChanged())

        Q_PRIVATE_SLOT(d_func(), void slotUnregisterWindow(QObject *))
    private:
        friend class SchedulerPrivate;
        SchedulerPrivate *d_func();
};

}
#endif
