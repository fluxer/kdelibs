/* -*- C++ -*-

This file implements the WeaverImpl class.


$ Author: Mirko Boehm $
$ Copyright: (C) 2005-2013 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://creative-destruction.me $

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

$Id: WeaverImpl.cpp 30 2005-08-16 16:16:04Z mirko $

*/

#include "WeaverImpl.h"

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <kdebug.h>

#include "Job.h"
#include "State.h"
#include "Thread.h"
#include "ThreadWeaver.h"
#include "WeaverObserver.h"
#include "SuspendedState.h"
#include "SuspendingState.h"
#include "DestructedState.h"
#include "WorkingHardState.h"
#include "ShuttingDownState.h"
#include "InConstructionState.h"

using namespace ThreadWeaver;

WeaverImpl::WeaverImpl( QObject* parent )
    : WeaverInterface(parent)
    , m_active(0)
    , m_inventoryMax( qMax(4, 2 * QThread::idealThreadCount() ) )
    , m_finishMutex( new QMutex )
    , m_jobAvailableMutex ( new QMutex )
    , m_state (0)
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    // initialize state objects:
    m_states[InConstruction] = new InConstructionState( this );
    setState ( InConstruction );
    m_states[WorkingHard] = new WorkingHardState( this );
    m_states[Suspending] = new SuspendingState( this );
    m_states[Suspended] = new SuspendedState( this );
    m_states[ShuttingDown] = new ShuttingDownState( this );
    m_states[Destructed] = new DestructedState( this );

    setState(  WorkingHard );
}

WeaverImpl::~WeaverImpl()
{   // the constructor may only be called from the thread that owns this
    // object (everything else would be what we professionals call "insane")
    Q_ASSERT( QThread::currentThread() == thread() );
    kDebug() << "destroying inventory.";
    setState ( ShuttingDown );

    m_jobAvailable.wakeAll();

    // problem: Some threads might not be asleep yet, just finding
    // out if a job is available. Those threads will suspend
    // waiting for their next job (a rare case, but not impossible).
    // Therefore, if we encounter a thread that has not exited, we
    // have to wake it again (which we do in the following for
    // loop).

    for (;;) {
        Thread* th = 0;
        {
            std::lock_guard<std::recursive_mutex> l(m_mutex);
            if (m_inventory.isEmpty()) break;
            th = m_inventory.takeFirst();
        }
        if ( !th->isFinished() )
        {
            for ( ;; )
            {
                m_jobAvailable.wakeAll();
                if ( th->wait( 100 ) ) break;
                kDebug() << "thread" << th->id() << "did not exit as expected, retrying.";
            }
        }
        emit ( threadExited ( th ) );
        delete th;
    }
    Q_ASSERT(m_inventory.isEmpty());
    delete m_finishMutex;
    delete m_jobAvailableMutex;
    kDebug() << "done";
    setState ( Destructed ); // m_state = Halted;
    // FIXME: delete state objects. what sense does DestructedState make then?
    // FIXME: make state objects static, since they are
}

void WeaverImpl::setState ( StateId id )
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    if ( m_state==0 || m_state->stateId() != id )
    {
        m_state = m_states[id];
        kDebug() << "state changed to" << m_state->stateName();
        if ( id == Suspended )
        {
            emit ( suspended() );
        }

        m_state->activated();

        emit ( stateChanged ( m_state ) );
    }
}

const State& WeaverImpl::state() const
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    return *m_state;
}

void WeaverImpl::setMaximumNumberOfThreads( int cap )
{
    Q_ASSERT_X ( cap > 0, "Weaver Impl", "Thread inventory size has to be larger than zero." );
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    m_inventoryMax = cap;
}

int WeaverImpl::maximumNumberOfThreads() const
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    return m_inventoryMax;
}

int WeaverImpl::currentNumberOfThreads () const
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    return m_inventory.count ();
}

