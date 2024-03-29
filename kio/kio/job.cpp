/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                  2000-2009 David Faure <faure@kde.org>
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

#include "job.h"
#include "job_p.h"
#include "clipboardupdater_p.h"
#include "jobuidelegate.h"
#include "kmimetype.h"
#include "slaveinterface.h"
#include "scheduler.h"
#include "kdirwatch.h"
#include "kprotocolinfo.h"
#include "kprotocolmanager.h"
#include "klocale.h"
#include "kconfig.h"
#include "kdebug.h"
#include "kde_file.h"
#include "kdirnotify.h"

#include <QList>
#include <QTimer>
#include <QFile>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

using namespace KIO;

#define MAX_READ_BUF_SIZE  (64 * 1024)       // 64 KB at a time seems reasonable...

// TODO: duplicate
static inline SlaveInterface *jobSlave(SimpleJob *job)
{
    return SimpleJobPrivate::get(job)->m_slave;
}

//this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
#define REPORT_TIMEOUT 200

Job::Job()
    : KCompositeJob(*new JobPrivate, 0)
{
    setCapabilities(KJob::Killable | KJob::Suspendable);
}

Job::Job(JobPrivate &dd)
    : KCompositeJob(dd, 0)
{
    setCapabilities(KJob::Killable | KJob::Suspendable);
}

Job::~Job()
{
}

JobUiDelegate* Job::ui() const
{
    return static_cast<JobUiDelegate*>(uiDelegate());
}

bool Job::addSubjob(KJob *jobBase)
{
    //kDebug(7007) << "addSubjob(" << jobBase << ") this=" << this;

    bool ok = KCompositeJob::addSubjob( jobBase );
    KIO::Job *job = qobject_cast<KIO::Job*>( jobBase );
    if (ok && job) {
        // Copy metadata into subjob (e.g. window-id, user-timestamp etc.)
        Q_D(Job);
        job->mergeMetaData(d->m_outgoingMetaData);

        // Forward information from that subjob.
        connect(job, SIGNAL(speed(KJob*,ulong)),
                SLOT(slotSpeed(KJob*,ulong)));

        if (ui() && job->ui()) {
            job->ui()->setWindow( ui()->window() );
            job->ui()->updateUserTimestamp( ui()->userTimestamp() );
        }
    }
    return ok;
}

bool Job::removeSubjob(KJob *jobBase)
{
    //kDebug(7007) << "removeSubjob(" << jobBase << ") this=" << this << "subjobs=" << subjobs().count();
    return KCompositeJob::removeSubjob(jobBase);
}

void JobPrivate::emitMoving(KIO::Job * job, const KUrl &src, const KUrl &dest)
{
    emit job->description(job, i18nc("@title job","Moving"),
                          qMakePair(i18nc("The source of a file operation", "Source"), src.pathOrUrl()),
                          qMakePair(i18nc("The destination of a file operation", "Destination"), dest.pathOrUrl()));
}

void JobPrivate::emitCopying(KIO::Job * job, const KUrl &src, const KUrl &dest)
{
    emit job->description(job, i18nc("@title job","Copying"),
                          qMakePair(i18nc("The source of a file operation", "Source"), src.pathOrUrl()),
                          qMakePair(i18nc("The destination of a file operation", "Destination"), dest.pathOrUrl()));
}

void JobPrivate::emitCreatingDir(KIO::Job * job, const KUrl &dir)
{
    emit job->description(job, i18nc("@title job","Creating directory"),
                          qMakePair(i18n("Directory"), dir.pathOrUrl()));
}

void JobPrivate::emitDeleting(KIO::Job *job, const KUrl &url)
{
    emit job->description(job, i18nc("@title job","Deleting"),
                          qMakePair(i18n("File"), url.pathOrUrl()));
}

void JobPrivate::emitStating(KIO::Job *job, const KUrl &url)
{
    emit job->description(job, i18nc("@title job","Examining"),
                          qMakePair(i18n("File"), url.pathOrUrl()));
}

void JobPrivate::emitTransferring(KIO::Job *job, const KUrl &url)
{
    emit job->description(job, i18nc("@title job","Transferring"),
                          qMakePair(i18nc("The source of a file operation", "Source"), url.pathOrUrl()));
}

bool Job::doKill()
{
    // kill all subjobs, without triggering their result slot
    Q_FOREACH( KJob* it, subjobs()) {
        it->kill( KJob::Quietly );
    }
    clearSubjobs();

    return true;
}

bool Job::doSuspend()
{
    Q_FOREACH(KJob* it, subjobs()) {
        if (!it->suspend()) {
            return false;
        }
    }

    return true;
}

bool Job::doResume()
{
    Q_FOREACH(KJob* it, subjobs()) {
        if (!it->resume()) {
            return false;
        }
    }

    return true;
}

void JobPrivate::slotSpeed(KJob *, unsigned long speed)
{
    // kDebug(7007) << speed;
    q_func()->emitSpeed(speed);
}

//Job::errorString is implemented in global.cpp

bool Job::isInteractive() const
{
    return uiDelegate() != 0;
}

void Job::setParentJob(Job *job)
{
    Q_D(Job);
    Q_ASSERT(d->m_parentJob == 0L);
    Q_ASSERT(job);
    d->m_parentJob = job;
}

Job* Job::parentJob() const
{
    return d_func()->m_parentJob;
}

MetaData Job::metaData() const
{
    return d_func()->m_incomingMetaData;
}

QString Job::queryMetaData(const QString &key)
{
    return d_func()->m_incomingMetaData.value(key, QString());
}

void Job::setMetaData( const KIO::MetaData &metaData)
{
    Q_D(Job);
    d->m_outgoingMetaData = metaData;
}

void Job::addMetaData(const QString &key, const QString &value)
{
    d_func()->m_outgoingMetaData.insert(key, value);
}

void Job::addMetaData(const MetaData &values)
{
    d_func()->m_outgoingMetaData += values;
}

void Job::mergeMetaData(const MetaData &values)
{
    Q_D(Job);
    QMapIterator<QString,QString> it(values);
    while(it.hasNext()) {
        it.next();
        // there's probably a faster way
        if (!d->m_outgoingMetaData.contains( it.key())) {
            d->m_outgoingMetaData.insert(it.key(), it.value());
        }
    }
}

MetaData Job::outgoingMetaData() const
{
    return d_func()->m_outgoingMetaData;
}

SimpleJob::SimpleJob(SimpleJobPrivate &dd)
  : Job(dd)
{
    Q_D(SimpleJob);
    if (!d->m_url.isValid()) {
        setError(ERR_MALFORMED_URL);
        setErrorText(d->m_url.url());
        QTimer::singleShot(0, this, SLOT(slotFinished()));
        return;
    }

    Scheduler::doJob(this);
}

bool SimpleJob::doKill()
{
    Q_D(SimpleJob);
    if ((d->m_extraFlags & JobPrivate::EF_KillCalled) == 0) {
        d->m_extraFlags |= JobPrivate::EF_KillCalled;
        Scheduler::cancelJob(this); // deletes the slave if not 0
    } else {
        kWarning(7007) << this << "This is overkill.";
    }
    return Job::doKill();
}

bool SimpleJob::doSuspend()
{
    Q_D(SimpleJob);
    if (d->m_slave) {
        d->m_slave->suspend();
    }
    return Job::doSuspend();
}

bool SimpleJob::doResume()
{
    Q_D(SimpleJob);
    if (d->m_slave) {
        d->m_slave->resume();
    }
    return Job::doResume();
}

const KUrl& SimpleJob::url() const
{
    return d_func()->m_url;
}

bool SimpleJob::isRedirectionHandlingEnabled() const
{
    return d_func()->m_redirectionHandlingEnabled;
}

void SimpleJob::setRedirectionHandlingEnabled(bool handle)
{
    Q_D(SimpleJob);
    d->m_redirectionHandlingEnabled = handle;
}

SimpleJob::~SimpleJob()
{
    Q_D(SimpleJob);
    // last chance to remove this job from the scheduler!
    if (d->m_schedSerial) {
        kDebug(7007) << "Killing job" << this << "in destructor!"  << kBacktrace();
        Scheduler::cancelJob(this);
    }
}

