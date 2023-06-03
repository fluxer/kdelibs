/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
   Copyright (c) 2000 Stephan Kulow <coolo@kde.org>

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

#include "slaveinterface.h"
#include "slaveinterface_p.h"
#include "usernotificationhandler_p.h"
#include "slavebase.h"
#include "connection.h"
#include "job_p.h"

#include <kdebug.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kstandarddirs.h>

#include <QtCore/QProcess>
#include <QtCore/QDir>

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

using namespace KIO;

#define SLAVE_CONNECTION_TIMEOUT_MIN       2

// Without debug info we consider it an error if the slave doesn't connect
// within 10 seconds.
// With debug info we give the slave an hour so that developers have a chance
// to debug their slave.
#ifdef NDEBUG
#define SLAVE_CONNECTION_TIMEOUT_MAX      10
#else
#define SLAVE_CONNECTION_TIMEOUT_MAX    3600
#endif

Q_GLOBAL_STATIC(UserNotificationHandler, globalUserNotificationHandler)

SlaveInterfacePrivate::SlaveInterfacePrivate(const QString &protocol)
    : connection(nullptr),
    filesize(0),
    offset(0),
    last_time(0),
    nums(0),
    slave_calcs_speed(false),
    parentWindow(nullptr),
    m_protocol(protocol),
    slaveconnserver(new KIO::ConnectionServer()),
    m_job(nullptr),
    m_pid(0),
    m_port(0),
    dead(false),
    contact_started(time(0)),
    m_idleSince(0),
    m_refCount(1)
{
    start_time.tv_sec = 0;
    start_time.tv_usec = 0;

    slaveconnserver->listenForRemote();
    if (!slaveconnserver->isListening()) {
        kWarning() << "Connection server not listening, could not connect";
    }
}

SlaveInterfacePrivate::~SlaveInterfacePrivate()
{
    delete slaveconnserver;
    delete connection;
}


SlaveInterface::SlaveInterface(const QString &protocol, QObject *parent)
    : QObject(parent),
    d_ptr(new SlaveInterfacePrivate(protocol))
{
    connect(&d_ptr->speed_timer, SIGNAL(timeout()), SLOT(calcSpeed()));
    d_ptr->slaveconnserver->setParent(this);
    d_ptr->connection = new Connection(this);
    connect(d_ptr->slaveconnserver, SIGNAL(newConnection()), SLOT(accept()));
}

SlaveInterface::~SlaveInterface()
{
    // Note: no kDebug() here (scheduler is deleted very late)
    delete d_ptr;
}

QString SlaveInterface::protocol() const
{
    Q_D(const SlaveInterface);
    return d->m_protocol;
}

void SlaveInterface::setProtocol(const QString & protocol)
{
    Q_D(SlaveInterface);
    d->m_protocol = protocol;
}

QString SlaveInterface::host() const
{
    Q_D(const SlaveInterface);
    return d->m_host;
}

quint16 SlaveInterface::port() const
{
    Q_D(const SlaveInterface);
    return d->m_port;
}

QString SlaveInterface::user() const
{
    Q_D(const SlaveInterface);
    return d->m_user;
}

QString SlaveInterface::passwd() const
{
    Q_D(const SlaveInterface);
    return d->m_passwd;
}

void SlaveInterface::setIdle()
{
    Q_D(SlaveInterface);
    d->m_idleSince = time(0);
}

void SlaveInterface::ref()
{
    Q_D(SlaveInterface);
    d->m_refCount++;
}

void SlaveInterface::deref()
{
    Q_D(SlaveInterface);
    d->m_refCount--;
    if (!d->m_refCount) {
        d->connection->disconnect(this);
        this->disconnect();
        deleteLater();
    }
}

time_t SlaveInterface::idleTime() const
{
    Q_D(const SlaveInterface);
    if (!d->m_idleSince) {
        return time_t(0);
    }
    return time_t(difftime(time(0), d->m_idleSince));
}

void SlaveInterface::setPID(pid_t pid)
{
    Q_D(SlaveInterface);
    d->m_pid = pid;
}

pid_t SlaveInterface::pid() const
{
    Q_D(const SlaveInterface);
    return d->m_pid;
}

void SlaveInterface::setJob(KIO::SimpleJob *job)
{
    Q_D(SlaveInterface);
    d->m_job = job;
}

KIO::SimpleJob *SlaveInterface::job() const
{
    Q_D(const SlaveInterface);
    return d->m_job;
}

bool SlaveInterface::isAlive() const
{
    Q_D(const SlaveInterface);
    return !d->dead;
}

void SlaveInterface::suspend()
{
    Q_D(SlaveInterface);
    d->connection->suspend();
}

void SlaveInterface::resume()
{
    Q_D(SlaveInterface);
    d->connection->resume();
}

