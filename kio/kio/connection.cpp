/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2007 Thiago Macieira <thiago@kde.org>

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

#include "connection.h"
#include "connection_p.h"

#include <errno.h>

#include <QQueue>
#include <QPointer>
#include <QDateTime>

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kurl.h>

using namespace KIO;

class KIO::ConnectionPrivate
{
public:
    inline ConnectionPrivate()
        : backend(0), suspended(false)
    { }

    void dequeue();
    void commandReceived(const Task &task);
    void disconnected();
    void setBackend(SocketConnectionBackend *b);

    QQueue<Task> outgoingTasks;
    QQueue<Task> incomingTasks;
    SocketConnectionBackend *backend;
    Connection *q;
    bool suspended;
};

class KIO::ConnectionServerPrivate
{
public:
    inline ConnectionServerPrivate()
        : backend(0)
    { }

    ConnectionServer *q;
    SocketConnectionBackend *backend;
};

void ConnectionPrivate::dequeue()
{
    if (!backend || suspended)
        return;

    while (!outgoingTasks.isEmpty()) {
       const Task task = outgoingTasks.dequeue();
       q->sendnow(task.cmd, task.data);
    }

    if (!incomingTasks.isEmpty())
        emit q->readyRead();
}

void ConnectionPrivate::commandReceived(const Task &task)
{
    //kDebug() << this << "Command " << task.cmd << " added to the queue";
    if (!suspended && incomingTasks.isEmpty())
        QMetaObject::invokeMethod(q, "dequeue", Qt::QueuedConnection);
    incomingTasks.enqueue(task);
}

void ConnectionPrivate::disconnected()
{
    q->close();
    QMetaObject::invokeMethod(q, "readyRead", Qt::QueuedConnection);
}

void ConnectionPrivate::setBackend(SocketConnectionBackend *b)
{
    backend = b;
    if (backend) {
        q->connect(backend, SIGNAL(commandReceived(Task)), SLOT(commandReceived(Task)));
        q->connect(backend, SIGNAL(disconnected()), SLOT(disconnected()));
        backend->setSuspended(suspended);
    }
}

SocketConnectionBackend::SocketConnectionBackend(QObject *parent)
    : QObject(parent), state(Idle), socket(0), tcpServer(0), len(-1),
    cmd(0), signalEmitted(false)
{
}

SocketConnectionBackend::~SocketConnectionBackend()
{
}

void SocketConnectionBackend::setSuspended(bool enable)
{
    if (state != Connected)
        return;
    Q_ASSERT(socket);
    Q_ASSERT(!tcpServer);

    if (enable) {
        //kDebug() << this << " suspending";
        socket->setReadBufferSize(1);
    } else {
        //kDebug() << this << " resuming";
        socket->setReadBufferSize(StandardBufferSize);
        if (socket->bytesAvailable() >= HeaderSize) {
            // there are bytes available
            QMetaObject::invokeMethod(this, "socketReadyRead", Qt::QueuedConnection);
        }

        // We read all bytes here, but we don't use readAll() because we need
        // to read at least one byte (even if there isn't any) so that the
        // socket notifier is reenabled
        QByteArray data = socket->read(socket->bytesAvailable() + 1);
        for (int i = data.size(); --i >= 0; )
            socket->ungetChar(data[i]);
    }
}

bool SocketConnectionBackend::connectToRemote(const KUrl &url)
{
    Q_ASSERT(state == Idle);
    Q_ASSERT(!socket);
    Q_ASSERT(!tcpServer);

    socket = new QTcpSocket(this);
    socket->connectToHost(url.host(),url.port());

    if (!socket->waitForConnected(1000)) {
        state = Idle;
        kDebug() << "could not connect to " << url;
        return false;
    }

    connect(socket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
    connect(socket, SIGNAL(disconnected()), SLOT(socketDisconnected()));
    state = Connected;
    return true;
}

void SocketConnectionBackend::socketDisconnected()
{
    state = Idle;
    emit disconnected();
}

bool SocketConnectionBackend::listenForRemote()
{
    Q_ASSERT(state == Idle);
    Q_ASSERT(!socket);
    Q_ASSERT(!tcpServer);

    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::LocalHost);
    if (!tcpServer->isListening()) {
        errorString = tcpServer->errorString();
        delete tcpServer;
        tcpServer = 0;
        return false;
    }

    address = "tcp://127.0.0.1:" + QString::number(tcpServer->serverPort());
    connect(tcpServer, SIGNAL(newConnection()), SIGNAL(newConnection()));

    state = Listening;
    return true;
}