void SimpleJobPrivate::start(SlaveInterface *slave)
{
    Q_Q(SimpleJob);
    m_slave = slave;

    // SlaveInterface::setJob can send us metadata if there is a persistent connection
    q->connect(
        slave, SIGNAL(metaData(KIO::MetaData)),
        SLOT(slotMetaData(KIO::MetaData))
    );

    slave->setJob(q);

    q->connect(
        slave, SIGNAL(error(int,QString)),
        SLOT(slotError(int,QString))
    );

    q->connect(
        slave, SIGNAL(warning(QString)),
        SLOT(slotWarning(QString))
    );

    q->connect(
        slave, SIGNAL(infoMessage(QString)),
        SLOT(_k_slotSlaveInfoMessage(QString))
    );

    q->connect(
        slave, SIGNAL(finished()),
        SLOT(slotFinished())
    );

    if ((m_extraFlags & EF_TransferJobDataSent) == 0) {
        // this is a "get" job
        q->connect(
            slave, SIGNAL(totalSize(KIO::filesize_t)),
            SLOT(slotTotalSize(KIO::filesize_t))
        );

        q->connect(
            slave, SIGNAL(processedSize(KIO::filesize_t)),
            SLOT(slotProcessedSize(KIO::filesize_t))
        );

        q->connect(
            slave, SIGNAL(speed(ulong)),
            SLOT(slotSpeed(ulong))
        );
    }

    if (ui() && ui()->window()) {
        m_outgoingMetaData.insert("window-id", QString::number((qptrdiff)ui()->window()->winId()));
    }

    if (ui() && ui()->userTimestamp()) {
        m_outgoingMetaData.insert("user-timestamp", QString::number(ui()->userTimestamp()));
    }

    if (ui() == 0) {
        // not interactive
        m_outgoingMetaData.insert("no-auth-prompt", "true");
    }

    if (!m_outgoingMetaData.isEmpty()) {
        KIO_ARGS << m_outgoingMetaData;
        slave->send(CMD_META_DATA, packedArgs);
    }

    slave->send(m_command, m_packedArgs);
}

void SimpleJobPrivate::slaveDone()
{
    Q_Q(SimpleJob);
    if (m_slave) {
        // Remove all signals between slave and job
        q->disconnect(m_slave);
    }
    // only finish a job once; Scheduler::jobFinished() resets schedSerial to zero.
    if (m_schedSerial) {
        Scheduler::jobFinished(q, m_slave);
    }
}

void SimpleJob::slotFinished()
{
    Q_D(SimpleJob);
    // Return slave to the scheduler
    d->slaveDone();

    if (!hasSubjobs()) {
        if (!error() && (d->m_command == CMD_MKDIR || d->m_command == CMD_RENAME)) {
            if (d->m_command == CMD_MKDIR) {
                KUrl urlDir(url());
                urlDir.setPath(urlDir.directory());
                org::kde::KDirNotify::emitFilesAdded(urlDir.url());
            } else /*if ( m_command == CMD_RENAME )*/ {
                KUrl src, dst;
                QDataStream str(d->m_packedArgs);
                str >> src >> dst;
                if (src.directory() == dst.directory()) {
                    // For the user, moving isn't renaming. Only renaming is.
                    org::kde::KDirNotify::emitFileRenamed(src.url(), dst.url());
                }

                org::kde::KDirNotify::emitFileMoved(src.url(), dst.url());
                ClipboardUpdater::update(src, dst);
            }
        }
        emitResult();
    }
}

void SimpleJob::slotError(int err, const QString &errorText)
{
    Q_D(SimpleJob);
    setError(err);
    setErrorText(errorText);
    if (error() == ERR_UNKNOWN_HOST && d->m_url.host().isEmpty()) {
        setErrorText(QString());
    }
    // error terminates the job
    slotFinished();
}

void SimpleJob::slotWarning(const QString &errorText)
{
    emit warning(this, errorText);
}

void SimpleJobPrivate::_k_slotSlaveInfoMessage(const QString &msg)
{
    emit q_func()->infoMessage(q_func(), msg );
}

void SimpleJobPrivate::slotTotalSize(KIO::filesize_t size)
{
    Q_Q(SimpleJob);
    if (size != q->totalAmount(KJob::Bytes)) {
        q->setTotalAmount(KJob::Bytes, size);
    }
}

void SimpleJobPrivate::slotProcessedSize(KIO::filesize_t size)
{
    Q_Q(SimpleJob);
    // kDebug(7007) << KIO::number(size);
    q->setProcessedAmount(KJob::Bytes, size);
}

void SimpleJobPrivate::slotSpeed(unsigned long speed)
{
    //kDebug(7007) << speed;
    q_func()->emitSpeed(speed);
}

void SimpleJobPrivate::restartAfterRedirection(KUrl *redirectionUrl)
{
    Q_Q(SimpleJob);
    // Return slave to the scheduler while we still have the old URL in place; the scheduler
    // requires a job URL to stay invariant while the job is running.
    slaveDone();

    m_url = *redirectionUrl;
    redirectionUrl->clear();
    if ((m_extraFlags & EF_KillCalled) == 0) {
        Scheduler::doJob(q);
    }
}

int SimpleJobPrivate::requestMessageBox(int _type, const QString &text, const QString &caption,
                                        const QString &buttonYes, const QString &buttonNo,
                                        const QString &iconYes, const QString &iconNo,
                                        const QString &dontAskAgainName)
{
    JobUiDelegate* delegate = ui();
    if (delegate) {
        const JobUiDelegate::MessageBoxType type = static_cast<JobUiDelegate::MessageBoxType>(_type);
        return delegate->requestMessageBox(
            type, text, caption, buttonYes, buttonNo,
            iconYes, iconNo, dontAskAgainName
        );
    }
    kWarning(7007) << "JobUiDelegate not set! Returing -1";
    return -1;
}

void SimpleJob::slotMetaData(const KIO::MetaData &metaData)
{
    Q_D(SimpleJob);
    d->m_incomingMetaData += metaData;
}

//////////
class KIO::MkdirJobPrivate: public SimpleJobPrivate
{
public:
    MkdirJobPrivate(const KUrl &url, int command, const QByteArray &packedArgs)
        : SimpleJobPrivate(url, command, packedArgs)
    {
    }

    KUrl m_redirectionURL;
    void slotRedirection(const KUrl &url);

    /**
     * @internal
     * Called by the scheduler when a @p slave gets to
     * work on this job.
     * @param slave the slave that starts working on this job
     */
    virtual void start(SlaveInterface *slave);

    Q_DECLARE_PUBLIC(MkdirJob)

    static inline MkdirJob *newJob(const KUrl &url, int command, const QByteArray &packedArgs)
    {
        MkdirJob *job = new MkdirJob(*new MkdirJobPrivate(url, command, packedArgs));
        job->setUiDelegate(new JobUiDelegate());
        return job;
    }
};

MkdirJob::MkdirJob(MkdirJobPrivate &dd)
    : SimpleJob(dd)
{
}

void MkdirJobPrivate::start(SlaveInterface *slave)
{
    Q_Q(MkdirJob);
    q->connect(
        slave, SIGNAL(redirection(KUrl)),
        SLOT(slotRedirection(KUrl))
    );

    SimpleJobPrivate::start(slave);
}

// Slave got a redirection request
void MkdirJobPrivate::slotRedirection(const KUrl &url)
{
     Q_Q(MkdirJob);
     kDebug(7007) << url;
     m_redirectionURL = url; // We'll remember that when the job finishes
     // Tell the user that we haven't finished yet
     emit q->redirection(q, m_redirectionURL);
}

void MkdirJob::slotFinished()
{
    Q_D(MkdirJob);

    if (!d->m_redirectionURL.isEmpty() && d->m_redirectionURL.isValid()) {
        //kDebug(7007) << "MkdirJob: Redirection to " << m_redirectionURL;
        if (d->m_redirectionHandlingEnabled) {
            KUrl dummyUrl;
            int permissions;
            QDataStream istream(d->m_packedArgs);
            istream >> dummyUrl >> permissions;

            d->m_packedArgs.truncate(0);
            QDataStream stream(&d->m_packedArgs, QIODevice::WriteOnly);
            stream << d->m_redirectionURL << permissions;

            d->restartAfterRedirection(&d->m_redirectionURL);
            return;
        }
    }

    // Return slave to the scheduler
    SimpleJob::slotFinished();
}

SimpleJob *KIO::mkdir(const KUrl &url, int permissions)
{
    //kDebug(7007) << "mkdir " << url;
    KIO_ARGS << url << permissions;
    return MkdirJobPrivate::newJob(url, CMD_MKDIR, packedArgs);
}