void WeaverImpl::registerObserver ( WeaverObserver *ext )
{
    connect ( this,  SIGNAL (stateChanged(ThreadWeaver::State*)),
              ext,  SIGNAL (weaverStateChanged(ThreadWeaver::State*)) );
    connect ( this,  SIGNAL (threadStarted(ThreadWeaver::Thread*)),
              ext,  SIGNAL (threadStarted(ThreadWeaver::Thread*)) );
    connect ( this,  SIGNAL (threadBusy(ThreadWeaver::Thread*,ThreadWeaver::Job*)),
              ext,  SIGNAL (threadBusy(ThreadWeaver::Thread*,ThreadWeaver::Job*)) );
    connect ( this,  SIGNAL (threadSuspended(ThreadWeaver::Thread*)),
              ext,  SIGNAL (threadSuspended(ThreadWeaver::Thread*)) );
    connect ( this,  SIGNAL (threadExited(ThreadWeaver::Thread*)) ,
              ext,  SIGNAL (threadExited(ThreadWeaver::Thread*)) );
}

void WeaverImpl::enqueue(Job* job)
{
    if (job) {
        adjustInventory ( 1 );
        kDebug() << "queueing job" << job << "of type " << job->metaObject()->className();
        std::lock_guard<std::recursive_mutex> l(m_mutex);
        job->aboutToBeQueued ( this );
        // find position for insertion:;
        // FIXME (after 0.6) optimize: factor out queue management into own class,
        // and use binary search for insertion (not done yet because
        // refactoring already planned):
        int i = m_assignments.size();
        if (i > 0) {
            while ( i > 0 && m_assignments.at(i - 1)->priority() < job->priority() ) --i;
            m_assignments.insert( i, (job) );
        } else {
            m_assignments.append (job);
        }
        assignJobs();
    }
}

void WeaverImpl::adjustInventory ( int numberOfNewJobs )
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);

    // no of threads that can be created:
    const int reserve = m_inventoryMax - m_inventory.count();

    if ( reserve > 0 )
    {
        for ( int i = 0; i < qMin ( reserve,  numberOfNewJobs ); ++i )
        {
            Thread *th = createThread();
            th->moveToThread( th ); // be sane from the start
            m_inventory.append(th);
            connect ( th,  SIGNAL (jobStarted(ThreadWeaver::Thread*,ThreadWeaver::Job*)),
                      SIGNAL (threadBusy(ThreadWeaver::Thread*,ThreadWeaver::Job*)) );
            connect ( th,  SIGNAL (jobDone(ThreadWeaver::Job*)),
                      SIGNAL (jobDone(ThreadWeaver::Job*)) );
            connect ( th,  SIGNAL (started(ThreadWeaver::Thread*)),
                      SIGNAL (threadStarted(ThreadWeaver::Thread*)) );

            th->start ();
            kDebug() << "thread created," << currentNumberOfThreads() << "threads in inventory.";
        }
    }
}

Thread* WeaverImpl::createThread()
{
    return new Thread( this );
}

bool WeaverImpl::dequeue ( Job* job )
{
    bool result;
    {
        std::lock_guard<std::recursive_mutex> l(m_mutex);

        int i = m_assignments.indexOf ( job );
        if ( i != -1 )
        {
            job->aboutToBeDequeued( this );

            m_assignments.removeAt( i );
            result = true;
            kDebug() << "job" << job << "dequeued," << m_assignments.size() << "jobs left.";
        } else {
            kDebug() << "job" << job << "not found in queue.";
            result = false;
        }
    }

    // from the queues point of view, a job is just as finished if
    // it gets dequeued:
    m_jobFinished.wakeOne();
    return result;
}

void WeaverImpl::dequeue ()
{
    kDebug() << "dequeueing all jobs.";
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    for ( int index = 0; index < m_assignments.size(); ++index )
    {
        m_assignments.at( index )->aboutToBeDequeued( this );
    }
    m_assignments.clear();

    Q_ASSERT ( m_assignments.isEmpty() );
}

