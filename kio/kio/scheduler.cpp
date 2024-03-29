/* This file is part of the KDE libraries
   Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                      Waldo Bastian <bastian@kde.org>
   Copyright (C) 2009, 2010 Andreas Hartmetz <ahartmetz@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "scheduler.h"
#include "scheduler_p.h"

#include "slaveconfig.h"
#include "authinfo.h"
#include "slaveinterface.h"
#include "connection.h"
#include "job_p.h"

#include <kdebug.h>
#include <kprotocolmanager.h>
#include <kprotocolinfo.h>


#include <QHash>
#include <QTextCodec>
#include <QCoreApplication>
#include <QThread>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QWidget>

#include <assert.h>

// Slaves may be idle for a certain time (1 minute) before they are killed.
static const int s_idleSlaveLifetime = 1 * 60;


using namespace KIO;

// TODO: duplicate
static inline SlaveInterface *getJobSlave(SimpleJob *job)
{
    return SimpleJobPrivate::get(job)->m_slave;
}

static inline void startJob(SimpleJob *job, SlaveInterface *slave)
{
    SimpleJobPrivate::get(job)->start(slave);
}

// here be uglies
// forward declaration to break cross-dependency of SlaveKeeper and SchedulerPrivate
static void setupSlave(KIO::SlaveInterface *slave, const KUrl &url, const QString &protocol,
                       const QStringList &proxyList, bool newSlave);
// same reason as above
static Scheduler *scheduler();

/********************************* SessionData ****************************/

SessionData::SessionData()
{
    initDone = false;
}

SessionData::~SessionData()
{
}

void SessionData::configDataFor( MetaData &configData, const QString &proto)
{
  if ( (proto.startsWith(QLatin1String("http"), Qt::CaseInsensitive)) )
  {
    if (!initDone)
        reset();

    // These might have already been set so check first
    // to make sure that we do not trumpt settings sent
    // by apps or end-user.
    if ( configData["Languages"].isEmpty() )
        configData["Languages"] = language;
    if ( configData["Charsets"].isEmpty() )
        configData["Charsets"] = charsets;
    if ( configData["UserAgent"].isEmpty() )
        configData["UserAgent"] = KProtocolManager::defaultUserAgent();
  }
}

void SessionData::reset()
{
    initDone = true;

    language = KProtocolManager::acceptLanguagesHeader();
    charsets = QString::fromLatin1(QTextCodec::codecForLocale()->name()).toLower();
    KProtocolManager::reparseConfiguration();
}

int SerialPicker::changedPrioritySerial(int oldSerial, int newPriority) const
{
    Q_ASSERT(newPriority >= -10 && newPriority <= 10);
    newPriority = qBound(-10, newPriority, 10);
    int unbiasedSerial = oldSerial % m_jobsPerPriority;
    return unbiasedSerial + newPriority * m_jobsPerPriority;
}


SlaveKeeper::SlaveKeeper()
{
    m_grimTimer.setSingleShot(true);
    connect(&m_grimTimer, SIGNAL(timeout()), this, SLOT(grimReaper()));
}

void SlaveKeeper::returnSlave(SlaveInterface *slave)
{
    Q_ASSERT(slave);
    slave->setIdle();
    m_idleSlaves.insert(slave->host(), slave);
    scheduleGrimReaper();
}

SlaveInterface *SlaveKeeper::takeSlaveForJob(SimpleJob *job)
{
    KUrl url = SimpleJobPrivate::get(job)->m_url;
    // TODO take port, username and password into account
    QMultiHash<QString, SlaveInterface *>::Iterator it = m_idleSlaves.find(url.host());
    if (it == m_idleSlaves.end()) {
        it = m_idleSlaves.begin();
    }
    if (it == m_idleSlaves.end()) {
        return 0;
    }
    SlaveInterface *slave = it.value();
    m_idleSlaves.erase(it);
    return slave;
}