SimpleJob *KIO::rmdir(const KUrl &url)
{
    // kDebug(7007) << "rmdir " << url;
    KIO_ARGS << url << qint8(false); // isFile is false
    return SimpleJobPrivate::newJob(url, CMD_DEL, packedArgs);
}

SimpleJob *KIO::chmod(const KUrl &url, int permissions)
{
    // kDebug(7007) << "chmod " << url;
    KIO_ARGS << url << permissions;
    return SimpleJobPrivate::newJob(url, CMD_CHMOD, packedArgs);
}

SimpleJob *KIO::chown(const KUrl &url, const QString &owner, const QString &group)
{
    KIO_ARGS << url << owner << group;
    return SimpleJobPrivate::newJob(url, CMD_CHOWN, packedArgs);
}

SimpleJob *KIO::setModificationTime(const KUrl &url, const QDateTime &mtime)
{
    // kDebug(7007) << "setModificationTime " << url << " " << mtime;
    KIO_ARGS << url << mtime;
    return SimpleJobPrivate::newJobNoUi(url, CMD_SETMODIFICATIONTIME, packedArgs);
}

SimpleJob *KIO::rename( const KUrl& src, const KUrl & dest, JobFlags flags )
{
    // kDebug(7007) << "rename " << src << " " << dest;
    KIO_ARGS << src << dest << (qint8) (flags & Overwrite);
    return SimpleJobPrivate::newJob(src, CMD_RENAME, packedArgs);
}

SimpleJob *KIO::symlink(const QString &target, const KUrl &dest, JobFlags flags)
{
    // kDebug(7007) << "symlink target=" << target << " " << dest;
    KIO_ARGS << target << dest << (qint8) (flags & Overwrite);
    return SimpleJobPrivate::newJob(dest, CMD_SYMLINK, packedArgs, flags);
}

SimpleJob *KIO::special(const KUrl &url, const QByteArray &data, JobFlags flags)
{
    // kDebug(7007) << "special " << url;
    return SimpleJobPrivate::newJob(url, CMD_SPECIAL, data, flags);
}

//////////

class KIO::StatJobPrivate: public SimpleJobPrivate
{
public:
    inline StatJobPrivate(const KUrl &url, int command, const QByteArray &packedArgs)
        : SimpleJobPrivate(url, command, packedArgs), m_bSource(true), m_details(2)
    {
    }

    UDSEntry m_statResult;
    KUrl m_redirectionURL;
    bool m_bSource;
    short int m_details;
    void slotStatEntry(const KIO::UDSEntry &entry);
    void slotRedirection(const KUrl &url);

    /**
     * @internal
     * Called by the scheduler when a @p slave gets to
     * work on this job.
     * @param slave the slave that starts working on this job
     */
    virtual void start(SlaveInterface *slave);

    Q_DECLARE_PUBLIC(StatJob)

    static inline StatJob *newJob(const KUrl &url, int command, const QByteArray &packedArgs, JobFlags flags)
    {
        StatJob *job = new StatJob(*new StatJobPrivate(url, command, packedArgs));
        job->setUiDelegate(new JobUiDelegate());
        if (!(flags & HideProgressInfo)) {
            KIO::getJobTracker()->registerJob(job);
            emitStating(job, url);
        }
        return job;
    }
};

StatJob::StatJob(StatJobPrivate &dd)
    : SimpleJob(dd)
{
}

void StatJob::setSide(StatSide side)
{
    d_func()->m_bSource = side == SourceSide;
}

void StatJob::setDetails( short int details )
{
    d_func()->m_details = details;
}

const UDSEntry& StatJob::statResult() const
{
    return d_func()->m_statResult;
}

KUrl StatJob::mostLocalUrl() const
{
    if (!url().isLocalFile()) {
        const UDSEntry& udsEntry = d_func()->m_statResult;
        const QString path = udsEntry.stringValue(KIO::UDSEntry::UDS_LOCAL_PATH);
        if (!path.isEmpty()) {
            return KUrl(path);
        }
    }
    return url();
}

void StatJobPrivate::start(SlaveInterface *slave)
{
    Q_Q(StatJob);
    m_outgoingMetaData.insert("statSide", m_bSource ? "source" : "dest");
    m_outgoingMetaData.insert("details", QString::number(m_details));

    q->connect(
        slave, SIGNAL(statEntry(KIO::UDSEntry)),
        SLOT(slotStatEntry(KIO::UDSEntry))
    );
    q->connect(
        slave, SIGNAL(redirection(KUrl)),
        SLOT(slotRedirection(KUrl))
    );

    SimpleJobPrivate::start(slave);
}

void StatJobPrivate::slotStatEntry(const KIO::UDSEntry &entry)
{
    // kDebug(7007);
    m_statResult = entry;
}

// Slave got a redirection request
void StatJobPrivate::slotRedirection(const KUrl &url)
{
     Q_Q(StatJob);
     kDebug(7007) << m_url << "->" << url;
     m_redirectionURL = url; // We'll remember that when the job finishes
     // Tell the user that we haven't finished yet
     emit q->redirection(q, m_redirectionURL);
}

void StatJob::slotFinished()
{
    Q_D(StatJob);

    if (!d->m_redirectionURL.isEmpty() && d->m_redirectionURL.isValid()) {
        // kDebug(7007) << "StatJob: Redirection to " << m_redirectionURL;
        if (d->m_redirectionHandlingEnabled) {
            d->m_packedArgs.truncate(0);
            QDataStream stream(&d->m_packedArgs, QIODevice::WriteOnly);
            stream << d->m_redirectionURL;

            d->restartAfterRedirection(&d->m_redirectionURL);
            return;
        }
    }

    // Return slave to the scheduler
    SimpleJob::slotFinished();
}

StatJob* KIO::stat(const KUrl &url, JobFlags flags)
{
    // Assume sideIsSource. Gets are more common than puts.
    return stat(url, StatJob::SourceSide, 2, flags);
}

StatJob *KIO::mostLocalUrl(const KUrl &url, JobFlags flags)
{
    StatJob* job = stat(url, StatJob::SourceSide, 2, flags);
    if (url.isLocalFile()) {
        QTimer::singleShot(0, job, SLOT(slotFinished()));
        Scheduler::cancelJob(job); // deletes the slave if not 0
    }
    return job;
}


StatJob* KIO::stat(const KUrl &url, KIO::StatJob::StatSide side, short int details, JobFlags flags)
{
    //kDebug(7007) << "stat" << url;
    KIO_ARGS << url;
    StatJob* job = StatJobPrivate::newJob(url, CMD_STAT, packedArgs, flags);
    job->setSide(side);
    job->setDetails(details);
    return job;
}

//////////

TransferJob::TransferJob(TransferJobPrivate &dd)
    : SimpleJob(dd)
{
    Q_D(TransferJob);
    if (d->m_command == CMD_PUT) {
        d->m_extraFlags |= JobPrivate::EF_TransferJobDataSent;
    }
}

// Slave sends data
void TransferJob::slotData(const QByteArray &_data)
{
    Q_D(TransferJob);
    if (d->m_command == CMD_GET && !d->m_isMimetypeEmitted) {
        kWarning(7007) << "mimeType() not emitted when sending first data!; job URL ="
                       << d->m_url << "data size =" << _data.size();
    }
    // shut up the warning, HACK: downside is that it changes the meaning of the variable
    d->m_isMimetypeEmitted = true;

    if (d->m_redirectionURL.isEmpty() || !d->m_redirectionURL.isValid() || error()) {
        emit data(this, _data);
    }
}

void KIO::TransferJob::setTotalSize(KIO::filesize_t bytes)
{
    setTotalAmount(KJob::Bytes, bytes);
}

// Slave got a redirection request
void TransferJob::slotRedirection( const KUrl &url)
{
    Q_D(TransferJob);
    kDebug(7007) << url;

    // Some websites keep redirecting to themselves where each redirection
    // acts as the stage in a state-machine. We define "endless redirections"
    // as 5 redirections to the same URL.
    if (d->m_redirectionList.count(url) > 5) {
       kDebug(7007) << "CYCLIC REDIRECTION!";
       setError( ERR_CYCLIC_LINK );
       setErrorText( d->m_url.pathOrUrl() );
    } else {
       d->m_redirectionURL = url; // We'll remember that when the job finishes
       d->m_redirectionList.append(url);
       // Tell the user that we haven't finished yet
       emit redirection(this, d->m_redirectionURL);
    }
}

