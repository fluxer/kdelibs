/* This file is part of the KDE libraries
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

#ifndef SCHEDULER_P_H
#define SCHEDULER_P_H

#include <QSet>
#include <QTimer>
#include <QMap>

// #define SCHEDULER_DEBUG

namespace KIO {

class SessionData
{
public:
    SessionData();
    ~SessionData();

    void configDataFor(KIO::MetaData &configData, const QString &proto);
    void reset();

private:
    bool initDone;
    QString charsets;
    QString language;
};

class SlaveKeeper : public QObject
{
    Q_OBJECT
public:
    SlaveKeeper();
    void returnSlave(KIO::SlaveInterface *slave);
    // pick suitable slave for job and return it, return null if no slave found.
    // the slave is removed from the keeper.
    KIO::SlaveInterface *takeSlaveForJob(KIO::SimpleJob *job);
    // remove slave from keeper
    bool removeSlave(KIO::SlaveInterface *slave);
    QList<KIO::SlaveInterface *> allSlaves() const;

private:
    void scheduleGrimReaper();

private slots:
    void grimReaper();

private:
    QMultiHash<QString, KIO::SlaveInterface *> m_idleSlaves;
    QTimer m_grimTimer;
};


class HostQueue
{
public:
    int lowestSerial() const;

    bool isQueueEmpty() const { return m_queuedJobs.isEmpty(); }
    bool isEmpty() const { return m_queuedJobs.isEmpty() && m_runningJobs.isEmpty(); }
    int runningJobsCount() const { return m_runningJobs.count(); }
#ifdef SCHEDULER_DEBUG
    QList<KIO::SimpleJob *> runningJobs() const { return m_runningJobs.toList(); }
#endif
    bool isJobRunning(KIO::SimpleJob *job) const { return m_runningJobs.contains(job); }

    void queueJob(KIO::SimpleJob *job);
    KIO::SimpleJob *takeFirstInQueue();
    bool removeJob(KIO::SimpleJob *job);

    QList<KIO::SlaveInterface *> allSlaves() const;
private:
    QMap<int, KIO::SimpleJob *> m_queuedJobs;
    QSet<KIO::SimpleJob *> m_runningJobs;
};

class SerialPicker
{
public:
    // note that serial number zero is the default value from job_p.h and invalid!
    SerialPicker()
     : m_offset(1) {}

    int next()
    {
        if (m_offset >= m_jobsPerPriority) {
            m_offset = 1;
        }
        return m_offset++;
    }

    int changedPrioritySerial(int oldSerial, int newPriority) const;

private:
    static const uint m_jobsPerPriority = 100000000;
    uint m_offset;
public:
    static const int maxSerial = m_jobsPerPriority * 20;
};


class ProtoQueue : public QObject
{
    Q_OBJECT
public:
    ProtoQueue(int maxSlaves, int maxSlavesPerHost);
    ~ProtoQueue();

    void queueJob(KIO::SimpleJob *job);
    void changeJobPriority(KIO::SimpleJob *job, int newPriority);
    void removeJob(KIO::SimpleJob *job);
    KIO::SlaveInterface *createSlave(const QString &protocol, KIO::SimpleJob *job, const KUrl &url);
    bool removeSlave (KIO::SlaveInterface *slave);
    QList<KIO::SlaveInterface *> allSlaves() const;

private slots:
    // start max one (non-connected) job and return
    void startAJob();

private:
    SerialPicker m_serialPicker;
    QTimer m_startJobTimer;
    QMap<int, HostQueue *> m_queuesBySerial;
    QHash<QString, HostQueue> m_queuesByHostname;
    SlaveKeeper m_slaveKeeper;
    int m_maxConnectionsPerHost;
    int m_maxConnectionsTotal;
    int m_runningJobsCount;
};

} // namespace KIO

#endif //SCHEDULER_P_H
