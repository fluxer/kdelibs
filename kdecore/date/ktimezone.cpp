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

#include "ktimezone.h"

#include <kdebug.h>
#include <ksystemtimezone.h>
#include <QFile>
#include <QDir>

// for reference:
// https://man7.org/linux/man-pages/man5/tzfile.5.html

extern QString zoneinfoDir(); // in ksystemtimezone.cpp

// #define KTIMEZONE_DUMP

/******************************************************************************/

class KTimeZoneTransition
{
public:
    KTimeZoneTransition();
    KTimeZoneTransition(const KTimeZoneTransition &other);

    QDateTime trasitiontime;
    quint8 transitionindex;
    qint32 utcoffset;
    QByteArray abbreviation;

    bool operator==(const KTimeZoneTransition &other) const
    {
        return (
            trasitiontime == other.trasitiontime
            && transitionindex == other.transitionindex
            && utcoffset == other.utcoffset
            && abbreviation == other.abbreviation
        );
    }
};

KTimeZoneTransition::KTimeZoneTransition()
    : trasitiontime(QDate(1970, 1, 1)),
    transitionindex(0),
    utcoffset(KTimeZone::InvalidOffset)
{
}

KTimeZoneTransition::KTimeZoneTransition(const KTimeZoneTransition &other)
    : trasitiontime(other.trasitiontime),
    transitionindex(other.transitionindex),
    utcoffset(other.utcoffset),
    abbreviation(other.abbreviation)
{
}

class KTimeZoneLocalTime
{
public:
    KTimeZoneLocalTime();
    KTimeZoneLocalTime(const KTimeZoneLocalTime &other);

    qint32 tt_utoff;
    quint8 tt_isdst;
    quint8 tt_desigidx;

    bool operator==(const KTimeZoneLocalTime &other) const
    {
        return (
            tt_utoff == other.tt_utoff
            && tt_isdst == other.tt_isdst
            && tt_desigidx == other.tt_desigidx
        );
    }
};

KTimeZoneLocalTime::KTimeZoneLocalTime()
    : tt_utoff(KTimeZone::InvalidOffset),
    tt_isdst(0),
    tt_desigidx(0)
{
}

KTimeZoneLocalTime::KTimeZoneLocalTime(const KTimeZoneLocalTime &other)
    : tt_utoff(other.tt_utoff),
    tt_isdst(other.tt_isdst),
    tt_desigidx(other.tt_desigidx)
{
}

class KTimeZonePrivate
{
public:
    KTimeZonePrivate();
    KTimeZonePrivate(const QString &nam,
                     const QString &country, float lat, float lon, const QString &cmnt);
    KTimeZonePrivate(const KTimeZonePrivate &rhs);

    KTimeZoneTransition findTransition(const QDateTime &datetime) const;

    QString name;
    QString countryCode;
    QString comment;
    float latitude;
    float longitude;
    QVector<KTimeZoneTransition> transitions;
};

KTimeZonePrivate::KTimeZonePrivate()
    : latitude(KTimeZone::UNKNOWN),
    longitude(KTimeZone::UNKNOWN)
{
}

