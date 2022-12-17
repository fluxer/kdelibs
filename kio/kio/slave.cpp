/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *                2000 Stephan Kulow <coolo@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "slave.h"

#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QtCore/QProcess>
#include <QtCore/QDir>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>

#include "kservice.h"
#include <kio/global.h>
#include "kio/connection.h"
#include <kprotocolinfo.h>

#include "slaveinterface_p.h"

using namespace KIO;

#define SLAVE_CONNECTION_TIMEOUT_MIN	   2

// Without debug info we consider it an error if the slave doesn't connect
// within 10 seconds.
// With debug info we give the slave an hour so that developers have a chance
// to debug their slave.
#ifdef NDEBUG
#define SLAVE_CONNECTION_TIMEOUT_MAX      10
#else
#define SLAVE_CONNECTION_TIMEOUT_MAX    3600
#endif

namespace KIO {

  /**
   * @internal
   */
    class SlavePrivate: public SlaveInterfacePrivate
    {
    public:
        SlavePrivate(const QString &protocol) :
            m_protocol(protocol),
            m_slaveProtocol(protocol),
            slaveconnserver(new KIO::ConnectionServer),
            m_job(0),
            m_pid(0),
            m_port(0),
            contacted(false),
            dead(false),
            contact_started(time(0)),
            m_idleSince(0),
            m_refCount(1)
        {
            slaveconnserver->listenForRemote();
            if ( !slaveconnserver->isListening() )
                kWarning() << "Connection server not listening, could not connect";
        }
        ~SlavePrivate()
        {
            delete slaveconnserver;
        }

        QString m_protocol;
        QString m_slaveProtocol;
        QString m_host;
        QString m_user;
        QString m_passwd;
        KIO::ConnectionServer *slaveconnserver;
        KIO::SimpleJob *m_job;
        pid_t m_pid;
        quint16 m_port;
        bool contacted;
        bool dead;
        time_t contact_started;
        time_t m_idleSince;
        int m_refCount;
  };
}

void Slave::accept()
{
    Q_D(Slave);
    d->slaveconnserver->setNextPendingConnection(d->connection);
    d->slaveconnserver->deleteLater();
    d->slaveconnserver = 0;

    connect(d->connection, SIGNAL(readyRead()), SLOT(gotInput()));
}

void Slave::timeout()
{
    Q_D(Slave);
   if (d->dead) //already dead? then slaveDied was emitted and we are done
      return;
   if (d->connection->isConnected())
      return;

   kDebug(7002) << "slave failed to connect to application pid=" << d->m_pid
                << " protocol=" << d->m_protocol;
   if (d->m_pid && (::kill(d->m_pid, 0) == 0))
   {
      int delta_t = (int) difftime(time(0), d->contact_started);
      kDebug(7002) << "slave is slow... pid=" << d->m_pid << " t=" << delta_t;
      if (delta_t < SLAVE_CONNECTION_TIMEOUT_MAX)
      {
         QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, this, SLOT(timeout()));
         return;
      }
   }
   kDebug(7002) << "Houston, we lost our slave, pid=" << d->m_pid;
   d->connection->close();
   d->dead = true;
   QString arg = d->m_protocol;
   if (!d->m_host.isEmpty())
      arg += "://"+d->m_host;
   kDebug(7002) << "slave died pid = " << d->m_pid;

   ref();
   // Tell the job about the problem.
   emit error(ERR_SLAVE_DIED, arg);
   // Tell the scheduler about the problem.
   emit slaveDied(this);
   // After the above signal we're dead!!
   deref();
}

Slave::Slave(const QString &protocol, QObject *parent)
    : SlaveInterface(*new SlavePrivate(protocol), parent)
{
    Q_D(Slave);
    d->slaveconnserver->setParent(this);
    d->connection = new Connection(this);
    connect(d->slaveconnserver, SIGNAL(newConnection()), SLOT(accept()));
}

Slave::~Slave()
{
    // kDebug(7002) << "destructing slave object pid = " << d->m_pid;
    //delete d;
}

QString Slave::protocol()
{
    Q_D(Slave);
    return d->m_protocol;
}

void Slave::setProtocol(const QString & protocol)
{
    Q_D(Slave);
    d->m_protocol = protocol;
}

QString Slave::slaveProtocol()
{
    Q_D(Slave);
    return d->m_slaveProtocol;
}

QString Slave::host()
{
    Q_D(Slave);
    return d->m_host;
}

quint16 Slave::port()
{
    Q_D(Slave);
    return d->m_port;
}

QString Slave::user()
{
    Q_D(Slave);
    return d->m_user;
}

QString Slave::passwd()
{
    Q_D(Slave);
    return d->m_passwd;
}

