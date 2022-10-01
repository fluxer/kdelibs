/*
   This file is part of the KDE libraries
   Copyright (c) 2005-2007,2009-2012 David Jarvie <djarvie@kde.org>

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

/** @file
 * System time zone functions
 * @author David Jarvie <djarvie@kde.org>.
 * @author S.R.Haque <srhaque@iee.org>.
 */

#ifndef KSYSTEMTIMEZONE_H
#define KSYSTEMTIMEZONE_H

#include "ktimezone.h"

#include <qobject.h>

/**
 * The KSystemTimeZones class represents the system time zone database, consisting
 * of a collection of individual system time zone definitions, indexed by name.
 * Each individual time zone is defined in a KSystemTimeZone or KTzfileTimeZone
 * instance. Additional time zones (of any class derived from KTimeZone) may be
 * added if desired.
 *
 * At initialisation, KSystemTimeZones on UNIX systems reads the zone.tab file
 * to obtain the list of system time zones, and creates a KTzfileTimeZone
 * instance for each one.
 *
 * @note KSystemTimeZones gets the system's time zone configuration, including
 * the current local system time zone and the location of zone.tab. If the local
 * timezone cannot be determinted, KSystemTimeZones will only know about the UTC
 * time zone.
 *
 * Note that KSystemTimeZones is not derived from KTimeZones, but instead contains
 * a KTimeZones instance which holds the system time zone database. Convenience
 * static methods are defined to access its data, or alternatively you can access
 * the KTimeZones instance directly via the timeZones() method.
 *
 * As an example, find the local time in Oman corresponding to the local system
 * time of 12:15:00 on 13th November 1999:
 * \code
 * QDateTime sampleTime(QDate(1999,11,13), QTime(12,15,0), Qt::LocalTime);
 * KTimeZone local = KSystemTimeZones::local();
 * KTimeZone oman  = KSystemTimeZones::zone("Asia/Muscat");
 * QDateTime omaniTime = local.convert(oman, sampleTime);
 * \endcode
 *
 * @note KTzfileTimeZone is used in preference to KSystemTimeZone on UNIX
 * systems since use of the standard system libraries by KSystemTimeZone
 * requires the use of tzset() in several methods. That function reads and
 * parses the local system time zone definition file every time it is called,
 * and this has been observed to make applications hang for many seconds when
 * a large number of KSystemTimeZone calls are made in succession.
 *
 * @short System time zone access
 * @see KTimeZones, KSystemTimeZone, KSystemTimeZoneSource, KTzfileTimeZone
 * @ingroup timezones
 * @author David Jarvie <djarvie@kde.org>.
 * @author S.R.Haque <srhaque@iee.org>.
 */
class KDECORE_EXPORT KSystemTimeZones
{
public:
    /**
     * Returns the unique KTimeZones instance containing the system time zones
     * collection. It is first created if it does not already exist.
     *
     * @return time zones.
     */
    static KTimeZones *timeZones();

    /**
     * Returns all the time zones defined in this collection.
     *
     * @return time zone collection
     */
    static const KTimeZones::ZoneMap zones();

    /**
     * Returns the time zone with the given name.
     *
     * The time zone definition is obtained using system library calls, and may
     * not contain historical data. If you need historical time change data,
     * use the potentially slower method readZone().
     *
     * @param name name of time zone
     * @return time zone (usually a KSystemTimeZone instance), or invalid if not found
     * @see readZone()
     */
    static KTimeZone zone(const QString &name);

    /**
     * Returns the time zone with the given name, containing the full time zone
     * definition read directly from the system time zone database. This may
     * incur a higher overhead than zone(), but will provide whatever historical
     * data the system holds.
     *
     * @param name name of time zone
     * @return time zone (usually a KTzfileTimeZone instance), or invalid if not found
     * @see zone()
     */
    static KTimeZone readZone(const QString &name);

    /**
     * Returns the current local system time zone.
     *
     * The idea of this routine is to provide a robust lookup of the local time
     * zone. On Unix systems, there are a variety of mechanisms for setting this
     * information, and no well defined way of getting it. For example, if you
     * set your time zone to "Europe/London", then the tzname[] maintained by
     * tzset() typically returns { "GMT", "BST" }. The function of this routine
     * is to actually return "Europe/London" (or rather, the corresponding
     * KTimeZone).
     *
     * Note that depending on how the system stores its current time zone, this
     * routine may return a synonym of the expected time zone. For example,
     * "Europe/London", "Europe/Guernsey" and some other time zones are all
     * identical and there may be no way for the routine to distinguish which
     * of these is the correct zone name from the user's point of view.
     *
     * @return local system time zone. If necessary, we will use a series of
     *         heuristics which end by returning UTC. We will never return NULL.
     *         Note that if UTC is returned as a default, it may not belong to the
     *         the collection returned by KSystemTimeZones::zones().
     */
    static KTimeZone local();

    /**
     * Returns the location of the system time zone zoneinfo database.
     *
     * @return path of directory containing the zoneinfo database
     */
    static QString zoneinfoDir();
};

#endif
