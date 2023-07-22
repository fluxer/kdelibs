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

#ifndef KTIMEZONES_H
#define KTIMEZONES_H

#include <kdecore_export.h>

#include <QDateTime>
#include <QList>

class KTimeZonePrivate;

/**
 * Class representing a time zone.
 *
 * The KTimeZone base class contains general descriptive data about the time zone and to translate
 * between UTC and local time.
 *
 * @short class representing a time zone
 * @ingroup timezones
 * @author Ivailo Monev <xakepa10@gmail.com>.
 */
class KDECORE_EXPORT KTimeZone
{
public:
    /**
     * Constructs a null time zone. A null time zone is invalid.
     *
     * @see isValid()
     */
    KTimeZone();

    /**
     * Creates a time zone.
     *
     * @param name        time zone's unique name, which must be the tzfile path relative
     *                    to the location specified for @p source
     * @param countryCode ISO 3166 2-character country code, empty if unknown
     * @param latitude    in degrees (between -90 and +90), UNKNOWN if not known
     * @param longitude   in degrees (between -180 and +180), UNKNOWN if not known
     * @param comment     description of the time zone, if any
     */
    KTimeZone(const QString &name,
              const QString &countryCode = QString(), float latitude = UNKNOWN, float longitude = UNKNOWN,
              const QString &comment = QString());

    KTimeZone(const KTimeZone &tz);
    KTimeZone &operator=(const KTimeZone &tz);

    ~KTimeZone();

    /**
     * Checks whether this is the same instance as another one. If either the name, country code or
     * one of the other properties of the time zone do not match the method returns false.
     *
     * @param rhs other instance
     * @return true if the same instance, else false
     */
    bool operator==(const KTimeZone &rhs) const;
    bool operator!=(const KTimeZone &rhs) const  { return !operator==(rhs); }

    /**
     * Checks whether the instance is valid.
     *
     * @return true if valid, false if invalid
     */
    bool isValid() const;

    /**
     * Returns the name of the time zone.
     *
     * @return name in system-dependent format
     */
    QString name() const;

    /**
     * Returns the two-letter country code of the time zone.
     *
     * @return upper case ISO 3166 2-character country code, empty if unknown
     */
    QString countryCode() const;

    /**
     * Returns the latitude of the time zone.
     *
     * @return latitude in degrees, UNKNOWN if not known
     */
    float latitude() const;

    /**
     * Returns the latitude of the time zone.
     *
     * @return latitude in degrees, UNKNOWN if not known
     */
    float longitude() const;

    /**
     * Returns any comment for the time zone.
     *
     * @return comment, may be empty
     */
    QString comment() const;

    /**
     * Returns the list of time zone abbreviations used by the time zone. This may include
     * historical ones which are no longer in use or have been superseded. The abbreviation for the
     * current offset is first in the list.
     *
     * @return list of abbreviations
     * @see abbreviation()
     */
    QList<QByteArray> abbreviations() const;

    /**
     * Returns the time zone abbreviation at a specified time.
     *
     * @param utcDateTime UTC date/time. An error occurs if @p utcDateTime.timeSpec() is not
                          @p Qt::UTC.
     * @return time zone abbreviation, or empty string if error
     * @see abbreviations()
     */
    QByteArray abbreviation(const QDateTime &utcDateTime) const;

    /**
     * Converts a date/time, which is interpreted as local time in this time zone, into UTC.
     *
     * Because of daylight savings time shifts, the date/time may occur twice. In
     * such cases, this method returns the UTC time for the first occurrence.
     *
     * @param zoneDateTime local date/time. An error occurs if @p zoneDateTime.timeSpec() is not
                           @p Qt::LocalTime.
     * @return UTC date/time, or invalid date/time if error
     * @see toZoneTime()
     */
    QDateTime toUtc(const QDateTime &zoneDateTime) const;

    /**
     * Converts a UTC date/time into local time in this time zone.
     *
     * @param utcDateTime UTC date/time. An error occurs if @p utcDateTime.timeSpec() is not
                          @p Qt::UTC.
     * @return local date/time, or invalid date/time if error
     * @see toUtc()
     */
    QDateTime toZoneTime(const QDateTime &utcDateTime) const;

    /**
     * Returns the current offset of this time zone to UTC. The offset is the number of seconds
     * which you must add to UTC to get local time in this time zone.
     *
     * @return offset to UTC in seconds
     */
    int currentOffset() const;

    /**
     * Returns a standard UTC time zone, with name "UTC".
     *
     * @return UTC time zone
     */
    static KTimeZone utc();

    /**
     * Indicates an invalid UTC offset. This is the offset for invalid time zones.
     */
    static const int InvalidOffset;

    /**
     * A representation for unknown locations, this is a float that does not represent a real
     * latitude or longitude.
     */
    static const float UNKNOWN;

private:
    KTimeZonePrivate *d;
};

typedef QList<KTimeZone> KTimeZoneList;

#endif