bool SlaveKeeper::removeSlave(SlaveInterface *slave)
{
    // ### performance not so great
    QMultiHash<QString, SlaveInterface *>::Iterator it = m_idleSlaves.begin();
    for (; it != m_idleSlaves.end(); ++it) {
        if (it.value() == slave) {
            m_idleSlaves.erase(it);
            return true;
        }
    }
    return false;
}

QList<SlaveInterface *> SlaveKeeper::allSlaves() const
{
    return m_idleSlaves.values();
}

void SlaveKeeper::scheduleGrimReaper()
{
    if (!m_grimTimer.isActive()) {
        m_grimTimer.start((s_idleSlaveLifetime / 2) * 1000);
    }
}

//private slot
void SlaveKeeper::grimReaper()
{
    QMultiHash<QString, SlaveInterface *>::Iterator it = m_idleSlaves.begin();
    while (it != m_idleSlaves.end()) {
        SlaveInterface *slave = it.value();
        if (slave->idleTime() >= s_idleSlaveLifetime) {
            it = m_idleSlaves.erase(it);
            if (slave->job()) {
                kDebug (7006) << "Idle slave" << slave << "still has job" << slave->job();
            }
            slave->kill();
            // avoid invoking slotSlaveDied() because its cleanup services are not needed
            slave->deref();
        } else {
            ++it;
        }
    }
    if (!m_idleSlaves.isEmpty()) {
        scheduleGrimReaper();
    }
}


int HostQueue::lowestSerial() const
{
    QMap<int, SimpleJob*>::ConstIterator first = m_queuedJobs.constBegin();
    if (first != m_queuedJobs.constEnd()) {
        return first.key();
    }
    return SerialPicker::maxSerial;
}

void HostQueue::queueJob(SimpleJob *job)
{
    const int serial = SimpleJobPrivate::get(job)->m_schedSerial;
    Q_ASSERT(serial != 0);
    Q_ASSERT(!m_queuedJobs.contains(serial));
    Q_ASSERT(!m_runningJobs.contains(job));
    m_queuedJobs.insert(serial, job);
}

SimpleJob *HostQueue::takeFirstInQueue()
{
    Q_ASSERT(!m_queuedJobs.isEmpty());
    QMap<int, SimpleJob *>::iterator first = m_queuedJobs.begin();
    SimpleJob *job = first.value();
    m_queuedJobs.erase(first);
    m_runningJobs.insert(job);
    return job;
}

bool HostQueue::removeJob(SimpleJob *job)
{
    const int serial = SimpleJobPrivate::get(job)->m_schedSerial;
    if (m_runningJobs.remove(job)) {
        Q_ASSERT(!m_queuedJobs.contains(serial));
        return true;
    }
    if (m_queuedJobs.remove(serial)) {
        return true;
    }
    return false;
}

QList<SlaveInterface *> HostQueue::allSlaves() const
{
    QList<SlaveInterface *> ret;
    Q_FOREACH (SimpleJob *job, m_runningJobs) {
        SlaveInterface *slave = getJobSlave(job);
        Q_ASSERT(slave);
        ret.append(slave);
    }
    return ret;
}

static void ensureNoDuplicates(QMap<int, HostQueue *> *queuesBySerial)
{
    Q_UNUSED(queuesBySerial);
#ifdef SCHEDULER_DEBUG
    // a host queue may *never* be in queuesBySerial twice.
    QSet<HostQueue *> seen;
    Q_FOREACH (HostQueue *hq, *queuesBySerial) {
        Q_ASSERT(!seen.contains(hq));
        seen.insert(hq);
    }
#endif
}