void Slave::setIdle()
{
    Q_D(Slave);
    d->m_idleSince = time(0);
}

bool Slave::isConnected()
{
    Q_D(Slave);
    return d->contacted;
}

void Slave::setConnected(bool c)
{
    Q_D(Slave);
    d->contacted = c;
}

void Slave::ref()
{
    Q_D(Slave);
    d->m_refCount++;
}

void Slave::deref()
{
    Q_D(Slave);
    d->m_refCount--;
    if (!d->m_refCount) {
        d->connection->disconnect(this);
        this->disconnect();
        deleteLater();
    }
}

time_t Slave::idleTime()
{
    Q_D(Slave);
    if (!d->m_idleSince) {
        return time_t(0);
    }
    return time_t(difftime(time(0), d->m_idleSince));
}

void Slave::setPID(pid_t pid)
{
    Q_D(Slave);
    d->m_pid = pid;
}

int Slave::slave_pid()
{
    Q_D(Slave);
    return d->m_pid;
}

void Slave::setJob(KIO::SimpleJob *job)
{
    Q_D(Slave);
    d->m_job = job;
}

KIO::SimpleJob *Slave::job() const
{
    Q_D(const Slave);
    return d->m_job;
}

bool Slave::isAlive()
{
    Q_D(Slave);
    return !d->dead;
}

void Slave::suspend()
{
    Q_D(Slave);
    d->connection->suspend();
}

void Slave::resume()
{
    Q_D(Slave);
    d->connection->resume();
}

bool Slave::suspended()
{
    Q_D(Slave);
    return d->connection->suspended();
}

void Slave::send(int cmd, const QByteArray &arr)
{
    Q_D(Slave);
    d->connection->send(cmd, arr);
}

void Slave::gotInput()
{
    Q_D(Slave);
    if (d->dead) //already dead? then slaveDied was emitted and we are done
        return;
    ref();
    if (!dispatch())
    {
        d->connection->close();
        d->dead = true;
        QString arg = d->m_protocol;
        if (!d->m_host.isEmpty())
            arg += "://"+d->m_host;
        kDebug(7002) << "slave died pid = " << d->m_pid;
        // Tell the job about the problem.
        emit error(ERR_SLAVE_DIED, arg);
        // Tell the scheduler about the problem.
        emit slaveDied(this);
    }
    deref();
    // Here we might be dead!!
}

void Slave::kill()
{
    Q_D(Slave);
    d->dead = true; // OO can be such simple.
    kDebug(7002) << "killing slave pid" << d->m_pid
                 << "(" << QString(d->m_protocol) + "://" + d->m_host << ")";
    if (d->m_pid)
    {
       ::kill(d->m_pid, SIGTERM);
       d->m_pid = 0;
    }
}

void Slave::setHost( const QString &host, quint16 port,
                     const QString &user, const QString &passwd)
{
    Q_D(Slave);
    d->m_host = host;
    d->m_port = port;
    d->m_user = user;
    d->m_passwd = passwd;

    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << d->m_host << d->m_port << d->m_user << d->m_passwd;
    d->connection->send( CMD_HOST, data );
}

void Slave::resetHost()
{
    Q_D(Slave);
    d->m_host = "<reset>";
}

void Slave::setConfig(const MetaData &config)
{
    Q_D(Slave);
    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << config;
    d->connection->send( CMD_CONFIG, data );
}

Slave* Slave::createSlave( const QString &protocol, const KUrl& url, int& error, QString& error_text )
{
    kDebug(7002) << "createSlave" << protocol << "for" << url;
    Slave *slave = new Slave(protocol);
    QString slaveAddress = slave->d_func()->slaveconnserver->address();

    QString slavename = KProtocolInfo::exec(protocol);
    if (slavename.isEmpty())
    {
        error_text = i18n("Unknown protocol '%1'.", protocol);
        error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
        delete slave;
        return 0;
    }
    QString slaveexe = KStandardDirs::locate("libexec", slavename);
    if (slaveexe.isEmpty())
    {
        error_text = i18n("Can not find io-slave for protocol '%1'.", protocol);
        error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
        delete slave;
        return 0;
    }

    kDebug() << "kioslave" << ", " << slaveexe << ", " << protocol << ", " << slaveAddress;

    const QStringList slaveargs = QStringList() << slaveexe << slaveAddress;
    Q_PID slavepid = 0;
    const bool result = QProcess::startDetached(
        KStandardDirs::findExe("kioslave"),
        slaveargs,
        QDir::currentPath(),
        &slavepid
    );
    if (!result || !slavepid) {
        error_text = i18n("Can not start io-slave for protocol '%1'.", protocol);
        error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
        delete slave;
        return 0;
    }
    slave->setPID(slavepid);

    return slave;
}

#include "moc_slave.cpp"
