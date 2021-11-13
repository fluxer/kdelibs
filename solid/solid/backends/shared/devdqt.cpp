/*
    Copyright 2021 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "devdqt.h"
#include "kshell.h"

#include <QDebug>

#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

// for reference:
// https://www.freebsd.org/cgi/man.cgi?query=devd.conf
// freebsd-src/lib/libdevdctl/consumer.cc

namespace DevdQt {

Device::Device()
{
}

Device::Device(const QByteArray &device)
    : m_device(device)
{
}

Device::Device(const Device &other)
    : m_device(other.m_device)
{
}

Device::~Device()
{
}

Device &Device::operator=(const Device &other)
{
    m_device = other.m_device;
    return *this;
}

bool Device::isValid() const
{
    return !m_device.isEmpty();
}

QByteArray Device::device() const
{
    return m_device;
}

Client::Client(QObject *parent)
    : QObject(parent),
    m_socket(0),
    m_monitor(0)
{
    // open the clients pipe
    m_socket = ::socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (Q_UNLIKELY(m_socket == -1)) {
        qWarning("DevdQt: unable to create socket");
        return;
    }

    struct sockaddr_un devdaddr;
    ::memset(&devdaddr, 0, sizeof(sockaddr_un));
    devdaddr.sun_family = AF_UNIX;
    ::strlcpy(devdaddr.sun_path, DEVD_PIPE, sizeof(devdaddr.sun_path));
    const int connectresult = ::connect(m_socket, reinterpret_cast<struct sockaddr*>(&devdaddr), sizeof(devdaddr));
    if (Q_UNLIKELY(connectresult == -1)) {
        qWarning("DevdQt: unable to connect to socket");
        return;
    }

    // start monitoring
    m_monitor = new QSocketNotifier(m_socket, QSocketNotifier::Read);
    QObject::connect(m_monitor, SIGNAL(activated(int)), this, SLOT(monitorReadyRead(int)));
}

Client::~Client()
{
    delete m_monitor;

    ::close(m_socket);
}

void Client::monitorReadyRead(int fd)
{
    QByteArray recvbuf(4086, Qt::Uninitialized);
    const ssize_t recvresult = ::recv(m_socket, recvbuf.data(), recvbuf.size(), MSG_WAITALL);
    if (Q_UNLIKELY(recvresult == -1)) {
        qWarning("DevdQt: unable to read data from socket");
        return;
    }

    QByteArray eventdevice;
    QByteArray eventtype;

    const QString recvstring = QString::fromLatin1(recvbuf.constData(), recvbuf.size());
    const QStringList shellpairs = KShell::splitArgs(recvstring);
    // qDebug() << Q_FUNC_INFO << shellpairs;
    foreach (const QString &shellpair, shellpairs) {
        if (shellpair.startsWith("cdev=")) {
            eventdevice = shellpair.right(shellpair.size() - 5).toLatin1();
        } else if (shellpair.startsWith("device=")) {
            eventdevice = shellpair.right(shellpair.size() - 7).toLatin1();
        } else if (shellpair.startsWith("type=")) {
            eventtype = shellpair.right(shellpair.size() - 5).toLatin1();
        }
    }
    // qDebug() << Q_FUNC_INFO << eventdevice << eventtype;

    Device device(eventdevice);
    if (eventtype == "create") {
        emit deviceAdded(device);
    } else if (eventtype == "destroy") {
        emit deviceRemoved(device);
    } else if (eventtype == "mediachange" || eventtype == "sizechange") {
        emit deviceChanged(device);
    }
}

}

#include "moc_devdqt.cpp"