static void verifyRunningJobsCount(QHash<QString, HostQueue> *queues, int runningJobsCount)
{
    Q_UNUSED(queues);
    Q_UNUSED(runningJobsCount);
#ifdef SCHEDULER_DEBUG
    int realRunningJobsCount = 0;
    Q_FOREACH (const HostQueue &hq, *queues) {
        realRunningJobsCount += hq.runningJobsCount();
    }
    Q_ASSERT(realRunningJobsCount == runningJobsCount);

    // ...and of course we may never run the same job twice!
    QSet<SimpleJob *> seenJobs;
    Q_FOREACH (const HostQueue &hq, *queues) {
        Q_FOREACH (SimpleJob *job, hq.runningJobs()) {
            Q_ASSERT(!seenJobs.contains(job));
            seenJobs.insert(job);
        }
    }
#endif
}


ProtoQueue::ProtoQueue(int maxSlaves, int maxSlavesPerHost)
 : m_maxConnectionsPerHost(maxSlavesPerHost ? maxSlavesPerHost : maxSlaves),
   m_maxConnectionsTotal(qMax(maxSlaves, maxSlavesPerHost)),
   m_runningJobsCount(0)

{
    kDebug(7006) << "m_maxConnectionsTotal:" << m_maxConnectionsTotal
                 << "m_maxConnectionsPerHost:" << m_maxConnectionsPerHost;
    Q_ASSERT(m_maxConnectionsPerHost >= 1);
    Q_ASSERT(maxSlaves >= maxSlavesPerHost);
    m_startJobTimer.setSingleShot(true);
    connect (&m_startJobTimer, SIGNAL(timeout()), SLOT(startAJob()));
}

ProtoQueue::~ProtoQueue()
{
    Q_FOREACH (SlaveInterface *slave, allSlaves()) {
        // kill the slave process, then remove the interface in our process
        slave->kill();
        slave->deref();
    }
}

void ProtoQueue::queueJob(SimpleJob *job)
{
    QString hostname = SimpleJobPrivate::get(job)->m_url.host();
    HostQueue &hq = m_queuesByHostname[hostname];
    const int prevLowestSerial = hq.lowestSerial();
    Q_ASSERT(hq.runningJobsCount() <= m_maxConnectionsPerHost);

    // nevert insert a job twice
    Q_ASSERT(SimpleJobPrivate::get(job)->m_schedSerial == 0);
    SimpleJobPrivate::get(job)->m_schedSerial = m_serialPicker.next();

    const bool wasQueueEmpty = hq.isQueueEmpty();
    hq.queueJob(job);
    // note that HostQueue::queueJob() into an empty queue changes its lowestSerial() too...
    // the queue's lowest serial job may have changed, so update the ordered list of queues.
    // however, we ignore all jobs that would cause more connections to a host than allowed.
    if (prevLowestSerial != hq.lowestSerial()) {
        if (hq.runningJobsCount() < m_maxConnectionsPerHost) {
            // if the connection limit didn't keep the HQ unscheduled it must have been lack of jobs
            if (m_queuesBySerial.remove(prevLowestSerial) == 0) {
                Q_UNUSED(wasQueueEmpty);
                Q_ASSERT(wasQueueEmpty);
            }
            m_queuesBySerial.insert(hq.lowestSerial(), &hq);
        } else {
#ifdef SCHEDULER_DEBUG
            // ### this assertion may fail if the limits were modified at runtime!
            // if the per-host connection limit is already reached the host queue's lowest serial
            // should not be queued.
            Q_ASSERT(!m_queuesBySerial.contains(prevLowestSerial));
#endif
        }
    }
    // just in case; startAJob() will refuse to start a job if it shouldn't.
    m_startJobTimer.start();

    ensureNoDuplicates(&m_queuesBySerial);
}

void ProtoQueue::changeJobPriority(SimpleJob *job, int newPrio)
{
    SimpleJobPrivate *jobPriv = SimpleJobPrivate::get(job);
    QHash<QString, HostQueue>::Iterator it = m_queuesByHostname.find(jobPriv->m_url.host());
    if (it == m_queuesByHostname.end()) {
        return;
    }
    HostQueue &hq = it.value();
    const int prevLowestSerial = hq.lowestSerial();
    if (hq.isJobRunning(job) || !hq.removeJob(job)) {
        return;
    }
    jobPriv->m_schedSerial = m_serialPicker.changedPrioritySerial(jobPriv->m_schedSerial, newPrio);
    hq.queueJob(job);
    const bool needReinsert = hq.lowestSerial() != prevLowestSerial;
    // the host queue might be absent from m_queuesBySerial because the connections per host limit
    // for that host has been reached.
    if (needReinsert && m_queuesBySerial.remove(prevLowestSerial)) {
        m_queuesBySerial.insert(hq.lowestSerial(), &hq);
    }
    ensureNoDuplicates(&m_queuesBySerial);
}

