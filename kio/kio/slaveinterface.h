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
   INF_MESSAGEBOX,
   INF_POSITION
   // add new ones here once a release is done, to avoid breaking binary compatibility
 };

 /**
  * Identifiers for KIO data messages.
  */
 enum Message {
   MSG_DATA = 100,
   MSG_DATA_REQ,
   MSG_ERROR,
   MSG_CONNECTED,
   MSG_FINISHED,
   MSG_STAT_ENTRY, // 105
   MSG_LIST_ENTRIES,
   MSG_RESUME,
   MSG_SLAVE_STATUS,
   MSG_SLAVE_ACK, // 110
   MSG_NEED_SUBURL_DATA,
   MSG_CANRESUME,
   MSG_OPENED,
   MSG_WRITTEN,
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

protected:
    SlaveInterface(SlaveInterfacePrivate &dd, QObject *parent = 0);
public:

    virtual ~SlaveInterface();

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

    /**
     * Returns the top level window used as parent when displaying
     * dialogs.
     *
     * @see setWindow
     * @since 4.8.2
     * @deprecated
     */
    KIO_DEPRECATED QWidget* window() const;

    /**
     * Sets the top level window used as a parent when displaying
     * dialogs.
     * @see window
     * @since 4.8.2
     * @deprecated
     */
    KIO_DEPRECATED void setWindow(QWidget* window);

Q_SIGNALS:
    ///////////
    // Messages sent by the slave
    ///////////

    void data( const QByteArray & );
    void dataReq( );
    void error( int , const QString & );
    void connected();
    void finished();
    void slaveStatus(pid_t, const QByteArray&, const QString &, bool);
    void listEntries( const KIO::UDSEntryList& );
    void statEntry( const KIO::UDSEntry& );
    void needSubUrlData();

    void canResume( KIO::filesize_t );

    void open();
    void written( KIO::filesize_t );

    ///////////
    // Info sent by the slave
    //////////
    void metaData( const KIO::MetaData & );
    void totalSize( KIO::filesize_t );
    void processedSize( KIO::filesize_t );
    void redirection( const KUrl& );
    void position( KIO::filesize_t );

    void speed( unsigned long );
    void mimeType( const QString & );
    void warning( const QString & );
    void infoMessage( const QString & );
    //void connectFinished(); //it does not get emitted anywhere

protected:
    /////////////////
    // Dispatching
    ////////////////

    virtual bool dispatch();
    virtual bool dispatch( int _cmd, const QByteArray &data );

    void messageBox( int type, const QString &text, const QString &caption,
                     const QString &buttonYes, const QString &buttonNo );

    void messageBox( int type, const QString &text, const QString &caption,
                     const QString &buttonYes, const QString &buttonNo,
                     const QString &dontAskAgainName );

protected Q_SLOTS:
    void calcSpeed();
protected:
    SlaveInterfacePrivate* const d_ptr;
    Q_DECLARE_PRIVATE(SlaveInterface)
};

}

// moved to udesentry.cpp!!!
// KIO_EXPORT QDataStream &operator <<(QDataStream &s, const KIO::UDSEntry &e );
// KIO_EXPORT QDataStream &operator >>(QDataStream &s, KIO::UDSEntry &e );

#endif
