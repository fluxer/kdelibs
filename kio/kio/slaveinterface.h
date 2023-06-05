/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#ifndef KIO_SLAVEINTERFACE_H
#define KIO_SLAVEINTERFACE_H

#include <unistd.h>
#include <sys/types.h>

#include <QtCore/QObject>

#include <kio/global.h>
#include <kio/udsentry.h>
#include <kio/authinfo.h>

class KUrl;

namespace KIO {

class SimpleJob;
class Connection;
// better there is one ...
class SlaveInterfacePrivate;

  // Definition of enum Command has been moved to global.h

 /**
  * Identifiers for KIO informational messages.
  */
 enum Info {
   INF_TOTAL_SIZE = 10,
   INF_PROCESSED_SIZE = 11,
   INF_SPEED,
   INF_REDIRECTION = 20,
   INF_MIME_TYPE = 21,
   INF_WARNING = 23,
   INF_INFOMESSAGE,
   INF_META_DATA,
   INF_MESSAGEBOX
   // add new ones here once a release is done, to avoid breaking binary compatibility
 };

 /**
  * Identifiers for KIO data messages.
  */
 enum Message {
   MSG_DATA = 100,
   MSG_DATA_REQ,
   MSG_ERROR,
   MSG_FINISHED,
   MSG_STAT_ENTRY,
   MSG_LIST_ENTRIES,
   MSG_RESUME,
   MSG_NEED_SUBURL_DATA,
   MSG_CANRESUME
   // add new ones here once a release is done, to avoid breaking binary compatibility
 };

/**
 * There are two classes that specifies the protocol between application
 * ( KIO::Job) and kioslave. SlaveInterface is the class to use on the application
 * end, SlaveBase is the one to use on the slave end.
 *
 * A call to foo() results in a call to slotFoo() on the other end.
 */
class KIO_EXPORT SlaveInterface : public QObject
{
    Q_OBJECT
public:
    explicit SlaveInterface(const QString &protocol, QObject *parent = 0);

    ~SlaveInterface();

    void setPID(pid_t);
    pid_t pid() const;

    void setJob(KIO::SimpleJob *job);
    KIO::SimpleJob *job() const;

    /**
     * Force termination
     */
    void kill();

    /**
     * @return true if the slave survived the last mission.
     */
    bool isAlive() const;

    /**
     * Set host for url
     * @param host to connect to.
     * @param port to connect to.
     * @param user to login as
     * @param passwd to login with
     */
    void setHost(const QString &host, quint16 port, const QString &user, const QString &passwd);

    /**
     * Clear host info.
     */
    void resetHost();

    /**
     * Configure slave
     */
    void setConfig(const MetaData &config);

    /**
     * The protocol this slave handles.
     *
     * @return name of protocol handled by this slave, as seen by the user
     */
    QString protocol() const;

    void setProtocol(const QString &protocol);

    /**
     * @return Host this slave is (was?) connected to
     */
    QString host() const;

    /**
     * @return port this slave is (was?) connected to
     */
    quint16 port() const;

    /**
     * @return User this slave is (was?) logged in as
     */
    QString user() const;

    /**
     * @return Passwd used to log in
     */
    QString passwd() const;

    /**
     * Creates a new slave.
     *
     * @param protocol the protocol
     * @param url is the url
     * @param error is the error code on failure and undefined else.
     * @param error_text is the error text on failure and undefined else.
     *
     * @return 0 on failure, or a pointer to a slave otherwise.
     */
    static SlaveInterface* createSlave(const QString &protocol, const KUrl &url, int &error, QString &error_text);

    // == communication with connected kioslave ==
    // whenever possible prefer these methods over the respective
    // methods in connection()
    /**
     * Suspends the operation of the attached kioslave.
     */
    void suspend();

    /**
     * Resumes the operation of the attached kioslave.
     */
    void resume();

    /**
     * Tells whether the kioslave is suspended.
     * @return true if the kioslave is suspended.
     */
    bool suspended() const;

    /**
     * Sends the given command to the kioslave.
     * @param cmd command id
     * @param arr byte array containing data
     */
    void send(int cmd, const QByteArray &arr = QByteArray());
    // == end communication with connected kioslave ==

    /**
     * @return The time this slave has been idle.
     */
    time_t idleTime() const;

    /**
     * Marks this slave as idle.
     */
    void setIdle();

    void ref();
    void deref();

    void setConnection( Connection* connection );
    Connection *connection() const;

    // Send our answer to the MSG_RESUME (canResume) request
    // (to tell the "put" job whether to resume or not)
    void sendResumeAnswer( bool resume );

    /**
     * Sends our answer for the INF_MESSAGEBOX request.
     *
     * @since 4.11
     */
    void sendMessageBoxAnswer(int result);

    void setOffset( KIO::filesize_t offset );
    KIO::filesize_t offset() const;

Q_SIGNALS:
    ///////////
    // Messages sent by the slave
    ///////////
    void data( const QByteArray & );
    void dataReq( );
    void error( int , const QString & );
    void finished();
    void listEntries( const KIO::UDSEntryList& );
    void statEntry( const KIO::UDSEntry& );
    void needSubUrlData();
    void canResume( KIO::filesize_t );

    ///////////
    // Info sent by the slave
    //////////
    void metaData( const KIO::MetaData & );
    void totalSize( KIO::filesize_t );
    void processedSize( KIO::filesize_t );
    void redirection( const KUrl& );
    void speed( unsigned long );
    void mimeType( const QString & );
    void warning( const QString & );
    void infoMessage( const QString & );

    ///////////
    // Info sent for the scheduler
    //////////
    void slaveDied(KIO::SlaveInterface *slave);

protected:
    /////////////////
    // Dispatching
    ////////////////
    bool dispatch();
    bool dispatch( int cmd, const QByteArray &data );

    void messageBox( int type, const QString &text, const QString &caption,
                     const QString &buttonYes, const QString &buttonNo );

    void messageBox( int type, const QString &text, const QString &caption,
                     const QString &buttonYes, const QString &buttonNo,
                     const QString &dontAskAgainName );

protected Q_SLOTS:
    void calcSpeed();
    void accept();
    void gotInput();
    void timeout();

protected:
    SlaveInterfacePrivate* const d_ptr;
    Q_DECLARE_PRIVATE(SlaveInterface)
};

}

// moved to udesentry.cpp!!!
// KIO_EXPORT QDataStream &operator <<(QDataStream &s, const KIO::UDSEntry &e );
// KIO_EXPORT QDataStream &operator >>(QDataStream &s, KIO::UDSEntry &e );

#endif
