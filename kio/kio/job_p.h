/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                  2000-2009 David Faure <faure@kde.org>
                       Waldo Bastian <bastian@kde.org>
    Copyright (C) 2007 Thiago Macieira <thiago@kde.org>
    Copyright (C) 2013 Dawit Alemayehu <adawit@kde.org>

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

#ifndef KIO_JOB_P_H
#define KIO_JOB_P_H

#include "job.h"
#include "kcompositejob_p.h"
#include "jobuidelegate.h"
#include "kjobtrackerinterface.h"

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( &packedArgs, QIODevice::WriteOnly ); stream

namespace KIO {
    class SlaveInterface;

    /**
     * Commands that can be invoked by a job.
     * @note protocol-specific commands shouldn't be added here, but should use special.
     */
    enum Command {
        CMD_HOST = '0', // 48
        CMD_NONE = '1',
        CMD_GET = '2',
        CMD_PUT = 'A',
        CMD_STAT = 'B',
        CMD_MIMETYPE = 'C',
        CMD_LISTDIR = 'D',
        CMD_MKDIR = 'E',
        CMD_RENAME = 'F',
        CMD_COPY = 'G',
        CMD_DEL = 'H',
        CMD_CHMOD = 'I',
        CMD_SPECIAL = 'J',
        CMD_SETMODIFICATIONTIME = 'K',
        CMD_REPARSECONFIGURATION = 'L',
        CMD_META_DATA = 'M',
        CMD_SYMLINK = 'N',
        CMD_MESSAGEBOXANSWER = 'O',
        CMD_RESUMEANSWER = 'P',
        CMD_CONFIG = 'Q',
        CMD_CHOWN = 'R'
    };

    class JobPrivate: public KCompositeJobPrivate
    {
    public:
        JobPrivate()
            : m_parentJob( 0L ), m_extraFlags(0)
            {}

        /**
         * Some extra storage space for jobs that don't have their own
         * private d pointer.
         */
        enum {
            EF_TransferJobDataSent = (1 << 0),
            EF_KillCalled          = (1 << 1)
        };

        // Maybe we could use the QObject parent/child mechanism instead
        // (requires a new ctor, and moving the ctor code to some init()).
        Job* m_parentJob;
        int m_extraFlags;
        MetaData m_incomingMetaData;
        MetaData m_outgoingMetaData;

        inline KIO::JobUiDelegate *ui() const
            { return static_cast<KIO::JobUiDelegate *>(uiDelegate); }

        void slotSpeed( KJob *job, unsigned long speed );

        static void emitMoving(KIO::Job*, const KUrl &src, const KUrl &dest);
        static void emitCopying(KIO::Job*, const KUrl &src, const KUrl &dest);
        static void emitCreatingDir(KIO::Job*, const KUrl &dir);
        static void emitDeleting(KIO::Job*, const KUrl &url);
        static void emitStating(KIO::Job*, const KUrl &url);
        static void emitTransferring(KIO::Job*, const KUrl &url);

        Q_DECLARE_PUBLIC(Job)
    };

    class SimpleJobPrivate: public JobPrivate
    {
    public:
        /**
         * Creates a new simple job.
         * @param url the url of the job
         * @param command the command of the job
         * @param packedArgs the arguments
         */
        SimpleJobPrivate(const KUrl& url, int command, const QByteArray &packedArgs)
            : m_slave(0), m_packedArgs(packedArgs), m_url(url), m_command(command),
              m_schedSerial(0), m_redirectionHandlingEnabled(true)
        {
        }

        SlaveInterface * m_slave;
        QByteArray m_packedArgs;
        KUrl m_url;
        int m_command;

        // for use in KIO::Scheduler
        //
        // There are two kinds of protocol:
        // (1) The protocol of the url
        // (2) The actual protocol that the io-slave uses.
        //
        // These two often match, but not necessarily. Most notably, they don't
        // match when doing ftp via a proxy.
        // In that case (1) is ftp, but (2) is http.
        //
        // JobData::protocol stores (2) while Job::url().protocol() returns (1).
        // The ProtocolInfoDict is indexed with (2).
        //
        // We schedule slaves based on (2) but tell the slave about (1) via
        // Slave::setProtocol().
        QString m_protocol;
        QStringList m_proxyList;
        int m_schedSerial;
        bool m_redirectionHandlingEnabled;

        /**
         * Forward signal from the slave.
         * @param data_size the processed size in bytes
         * @see processedSize()
         */
        void slotProcessedSize( KIO::filesize_t data_size );
        /**
         * Forward signal from the slave.
         * @param speed the speed in bytes/s
         * @see speed()
         */
        void slotSpeed( unsigned long speed );
        /**
         * Forward signal from the slave
         * Can also be called by the parent job, when it knows the size.
         * @param data_size the total size
         */
        void slotTotalSize( KIO::filesize_t data_size );