bool SlaveInterface::suspended() const
{
    Q_D(const SlaveInterface);
    return d->connection->suspended();
}

void SlaveInterface::send(int cmd, const QByteArray &arr)
{
    Q_D(SlaveInterface);
    d->connection->send(cmd, arr);
}

void SlaveInterface::kill()
{
    Q_D(SlaveInterface);
    d->dead = true; // OO can be such simple.
    kDebug(7002) << "killing slave pid" << d->m_pid
                 << "(" << d->m_protocol + "://" + d->m_host << ")";
    if (d->m_pid) {
       ::kill(d->m_pid, SIGTERM);
       d->m_pid = 0;
    }
}

void SlaveInterface::setHost( const QString &host, quint16 port,
                              const QString &user, const QString &passwd)
{
    Q_D(SlaveInterface);
    d->m_host = host;
    d->m_port = port;
    d->m_user = user;
    d->m_passwd = passwd;

    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << d->m_host << d->m_port << d->m_user << d->m_passwd;
    d->connection->send( CMD_HOST, data );
}

void SlaveInterface::resetHost()
{
    Q_D(SlaveInterface);
    d->m_host = "<reset>";
}

void SlaveInterface::setConfig(const MetaData &config)
{
    Q_D(SlaveInterface);
    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << config;
    d->connection->send( CMD_CONFIG, data );
}