KTimeZonePrivate::KTimeZonePrivate(const QString &nam,
                                   const QString &country, float lat, float lon, const QString &cmnt)
    : name(nam),
    countryCode(country.toUpper()),
    comment(cmnt),
    latitude(lat),
    longitude(lon)
{
    // Detect duff values.
    if (latitude > 90 || latitude < -90) {
        latitude = KTimeZone::UNKNOWN;
    }
    if (longitude > 180 || longitude < -180) {
        longitude = KTimeZone::UNKNOWN;
    }

    QFile tzfile(zoneinfoDir() + QDir::separator() + name);
    if (!tzfile.open(QFile::ReadOnly)) {
        kWarning() << "Could not open" << tzfile.fileName();
        return;
    }

    kDebug() << "Parsing" << tzfile.fileName();
    QDataStream tzstream(&tzfile);
    tzstream.setByteOrder(QDataStream::BigEndian);

    char tzmagic[5];
    ::memset(tzmagic, 0, sizeof(tzmagic) * sizeof(char));
    tzstream.readRawData(tzmagic, 4);
    if (qstrcmp(tzmagic, "TZif") != 0) {
        kWarning() << "Invalid magic bits" << tzfile.fileName() << tzmagic;
        return;
    }

    // 1 bit for version plus 15 reserved
    tzstream.skipRawData(16);

    Q_ASSERT(sizeof(quint8) == 1);
    Q_ASSERT(sizeof(quint32) == 4);
    quint32 tzh_ttisutcnt = 0;
    quint32 tzh_ttisstdcnt = 0;
    quint32 tzh_leapcnt = 0;
    quint32 tzh_timecnt = 0;
    quint32 tzh_typecnt = 0;
    quint32 tzh_charcnt = 0;
    tzstream >> tzh_ttisutcnt;
    tzstream >> tzh_ttisstdcnt;
    tzstream >> tzh_leapcnt;
    tzstream >> tzh_timecnt;
    tzstream >> tzh_typecnt;
    tzstream >> tzh_charcnt;

    // NOTE: should not be less than or equal to zero
    if (Q_UNLIKELY(tzh_typecnt <= 0)) {
        kWarning() << "Invalid number of local time types" << tzfile.fileName();
        return;
    }

    // get transitions
    qint32 tt_time = 0;
    transitions.resize(tzh_timecnt);
    for (quint32 i = 0; i < tzh_timecnt; i++) {
        tzstream >> tt_time;
        transitions[i].trasitiontime = transitions[i].trasitiontime.addSecs(tt_time);
    }
    quint8 tt_index = 0;
    for (quint32 i = 0; i < tzh_timecnt; i++) {
        tzstream >> tt_index;
        transitions[i].transitionindex = tt_index;
    }

    // get local time
    QVector<KTimeZoneLocalTime> localtimes(tzh_typecnt);
    qint32 tt_utoff = 0;
    quint8 tt_isdst = 0;
    quint8 tt_desigidx = 0;
    for (quint32 i = 0; i < tzh_typecnt; i++) {
        tzstream >> tt_utoff >> tt_isdst >> tt_desigidx;
        localtimes[i].tt_utoff = tt_utoff;
        localtimes[i].tt_isdst = tt_isdst;
        localtimes[i].tt_desigidx = tt_desigidx;
    }

    // get the zone abbreviations
    QByteArray abbreviationsbuffer(tzh_charcnt, '\0');
    tzstream.readRawData(abbreviationsbuffer.data(), abbreviationsbuffer.size());

#ifdef KTIMEZONE_DUMP
    qDebug() << "Transitions for" << tzfile.fileName() << transitions.size();
#endif
    for (quint32 i = 0; i < transitions.size(); i++) {
        const int localtimeindex = transitions[i].transitionindex;

        const KTimeZoneLocalTime localtime = localtimes[localtimeindex];
        transitions[i].utcoffset = localtime.tt_utoff;

        const int abbreviationindex = localtime.tt_desigidx;
        transitions[i].abbreviation = abbreviationsbuffer.mid(
            abbreviationindex, qstrlen(abbreviationsbuffer.constData() + abbreviationindex)
        );
#ifdef KTIMEZONE_DUMP
        qDebug() << "  Transition for" << tzfile.fileName();
        qDebug() << "    -> time =" << transitions[i].trasitiontime.toString();
        qDebug() << "    -> offset = " << transitions[i].utcoffset;
        qDebug() << "    -> abbreviation = " << transitions[i].abbreviation;
#endif
    }
}

KTimeZonePrivate::KTimeZonePrivate(const KTimeZonePrivate &rhs)
    : name(rhs.name),
    countryCode(rhs.countryCode),
    comment(rhs.comment),
    latitude(rhs.latitude),
    longitude(rhs.longitude),
    transitions(rhs.transitions)
{
}

KTimeZoneTransition KTimeZonePrivate::findTransition(const QDateTime &datetime) const
{
    for (int i = transitions.size(); i > 0; i--) {
        const KTimeZoneTransition transition = transitions[i - 1];
        if (datetime >= transition.trasitiontime) {
            return transition;
        }
    }
    return KTimeZoneTransition();
}

/******************************************************************************/

const int   KTimeZone::InvalidOffset = 0x80000000;
const float KTimeZone::UNKNOWN = 1000.0;