void ProtoQueue::removeJob(SimpleJob *job)
{
    SimpleJobPrivate *jobPriv = SimpleJobPrivate::get(job);
    HostQueue &hq = m_queuesByHostname[jobPriv->m_url.host()];
    const int prevLowestSerial = hq.lowestSerial();
    const int prevRunningJobs = hq.runningJobsCount();

    Q_ASSERT(hq.runningJobsCount() <= m_maxConnectionsPerHost);

    if (hq.removeJob(job)) {
        if (hq.lowestSerial() != prevLowestSerial) {
            // we have dequeued the not yet running job with the lowest serial
            Q_ASSERT(!jobPriv->m_slave);
            Q_ASSERT(prevRunningJobs == hq.runningJobsCount());
            if (m_queuesBySerial.remove(prevLowestSerial) == 0) {
                // make sure that the queue was not scheduled for a good reason
                Q_ASSERT(hq.runningJobsCount() == m_maxConnectionsPerHost);
            }
        } else {
            if (prevRunningJobs != hq.runningJobsCount()) {
                // we have dequeued a previously running job
                Q_ASSERT(prevRunningJobs - 1 == hq.runningJobsCount());
                m_runningJobsCount--;
                Q_ASSERT(m_runningJobsCount >= 0);
            }
        }
        if (!hq.isQueueEmpty() && hq.runningJobsCount() < m_maxConnectionsPerHost) {
            // this may be a no-op, but it's faster than first checking if it's already in.
            m_queuesBySerial.insert(hq.lowestSerial(), &hq);
        }

        if (hq.isEmpty()) {
            // no queued jobs, no running jobs. this destroys hq from above.
            m_queuesByHostname.remove(jobPriv->m_url.host());
        }

        if (jobPriv->m_slave && jobPriv->m_slave->isAlive()) {
            m_slaveKeeper.returnSlave(jobPriv->m_slave);
        }
        // just in case; startAJob() will refuse to start a job if it shouldn't.
        m_startJobTimer.start();
    } else {
        // should be a connected slave
        // if the assertion fails the job has probably changed the host part of its URL while
        // running, so we can't find it by hostname. don't do this.
        Q_ASSERT(false);
    }

    ensureNoDuplicates(&m_queuesBySerial);
}

SlaveInterface *ProtoQueue::createSlave(const QString &protocol, SimpleJob *job, const KUrl &url)
{
    int error;
    QString errortext;
    SlaveInterface *slave = SlaveInterface::createSlave(protocol, url, error, errortext);
    if (slave) {
        connect(
            slave, SIGNAL(slaveDied(KIO::SlaveInterface*)),
            scheduler(), SLOT(slotSlaveDied(KIO::SlaveInterface*))
        );
    } else {
        kError() << "couldn't create slave:" << errortext;
        if (job) {
            job->slotError(error, errortext);
        }
    }
    return slave;
}

bool ProtoQueue::removeSlave (KIO::SlaveInterface *slave)
{
    const bool removedUnconnected = m_slaveKeeper.removeSlave(slave);
    Q_ASSERT(!removedUnconnected);
    return removedUnconnected;
}

QList<SlaveInterface *> ProtoQueue::allSlaves() const
{
    QList<SlaveInterface *> ret(m_slaveKeeper.allSlaves());
    Q_FOREACH (const HostQueue &hq, m_queuesByHostname) {
        ret.append(hq.allSlaves());
    }
    return ret;
}

