/*
   This file is part of the KDE libraries
   Copyright (c) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KSYSTEMTIMEZONE_H
#define KSYSTEMTIMEZONE_H

#include "ktimezone.h"

/**
 * The KSystemTimeZones class represents the system time zone database. Each individual time zone
 * is defined in a KTimeZone instance.
 *
 * At initialisation, KSystemTimeZones on UNIX systems reads the zone.tab file to obtain the list
 * of system time zones and creates a KTimeZone instance for each one.
 *
 * @note KSystemTimeZones gets the system's time zone configuration, including the current local
 * system time zone and the location of zone.tab. If the local timezone cannot be determinted,
 * KSystemTimeZones will only know about the UTC time zone.
 *
 * Convenience static methods are defined to access its data, alternatively you can access the
 * KTimeZone list instance directly via the zones() method.
 *
 *
 * @short System time zone access
 * @see KTimeZone
 * @ingroup timezones
 * @author Ivailo Monev <xakepa10@gmail.com>.
 */
class KDECORE_EXPORT KSystemTimeZones
{
public:
    /**
     * Returns all the system time zones.
     *
     * @return time zone list
     */
    static const KTimeZoneList zones();

    /**
     * Returns the time zone with the given name.
     *
     * The time zone instance is is a cached one.
     *
     * @param name name of time zone
     * @return time zone, or invalid KTimeZone instance if not found
     * @see zones()
     */
    static KTimeZone zone(const QString &name);

    /**
     * Returns the current local system time zone.
     *
     * The idea of this routine is to provide a robust lookup of the local time zone. On Unix
     * systems, there are a variety of mechanisms for setting this information, and no well defined
     * way of getting it. For example, if you set your time zone to "Europe/London", then the
     * tzname[] maintained by tzset() typically returns { "GMT", "BST" }. The function of this
     * routine is to actually return "Europe/London" (or rather, the corresponding KTimeZone).
     *
     * Note that depending on how the system stores its current time zone, this routine may return
     * a synonym of the expected time zone. For example, "Europe/London", "Europe/Guernsey" and
     * some other time zones are all identical and there may be no way for the routine to
     * distinguish which of these is the correct zone name from the user's point of view.
     *
     * @return local system time zone. We will never return invalid KTimeZone, the fallback is to
               return the UTC time zone.
     */
    static KTimeZone local();

    /**
     * Returns the location of the system time zone zoneinfo database.
     *
     * @return path of directory containing the zoneinfo database
     */
    static QString zoneinfoDir();
};

#endif // KSYSTEMTIMEZONE_H