void TransferJob::slotFinished()
{
    Q_D(TransferJob);

    kDebug(7007) << d->m_url;
    if (!d->m_redirectionURL.isEmpty() && d->m_redirectionURL.isValid()) {
        // kDebug(7007) << "Redirection to" << m_redirectionURL;
        if (d->m_redirectionHandlingEnabled) {
            // Honour the redirection
            // We take the approach of "redirecting this same job"
            // Another solution would be to create a subjob, but the same problem
            // happens (unpacking+repacking)
            const QString redirectToGet = queryMetaData(QLatin1String("redirect-to-get"));
            if (redirectToGet == QLatin1String("true")) {
                d->m_command = CMD_GET;
                d->m_outgoingMetaData.remove(QLatin1String("content-type"));
            }
            d->m_incomingMetaData.clear();
            if (queryMetaData("cache") != "reload") {
                addMetaData("cache","refresh");
            }
            d->m_internalSuspended = false;
            // The very tricky part is the packed arguments business
            switch(d->m_command) {
                case CMD_GET:
                case CMD_STAT:
                case CMD_DEL: {
                    d->m_packedArgs.truncate(0);
                    QDataStream stream(&d->m_packedArgs, QIODevice::WriteOnly);
                    stream << d->m_redirectionURL;
                    break;
                }
                case CMD_PUT: {
                    int permissions;
                    qint8 iOverwrite, iResume;
                    KUrl dummyUrl;
                    QDataStream istream(d->m_packedArgs);
                    istream >> dummyUrl >> iOverwrite >> iResume >> permissions;
                    d->m_packedArgs.truncate(0);
                    QDataStream stream(&d->m_packedArgs, QIODevice::WriteOnly);
                    stream << d->m_redirectionURL << iOverwrite << iResume << permissions;
                    break;
                }
            }
            d->restartAfterRedirection(&d->m_redirectionURL);
            return;
        }
    }

    SimpleJob::slotFinished();
}

QString TransferJob::mimetype() const
{
    return d_func()->m_mimetype;
}

// Slave requests data
void TransferJob::slotDataReq()
{
    Q_D(TransferJob);
    QByteArray dataForSlave;

    emit dataReq(this, dataForSlave);

    static const int max_size = 14 * 1024 * 1024;
    if (dataForSlave.size() > max_size) {
        kWarning(7007) << "send " << dataForSlave.size() / 1024 / 1024 << "MB of data in TransferJob::dataReq. This needs to be splitted, which requires a copy.";
    }

    d->m_slave->send(MSG_DATA, dataForSlave);
    if (d->m_extraFlags & JobPrivate::EF_TransferJobDataSent) {
        // put job -> emit progress 
        KIO::filesize_t size = processedAmount(KJob::Bytes)+dataForSlave.size();
        setProcessedAmount(KJob::Bytes, size);
    }

    if (d->m_subJob) {
        // Bitburger protocol in action
        d->internalSuspend(); // Wait for more data from subJob.
        d->m_subJob->d_func()->internalResume(); // Ask for more!
    }
}

void TransferJob::slotMimetype(const QString &type)
{
    Q_D(TransferJob);
    d->m_mimetype = type;
    if (d->m_command == CMD_GET && d->m_isMimetypeEmitted) {
        kWarning(7007) << "mimetype() emitted again, or after sending first data!; job URL ="
                       << d->m_url;
    }
    d->m_isMimetypeEmitted = true;
    emit mimetype(this, type);
}


void TransferJobPrivate::internalSuspend()
{
    m_internalSuspended = true;
    if (m_slave) {
        m_slave->suspend();
    }
}

void TransferJobPrivate::internalResume()
{
    m_internalSuspended = false;
    if (m_slave && !suspended) {
        m_slave->resume();
    }
}

bool TransferJob::doResume()
{
    Q_D(TransferJob);
    if (!SimpleJob::doResume()) {
        return false;
    }
    if (d->m_internalSuspended) {
        d->internalSuspend();
    }
    return true;
}

void TransferJobPrivate::start(SlaveInterface *slave)
{
    Q_Q(TransferJob);
    Q_ASSERT(slave);
    JobPrivate::emitTransferring(q, m_url);
    q->connect(
        slave, SIGNAL(data(QByteArray)),
        SLOT(slotData(QByteArray))
    );

    q->connect(
        slave, SIGNAL(dataReq()),
        SLOT(slotDataReq())
    );

    q->connect(
        slave, SIGNAL(redirection(KUrl)),
        SLOT(slotRedirection(KUrl))
    );

    q->connect(
        slave, SIGNAL(mimeType(QString)),
        SLOT(slotMimetype(QString))
    );

    q->connect(
        slave, SIGNAL(canResume(KIO::filesize_t)),
        SLOT(slotCanResume(KIO::filesize_t))
    );

    if (slave->suspended()) {
       m_mimetype = "unknown";
       // WABA: The slave was put on hold. Resume operation.
       slave->resume();
    }

    SimpleJobPrivate::start(slave);
    if (m_internalSuspended) {
        slave->suspend();
    }
}

void TransferJobPrivate::slotCanResume(KIO::filesize_t offset)
{
    Q_Q(TransferJob);
    emit q->canResume(q, offset);
}

void TransferJob::slotResult(KJob *job)
{
    Q_D(TransferJob);
    // This can only be our subjob.
    Q_ASSERT(job == d->m_subJob);

   SimpleJob::slotResult(job);

   if (!error() && job == d->m_subJob) {
      d->m_subJob = 0; // No action required
      d->internalResume(); // Make sure we get the remaining data.
   }
}

void TransferJob::setModificationTime(const QDateTime &mtime)
{
    addMetaData("modified", mtime.toString(Qt::ISODate));
}

TransferJob* KIO::get(const KUrl &url, LoadType reload, JobFlags flags)
{
    // Send decoded path and encoded query
    KIO_ARGS << url;
    TransferJob* job = TransferJobPrivate::newJob(url, CMD_GET, packedArgs, flags);
    if (reload == Reload) {
        job->addMetaData("cache", "reload");
    }
    return job;
}

class KIO::StoredTransferJobPrivate: public TransferJobPrivate
{
public:
    StoredTransferJobPrivate(const KUrl &url, int command,
                             const QByteArray &packedArgs)
        : TransferJobPrivate(url, command, packedArgs),
          m_uploadOffset(0)
    {
    }

    QByteArray m_data;
    int m_uploadOffset;

    void slotStoredData(KIO::Job *job, const QByteArray &data);
    void slotStoredDataReq(KIO::Job *job, QByteArray &data);

    Q_DECLARE_PUBLIC(StoredTransferJob)

    static inline StoredTransferJob *newJob(const KUrl &url, int command,
                                            const QByteArray &packedArgs,
                                            JobFlags flags)
    {
        StoredTransferJob* job = new StoredTransferJob(
            *new StoredTransferJobPrivate(url, command, packedArgs)
        );
        job->setUiDelegate(new JobUiDelegate());
        if (!(flags & HideProgressInfo)) {
            KIO::getJobTracker()->registerJob(job);
        }
        return job;
    }
};

TransferJob *KIO::put(const KUrl& url, int permissions, JobFlags flags)
{
    KIO_ARGS << url << qint8((flags & Overwrite) ? 1 : 0 ) << qint8((flags & Resume) ? 1 : 0 ) << permissions;
    return TransferJobPrivate::newJob(url, CMD_PUT, packedArgs, flags);
}

//////////

StoredTransferJob::StoredTransferJob(StoredTransferJobPrivate &dd)
    : TransferJob(dd)
{
    connect(
        this, SIGNAL(data(KIO::Job*,QByteArray)),
        SLOT(slotStoredData(KIO::Job*,QByteArray))
    );
    connect(
        this, SIGNAL(dataReq(KIO::Job*,QByteArray&)),
        SLOT(slotStoredDataReq(KIO::Job*,QByteArray&))
    );
}

void StoredTransferJob::setData(const QByteArray &arr)
{
    Q_D(StoredTransferJob);
    Q_ASSERT(d->m_data.isNull()); // check that we're only called once
    Q_ASSERT(d->m_uploadOffset == 0); // no upload started yet
    d->m_data = arr;
    setTotalSize(d->m_data.size());
}

QByteArray StoredTransferJob::data() const
{
    return d_func()->m_data;
}

void StoredTransferJobPrivate::slotStoredData(KIO::Job *, const QByteArray &data)
{
    // check for end-of-data marker:
    if (data.size() == 0) {
        return;
    }
    m_data.append(data.constData(), data.size());
}