//private slot
void ProtoQueue::startAJob()
{
    ensureNoDuplicates(&m_queuesBySerial);
    verifyRunningJobsCount(&m_queuesByHostname, m_runningJobsCount);

#ifdef SCHEDULER_DEBUG
    kDebug(7006) << "m_runningJobsCount:" << m_runningJobsCount;
    Q_FOREACH (const HostQueue &hq, m_queuesByHostname) {
        Q_FOREACH (SimpleJob *job, hq.runningJobs()) {
            kDebug(7006) << SimpleJobPrivate::get(job)->m_url;
        }
    }
#endif
    if (m_runningJobsCount >= m_maxConnectionsTotal) {
#ifdef SCHEDULER_DEBUG
        kDebug(7006) << "not starting any jobs because maxConnectionsTotal has been reached.";
#endif
        return;
    }

    QMap<int, HostQueue *>::iterator first = m_queuesBySerial.begin();
    if (first != m_queuesBySerial.end()) {
        // pick a job and maintain the queue invariant: lower serials first
        HostQueue *hq = first.value();
        const int prevLowestSerial = first.key();
        Q_UNUSED(prevLowestSerial);
        Q_ASSERT(hq->lowestSerial() == prevLowestSerial);
        // the following assertions should hold due to queueJob(), takeFirstInQueue() and
        // removeJob() being correct
        Q_ASSERT(hq->runningJobsCount() < m_maxConnectionsPerHost);
        SimpleJob *startingJob = hq->takeFirstInQueue();
        Q_ASSERT(hq->runningJobsCount() <= m_maxConnectionsPerHost);
        Q_ASSERT(hq->lowestSerial() != prevLowestSerial);

        m_queuesBySerial.erase(first);
        // we've increased hq's runningJobsCount() by calling nexStartingJob()
        // so we need to check again.
        if (!hq->isQueueEmpty() && hq->runningJobsCount() < m_maxConnectionsPerHost) {
            m_queuesBySerial.insert(hq->lowestSerial(), hq);
        }

        // always increase m_runningJobsCount because it's correct if there is a slave and if there
        // is no slave, removeJob() will balance the number again. removeJob() would decrease the
        // number too much otherwise.
        // Note that createSlave() can call slotError() on a job which in turn calls removeJob(),
        // so increase the count here already.
        m_runningJobsCount++;

        bool isNewSlave = false;
        SlaveInterface *slave = m_slaveKeeper.takeSlaveForJob(startingJob);
        SimpleJobPrivate *jobPriv = SimpleJobPrivate::get(startingJob);
        if (!slave) {
            isNewSlave = true;
            slave = createSlave(jobPriv->m_protocol, startingJob, jobPriv->m_url);
        }

        if (slave) {
            jobPriv->m_slave = slave;
            setupSlave(slave, jobPriv->m_url, jobPriv->m_protocol, jobPriv->m_proxyList, isNewSlave);
            startJob(startingJob, slave);
        } else {
            // dispose of our records about the job and mark the job as unknown
            // (to prevent crashes later)
            // note that the job's slotError() can have called removeJob() first, so check that
            // it's not a ghost job with null serial already.
            if (jobPriv->m_schedSerial) {
                removeJob(startingJob);
                jobPriv->m_schedSerial = 0;
            }
        }
    } else {
#ifdef SCHEDULER_DEBUG
        kDebug(7006) << "not starting any jobs because there is no queued job.";
#endif
    }

    if (!m_queuesBySerial.isEmpty()) {
        m_startJobTimer.start();
    }
}



class KIO::SchedulerPrivate
{
public:
    SchedulerPrivate()
     : q(new Scheduler()),
       m_ignoreConfigReparse(false)
    {
    }

    ~SchedulerPrivate()
    {
        delete q;
        q = 0;
        Q_FOREACH (ProtoQueue *p, m_protocols) {
            Q_FOREACH (SlaveInterface *slave, p->allSlaves()) {
                slave->kill();
            }
            p->deleteLater();
        }
    }
    Scheduler *q;

