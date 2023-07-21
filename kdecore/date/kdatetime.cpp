/*
    This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "kdatetime.h"
#include "ksystemtimezone.h"

KDateTime::KDateTime()
    : QDateTime()
{
}

KDateTime::KDateTime(const QDate &date)
    : QDateTime(date)
{
}

KDateTime::KDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec)
    : QDateTime(date, time, spec)
{
}

KDateTime::KDateTime(const QDateTime &other)
    : QDateTime(other)
{
}

KDateTime::KDateTime(const KDateTime &other)
    : QDateTime(other)
{
}

KTimeZone KDateTime::timeZone() const
{
    switch (timeSpec()) {
        case Qt::LocalTime: {
            return KSystemTimeZones::local();
        }
        case Qt::UTC: {
            return KTimeZone::utc();
        }
        case Qt::OffsetFromUTC: {
            return KTimeZone();
        }
    }
    return KTimeZone();
}

bool KDateTime::isNightTime() const
{
    const int month = date().month();
    const int hour = time().hour();
    if (month <= 3 || month >= 9) {
        return (hour >= 19 || hour <= 6);
    }
    return (hour >= 20 || hour <= 5);
}

QDate KDateTime::currentLocalDate()
{
    return QDateTime::currentDateTime().date();
}

KDateTime KDateTime::currentLocalDateTime()
{
    return QDateTime::currentDateTime();
}

KDateTime KDateTime::currentUtcDateTime()
{
    return QDateTime::currentDateTimeUtc();
}

KDateTime KDateTime::currentDateTime(const KTimeZone &zone)
{
    return zone.toZoneTime(QDateTime::currentDateTimeUtc());
}

QT_BEGIN_NAMESPACE
QDataStream& operator<<(QDataStream &s, const KDateTime &kdt)
{
    s << kdt.date() << kdt.time() << int(kdt.timeSpec());
    return s;
}

QDataStream& operator>>(QDataStream &s, KDateTime &kdt)
{
    QDate date;
    QTime time;
    int spec = int(Qt::UTC);
    s >> date >> time >> spec;
    kdt = KDateTime(date, time, static_cast<Qt::TimeSpec>(spec));
    return s;
}
QT_END_NAMESPACE