void StoredTransferJobPrivate::slotStoredDataReq(KIO::Job *, QByteArray &data)
{
    // Inspired from kmail's KMKernel::byteArrayToRemoteFile
    // send the data in 64 KB chunks
    const int MAX_CHUNK_SIZE = (64 * 1024);
    int remainingBytes = (m_data.size() - m_uploadOffset);
    if (remainingBytes > MAX_CHUNK_SIZE) {
        // send MAX_CHUNK_SIZE bytes to the receiver (deep copy)
        data = QByteArray(m_data.data() + m_uploadOffset, MAX_CHUNK_SIZE);
        m_uploadOffset += MAX_CHUNK_SIZE;
        // kDebug() << "Sending " << MAX_CHUNK_SIZE << " bytes ("
        //          << remainingBytes - MAX_CHUNK_SIZE << " bytes remain)\n";
    } else {
        // send the remaining bytes to the receiver (deep copy)
        data = QByteArray(m_data.data() + m_uploadOffset, remainingBytes);
        m_data = QByteArray();
        m_uploadOffset = 0;
        //kDebug() << "Sending " << remainingBytes << " bytes\n";
    }
}

StoredTransferJob *KIO::storedGet(const KUrl &url, LoadType reload, JobFlags flags)
{
    // Send decoded path and encoded query
    KIO_ARGS << url;
    StoredTransferJob * job = StoredTransferJobPrivate::newJob(url, CMD_GET, packedArgs, flags);
    if (reload == KIO::Reload) {
        job->addMetaData("cache", "reload");
    }
    return job;
}

StoredTransferJob *KIO::storedPut(const QByteArray &arr, const KUrl &url, int permissions,
                                  JobFlags flags)
{
    KIO_ARGS << url << qint8( (flags & Overwrite) ? 1 : 0 ) << qint8((flags & Resume) ? 1 : 0) << permissions;
    StoredTransferJob* job = StoredTransferJobPrivate::newJob(url, CMD_PUT, packedArgs, flags);
    job->setData(arr);
    return job;
}

//////////

class KIO::MimetypeJobPrivate: public KIO::TransferJobPrivate
{
public:
    MimetypeJobPrivate(const KUrl &url, int command, const QByteArray &packedArgs)
        : TransferJobPrivate(url, command, packedArgs)
    {
    }

    Q_DECLARE_PUBLIC(MimetypeJob)

    static inline MimetypeJob *newJob(const KUrl &url, int command, const QByteArray &packedArgs,
                                      JobFlags flags)
    {
        MimetypeJob *job = new MimetypeJob(*new MimetypeJobPrivate(url, command, packedArgs));
        job->setUiDelegate(new JobUiDelegate());
        if (!(flags & HideProgressInfo)) {
            KIO::getJobTracker()->registerJob(job);
            emitStating(job, url);
        }
        return job;
    }
};

MimetypeJob::MimetypeJob(MimetypeJobPrivate &dd)
    : TransferJob(dd)
{
}

void MimetypeJob::slotFinished()
{
    Q_D(MimetypeJob);
    // kDebug(7007);
    if (error() == KIO::ERR_IS_DIRECTORY) {
        // It is in fact a directory. This happens when HTTP redirects to FTP.
        // Due to the "protocol doesn't support listing" code in KRun, we
        // assumed it was a file.
        kDebug(7007) << "It is in fact a directory!";
        d->m_mimetype = QString::fromLatin1("inode/directory");
        emit TransferJob::mimetype(this, d->m_mimetype);
        setError(0);
    }

    if (!d->m_redirectionURL.isEmpty() && d->m_redirectionURL.isValid() && !error()) {
        //kDebug(7007) << "Redirection to " << m_redirectionURL;
        if (d->m_redirectionHandlingEnabled) {
            d->m_internalSuspended = false;
            d->m_packedArgs.truncate(0);
            QDataStream stream(&d->m_packedArgs, QIODevice::WriteOnly);
            stream << d->m_redirectionURL;

            d->restartAfterRedirection(&d->m_redirectionURL);
            return;
        }
    }

    // Return slave to the scheduler
    TransferJob::slotFinished();
}

MimetypeJob *KIO::mimetype(const KUrl &url, JobFlags flags)
{
    KIO_ARGS << url;
    return MimetypeJobPrivate::newJob(url, CMD_MIMETYPE, packedArgs, flags);
}

//////////////////////////

class KIO::DirectCopyJobPrivate: public KIO::SimpleJobPrivate
{
public:
    DirectCopyJobPrivate(const KUrl &url, int command, const QByteArray &packedArgs)
        : SimpleJobPrivate(url, command, packedArgs)
    {
    }

    /**
     * @internal
     * Called by the scheduler when a @p slave gets to
     * work on this job.
     * @param slave the slave that starts working on this job
     */
    virtual void start(SlaveInterface *slave);

    Q_DECLARE_PUBLIC(DirectCopyJob)
};

DirectCopyJob::DirectCopyJob(const KUrl &url, const QByteArray &packedArgs)
    : SimpleJob(*new DirectCopyJobPrivate(url, CMD_COPY, packedArgs))
{
    setUiDelegate(new JobUiDelegate());
}

void DirectCopyJobPrivate::start(SlaveInterface *slave)
{
    Q_Q(DirectCopyJob);
    q->connect(
        slave, SIGNAL(canResume(KIO::filesize_t)),
        SLOT(slotCanResume(KIO::filesize_t))
    );
    SimpleJobPrivate::start(slave);
}

void DirectCopyJob::slotCanResume(KIO::filesize_t offset)
{
    emit canResume(this, offset);
}

//////////////////////////

/** @internal */
class KIO::FileCopyJobPrivate: public KIO::JobPrivate
{
public:
    FileCopyJobPrivate(const KUrl &src, const KUrl &dest, int permissions,
                       bool move, JobFlags flags)
        : m_sourceSize(filesize_t(-1)), m_src(src), m_dest(dest),
        m_moveJob(nullptr), m_copyJob(nullptr), m_delJob(nullptr),
        m_chmodJob(nullptr), m_getJob(nullptr), m_putJob(nullptr),
        m_permissions(permissions),
        m_move(move), m_canResume(false), m_resumeAnswerSent(false), m_mustChmod(false),
        m_flags(flags)
    {
    }

    KIO::filesize_t m_sourceSize;
    QDateTime m_modificationTime;
    KUrl m_src;
    KUrl m_dest;
    QByteArray m_buffer;
    SimpleJob *m_moveJob;
    SimpleJob *m_copyJob;
    SimpleJob *m_delJob;
    SimpleJob *m_chmodJob;
    TransferJob *m_getJob;
    TransferJob *m_putJob;
    int m_permissions;
    bool m_move;
    bool m_canResume;
    bool m_resumeAnswerSent;
    bool m_mustChmod;
    JobFlags m_flags;

    void startBestCopyMethod();
    void startCopyJob();
    void startCopyJob(const KUrl &slave_url);
    void startRenameJob(const KUrl &slave_url);
    void startDataPump();
    void connectSubjob(SimpleJob *job);

    void slotStart();
    void slotData(KIO::Job *, const QByteArray &data);
    void slotDataReq(KIO::Job *, QByteArray &data);
    void slotMimetype(KIO::Job*, const QString &type);
    /**
     * Forward signal from subjob
     * @param job the job that emitted this signal
     * @param size the processed size in bytes
     */
    void slotProcessedSize(KJob *job, qulonglong size);
    /**
     * Forward signal from subjob
     * @param job the job that emitted this signal
     * @param size the total size
     */
    void slotTotalSize(KJob *job, qulonglong size);
    /**
     * Forward signal from subjob
     * @param job the job that emitted this signal
     * @param pct the percentage
     */
    void slotPercent(KJob *job, unsigned long pct);
    /**
     * Forward signal from subjob
     * @param job the job that emitted this signal
     * @param offset the offset to resume from
     */
    void slotCanResume(KIO::Job *job, KIO::filesize_t offset);

    Q_DECLARE_PUBLIC(FileCopyJob)

    static inline FileCopyJob* newJob(const KUrl& src, const KUrl& dest, int permissions, bool move,
                                      JobFlags flags)
    {
        //kDebug(7007) << src << "->" << dest;
        FileCopyJob *job = new FileCopyJob(
            *new FileCopyJobPrivate(src, dest, permissions, move, flags)
        );
        job->setProperty("destUrl", dest.url());
        job->setUiDelegate(new JobUiDelegate());
        if (!(flags & HideProgressInfo)) {
            KIO::getJobTracker()->registerJob(job);
        }
        return job;
    }
};