    bool m_ignoreConfigReparse;

    SessionData sessionData;
    QMap<QObject *,WId> m_windowList;

    void doJob(SimpleJob *job);
    void setJobPriority(SimpleJob *job, int priority);
    void cancelJob(SimpleJob *job);
    void jobFinished(KIO::SimpleJob *job, KIO::SlaveInterface *slave);
    void registerWindow(QWidget *wid);

    MetaData metaDataFor(const QString &protocol, const QStringList &proxyList, const KUrl &url);
    void setupSlave(KIO::SlaveInterface *slave, const KUrl &url, const QString &protocol,
                    const QStringList &proxyList, bool newSlave);

    void slotSlaveDied(KIO::SlaveInterface *slave);

    void slotReparseSlaveConfiguration(const QString &, const QDBusMessage&);

    void slotUnregisterWindow(QObject *);

    ProtoQueue *protoQ(const QString& protocol, const QString& host)
    {
        ProtoQueue *pq = m_protocols.value(protocol, 0);
        if (!pq) {
            kDebug(7006) << "creating ProtoQueue instance for" << protocol;

            const int maxSlaves = KProtocolInfo::maxSlaves(protocol);
            int maxSlavesPerHost = -1;
            if (!host.isEmpty()) {
                bool ok = false;
                const int value = SlaveConfig::self()->configData(protocol, host, QLatin1String("MaxConnections")).toInt(&ok);
                if (ok)
                    maxSlavesPerHost = value;
            }
            if (maxSlavesPerHost == -1) {
                maxSlavesPerHost = KProtocolInfo::maxSlavesPerHost(protocol);
            }
            // Never allow maxSlavesPerHost to exceed maxSlaves.
            pq = new ProtoQueue(maxSlaves, qMin(maxSlaves, maxSlavesPerHost));
            m_protocols.insert(protocol, pq);
        }
        return pq;
    }
private:
    QHash<QString, ProtoQueue *> m_protocols;
};


K_GLOBAL_STATIC(SchedulerPrivate, schedulerPrivate)

Scheduler *Scheduler::self()
{
    return schedulerPrivate->q;
}

SchedulerPrivate *Scheduler::d_func()
{
    return schedulerPrivate;
}

//static
Scheduler *scheduler()
{
    return schedulerPrivate->q;
}


Scheduler::Scheduler()
{
    setObjectName( "scheduler" );

    const QString dbusPath = "/KIO/Scheduler";
    const QString dbusInterface = "org.kde.KIO.Scheduler";
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject( "/KIO/Scheduler", this, QDBusConnection::ExportScriptableSlots |
                                                 QDBusConnection::ExportScriptableSignals );
    dbus.connect(QString(), dbusPath, dbusInterface, "reparseSlaveConfiguration",
                 this, SLOT(slotReparseSlaveConfiguration(QString,QDBusMessage)));
}

Scheduler::~Scheduler()
{
}

void Scheduler::doJob(SimpleJob *job)
{
    schedulerPrivate->doJob(job);
}


void Scheduler::setJobPriority(SimpleJob *job, int priority)
{
    schedulerPrivate->setJobPriority(job, priority);
}

void Scheduler::cancelJob(SimpleJob *job)
{
    schedulerPrivate->cancelJob(job);
}

void Scheduler::jobFinished(KIO::SimpleJob *job, KIO::SlaveInterface *slave)
{
    schedulerPrivate->jobFinished(job, slave);
}

void Scheduler::registerWindow(QWidget *wid)
{
    schedulerPrivate->registerWindow(wid);
}

void Scheduler::unregisterWindow(QObject *wid)
{
    schedulerPrivate->slotUnregisterWindow(wid);
}