SlaveInterface* SlaveInterface::createSlave( const QString &protocol, const KUrl& url, int& error, QString& error_text )
{
    kDebug(7002) << "createSlave" << protocol << "for" << url;
    SlaveInterface *slave = new SlaveInterface(protocol);
    const QString slaveaddress = slave->d_func()->slaveconnserver->address();

    const QString slavename = KProtocolInfo::exec(protocol);
    if (slavename.isEmpty()) {
        error_text = i18n("Unknown protocol '%1'.", protocol);
        error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
        delete slave;
        return 0;
    }
    const QString slaveexe = KStandardDirs::locate("libexec", slavename);
    if (slaveexe.isEmpty()) {
        error_text = i18n("Can not find io-slave for protocol '%1'.", protocol);
        error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
        delete slave;
        return 0;
    }

    kDebug() << "kioslave" << ", " << slaveexe << ", " << protocol << ", " << slaveaddress;

    const QStringList slaveargs = QStringList() << slaveexe << slaveaddress;
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

void SlaveInterface::setConnection( Connection* connection )
{
    Q_D(SlaveInterface);
    d->connection = connection;
}

Connection *SlaveInterface::connection() const
{
    Q_D(const SlaveInterface);
    return d->connection;
}

bool SlaveInterface::dispatch()
{
    Q_D(SlaveInterface);
    Q_ASSERT(d->connection);

    int cmd;
    QByteArray data;

    int ret = d->connection->read(&cmd, data);
    if (ret == -1)
      return false;

    return dispatch(cmd, data);
}

void SlaveInterface::calcSpeed()
{
    Q_D(SlaveInterface);
    if (d->slave_calcs_speed) {
        d->speed_timer.stop();
        return;
    }

    struct timeval tv;
    gettimeofday(&tv, 0);

    long diff = ((tv.tv_sec - d->start_time.tv_sec) * 1000000 +
                  tv.tv_usec - d->start_time.tv_usec) / 1000;
    if (diff - d->last_time >= 900) {
        d->last_time = diff;
        if (d->nums == max_nums) {
            // let's hope gcc can optimize that well enough
            // otherwise I'd try memcpy :)
            for (unsigned int i = 1; i < max_nums; ++i) {
                d->times[i-1] = d->times[i];
                d->sizes[i-1] = d->sizes[i];
            }
            d->nums--;
        }
        d->times[d->nums] = diff;
        d->sizes[d->nums++] = d->filesize - d->offset;

        KIO::filesize_t lspeed = 1000 * (d->sizes[d->nums-1] - d->sizes[0]) / (d->times[d->nums-1] - d->times[0]);

//      kDebug() << (long)d->filesize << diff
//          << long(d->sizes[d->nums-1] - d->sizes[0])
//          << d->times[d->nums-1] - d->times[0]
//          << long(lspeed) << double(d->filesize) / diff
//          << convertSize(lspeed)
//          << convertSize(long(double(d->filesize) / diff) * 1000);

        if (!lspeed) {
            d->nums = 1;
            d->times[0] = diff;
            d->sizes[0] = d->filesize - d->offset;
        }
        emit speed(lspeed);
    }
}

bool SlaveInterface::dispatch(int cmd, const QByteArray &rawdata)
{
    Q_D(SlaveInterface);
    //kDebug(7007) << "dispatch " << cmd;

    switch(cmd) {
        case MSG_DATA: {
            emit data(rawdata);
            break;
        }
        case MSG_DATA_REQ: {
            emit dataReq();
            break;
        }
        case MSG_OPENED: {
            emit open();
            break;
        }
        case MSG_FINISHED: {
            // kDebug(7007) << "Finished [this = " << this << "]";
            d->offset = 0;
            d->speed_timer.stop();
            emit finished();
            break;
        }
        case MSG_STAT_ENTRY: {
            QDataStream stream(rawdata);
            UDSEntry entry;
            stream >> entry;
            emit statEntry(entry);
            break;
        }
        case MSG_LIST_ENTRIES: {
            QDataStream stream(rawdata);
            quint32 count;
            stream >> count;
            UDSEntryList list;
            list.reserve(count);
            UDSEntry entry;
            for (uint i = 0; i < count; i++) {
                stream >> entry;
                list.append(entry);
            }
            emit listEntries(list);
            break;
        }
        case MSG_RESUME: { // From the put job
            QDataStream stream(rawdata);
            stream >> d->offset;
            emit canResume(d->offset);
            break;
        }
        case MSG_CANRESUME: { // From the get job
            d->filesize = d->offset;
            emit canResume(0); // the arg doesn't matter
            break;
        }
        case MSG_ERROR: {
            QDataStream stream(rawdata);
            qint32 i;
            QString str;
            stream >> i >> str;
            kDebug(7007) << "error " << i << " " << str;
            emit error(i, str);
            break;
        }
        case MSG_CONNECTED: {
            emit connected();
            break;
        }
        case MSG_WRITTEN: {
            QDataStream stream(rawdata);
            KIO::filesize_t size;
            stream >> size;
            emit written(size);
            break;
        }
        case INF_TOTAL_SIZE: {
            QDataStream stream(rawdata);
            KIO::filesize_t size;
            stream >> size;
            gettimeofday(&d->start_time, 0);
            d->last_time = 0;
            d->filesize = d->offset;
            d->sizes[0] = d->filesize - d->offset;
            d->times[0] = 0;
            d->nums = 1;
            d->speed_timer.start(1000);
            d->slave_calcs_speed = false;
            emit totalSize(size);
            break;
        }
        case INF_PROCESSED_SIZE: {
            QDataStream stream(rawdata);
            stream >> d->filesize;
            emit processedSize(d->filesize);
            break;
        }
        case INF_POSITION: {
            QDataStream stream(rawdata);
            KIO::filesize_t pos;
            stream >> pos;
            emit position(pos);
            break;
        }
        case INF_SPEED: {
            QDataStream stream(rawdata);
            quint32 ul;
            stream >> ul;
            d->slave_calcs_speed = true;
            d->speed_timer.stop();
            emit speed(ul);
            break;
        }
        case INF_REDIRECTION: {
            QDataStream stream(rawdata);
            KUrl url;
            stream >> url;
            emit redirection(url);
            break;
        }
        case INF_MIME_TYPE: {
            QDataStream stream(rawdata);
            QString str;
            stream >> str;
            emit mimeType(str);
            if (!d->connection->suspended())
                d->connection->sendnow(CMD_NONE, QByteArray());
            break;
        }
        case INF_WARNING: {
            QDataStream stream(rawdata);
            QString str;
            stream >> str;
            emit warning(str);
            break;
        }
        case INF_MESSAGEBOX: {
            kDebug(7007) << "needs a msg box";
            QDataStream stream(rawdata);
            QString text, caption, buttonYes, buttonNo, dontAskAgainName;
            qint32 type;
            stream >> type >> text >> caption >> buttonYes >> buttonNo >> dontAskAgainName;
            messageBox(type, text, caption, buttonYes, buttonNo, dontAskAgainName);
            break;
        }
        case INF_INFOMESSAGE: {
            QDataStream stream(rawdata);
            QString msg;
            stream >> msg;
            emit infoMessage(msg);
            break;
        }
        case INF_META_DATA: {
            QDataStream stream(rawdata);
            MetaData m;
            stream >> m;
            emit metaData(m);
            break;
        }
        case MSG_NEED_SUBURL_DATA: {
            emit needSubUrlData();
            break;
        }
        default: {
            kWarning(7007) << "Slave sends unknown command (" << cmd << "), dropping slave";
            return false;
        }
    }
    return true;
}

void SlaveInterface::setOffset( KIO::filesize_t o)
{
    Q_D(SlaveInterface);
    d->offset = o;
}

KIO::filesize_t SlaveInterface::offset() const
{
    Q_D(const SlaveInterface);
    return d->offset;
}

void SlaveInterface::sendResumeAnswer( bool resume )
{
    Q_D(SlaveInterface);
    kDebug(7007) << "ok for resuming:" << resume;
    d->connection->sendnow( resume ? CMD_RESUMEANSWER : CMD_NONE, QByteArray() );
}

void SlaveInterface::sendMessageBoxAnswer(int result)
{
    Q_D(SlaveInterface);
    if (!d->connection) {
        return;
    }

    if (d->connection->suspended()) {
        d->connection->resume();
    }
    QByteArray packedArgs;
    QDataStream stream(&packedArgs, QIODevice::WriteOnly);
    stream << result;
    d->connection->sendnow(CMD_MESSAGEBOXANSWER, packedArgs);
    kDebug(7007) << "message box answer" << result;
}

void SlaveInterface::messageBox(int type, const QString &text, const QString &caption,
                                const QString &buttonYes, const QString &buttonNo)
{
    messageBox(type, text, caption, buttonYes, buttonNo, QString());
}

void SlaveInterface::messageBox(int type, const QString &text, const QString &caption,
                                const QString &buttonYes, const QString &buttonNo, const QString &dontAskAgainName)
{
    Q_D(SlaveInterface);

    if (d->connection) {
        d->connection->suspend();
    }

    QHash<UserNotificationHandler::MessageBoxDataType, QVariant> data;
    data.insert(UserNotificationHandler::MSG_TEXT, text);
    data.insert(UserNotificationHandler::MSG_CAPTION, caption);
    data.insert(UserNotificationHandler::MSG_YES_BUTTON_TEXT, buttonYes);
    data.insert(UserNotificationHandler::MSG_NO_BUTTON_TEXT, buttonNo);
    data.insert(UserNotificationHandler::MSG_DONT_ASK_AGAIN, dontAskAgainName);

    // SMELL: the braindead way to support button icons
    // TODO: Fix this in KIO::SlaveBase.
    if (buttonYes == i18n("&Details")) {
        data.insert(UserNotificationHandler::MSG_YES_BUTTON_ICON, QLatin1String("help-about"));
    }

    if (buttonNo == i18n("Co&ntinue")) {
        data.insert(UserNotificationHandler::MSG_NO_BUTTON_ICON, QLatin1String("arrow-right"));
    }

    globalUserNotificationHandler()->requestMessageBox(this, type, data);
}

void SlaveInterface::setWindow(QWidget* window)
{
    Q_D(SlaveInterface);
    d->parentWindow = window;
}

QWidget* SlaveInterface::window() const
{
    Q_D(const SlaveInterface);
    return d->parentWindow;
}

void SlaveInterface::accept()
{
    Q_D(SlaveInterface);
    d->slaveconnserver->setNextPendingConnection(d->connection);
    d->slaveconnserver->deleteLater();
    d->slaveconnserver = 0;

    connect(d->connection, SIGNAL(readyRead()), SLOT(gotInput()));
}

void SlaveInterface::gotInput()
{
    Q_D(SlaveInterface);
    if (d->dead) {
        // already dead? then slaveDied was emitted and we are done
        return;
    }
    ref();
    if (!dispatch()) {
        d->connection->close();
        d->dead = true;
        QString arg = d->m_protocol;
        if (!d->m_host.isEmpty()) {
            arg += QString::fromLatin1("://") + d->m_host;
        }
        kDebug(7002) << "slave died pid = " << d->m_pid;
        // Tell the job about the problem.
        emit error(ERR_SLAVE_DIED, arg);
        // Tell the scheduler about the problem.
        emit slaveDied(this);
    }
    deref();
    // Here we might be dead!!
}

void SlaveInterface::timeout()
{
    Q_D(SlaveInterface);
    if (d->dead) {
        // already dead? then slaveDied was emitted and we are done
        return;
    }
    if (d->connection->isConnected()) {
        return;
    }

    kDebug(7002) << "slave failed to connect to application pid=" << d->m_pid
                 << " protocol=" << d->m_protocol;
    if (d->m_pid && (::kill(d->m_pid, 0) == 0)) {
        int delta_t = (int) difftime(time(0), d->contact_started);
        kDebug(7002) << "slave is slow... pid=" << d->m_pid << " t=" << delta_t;
        if (delta_t < SLAVE_CONNECTION_TIMEOUT_MAX) {
            QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, this, SLOT(timeout()));
            return;
        }
    }
    kDebug(7002) << "Houston, we lost our slave, pid=" << d->m_pid;
    d->connection->close();
    d->dead = true;
    QString arg = d->m_protocol;
    if (!d->m_host.isEmpty()) {
        arg += QString::fromLatin1("://") + d->m_host;
    }
    kDebug(7002) << "slave died pid = " << d->m_pid;

    ref();
    // Tell the job about the problem.
    emit error(ERR_SLAVE_DIED, arg);
    // Tell the scheduler about the problem.
    emit slaveDied(this);
    // After the above signal we're dead!!
    deref();
}

#include "moc_slaveinterface.cpp"