/*
 * The FileCopyJob works according to the famous Bavarian
 * 'Alternating Bitburger Protocol': we either drink a beer or we
 * we order a beer, but never both at the same time.
 * Translated to io-slaves: We alternate between receiving a block of data
 * and sending it away.
 */
FileCopyJob::FileCopyJob(FileCopyJobPrivate &dd)
    : Job(dd)
{
    // kDebug(7007);
    QTimer::singleShot(0, this, SLOT(slotStart()));
}

void FileCopyJobPrivate::slotStart()
{
    Q_Q(FileCopyJob);
    if (!m_move) {
        JobPrivate::emitCopying(q, m_src, m_dest);
    } else {
        JobPrivate::emitMoving(q, m_src, m_dest);
    }

    if (m_move) {
        // The if() below must be the same as the one in startBestCopyMethod
        if ((m_src.protocol() == m_dest.protocol()) &&
            (m_src.host() == m_dest.host()) &&
            (m_src.port() == m_dest.port()) &&
            (m_src.userName() == m_dest.userName()) &&
            (m_src.password() == m_dest.password()))
        {
            startRenameJob(m_src);
            return;
        } else if (m_src.isLocalFile() && KProtocolManager::canRenameFromFile(m_dest)) {
            startRenameJob(m_dest);
            return;
        } else if (m_dest.isLocalFile() && KProtocolManager::canRenameToFile(m_src)) {
            startRenameJob(m_src);
            return;
        }
        // No fast-move available, use copy + del.
    }
    startBestCopyMethod();
}

void FileCopyJobPrivate::startBestCopyMethod()
{
    if ((m_src.protocol() == m_dest.protocol()) &&
        (m_src.host() == m_dest.host()) &&
        (m_src.port() == m_dest.port()) &&
        (m_src.userName() == m_dest.userName()) &&
        (m_src.password() == m_dest.password()))
    {
        startCopyJob();
    } else if (m_src.isLocalFile() && KProtocolManager::canCopyFromFile(m_dest)) {
        startCopyJob(m_dest);
    } else if (m_dest.isLocalFile() && KProtocolManager::canCopyToFile(m_src)) {
        startCopyJob(m_src);
    } else {
        startDataPump();
    }
}

void FileCopyJob::setSourceSize(KIO::filesize_t size)
{
    Q_D(FileCopyJob);
    d->m_sourceSize = size;
    if (size != (KIO::filesize_t) -1) {
        setTotalAmount(KJob::Bytes, size);
    }
}

void FileCopyJob::setModificationTime(const QDateTime &mtime)
{
    Q_D(FileCopyJob);
    d->m_modificationTime = mtime;
}

KUrl FileCopyJob::srcUrl() const
{
    return d_func()->m_src;
}

KUrl FileCopyJob::destUrl() const
{
    return d_func()->m_dest;
}

void FileCopyJobPrivate::startCopyJob()
{
    startCopyJob(m_src);
}

void FileCopyJobPrivate::startCopyJob(const KUrl &slave_url)
{
    Q_Q(FileCopyJob);
    // kDebug(7007);
    KIO_ARGS << m_src << m_dest << m_permissions << (qint8)(m_flags & Overwrite);
    m_copyJob = new DirectCopyJob(slave_url, packedArgs);
    if (m_modificationTime.isValid()) {
        m_copyJob->addMetaData("modified", m_modificationTime.toString(Qt::ISODate )); // #55804
    }
    q->addSubjob(m_copyJob);
    connectSubjob(m_copyJob);
    q->connect(
        m_copyJob, SIGNAL(canResume(KIO::Job*,KIO::filesize_t)),
        SLOT(slotCanResume(KIO::Job*,KIO::filesize_t))
    );
}

void FileCopyJobPrivate::startRenameJob(const KUrl &slave_url)
{
    Q_Q(FileCopyJob);
    m_mustChmod = true; // CMD_RENAME by itself doesn't change permissions
    KIO_ARGS << m_src << m_dest << (qint8)(m_flags & Overwrite);
    m_moveJob = SimpleJobPrivate::newJobNoUi(slave_url, CMD_RENAME, packedArgs);
    if (m_modificationTime.isValid()) {
        m_moveJob->addMetaData("modified", m_modificationTime.toString(Qt::ISODate)); // #55804
    }
    q->addSubjob(m_moveJob);
    connectSubjob(m_moveJob);
}

void FileCopyJobPrivate::connectSubjob(SimpleJob *job)
{
    Q_Q(FileCopyJob);
    q->connect(
        job, SIGNAL(totalSize(KJob*,qulonglong)),
        SLOT(slotTotalSize(KJob*,qulonglong))
    );

    q->connect(
        job, SIGNAL(processedSize(KJob*,qulonglong)),
        SLOT(slotProcessedSize(KJob*,qulonglong))
    );

    q->connect(
        job, SIGNAL(percent(KJob*,ulong)),
        SLOT(slotPercent(KJob*,ulong))
    );
}

bool FileCopyJob::doSuspend()
{
    Q_D(FileCopyJob);
    if (d->m_moveJob) {
        d->m_moveJob->suspend();
    }

    if (d->m_copyJob) {
        d->m_copyJob->suspend();
    }

    if (d->m_getJob) {
        d->m_getJob->suspend();
    }

    if (d->m_putJob) {
        d->m_putJob->suspend();
    }

    Job::doSuspend();
    return true;
}

bool FileCopyJob::doResume()
{
    Q_D(FileCopyJob);
    if (d->m_moveJob) {
        d->m_moveJob->resume();
    }

    if (d->m_copyJob) {
        d->m_copyJob->resume();
    }

    if (d->m_getJob) {
        d->m_getJob->resume();
    }

    if (d->m_putJob) {
        d->m_putJob->resume();
    }

    Job::doResume();
    return true;
}

void FileCopyJobPrivate::slotProcessedSize(KJob *, qulonglong size)
{
    Q_Q(FileCopyJob);
    q->setProcessedAmount(KJob::Bytes, size);
}

void FileCopyJobPrivate::slotTotalSize(KJob *, qulonglong size)
{
    Q_Q(FileCopyJob);
    if (size != q->totalAmount(KJob::Bytes)) {
        q->setTotalAmount(KJob::Bytes, size);
    }
}

void FileCopyJobPrivate::slotPercent(KJob *, unsigned long pct)
{
    Q_Q(FileCopyJob);
    if (pct > q->percent()) {
        q->setPercent(pct);
    }
}

void FileCopyJobPrivate::startDataPump()
{
    Q_Q(FileCopyJob);
    // kDebug(7007);

    m_canResume = false;
    m_resumeAnswerSent = false;
    m_getJob = 0L; // for now
    m_putJob = put( m_dest, m_permissions, (m_flags | HideProgressInfo) /* no GUI */);
    // kDebug(7007) << "m_putJob=" << m_putJob << "m_dest=" << m_dest;
    if (m_modificationTime.isValid()) {
        m_putJob->setModificationTime(m_modificationTime);
    }

    // The first thing the put job will tell us is whether we can
    // resume or not (this is always emitted)
    q->connect(
        m_putJob, SIGNAL(canResume(KIO::Job*,KIO::filesize_t)),
        SLOT(slotCanResume(KIO::Job*,KIO::filesize_t))
    );
    q->connect(
        m_putJob, SIGNAL(dataReq(KIO::Job*,QByteArray&)),
        SLOT(slotDataReq(KIO::Job*,QByteArray&))
    );
    q->addSubjob(m_putJob);
}