void Scheduler::emitReparseSlaveConfiguration()
{
    // Do it immediately in this process, otherwise we might send a request before reparsing
    // (e.g. when changing useragent in the plugin)
    schedulerPrivate->slotReparseSlaveConfiguration(QString(), QDBusMessage());

    schedulerPrivate->m_ignoreConfigReparse = true;
    emit self()->reparseSlaveConfiguration( QString() );
}


void SchedulerPrivate::slotReparseSlaveConfiguration(const QString &proto, const QDBusMessage&)
{
    if (m_ignoreConfigReparse) {
        kDebug(7006) << "Ignoring signal sent by myself";
        m_ignoreConfigReparse = false;
        return;
    }

    kDebug(7006) << "proto=" << proto;
    KProtocolManager::reparseConfiguration();
    SlaveConfig::self()->reset();
    sessionData.reset();

    QHash<QString, ProtoQueue *>::ConstIterator it = proto.isEmpty() ? m_protocols.constBegin() :
                                                                       m_protocols.constFind(proto);
    // not found?
    if (it == m_protocols.constEnd()) {
        return;
    }
    QHash<QString, ProtoQueue *>::ConstIterator endIt = proto.isEmpty() ? m_protocols.constEnd() :
                                                                          it + 1;
    for (; it != endIt; ++it) {
        Q_FOREACH(SlaveInterface *slave, (*it)->allSlaves()) {
            slave->send(CMD_REPARSECONFIGURATION);
            slave->resetHost();
        }
    }
}

void SchedulerPrivate::doJob(SimpleJob *job)
{
    kDebug(7006) << job;
    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        kWarning(7006) << "KIO is not thread-safe.";
    }

    KIO::SimpleJobPrivate *const jobPriv = SimpleJobPrivate::get(job);
    jobPriv->m_proxyList.clear();
    jobPriv->m_protocol = KProtocolManager::slaveProtocol(job->url(), jobPriv->m_proxyList);

    ProtoQueue *proto = protoQ(jobPriv->m_protocol, job->url().host());
    proto->queueJob(job);
}


void SchedulerPrivate::setJobPriority(SimpleJob *job, int priority)
{
    kDebug(7006) << job << priority;
    ProtoQueue *proto = protoQ(SimpleJobPrivate::get(job)->m_protocol, job->url().host());
    proto->changeJobPriority(job, priority);
}

void SchedulerPrivate::cancelJob(SimpleJob *job)
{
    // this method is called all over the place in job.cpp, so just do this check here to avoid
    // much boilerplate in job code.
    if (SimpleJobPrivate::get(job)->m_schedSerial == 0) {
        //kDebug(7006) << "Doing nothing because I don't know job" << job;
        return;
    }
    SlaveInterface *slave = getJobSlave(job);
    kDebug(7006) << job << slave;
    if (slave) {
        kDebug(7006) << "Scheduler: killing slave " << slave->pid();
        slave->kill();
    }
    jobFinished(job, slave);
}

void SchedulerPrivate::jobFinished(SimpleJob *job, SlaveInterface *slave)
{
    kDebug(7006) << job << slave;
    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        kWarning(7006) << "KIO is not thread-safe.";
    }

    KIO::SimpleJobPrivate *const jobPriv = SimpleJobPrivate::get(job);

    // make sure that we knew about the job!
    Q_ASSERT(jobPriv->m_schedSerial);

    ProtoQueue *pq = m_protocols.value(jobPriv->m_protocol);
    if (pq) {
       pq->removeJob(job);
    }

    if (slave) {
        slave->setJob(0);
        slave->disconnect(job);
    }
    jobPriv->m_schedSerial = 0; // this marks the job as unscheduled again
    jobPriv->m_slave = 0;
}

// static
void setupSlave(KIO::SlaveInterface *slave, const KUrl &url, const QString &protocol,
                const QStringList &proxyList , bool newSlave)
{
    schedulerPrivate->setupSlave(slave, url, protocol, proxyList, newSlave);
}