KTimeZone::KTimeZone()
    : d(new KTimeZonePrivate())
{
}

KTimeZone::KTimeZone(const QString &name,
                     const QString &countryCode, float latitude, float longitude,
                     const QString &comment)
    : d(new KTimeZonePrivate(name, countryCode, latitude, longitude, comment))
{
}

KTimeZone::KTimeZone(const KTimeZone &tz)
    : d(new KTimeZonePrivate(*tz.d))
{
}

KTimeZone::~KTimeZone()
{
    delete d;
}

KTimeZone &KTimeZone::operator=(const KTimeZone &tz)
{
    if (d != tz.d) {
        d->name = tz.name();
        d->countryCode = tz.countryCode();
        d->comment = tz.comment();
        d->latitude = tz.latitude();
        d->longitude = tz.longitude();
        d->transitions = tz.d->transitions;
    }
    return *this;
}

bool KTimeZone::operator==(const KTimeZone &rhs) const
{
    return (
        name() == rhs.name()
        && countryCode() == rhs.countryCode()
        && comment() == rhs.comment()
        && latitude() == rhs.latitude()
        && longitude() == rhs.longitude()
        && d->transitions == rhs.d->transitions
    );
}

bool KTimeZone::isValid() const
{
    if (d->name.isEmpty()) {
        return false;
    }
    // any other check would be way too expensive (e.g. parsing the time zone file to verify it is
    // valid) but the check bellow will make sure KTimeZone("foo") is not valid and (possibly) mark
    // no longer existing time zones (due to system tzdata file changes) as invalid
    return QFile::exists(zoneinfoDir() + QDir::separator() + d->name);
}

QString KTimeZone::countryCode() const
{
    return d->countryCode;
}

float KTimeZone::latitude() const
{
    return d->latitude;
}

float KTimeZone::longitude() const
{
    return d->longitude;
}

QString KTimeZone::comment() const
{
    return d->comment;
}

QString KTimeZone::name() const
{
    return d->name;
}

QList<QByteArray> KTimeZone::abbreviations() const
{
    QList<QByteArray> result;
    foreach (const KTimeZoneTransition &transition, d->transitions) {
        if (!result.contains(transition.abbreviation)) {
            result.append(transition.abbreviation);
        }
    }
    return result;
}

QByteArray KTimeZone::abbreviation(const QDateTime &utcDateTime) const
{
    if (utcDateTime.timeSpec() != Qt::UTC) {
        return QByteArray();
    }
    const KTimeZoneTransition transition = d->findTransition(utcDateTime);
    return transition.abbreviation;
}

QDateTime KTimeZone::toUtc(const QDateTime &zoneDateTime) const
{
    if (!zoneDateTime.isValid() || zoneDateTime.timeSpec() != Qt::LocalTime) {
        return QDateTime();
    }
    const KTimeZoneTransition transition = d->findTransition(zoneDateTime.toUTC());
    if (transition.utcoffset == KTimeZone::InvalidOffset) {
        QDateTime dt = zoneDateTime;
        dt.setTimeSpec(Qt::UTC);
        return dt;
    }
    QDateTime dt = zoneDateTime.addSecs(-transition.utcoffset);
    dt.setTimeSpec(Qt::UTC);
    return dt;
}

QDateTime KTimeZone::toZoneTime(const QDateTime &utcDateTime) const
{
    if (!utcDateTime.isValid() || utcDateTime.timeSpec() != Qt::UTC) {
        QDateTime dt = utcDateTime;
        dt.setTimeSpec(Qt::LocalTime);
        return dt;
    }
    const KTimeZoneTransition transition = d->findTransition(utcDateTime);
    if (transition.utcoffset == KTimeZone::InvalidOffset) {
        QDateTime dt = utcDateTime;
        dt.setTimeSpec(Qt::LocalTime);
        return dt;
    }
    QDateTime dt = utcDateTime.addSecs(transition.utcoffset);
    dt.setTimeSpec(Qt::LocalTime);
    return dt;
}

int KTimeZone::currentOffset() const
{
    return toZoneTime(QDateTime::currentDateTimeUtc()).utcOffset();
}

KTimeZone KTimeZone::utc()
{
    static KTimeZone utcZone(QLatin1String("UTC"));
    return utcZone;
}