void FileCopyJobPrivate::slotCanResume(KIO::Job *job, KIO::filesize_t offset)
{
    Q_Q(FileCopyJob);
    if (job == m_putJob || job == m_copyJob) {
        // kDebug(7007) << "'can resume' from PUT job. offset=" << KIO::number(offset);
        if (offset) {
            RenameDialog_Result res = R_RESUME;

            if (!KProtocolManager::autoResume() && !(m_flags & Overwrite)) {
                QString newPath;
                KIO::Job* job = (q->parentJob() ? q->parentJob() : q);
                // Ask confirmation about resuming previous transfer
                res = ui()->askFileRename(
                      job, i18n("File Already Exists"),
                      m_src.url(),
                      m_dest.url(),
                      (RenameDialog_Mode) (M_OVERWRITE | M_RESUME | M_NORENAME), newPath,
                      m_sourceSize, offset
                );
            }

            if (res == R_OVERWRITE || (m_flags & Overwrite)) {
                offset = 0;
            } else if (res == R_CANCEL) {
                if (job == m_putJob) {
                    m_putJob->kill(FileCopyJob::Quietly);
                    q->removeSubjob(m_putJob);
                    m_putJob = nullptr;
                } else {
                    m_copyJob->kill(FileCopyJob::Quietly);
                    q->removeSubjob(m_copyJob);
                    m_copyJob = nullptr;
                }
                q->setError(ERR_USER_CANCELED);
                q->emitResult();
                return;
            }
        } else {
            // No need for an answer
            m_resumeAnswerSent = true;
        }

        if (job == m_putJob) {
            m_getJob = KIO::get(m_src, NoReload, HideProgressInfo);
            // kDebug(7007) << "m_getJob=" << m_getJob << m_src;
            // Set size in subjob. This helps if the slave doesn't emit totalSize.
            if (m_sourceSize != (KIO::filesize_t)-1) {
                m_getJob->setTotalAmount(KJob::Bytes, m_sourceSize);
            }
            if (offset) {
                //kDebug(7007) << "Setting metadata for resume to" << (unsigned long) offset;
                // TODO KDE4: rename to seek or offset and document it
                // This isn't used only for resuming, but potentially also for extracting (#72302).
                m_getJob->addMetaData("resume", KIO::number(offset));

                // Might or might not get emitted
                q->connect(
                    m_getJob, SIGNAL(canResume(KIO::Job*,KIO::filesize_t)),
                    SLOT(slotCanResume(KIO::Job*,KIO::filesize_t))
                );
            }
            jobSlave(m_putJob)->setOffset(offset);

            m_putJob->d_func()->internalSuspend();
            q->addSubjob(m_getJob);
            connectSubjob(m_getJob); // Progress info depends on get
            m_getJob->d_func()->internalResume(); // Order a beer

            q->connect(
                m_getJob, SIGNAL(data(KIO::Job*,QByteArray)),
                SLOT(slotData(KIO::Job*,QByteArray))
            );
            q->connect(
                m_getJob, SIGNAL(mimetype(KIO::Job*,QString)),
                SLOT(slotMimetype(KIO::Job*,QString))
            );
        } else  {
            // copyjob
            jobSlave(m_copyJob)->sendResumeAnswer(offset != 0);
        }
    } else if (job == m_getJob) {
        // Cool, the get job said ok, we can resume
        m_canResume = true;
        // kDebug(7007) << "'can resume' from the GET job -> we can resume";

        jobSlave(m_getJob)->setOffset(jobSlave(m_putJob)->offset());
    } else {
        kWarning(7007) << "unknown job=" << job
                       << "m_getJob=" << m_getJob << "m_putJob=" << m_putJob;
    }
}

void FileCopyJobPrivate::slotData(KIO::Job *, const QByteArray &data)
{
    // kDebug(7007) << "data size:" << data.size();
    Q_ASSERT(m_putJob);
    if (!m_putJob) {
        // Don't crash
        return;
    }
    m_getJob->d_func()->internalSuspend();
    m_putJob->d_func()->internalResume(); // Drink the beer
    m_buffer += data;

    // On the first set of data incoming, we tell the "put" slave about our
    // decision about resuming
    if (!m_resumeAnswerSent) {
        m_resumeAnswerSent = true;
        // kDebug(7007) << "(first time) -> send resume answer " << m_canResume;
        jobSlave(m_putJob)->sendResumeAnswer( m_canResume );
    }
}

void FileCopyJobPrivate::slotDataReq(KIO::Job * , QByteArray &data)
{
    Q_Q(FileCopyJob);
    // kDebug(7007);
    if (!m_resumeAnswerSent && !m_getJob) {
        // This can't happen
        q->setError(ERR_INTERNAL);
        q->setErrorText("'Put' job did not send canResume or 'Get' job did not send data!");
        m_putJob->kill(FileCopyJob::Quietly);
        q->removeSubjob(m_putJob);
        m_putJob = nullptr;
        q->emitResult();
        return;
    }
    if (m_getJob) {
        m_getJob->d_func()->internalResume(); // Order more beer
        m_putJob->d_func()->internalSuspend();
    }
    data = m_buffer;
    m_buffer = QByteArray();
}

void FileCopyJobPrivate::slotMimetype(KIO::Job *, const QString &type)
{
    Q_Q(FileCopyJob);
    emit q->mimetype(q, type);
}

void FileCopyJob::slotResult(KJob *job)
{
    Q_D(FileCopyJob);
    // kDebug(7007) << "this=" << this << "job=" << job;
    removeSubjob(job);
    // Did job have an error ?
    if (job->error()) {
        if (job == d->m_moveJob && job->error() == ERR_UNSUPPORTED_ACTION) {
            d->m_moveJob = nullptr;
            d->startBestCopyMethod();
            return;
        } else if (job == d->m_copyJob && job->error() == ERR_UNSUPPORTED_ACTION) {
            d->m_copyJob = nullptr;
            d->startDataPump();
            return;
        } else if (job == d->m_getJob) {
            d->m_getJob = nullptr;
            if (d->m_putJob) {
                d->m_putJob->kill(Quietly);
                removeSubjob(d->m_putJob);
            }
        } else if (job == d->m_putJob) {
            d->m_putJob = nullptr;
            if (d->m_getJob) {
                d->m_getJob->kill(Quietly);
                removeSubjob(d->m_getJob);
            }
        }
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
   }

   if (d->m_mustChmod) {
       // If d->m_permissions == -1, keep the default permissions
       if (d->m_permissions != -1) {
           d->m_chmodJob = chmod(d->m_dest, d->m_permissions);
       }
       d->m_mustChmod = false;
   }

    if (job == d->m_moveJob) {
         // Finished
        d->m_moveJob = nullptr;
    }

    if (job == d->m_copyJob) {
        d->m_copyJob = nullptr;
        if (d->m_move) {
            d->m_delJob = file_delete( d->m_src, HideProgressInfo); // Delete source
            addSubjob(d->m_delJob);
        }
    }

    if (job == d->m_getJob) {
        // kDebug(7007) << "m_getJob finished";
        d->m_getJob = nullptr; // No action required
        if (d->m_putJob) {
            d->m_putJob->d_func()->internalResume();
        }
    }

    if (job == d->m_putJob) {
        // kDebug(7007) << "m_putJob finished";
        d->m_putJob = nullptr;
        if (d->m_getJob) {
            // The get job is still running, probably after emitting data(QByteArray())
            // and before we receive its finished().
            d->m_getJob->d_func()->internalResume();
        }
        if (d->m_move) {
            d->m_delJob = file_delete(d->m_src, HideProgressInfo); // Delete source
            addSubjob(d->m_delJob);
        }
   }

    if (job == d->m_delJob) {
        // Finished
        d->m_delJob = nullptr;
    }

    if (job == d->m_chmodJob) {
        // Finished
        d->m_chmodJob = nullptr;
    }

    if (!hasSubjobs()) {
        emitResult();
    }
}

FileCopyJob *KIO::file_copy(const KUrl &src, const KUrl &dest, int permissions,
                            JobFlags flags)
{
    return FileCopyJobPrivate::newJob(src, dest, permissions, false, flags);
}

FileCopyJob *KIO::file_move(const KUrl &src, const KUrl &dest, int permissions,
                            JobFlags flags)
{
    FileCopyJob* job = FileCopyJobPrivate::newJob(src, dest, permissions, true, flags);
    ClipboardUpdater::create(job, ClipboardUpdater::UpdateContent);
    return job;
}

SimpleJob *KIO::file_delete( const KUrl& src, JobFlags flags )
{
    KIO_ARGS << src << qint8(true); // isFile
    SimpleJob* job = SimpleJobPrivate::newJob(src, CMD_DEL, packedArgs, flags);
    ClipboardUpdater::create(job, ClipboardUpdater::RemoveContent);
    return job;
}

//////////

class KIO::ListJobPrivate: public KIO::SimpleJobPrivate
{
public:
    ListJobPrivate(const KUrl& url, bool _recursive,
                   const QString &prefix, const QString &displayPrefix,
                   bool _includeHidden)
        : SimpleJobPrivate(url, CMD_LISTDIR, QByteArray()),
        recursive(_recursive), includeHidden(_includeHidden),
        m_prefix(prefix), m_displayPrefix(displayPrefix), m_processedEntries(0)
    {
    }