bool SocketConnectionBackend::waitForIncomingTask(int ms)
{
    Q_ASSERT(state == Connected);
    Q_ASSERT(socket);
    if (socket->state() != QAbstractSocket::ConnectedState) {
        state = Idle;
        return false;           // socket has probably closed, what do we do?
    }

    signalEmitted = false;
    if (socket->bytesAvailable())
        socketReadyRead();
    if (signalEmitted)
        return true;            // there was enough data in the socket

    // not enough data in the socket, so wait for more
    QTime timer;
    timer.start();

    while (socket->state() == QAbstractSocket::ConnectedState && !signalEmitted &&
           (ms == -1 || timer.elapsed() < ms))
        if (!socket->waitForReadyRead(ms == -1 ? -1 : ms - timer.elapsed()))
            break;

    if (signalEmitted)
        return true;
    if (socket->state() != QAbstractSocket::ConnectedState)
        state = Idle;
    return false;
}

bool SocketConnectionBackend::sendCommand(const Task &task)
{
    Q_ASSERT(state == Connected);
    Q_ASSERT(socket);

    static char buffer[HeaderSize + 2];
    sprintf(buffer, "%6x_%2x_", task.data.size(), task.cmd);
    socket->write(buffer, HeaderSize);
    socket->write(task.data);

    //kDebug() << this << " Sending command " << hex << task.cmd << " of "
    //         << task.data.size() << " bytes (" << socket->bytesToWrite()
    //         << " bytes left to write";

    // blocking mode:
    while (socket->bytesToWrite() > 0 && socket->state() == QAbstractSocket::ConnectedState)
        socket->waitForBytesWritten(-1);

    return socket->state() == QAbstractSocket::ConnectedState;
}

SocketConnectionBackend *SocketConnectionBackend::nextPendingConnection()
{
    Q_ASSERT(state == Listening);
    Q_ASSERT(tcpServer);
    Q_ASSERT(!socket);

    //kDebug() << "Got a new connection";

    QTcpSocket *newSocket = tcpServer->nextPendingConnection();
    if (!newSocket)
        return 0;               // there was no connection...

    SocketConnectionBackend *result = new SocketConnectionBackend();
    result->state = Connected;
    result->socket = newSocket;
    newSocket->setParent(result);
    connect(newSocket, SIGNAL(readyRead()), result, SLOT(socketReadyRead()));
    connect(newSocket, SIGNAL(disconnected()), result, SLOT(socketDisconnected()));

    return result;
}

void SocketConnectionBackend::socketReadyRead()
{
    bool shouldReadAnother;
    do {
        if (!socket)
            // might happen if the invokeMethods were delivered after we disconnected
            return;

        // kDebug() << this << "Got " << socket->bytesAvailable() << " bytes";
        if (len == -1) {
            // We have to read the header
            static char buffer[HeaderSize];

            if (socket->bytesAvailable() < HeaderSize) {
                return;             // wait for more data
            }

            socket->read(buffer, sizeof buffer);
            buffer[6] = 0;
            buffer[9] = 0;

            char *p = buffer;
            while( *p == ' ' ) p++;
            len = strtol( p, 0L, 16 );

            p = buffer + 7;
            while( *p == ' ' ) p++;
            cmd = strtol( p, 0L, 16 );

            // kDebug() << this << " Beginning of command " << hex << cmd << " of size "
            //        << len;
        }

        QPointer<SocketConnectionBackend> that = this;

        // kDebug() << this <<  "Want to read " << len << " bytes";
        if (socket->bytesAvailable() >= len) {
            Task task;
            task.cmd = cmd;
            if (len)
                task.data = socket->read(len);
            len = -1;

            signalEmitted = true;
            emit commandReceived(task);
        } else if (len > StandardBufferSize) {
            kDebug(7017) << this << "Jumbo packet of" << len << "bytes";
            socket->setReadBufferSize(len + 1);
        }

        // If we're dead, better don't try anything.
        if (that.isNull())
            return;

        // Do we have enough for an another read?
        if (len == -1)
            shouldReadAnother = socket->bytesAvailable() >= HeaderSize;
        else
            shouldReadAnother = socket->bytesAvailable() >= len;
    }
    while (shouldReadAnother);
}

Connection::Connection(QObject *parent)
    : QObject(parent), d(new ConnectionPrivate)
{
    d->q = this;
}

Connection::~Connection()
{
    close();
    delete d;
}

void Connection::suspend()
{
    //kDebug() << this << "Suspended";
    d->suspended = true;
    if (d->backend)
        d->backend->setSuspended(true);
}

void Connection::resume()
{
    // send any outgoing or incoming commands that may be in queue
    QMetaObject::invokeMethod(this, "dequeue", Qt::QueuedConnection);

    //kDebug() << this << "Resumed";
    d->suspended = false;
    if (d->backend)
        d->backend->setSuspended(false);
}

void Connection::close()
{
    if (d->backend) {
        d->backend->disconnect(this);
        d->backend->deleteLater();
        d->backend = 0;
    }
    d->outgoingTasks.clear();
    d->incomingTasks.clear();
}