        /**
         * Called on a slave's info message.
         * @param s the info message
         * @see infoMessage()
         */
        void _k_slotSlaveInfoMessage( const QString &s );

        /**
         * @internal
         * Called by the scheduler when a slave gets to
         * work on this job.
         **/
        virtual void start( KIO::SlaveInterface *slave );

        /**
         * @internal
         * Called to detach a slave from a job.
         **/
        void slaveDone();

        /**
         * Called by subclasses to restart the job after a redirection was signalled.
         * The m_redirectionURL data member can appear in several subclasses, so we have it
         * passed in. The regular URL will be set to the redirection URL which is then cleared.
         */
        void restartAfterRedirection(KUrl *redirectionUrl);

        /**
         * Request the ui delegate to show a message box.
         * @internal
         */
        int requestMessageBox(int type, const QString& text,
                              const QString& caption,
                              const QString& buttonYes,
                              const QString& buttonNo,
                              const QString& iconYes = QString(),
                              const QString& iconNo = QString(),
                              const QString& dontAskAgainName = QString());

        Q_DECLARE_PUBLIC(SimpleJob)

        static inline SimpleJobPrivate *get(KIO::SimpleJob *job)
            { return job->d_func(); }
        static inline SimpleJob *newJobNoUi(const KUrl& url, int command, const QByteArray &packedArgs)
        {
            SimpleJob *job = new SimpleJob(*new SimpleJobPrivate(url, command, packedArgs));
            return job;
        }
        static inline SimpleJob *newJob(const KUrl& url, int command, const QByteArray &packedArgs,
                                        JobFlags flags = HideProgressInfo )
        {
            SimpleJob *job = new SimpleJob(*new SimpleJobPrivate(url, command, packedArgs));
            job->setUiDelegate(new JobUiDelegate);
            if (!(flags & HideProgressInfo))
                KIO::getJobTracker()->registerJob(job);
            return job;
        }
    };

    class MkdirJobPrivate;
    /**
     * A KIO job that creates a directory
     * @see KIO::mkdir()
     */
    class KIO_EXPORT MkdirJob : public SimpleJob {

    Q_OBJECT

    Q_SIGNALS:
        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
         * @param job the job that is redirected
         * @param url the new url
         */
        void redirection( KIO::Job *job, const KUrl &url );

    protected Q_SLOTS:
        virtual void slotFinished();

    public:
        MkdirJob(MkdirJobPrivate &dd);

    private:
        Q_PRIVATE_SLOT(d_func(), void slotRedirection( const KUrl &url))
        Q_DECLARE_PRIVATE(MkdirJob)
    };

    class TransferJobPrivate: public SimpleJobPrivate
    {
    public:
        inline TransferJobPrivate(const KUrl& url, int command, const QByteArray &packedArgs)
            : SimpleJobPrivate(url, command, packedArgs),
              m_internalSuspended(false),
              m_isMimetypeEmitted(false), m_subJob(0)
            { }

        bool m_internalSuspended;
        KUrl m_redirectionURL;
        KUrl::List m_redirectionList;
        QString m_mimetype;
        bool m_isMimetypeEmitted;
        TransferJob *m_subJob;

        /**
         * Flow control. Suspend data processing from the slave.
         */
        void internalSuspend();
        /**
         * Flow control. Resume data processing from the slave.
         */
        void internalResume();
        /**
         * @internal
         * Called by the scheduler when a slave gets to
         * work on this job.
         * @param slave the slave that works on the job
         */
        virtual void start( KIO::SlaveInterface *slave );

        void slotCanResume( KIO::filesize_t offset );

        Q_DECLARE_PUBLIC(TransferJob)
        static inline TransferJob *newJob(const KUrl& url, int command,
                                          const QByteArray &packedArgs,
                                          JobFlags flags)
        {
            TransferJob *job = new TransferJob(*new TransferJobPrivate(url, command, packedArgs));
            job->setUiDelegate(new JobUiDelegate);
            if (!(flags & HideProgressInfo))
                KIO::getJobTracker()->registerJob(job);
            return job;
        }
    };

    class DirectCopyJobPrivate;
    /**
     * @internal
     * Used for direct copy from or to the local filesystem (i.e. SlaveBase::copy())
     */
    class DirectCopyJob : public SimpleJob
    {
        Q_OBJECT

    public:
        DirectCopyJob(const KUrl &url, const QByteArray &packedArgs);

    public Q_SLOTS:
        void slotCanResume( KIO::filesize_t offset );

    Q_SIGNALS:
        /**
         * @internal
         * Emitted if the job found an existing partial file
         * and supports resuming. Used by FileCopyJob.
         */
        void canResume( KIO::Job *job, KIO::filesize_t offset );

    private:
        Q_DECLARE_PRIVATE(DirectCopyJob)
    };
}

#endif