    bool recursive;
    bool includeHidden;
    QString m_prefix;
    QString m_displayPrefix;
    unsigned long m_processedEntries;
    KUrl m_redirectionURL;

    /**
     * @internal
     * Called by the scheduler when a @p slave gets to
     * work on this job.
     * @param slave the slave that starts working on this job
     */
    virtual void start(SlaveInterface *slave);

    void slotListEntries(const KIO::UDSEntryList &list);
    void slotRedirection(const KUrl &url);
    void gotEntries(KIO::Job * subjob, const KIO::UDSEntryList &list);

    Q_DECLARE_PUBLIC(ListJob)

    static inline ListJob *newJob(const KUrl &u, bool _recursive,
                                  const QString &prefix, const QString &displayPrefix,
                                  bool _includeHidden, JobFlags flags = HideProgressInfo)
    {
        ListJob *job = new ListJob(*new ListJobPrivate(u, _recursive, prefix, displayPrefix, _includeHidden));
        job->setUiDelegate(new JobUiDelegate());
        if (!(flags & HideProgressInfo)) {
            KIO::getJobTracker()->registerJob(job);
        }
        return job;
    }
    static inline ListJob *newJobNoUi(const KUrl &u, bool _recursive,
                                      const QString &prefix, const QString &displayPrefix,
                                      bool _includeHidden)
    {
        return new ListJob(*new ListJobPrivate(u, _recursive, prefix, displayPrefix, _includeHidden));
    }
};

ListJob::ListJob(ListJobPrivate &dd)
    : SimpleJob(dd)
{
    Q_D(ListJob);
    // We couldn't set the args when calling the parent constructor,
    // so do it now.
    QDataStream stream(&d->m_packedArgs, QIODevice::WriteOnly);
    stream << d->m_url;
}

void ListJobPrivate::slotListEntries(const KIO::UDSEntryList &list)
{
    Q_Q(ListJob);
    // Emit progress info (takes care of emit processedSize and percent)
    m_processedEntries += list.count();
    slotProcessedSize(m_processedEntries);

    if (recursive) {
        UDSEntryList::ConstIterator it = list.begin();
        const UDSEntryList::ConstIterator end = list.end();

        for (; it != end; ++it) {
            const UDSEntry &entry = *it;

            KUrl itemURL;
            // const UDSEntry::ConstIterator end2 = entry.end();
            // UDSEntry::ConstIterator it2 = entry.find(KIO::UDSEntry::UDS_URL);
            // if ( it2 != end2 )
            if (entry.contains(KIO::UDSEntry::UDS_URL)) {
                // itemURL = it2.value().toString();
                itemURL = entry.stringValue(KIO::UDSEntry::UDS_URL);
            } else {
                // no URL, use the name
                itemURL = q->url();
                const QString fileName = entry.stringValue(KIO::UDSEntry::UDS_NAME);
                Q_ASSERT(!fileName.isEmpty()); // we'll recurse forever otherwise :)
                itemURL.addPath(fileName);
            }

            if (entry.isDir() && !entry.isLink()) {
                const QString filename = itemURL.fileName();
                QString displayName = entry.stringValue(KIO::UDSEntry::UDS_DISPLAY_NAME);
                if (displayName.isEmpty()) {
                    displayName = filename;
                }
                // skip hidden dirs when listing if requested
                if (filename != ".." && filename != "." && (includeHidden || filename[0] != '.')) {
                    ListJob *job = ListJobPrivate::newJobNoUi(
                        itemURL,
                        true /*recursive*/,
                        m_prefix + filename + '/',
                        m_displayPrefix + displayName + '/',
                        includeHidden
                    );
                    Scheduler::setJobPriority(job, 1);
                    q->connect(
                        job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)),
                        SLOT(gotEntries(KIO::Job*,KIO::UDSEntryList))
                    );
                    q->addSubjob(job);
                }
            }
        }
    }

    // Not recursive, or top-level of recursive listing : return now (send . and .. as well)
    // exclusion of hidden files also requires the full sweep, but the case for full-listing
    // a single dir is probably common enough to justify the shortcut
    if (m_prefix.isNull() && includeHidden) {
        emit q->entries(q, list);
    } else {
        // cull the unwanted hidden dirs and/or parent dir references from the listing, then emit that
        UDSEntryList newlist;

        foreach (KIO::UDSEntry entry, list) {
            const QString filename = entry.stringValue(KIO::UDSEntry::UDS_NAME);
            QString displayName = entry.stringValue(KIO::UDSEntry::UDS_DISPLAY_NAME);
            if (displayName.isEmpty()) {
                displayName = filename;
            }
            // Avoid returning entries like subdir/. and subdir/.., but include . and .. for
            // the toplevel dir, and skip hidden files/dirs if that was requested
            if ((m_prefix.isNull() || (filename != ".." && filename != "."))
                && (includeHidden || (filename[0] != '.')))
            {
                // ## Didn't find a way to use the iterator instead of re-doing a key lookup
                entry.insert(KIO::UDSEntry::UDS_NAME, m_prefix + filename);
                entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, m_displayPrefix + displayName);
                newlist.append(entry);
            }
        }

        emit q->entries(q, newlist);
    }
}

void ListJobPrivate::gotEntries(KIO::Job *, const KIO::UDSEntryList &list)
{
    // Forward entries received by subjob - faking we received them ourselves
    Q_Q(ListJob);
    emit q->entries(q, list);
}

void ListJob::slotResult(KJob *job)
{
    if (job->error()) {
        // If we can't list a subdir, the result is still ok
        // This is why we override KCompositeJob::slotResult - to not set
        // an error on parent job.
        // Let's emit a signal about this though
        emit subError(this, static_cast<KIO::ListJob*>(job));
    }
    removeSubjob(job);
    if (!hasSubjobs()) {
        emitResult();
    }
}

void ListJobPrivate::slotRedirection(const KUrl &url)
{
    Q_Q(ListJob);
    m_redirectionURL = url; // We'll remember that when the job finishes
    emit q->redirection(q, m_redirectionURL);
}

void ListJob::slotFinished()
{
    Q_D(ListJob);

    if (!d->m_redirectionURL.isEmpty() && d->m_redirectionURL.isValid() && !error()) {
        // kDebug(7007) << "Redirection to " << d->m_redirectionURL;
        if ( d->m_redirectionHandlingEnabled) {
            d->m_packedArgs.truncate(0);
            QDataStream stream( &d->m_packedArgs, QIODevice::WriteOnly);
            stream << d->m_redirectionURL;

            d->restartAfterRedirection(&d->m_redirectionURL);
            return;
        }
    }

    // Return slave to the scheduler
    SimpleJob::slotFinished();
}

ListJob *KIO::listDir(const KUrl &url, JobFlags flags, bool includeHidden)
{
    return ListJobPrivate::newJob(url, false, QString(), QString(), includeHidden, flags);
}

ListJob *KIO::listRecursive(const KUrl &url, JobFlags flags, bool includeHidden)
{
    return ListJobPrivate::newJob(url, true, QString(), QString(), includeHidden, flags);
}

void ListJobPrivate::start(SlaveInterface *slave)
{
    Q_Q(ListJob);
    q->connect(
        slave, SIGNAL(listEntries(KIO::UDSEntryList)),
        SLOT(slotListEntries(KIO::UDSEntryList))
    );
    q->connect(
        slave, SIGNAL(totalSize(KIO::filesize_t)),
        SLOT(slotTotalSize(KIO::filesize_t))
    );
    q->connect(
        slave, SIGNAL(redirection(KUrl)),
        SLOT(slotRedirection(KUrl))
    );

    SimpleJobPrivate::start(slave);
}

const KUrl& ListJob::redirectionUrl() const
{
    return d_func()->m_redirectionURL;
}

//
class KIO::SpecialJobPrivate: public TransferJobPrivate
{
    SpecialJobPrivate(const KUrl &url, int command, const QByteArray &packedArgs)
        : TransferJobPrivate(url, command, packedArgs)
    {
    }
};

SpecialJob::SpecialJob(const KUrl &url, const QByteArray &packedArgs)
    : TransferJob(*new TransferJobPrivate(url, CMD_SPECIAL, packedArgs))
{
}

void SpecialJob::setArguments(const QByteArray &data)
{
    Q_D(SpecialJob);
    d->m_packedArgs = data;
}

QByteArray SpecialJob::arguments() const
{
    return d_func()->m_packedArgs;
}

#include "moc_jobclasses.cpp"
#include "moc_job_p.cpp"
