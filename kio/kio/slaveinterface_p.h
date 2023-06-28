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

#ifndef KIO_SLAVEINTERFACEPRIVATE_H
#define KIO_SLAVEINTERFACEPRIVATE_H

#include "global.h"
#include "connection.h"

#include <QtCore/QTimer>

#include <sys/time.h>

static const unsigned int max_nums = 8;

class KIO::SlaveInterfacePrivate
{
public:
    SlaveInterfacePrivate(const QString &protocol);
    ~SlaveInterfacePrivate();

    Connection *connection;
    QTimer speed_timer;

    KIO::filesize_t sizes[max_nums];
    long times[max_nums];

    KIO::filesize_t filesize, offset;
    size_t last_time;
    struct timeval start_time;
    uint nums;
    bool slave_calcs_speed;

    QString m_protocol;
    QString m_host;
    QString m_user;
    QString m_passwd;
    KIO::ConnectionServer *slaveconnserver;
    KIO::SimpleJob *m_job;
    quint16 m_port;
    bool dead;
    time_t contact_started;
    time_t m_idleSince;
    int m_refCount;
};

#endif