bool Connection::isConnected() const
{
    return d->backend && d->backend->state == SocketConnectionBackend::Connected;
}

bool Connection::inited() const
{
    return d->backend;
}

bool Connection::suspended() const
{
    return d->suspended;
}

void Connection::connectToRemote(const QString &address)
{
    /*
        establish the server to get its address if address is empty
        for compatibilty with local mode (which is no more, but it's
        uses are still present)
    */
    d->setBackend(new SocketConnectionBackend(this));
    d->backend->listenForRemote();

    kDebug(7017) << "Connection requested to " << address;
    KUrl url = address;
    if (Q_UNLIKELY(address.isEmpty() && d->backend)) {
        kWarning(7017) << "address is empty, using address from backend";
        url = d->backend->address;
    }

    const QString scheme = url.protocol();
    if (Q_UNLIKELY(scheme != QLatin1String("tcp"))) {
        kWarning(7017) << "Unknown requested KIO::Connection protocol='" << scheme
        << "' (" << url << ")";
        Q_ASSERT(0);
        return;
    }

    // connection succeeded
    if (!d->backend->connectToRemote(url)) {
        kWarning(7017) << "could not connect to " << url;
        delete d->backend;
        d->backend = 0;
        return;
    }

    d->dequeue();
}

QString Connection::errorString() const
{
    if (d->backend)
        return d->backend->errorString;
    return QString();
}

bool Connection::send(int cmd, const QByteArray& data)
{
    if (!inited() || !d->outgoingTasks.isEmpty()) {
        Task task;
        task.cmd = cmd;
        task.data = data;
        d->outgoingTasks.enqueue(task);
        return true;
    } else {
        return sendnow(cmd, data);
    }
}

bool Connection::sendnow(int _cmd, const QByteArray &data)
{
    if (data.size() > 0xffffff)
        return false;

    if (!isConnected())
        return false;

    //kDebug() << this << "Sending command " << _cmd << " of size " << data.size();
    Task task;
    task.cmd = _cmd;
    task.data = data;
    return d->backend->sendCommand(task);
}

bool Connection::hasTaskAvailable() const
{
    return !d->incomingTasks.isEmpty();
}

bool Connection::waitForIncomingTask(int ms)
{
    if (!isConnected())
        return false;

    if (d->backend)
        return d->backend->waitForIncomingTask(ms);
    return false;
}

int Connection::read( int* _cmd, QByteArray &data )
{
    // if it's still empty, then it's an error
    if (d->incomingTasks.isEmpty()) {
        //kWarning() << this << "Task list is empty!";
        return -1;
    }
    const Task task = d->incomingTasks.dequeue();
    //kDebug() << this << "Command " << task.cmd << " removed from the queue (size "
    //         << task.data.size() << ")";
    *_cmd = task.cmd;
    data = task.data;

    // if we didn't empty our reading queue, emit again
    if (!d->suspended && !d->incomingTasks.isEmpty())
        QMetaObject::invokeMethod(this, "dequeue", Qt::QueuedConnection);

    return data.size();
}

ConnectionServer::ConnectionServer(QObject *parent)
    : QObject(parent), d(new ConnectionServerPrivate)
{
    d->q = this;
}

ConnectionServer::~ConnectionServer()
{
    delete d;
}

void ConnectionServer::listenForRemote()
{
    d->backend = new SocketConnectionBackend(this);
    if (!d->backend->listenForRemote()) {
        delete d->backend;
        d->backend = 0;
        return;
    }

    connect(d->backend, SIGNAL(newConnection()), SIGNAL(newConnection()));
    kDebug(7017) << "Listening on " << d->backend->address;
}

QString ConnectionServer::address() const
{
    if (d->backend)
        return d->backend->address;
    return QString();
}

bool ConnectionServer::isListening() const
{
    return d->backend && d->backend->state == SocketConnectionBackend::Listening;
}

void ConnectionServer::close()
{
    delete d->backend;
    d->backend = 0;
}

Connection *ConnectionServer::nextPendingConnection()
{
    if (!isListening())
        return 0;

    SocketConnectionBackend *newBackend = d->backend->nextPendingConnection();
    if (!newBackend)
        return 0;               // no new backend...

    Connection *result = new Connection;
    result->d->setBackend(newBackend);
    newBackend->setParent(result);

    return result;
}

void ConnectionServer::setNextPendingConnection(Connection *conn)
{
    SocketConnectionBackend *newBackend = d->backend->nextPendingConnection();
    Q_ASSERT(newBackend);

    conn->d->backend = newBackend;
    conn->d->setBackend(newBackend);
    newBackend->setParent(conn);

    conn->d->dequeue();
}

#include "moc_connection_p.cpp"
#include "moc_connection.cpp"