MetaData SchedulerPrivate::metaDataFor(const QString &protocol, const QStringList &proxyList, const KUrl &url)
{
    const QString host = url.host();
    MetaData configData = SlaveConfig::self()->configData(protocol, host);
    sessionData.configDataFor( configData, protocol );
    if (proxyList.isEmpty()) {
        configData.remove(QLatin1String("UseProxy"));
        configData.remove(QLatin1String("ProxyUrls"));
    } else {
        configData[QLatin1String("UseProxy")] = proxyList.first();
        configData[QLatin1String("ProxyUrls")] = proxyList.join(QLatin1String(","));
    }

    return configData;
}

void SchedulerPrivate::setupSlave(KIO::SlaveInterface *slave, const KUrl &url, const QString &protocol,
                                  const QStringList &proxyList, bool newSlave)
{
    int port = url.port();
    if ( port == -1 ) // no port is -1 in QUrl, but in kde3 we used 0 and the kioslaves assume that.
        port = 0;
    const QString host = url.host();
    const QString user = url.userName();
    const QString passwd = url.password();

    if (newSlave || slave->host() != host || slave->port() != port ||
        slave->user() != user || slave->passwd() != passwd) {

        MetaData configData = metaDataFor(protocol, proxyList, url);
        slave->setConfig(configData);
        slave->setProtocol(url.protocol());
        slave->setHost(host, port, user, passwd);
    }
}

void SchedulerPrivate::slotSlaveDied(KIO::SlaveInterface *slave)
{
    kDebug(7006) << slave;
    Q_ASSERT(slave);
    Q_ASSERT(!slave->isAlive());
    ProtoQueue *pq = m_protocols.value(slave->protocol());
    if (pq) {
       if (slave->job()) {
           pq->removeJob(slave->job());
       }
       // in case this was a connected slave...
       pq->removeSlave(slave);
    }
    slave->deref(); // Delete slave
}

/*
  Returns the top most window associated with widget.

  Unlike QWidget::window(), this function does its best to find and return the
  main application window associated with the given widget.

  If widget itself is a dialog or its parent is a dialog, and that dialog has a
  parent widget then this function will iterate through all those widgets to
  find the top most window, which most of the time is the main window of the
  application. By contrast, QWidget::window() would simply return the first
  file dialog it encountered since it is the "next ancestor widget that has (or
  could have) a window-system frame".
*/
static QWidget* topLevelWindow(QWidget* widget)
{
    QWidget* w = widget;
    while (w && w->parentWidget()) {
        w = w->parentWidget();
    }
    return (w ? w->window() : 0);
}

void SchedulerPrivate::registerWindow(QWidget *wid)
{
   if (!wid)
      return;

   QWidget* window = topLevelWindow(wid);
   QObject *obj = static_cast<QObject *>(window);

   if (!m_windowList.contains(obj))
   {
      // We must store the window Id because by the time
      // the destroyed signal is emitted we can no longer
      // access QWidget::winId() (already destructed)
      WId windowId = window->winId();
      m_windowList.insert(obj, windowId);
      q->connect(window, SIGNAL(destroyed(QObject*)),
                 SLOT(slotUnregisterWindow(QObject*)));
      QDBusInterface("org.kde.kded", "/kded", "org.kde.kded").
          call(QDBus::NoBlock, "registerWindowId", qlonglong(windowId));
   }
}

void SchedulerPrivate::slotUnregisterWindow(QObject *obj)
{
   if (!obj)
      return;

   QMap<QObject *, WId>::Iterator it = m_windowList.find(obj);
   if (it == m_windowList.end())
      return;
   WId windowId = it.value();
   q->disconnect(it.key(), SIGNAL(destroyed(QObject*)),
                 q, SLOT(slotUnregisterWindow(QObject*)));
   m_windowList.erase( it );
   QDBusInterface("org.kde.kded", "/kded", "org.kde.kded").
       call(QDBus::NoBlock, "unregisterWindowId", qlonglong(windowId));
}

#include "moc_scheduler.cpp"
#include "moc_scheduler_p.cpp"
