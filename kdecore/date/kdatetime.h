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

#ifndef KDATETIME_H
#define KDATETIME_H

#include <ktimezone.h>

#include <QMetaType>
#include <QDataStream>

/**
 * @short A class representing a date and time with an associated time zone
 *
 * @see KTimeZone, KSystemTimeZones, QDateTime, QDate, QTime
 * @see <a href="http://www.w3.org/TR/timezone/">W3C: Working with Time Zones</a>
 * @author Ivailo Monev \<xakepa10@gmail.com\>.
 * @warning KDateTime and KLocale do not output interchangable formats
 */
class KDECORE_EXPORT KDateTime : public QDateTime
{
public:
    KDateTime();
    KDateTime(const QDate &date);
    KDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec = Qt::LocalTime);
    KDateTime(const QDateTime &other);
    KDateTime(const KDateTime &other);

    /**
     * Returns the time zone for the date and time according to the time
     * specification type as follows:
     * - @c Qt::LocalTime  : the local time zone is returned.
     * - @c Qt::UTC        : the UTC time zone is returned.
     *
     * @return time zone as defined above, or invalid in all other cases
     */
    KTimeZone timeZone() const;
    /**
     * @return @c true if date represents night time of the day, @c false otherwise
     * @since 4.20
     */
    bool isNightTime() const;

    static QDate currentLocalDate();
    static KDateTime currentLocalDateTime();
    static KDateTime currentUtcDateTime();
    static KDateTime currentDateTime(const KTimeZone &zone);
};

Q_DECLARE_METATYPE(KDateTime)

QT_BEGIN_NAMESPACE
/** Write @p kdt to the datastream @p out, in binary format. */
QDataStream KDECORE_EXPORT& operator<<(QDataStream &out, const KDateTime &kdt);
/** Read a KDateTime object into @p kdt from @p in, in binary format. */
QDataStream KDECORE_EXPORT& operator>>(QDataStream &in, KDateTime &kdt);
QT_END_NAMESPACE

#endif // KDATETIME_H
