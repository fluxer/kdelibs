/* This file is part of the KDE libraries
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

#ifndef KIO_CONNECTION_P_H
#define KIO_CONNECTION_P_H

#include <QTcpSocket>
#include <QTcpServer>

class KUrl;

namespace KIO {
    struct Task {
        int cmd;
        QByteArray data;
    };

    class SocketConnectionBackend: public QObject
    {
        Q_OBJECT
    public:
        QString address;
        QString errorString;
        enum { Idle, Listening, Connected } state;

    private:
        enum { HeaderSize = 10, StandardBufferSize = 32*1024 };

        QTcpSocket *socket;
        QTcpServer *tcpServer;
        long len;
        int cmd;
        bool signalEmitted;

    public:
        explicit SocketConnectionBackend(QObject *parent = 0);
        ~SocketConnectionBackend();

        void setSuspended(bool enable);
        bool connectToRemote(const KUrl &url);
        bool listenForRemote();
        bool waitForIncomingTask(int ms);
        bool sendCommand(const Task &task);
        SocketConnectionBackend *nextPendingConnection();
    public slots:
        void socketReadyRead();
        void socketDisconnected();

    Q_SIGNALS:
        void disconnected();
        void commandReceived(const Task &task);
        void newConnection();
    };
}

#endif
