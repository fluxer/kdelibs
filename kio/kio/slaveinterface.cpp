/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <kdebug.h>
#include <klocale.h>

using namespace KIO;

Q_GLOBAL_STATIC(UserNotificationHandler, globalUserNotificationHandler)

SlaveInterface::SlaveInterface(SlaveInterfacePrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{
    connect(&d_ptr->speed_timer, SIGNAL(timeout()), SLOT(calcSpeed()));
}

SlaveInterface::~SlaveInterface()
{
    // Note: no kDebug() here (scheduler is deleted very late)

    delete d_ptr;
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
        case MSG_SLAVE_STATUS: {
            QDataStream stream(rawdata);
            qint64 pid;
            QByteArray protocol;
            QString str;
            qint8 b;
            stream >> pid >> protocol >> str >> b;
            emit slaveStatus(static_cast<pid_t>(pid), protocol, str, (b != 0));
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
            int type;
            stream >> type >> text >> caption >> buttonYes >> buttonNo;
            if (stream.atEnd()) {
                messageBox(type, text, caption, buttonYes, buttonNo);
            } else {
                stream >> dontAskAgainName;
                messageBox(type, text, caption, buttonYes, buttonNo, dontAskAgainName);
            }
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

#include "moc_slaveinterface.cpp"