void WeaverImpl::suspend ()
{
    m_state->suspend();
}

void WeaverImpl::resume ( )
{
    m_state->resume();
}

void WeaverImpl::assignJobs()
{
    m_jobAvailable.wakeAll();
}

bool WeaverImpl::isEmpty() const
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    return  m_assignments.isEmpty();
}


void WeaverImpl::incActiveThreadCount()
{
    adjustActiveThreadCount ( 1 );
}

void WeaverImpl::decActiveThreadCount()
{
    adjustActiveThreadCount ( -1 );
    // the done job could have freed another set of jobs, and we do not know how
    // many - therefore we need to wake all threads:
    m_jobFinished.wakeAll();
}

void WeaverImpl::adjustActiveThreadCount( int diff )
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    m_active += diff;
    kDebug() << m_active << "active threads (" << queueLength() <<  "in queue).";

    if ( m_assignments.isEmpty() && m_active == 0)
    {
        Q_ASSERT ( diff < 0 ); // cannot reach Zero otherwise
        emit ( finished() );
    }
}

int WeaverImpl::activeThreadCount()
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    return m_active;
}

Job* WeaverImpl::takeFirstAvailableJob(Job *previous)
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    if (previous) {
        // cleanup and send events:
        decActiveThreadCount();
    }
    Job *next = 0;
    for (int index = 0; index < m_assignments.size(); ++index) {
        if ( m_assignments.at(index)->canBeExecuted() ) {
            next = m_assignments.at(index);
            m_assignments.removeAt (index);
            break;
        }
    }
    if (next) {
        incActiveThreadCount();
    }
    return next;
}

Job* WeaverImpl::applyForWork(Thread *th, Job* previous)
{
    return m_state->applyForWork ( th,  previous );
}

void WeaverImpl::waitForAvailableJob(Thread* th)
{
    m_state->waitForAvailableJob ( th );
}

void WeaverImpl::blockThreadUntilJobsAreBeingAssigned ( Thread *th )
{   // th is the thread that calls this method:
    kDebug() << "thread"<< th->id() << "blocked.";
    emit threadSuspended ( th );
    QMutexLocker l( m_jobAvailableMutex );
    m_jobAvailable.wait( m_jobAvailableMutex );
    kDebug() << "thread"<< th->id() << "resumed.";
}

int WeaverImpl::queueLength() const
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    return m_assignments.count();
}

bool WeaverImpl::isIdle () const
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    return isEmpty() && m_active == 0;
}

void WeaverImpl::finish()
{
#ifdef QT_NO_DEBUG
    const int MaxWaitMilliSeconds = 50;
#else
    const int MaxWaitMilliSeconds = 500;
#endif
    while ( !isIdle() ) {
        Q_ASSERT(state().stateId() == WorkingHard);
        kDebug() << "not done, waiting.";
        QMutexLocker l( m_finishMutex );
        if ( m_jobFinished.wait( l.mutex(), MaxWaitMilliSeconds ) == false ) {
            kDebug() << "wait timed out," << queueLength() << "jobs left, waking threads.";
            m_jobAvailable.wakeAll();
        }
    }
    kDebug() << "done.\n\n";
}

void WeaverImpl::requestAbort()
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    for ( int i = 0; i<m_inventory.size(); ++i ) {
        m_inventory[i]->requestAbort();
    }
}

void WeaverImpl::dumpJobs()
{
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    kDebug() << "current jobs:";
    for ( int index = 0; index < m_assignments.size(); ++index ) {
        kDebug() << "-->"
                 << index << ":"
                 << m_assignments.at( index )
                 << m_assignments.at( index )->metaObject()->className()
                 << "(priority" << m_assignments.at(index)->priority()
                 << ", can be executed:" << m_assignments.at(index)->canBeExecuted() << ")";
    }
}

#include "moc_WeaverImpl.cpp"
