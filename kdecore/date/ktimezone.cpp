/*
   This file is part of the KDE libraries
   Copyright (c) 2005-2008,2011 David Jarvie <djarvie@kde.org>
   Copyright (c) 2005 S.R.Haque <srhaque@iee.org>.

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

// This file requires HAVE_STRUCT_TM_TM_ZONE to be defined if struct tm member tm_zone is available.
// This file requires HAVE_TM_GMTOFF to be defined if struct tm member tm_gmtoff is available.

#include "ktimezone.h"

#include <config.h>
#include <config-date.h>

#include <sys/time.h>
#include <time.h>
#include <climits>
#include <cstdlib>

#include <QtCore/QSet>
#include <QtCore/QSharedData>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>

#include <kdebug.h>
#include <kglobal.h>
#include <ksystemtimezone.h>

extern QString zoneinfoDir(); // in ksystemtimezone.cpp

/* Return the offset to UTC in the current time zone at the specified UTC time.
 * The thread-safe function localtime_r() is used in preference if available.
 */
static int gmtoff(time_t t)
{
#ifdef _POSIX_THREAD_SAFE_FUNCTIONS
    tm tmtime;
    if (!localtime_r(&t, &tmtime))
        return 0;
#ifdef HAVE_TM_GMTOFF
    return tmtime.tm_gmtoff;
#else
    int lwday = tmtime.tm_wday;
    int lt = 3600*tmtime.tm_hour + 60*tmtime.tm_min + tmtime.tm_sec;
    if (!gmtime_r(&t, &tmtime))
        return 0;
    int uwday = tmtime.tm_wday;
    int ut = 3600*tmtime.tm_hour + 60*tmtime.tm_min + tmtime.tm_sec;
#endif
#else
    tm *tmtime = localtime(&t);
    if (!tmtime)
        return 0;
#ifdef HAVE_TM_GMTOFF
    return tmtime->tm_gmtoff;
#else
    int lwday = tmtime->tm_wday;
    int lt = 3600*tmtime->tm_hour + 60*tmtime->tm_min + tmtime->tm_sec;
    tmtime = gmtime(&t);
    int uwday = tmtime->tm_wday;
    int ut = 3600*tmtime->tm_hour + 60*tmtime->tm_min + tmtime->tm_sec;
#endif
#endif
#ifndef HAVE_TM_GMTOFF
    if (lwday != uwday)
    {
      // Adjust for different day
      if (lwday == uwday + 1  ||  (lwday == 0 && uwday == 6))
        lt += 24*3600;
      else
        lt -= 24*3600;
    }
    return lt - ut;
#endif
}

// Use this replacement for QDateTime::setTime_t(uint) since our time
// values are signed.
static QDateTime fromTime_t(qint32 seconds)
{
    static const QDate epochDate(1970,1,1);
    static const QTime epochTime(0,0,0);
    int days = seconds / 86400;
    seconds -= days * 86400;
    if (seconds < 0)
    {
        --days;
        seconds += 86400;
    }
    return QDateTime(epochDate.addDays(days), epochTime.addSecs(seconds), Qt::UTC);
}

/******************************************************************************/

class KTimeZonesPrivate
{
public:
    KTimeZonesPrivate() {}

    KTimeZones::ZoneMap zones;
};


KTimeZones::KTimeZones()
  : d(new KTimeZonesPrivate)
{
}

KTimeZones::~KTimeZones()
{
    delete d;
}

const KTimeZones::ZoneMap KTimeZones::zones() const
{
    return d->zones;
}

bool KTimeZones::add(const KTimeZone &zone)
{
    if (!zone.isValid())
        return false;
    if (d->zones.find(zone.name()) != d->zones.end())
        return false;    // name already exists
    d->zones.insert(zone.name(), zone);
    return true;
}

KTimeZone KTimeZones::remove(const KTimeZone &zone)
{
    if (zone.isValid())
    {
        for (ZoneMap::Iterator it = d->zones.begin(), end = d->zones.end();  it != end;  ++it)
        {
            if (it.value() == zone)
            {
                d->zones.erase(it);
                return zone;
            }
        }
    }
    return KTimeZone();
}

KTimeZone KTimeZones::remove(const QString &name)
{
    if (!name.isEmpty())
    {
        ZoneMap::Iterator it = d->zones.find(name);
        if (it != d->zones.end())
        {
            KTimeZone zone = it.value();
            d->zones.erase(it);
            return zone;
        }
    }
    return KTimeZone();
}

void KTimeZones::clear()
{
  d->zones.clear();
}

KTimeZone KTimeZones::zone(const QString &name) const
{
    if (!name.isEmpty())
    {
        ZoneMap::ConstIterator it = d->zones.constFind(name);
        if (it != d->zones.constEnd())
            return it.value();
        if (name == KTimeZone::utc().name())
            return KTimeZone::utc();
    }
    return KTimeZone();    // error
}


/******************************************************************************/

class KTimeZonePhasePrivate : public QSharedData
{
    public:
        QByteArray       abbreviations;  // time zone abbreviations (zero-delimited)
        QString          comment;        // optional comment
        int              utcOffset;      // seconds to add to UTC
        bool             dst;            // true if daylight savings time

        explicit KTimeZonePhasePrivate(int offset = 0, bool ds = false)
        : QSharedData(),
          utcOffset(offset),
          dst(ds)
        {}
        KTimeZonePhasePrivate(const KTimeZonePhasePrivate& rhs)
        : QSharedData(rhs),
          abbreviations(rhs.abbreviations),
          comment(rhs.comment),
          utcOffset(rhs.utcOffset),
          dst(rhs.dst)
        {}
        bool operator==(const KTimeZonePhasePrivate &rhs) const
        {
            return abbreviations == rhs.abbreviations
               &&  comment       == rhs.comment
               &&  utcOffset     == rhs.utcOffset
               &&  dst           == rhs.dst;
        }
};


KTimeZone::Phase::Phase()
  : d(new KTimeZonePhasePrivate)
{
}

KTimeZone::Phase::Phase(int utcOffset, const QByteArray &abbrevs,
                        bool dst, const QString &cmt)
  : d(new KTimeZonePhasePrivate(utcOffset, dst))
{
    d->abbreviations = abbrevs;
    d->comment       = cmt;
}

KTimeZone::Phase::Phase(int utcOffset, const QList<QByteArray> &abbrevs,
                        bool dst, const QString &cmt)
  : d(new KTimeZonePhasePrivate(utcOffset, dst))
{
    for (int i = 0, end = abbrevs.count();  i < end;  ++i)
    {
        if (i > 0)
            d->abbreviations += '\0';
        d->abbreviations += abbrevs[i];
    }
    d->comment = cmt;
}

KTimeZone::Phase::Phase(const KTimeZone::Phase &rhs)
  : d(rhs.d)
{
}

KTimeZone::Phase::~Phase()
{
}

KTimeZone::Phase &KTimeZone::Phase::operator=(const KTimeZone::Phase &rhs)
{
    d = rhs.d;
    return *this;
}

bool KTimeZone::Phase::operator==(const KTimeZone::Phase &rhs) const
{
    return d == rhs.d  ||  *d == *rhs.d;
}

int KTimeZone::Phase::utcOffset() const
{
    return d->utcOffset;
}

QList<QByteArray> KTimeZone::Phase::abbreviations() const
{
    return d->abbreviations.split('\0');
}

bool KTimeZone::Phase::isDst() const
{
    return d->dst;
}

QString KTimeZone::Phase::comment() const
{
    return d->comment;
}


/******************************************************************************/

class KTimeZoneTransitionPrivate
{
public:
    QDateTime time;
    KTimeZone::Phase phase;
};


KTimeZone::Transition::Transition()
    : d(new KTimeZoneTransitionPrivate)
{
}

KTimeZone::Transition::Transition(const QDateTime &t, const KTimeZone::Phase &p)
    : d(new KTimeZoneTransitionPrivate)
{
    d->time  = t;
    d->phase = p;
}

KTimeZone::Transition::Transition(const KTimeZone::Transition &t)
    : d(new KTimeZoneTransitionPrivate)
{
    d->time  = t.d->time;
    d->phase = t.d->phase;
}

KTimeZone::Transition::~Transition()
{
    delete d;
}

KTimeZone::Transition &KTimeZone::Transition::operator=(const KTimeZone::Transition &t)
{
    d->time  = t.d->time;
    d->phase = t.d->phase;
    return *this;
}

bool KTimeZone::Transition::operator<(const KTimeZone::Transition &rhs) const
{
    return d->time < rhs.d->time;
}

QDateTime        KTimeZone::Transition::time() const   { return d->time; }
KTimeZone::Phase KTimeZone::Transition::phase() const  { return d->phase; }


/******************************************************************************/

class KTimeZoneDataPrivate
{
    public:
        QList<KTimeZone::Phase>       phases;
        QList<KTimeZone::Transition>  transitions;
        QList<int>                    utcOffsets;
        QList<QByteArray>             abbreviations;
        KTimeZone::Phase              prePhase;    // phase to use before the first transition

        KTimeZoneDataPrivate() {}
        // Find the last transition before a specified UTC or local date/time.
        int transitionIndex(const QDateTime &dt) const;
        bool transitionIndexes(const QDateTime &start, const QDateTime &end, int &ixstart, int &ixend) const;
        bool isSecondOccurrence(const QDateTime &utcLocalTime, int transitionIndex) const;
};


/******************************************************************************/

class KTimeZonePrivate : public QSharedData
{
public:
    KTimeZonePrivate() : source(0), data(0), refCount(1), cachedTransitionIndex(-1) {}
    KTimeZonePrivate(KTimeZoneSource *src, const QString& nam,
                     const QString &country, float lat, float lon, const QString &cmnt);
    KTimeZonePrivate(const KTimeZonePrivate &);
    ~KTimeZonePrivate()  { delete data; }
    KTimeZonePrivate &operator=(const KTimeZonePrivate &);
    static KTimeZoneSource *utcSource();
    static void cleanup();

    KTimeZoneSource *source;
    QString name;
    QString countryCode;
    QString comment;
    float   latitude;
    float   longitude;
    mutable KTimeZoneData *data;
    int     refCount; // holds the number of KTimeZoneBackend instances using the KTimeZonePrivate instance as a d-pointer.
    int       cachedTransitionIndex;
    QDateTime cachedTransitionStartZoneTime;
    QDateTime cachedTransitionEndZoneTime;
    bool      cachedTransitionTimesValid;

private:
    static KTimeZoneSource *mUtcSource;
};

KTimeZoneSource *KTimeZonePrivate::mUtcSource = 0;


KTimeZonePrivate::KTimeZonePrivate(KTimeZoneSource *src, const QString& nam,
                 const QString &country, float lat, float lon, const QString &cmnt)
  : source(src),
    name(nam),
    countryCode(country.toUpper()),
    comment(cmnt),
    latitude(lat),
    longitude(lon),
    data(0),
    refCount(1),
    cachedTransitionIndex(-1)
{
    // Detect duff values.
    if (latitude > 90 || latitude < -90)
        latitude = KTimeZone::UNKNOWN;
    if (longitude > 180 || longitude < -180)
        longitude = KTimeZone::UNKNOWN;
}

KTimeZonePrivate::KTimeZonePrivate(const KTimeZonePrivate &rhs)
  : QSharedData(rhs),
    source(rhs.source),
    name(rhs.name),
    countryCode(rhs.countryCode),
    comment(rhs.comment),
    latitude(rhs.latitude),
    longitude(rhs.longitude),
    refCount(1),
    cachedTransitionIndex(rhs.cachedTransitionIndex),
    cachedTransitionStartZoneTime(rhs.cachedTransitionStartZoneTime),
    cachedTransitionEndZoneTime(rhs.cachedTransitionEndZoneTime),
    cachedTransitionTimesValid(rhs.cachedTransitionTimesValid)
{
    if (rhs.data)
        data = rhs.data->clone();
    else
        data = 0;
}

KTimeZonePrivate &KTimeZonePrivate::operator=(const KTimeZonePrivate &rhs)
{
    // Changing the contents of a KTimeZonePrivate instance by means of operator=() doesn't affect how
    // many references to it are held.
    source      = rhs.source;
    name        = rhs.name;
    countryCode = rhs.countryCode;
    comment     = rhs.comment;
    latitude    = rhs.latitude;
    longitude   = rhs.longitude;
    cachedTransitionIndex         = rhs.cachedTransitionIndex;
    cachedTransitionStartZoneTime = rhs.cachedTransitionStartZoneTime;
    cachedTransitionEndZoneTime   = rhs.cachedTransitionEndZoneTime;
    cachedTransitionTimesValid    = rhs.cachedTransitionTimesValid;
    delete data;
    if (rhs.data)
        data = rhs.data->clone();
    else
        data = 0;
    // refCount is unchanged
    return *this;
}

KTimeZoneSource *KTimeZonePrivate::utcSource()
{
    if (!mUtcSource)
    {
        mUtcSource = new KTimeZoneSource(zoneinfoDir());
        qAddPostRoutine(KTimeZonePrivate::cleanup);
    }
    return mUtcSource;
}

void KTimeZonePrivate::cleanup()
{
    delete mUtcSource;
}


/******************************************************************************/

K_GLOBAL_STATIC(KTimeZonePrivate, s_emptyTimeZonePrivate)

KTimeZoneBackend::KTimeZoneBackend()
  : d(&*s_emptyTimeZonePrivate)
{
    ++d->refCount;
}

KTimeZoneBackend::KTimeZoneBackend(const QString &name)
  : d(new KTimeZonePrivate(KTimeZonePrivate::utcSource(), name, QString(), KTimeZone::UNKNOWN, KTimeZone::UNKNOWN, QString()))
{}

KTimeZoneBackend::KTimeZoneBackend(KTimeZoneSource *source, const QString &name,
        const QString &countryCode, float latitude, float longitude, const QString &comment)
  : d(new KTimeZonePrivate(source, name, countryCode, latitude, longitude, comment))
{}

KTimeZoneBackend::KTimeZoneBackend(const KTimeZoneBackend &other)
  : d(other.d)
{
    ++d->refCount;
}

KTimeZoneBackend::~KTimeZoneBackend()
{
    if (d && --d->refCount == 0)
        delete d;
    d = 0;
}

KTimeZoneBackend &KTimeZoneBackend::operator=(const KTimeZoneBackend &other)
{
    if (d != other.d)
    {
        if (--d->refCount == 0)
            delete d;
        d = other.d;
        ++d->refCount;
    }
    return *this;
}

QByteArray KTimeZoneBackend::type() const
{
    return "KTimeZone";
}

KTimeZoneBackend *KTimeZoneBackend::clone() const
{
    return new KTimeZoneBackend(*this);
}

int KTimeZoneBackend::offsetAtZoneTime(const KTimeZone* caller, const QDateTime &zoneDateTime, int *secondOffset) const
{
    if (!zoneDateTime.isValid()  ||  zoneDateTime.timeSpec() != Qt::LocalTime)    // check for invalid time
    {
        if (secondOffset)
            *secondOffset = 0;
        return 0;
    }
    const QList<KTimeZone::Transition> transitions = caller->transitions();
    int index = d->cachedTransitionIndex;
    if (index >= 0 && index < transitions.count())
    {
        // There is a cached transition - check whether zoneDateTime uses it.
        // Caching is used because this method has been found to consume
        // significant CPU in real life applications.
        if (!d->cachedTransitionTimesValid)
        {
            const int offset = transitions[index].phase().utcOffset();
	    const int preoffset = (index > 0) ? transitions[index - 1].phase().utcOffset() : d->data ? d->data->previousUtcOffset() : 0;
            d->cachedTransitionStartZoneTime = transitions[index].time().addSecs(qMax(offset, preoffset));
            if (index + 1 < transitions.count())
	    {
                const int postoffset = transitions[index + 1].phase().utcOffset();
                d->cachedTransitionEndZoneTime = transitions[index + 1].time().addSecs(qMin(offset, postoffset));
	    }
            d->cachedTransitionTimesValid = true;
        }
        QDateTime dtutc = zoneDateTime;
        dtutc.setTimeSpec(Qt::UTC);
        if (dtutc >= d->cachedTransitionStartZoneTime
        &&  (index + 1 >= transitions.count() || dtutc < d->cachedTransitionEndZoneTime))
        {
            // The time falls within the cached transition limits, so return its UTC offset
            const int offset = transitions[index].phase().utcOffset();
            if (secondOffset)
                *secondOffset = offset;
#ifdef ENABLE_TESTING
            kDebug(161) << "-> Using cache";   // enable the debug area to see this in the tests
#endif
            return offset;
        }
    }

    // The time doesn't fall within the cached transition, or there isn't a cached transition
#ifdef ENABLE_TESTING
    kDebug(161) << "-> No cache";   // enable the debug area to see this in the tests
#endif
    bool validTime;
    int secondIndex = -1;
    index = caller->transitionIndex(zoneDateTime, (secondOffset ? &secondIndex : 0), &validTime);
    const KTimeZone::Transition* tr = (index >= 0) ? &transitions[index] : 0;
    const int offset = tr ? tr->phase().utcOffset()
                          : validTime ? (d->data ? d->data->previousUtcOffset() : 0)
                                      : KTimeZone::InvalidOffset;
    if (secondOffset)
        *secondOffset = (secondIndex >= 0) ? transitions.at(secondIndex).phase().utcOffset() : offset;

    // Cache transition data for subsequent date/time values which occur after the same transition.
    d->cachedTransitionIndex = index;
    d->cachedTransitionTimesValid = false;
    return offset;
}

int KTimeZoneBackend::offsetAtUtc(const KTimeZone* caller, const QDateTime &utcDateTime) const
{
    if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC)    // check for invalid time
        return 0;
    const QList<KTimeZone::Transition> transitions = caller->transitions();
    int index = d->cachedTransitionIndex;
    if (index >= 0 && index < transitions.count())
    {
        // There is a cached transition - check whether utcDateTime uses it.
        if (utcDateTime >= transitions[index].time()
        &&  (index + 1 >= transitions.count()
             || utcDateTime < transitions[index + 1].time()))
        {
            // The time falls within the cached transition, so return its UTC offset
#ifdef ENABLE_TESTING
            kDebug(161) << "Using cache";   // enable the debug area to see this in the tests
#endif
            return transitions[index].phase().utcOffset();
        }
    }

    // The time doesn't fall within the cached transition, or there isn't a cached transition
#ifdef ENABLE_TESTING
    kDebug(161) << "No cache";   // enable the debug area to see this in the tests
#endif
    index = caller->transitionIndex(utcDateTime);
    d->cachedTransitionIndex = index;   // cache transition data
    d->cachedTransitionTimesValid = false;
    const KTimeZone::Transition* tr = (index >= 0) ? &transitions.at(index) : 0;
    return tr ? tr->phase().utcOffset() : (d->data ? d->data->previousUtcOffset() : 0);
}

int KTimeZoneBackend::offset(const KTimeZone* caller, time_t t) const
{
    return offsetAtUtc(caller, KTimeZone::fromTime_t(t));
}

bool KTimeZoneBackend::isDstAtUtc(const KTimeZone* caller, const QDateTime &utcDateTime) const
{
    if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC)    // check for invalid time
        return false;
    const KTimeZone::Transition *tr = caller->transition(utcDateTime);
    if (!tr)
        return false;
    return tr->phase().isDst();
}

bool KTimeZoneBackend::isDst(const KTimeZone* caller, time_t t) const
{
    return isDstAtUtc(caller, KTimeZone::fromTime_t(t));
}

/******************************************************************************/

#if SIZEOF_TIME_T == 8
const time_t KTimeZone::InvalidTime_t = 0x800000000000000LL;
#else
const time_t KTimeZone::InvalidTime_t = 0x80000000;
#endif
const int    KTimeZone::InvalidOffset = 0x80000000;
const float  KTimeZone::UNKNOWN = 1000.0;


KTimeZone::KTimeZone()
  : d(new KTimeZoneBackend())
{}

KTimeZone::KTimeZone(const QString &name)
  : d(new KTimeZoneBackend(name))
{}

KTimeZone::KTimeZone(KTimeZoneSource *source, const QString &name,
        const QString &countryCode, float latitude, float longitude,
        const QString &comment)
  : d(new KTimeZoneBackend(source, name, countryCode, latitude, longitude, comment))
{}

KTimeZone::KTimeZone(const KTimeZone &tz)
  : d(tz.d->clone())
{}

KTimeZone::~KTimeZone()
{
    delete d;
}

KTimeZone::KTimeZone(KTimeZoneBackend *impl)
  : d(impl)
{
    // 'impl' should be a newly constructed object, with refCount = 1
    Q_ASSERT(d->d->refCount == 1 || d->d == &*s_emptyTimeZonePrivate);
}

KTimeZone &KTimeZone::operator=(const KTimeZone &tz)
{
    if (d != tz.d)
    {
        delete d;
        d = tz.d->clone();
    }
    return *this;
}

bool KTimeZone::operator==(const KTimeZone &rhs) const
{
    return d->d == rhs.d->d;
}

bool KTimeZone::isValid() const
{
    return !d->d->name.isEmpty();
}

QString KTimeZone::countryCode() const
{
    return d->d->countryCode;
}

float KTimeZone::latitude() const
{
    return d->d->latitude;
}

float KTimeZone::longitude() const
{
    return d->d->longitude;
}

QString KTimeZone::comment() const
{
    return d->d->comment;
}

QString KTimeZone::name() const
{
    return d->d->name;
}

QList<QByteArray> KTimeZone::abbreviations() const
{
    if (!data(true))
        return QList<QByteArray>();
    return d->d->data->abbreviations();
}

QByteArray KTimeZone::abbreviation(const QDateTime &utcDateTime) const
{
    if (utcDateTime.timeSpec() != Qt::UTC  ||  !data(true))
        return QByteArray();
    return d->d->data->abbreviation(utcDateTime);
}

QList<int> KTimeZone::utcOffsets() const
{
    if (!data(true))
        return QList<int>();
    return d->d->data->utcOffsets();
}

QList<KTimeZone::Phase> KTimeZone::phases() const
{
    if (!data(true))
        return QList<KTimeZone::Phase>();
    return d->d->data->phases();
}

QList<KTimeZone::Transition> KTimeZone::transitions(const QDateTime &start, const QDateTime &end) const
{
    if (!data(true))
        return QList<KTimeZone::Transition>();
    return d->d->data->transitions(start, end);
}

const KTimeZone::Transition *KTimeZone::transition(const QDateTime &dt, const Transition **secondTransition,
                                                   bool *validTime) const
{
    if (!data(true)) {
        if (validTime)
            *validTime = false;
        return 0;
    }
    return d->d->data->transition(dt, secondTransition, validTime);
}

int KTimeZone::transitionIndex(const QDateTime &dt, int *secondIndex, bool *validTime) const
{
    if (!data(true)) {
        if (validTime)
            *validTime = false;
        return -1;
    }
    return d->d->data->transitionIndex(dt, secondIndex, validTime);
}

QList<QDateTime> KTimeZone::transitionTimes(const Phase &phase, const QDateTime &start, const QDateTime &end) const
{
    if (!data(true))
        return QList<QDateTime>();
    return d->d->data->transitionTimes(phase, start, end);
}

KTimeZoneSource *KTimeZone::source() const
{
    return d->d->source;
}

const KTimeZoneData *KTimeZone::data(bool create) const
{
    if (!isValid())
        return 0;
    if (create && !d->d->data && d->d->source->useZoneParse())
        d->d->data = d->d->source->parse(*this);
    return d->d->data;
}

void KTimeZone::setData(KTimeZoneData *data, KTimeZoneSource *source)
{
    if (!isValid())
        return;
    delete d->d->data;
    d->d->data = data;
    if (source)
        d->d->source = source;
}

bool KTimeZone::updateBase(const KTimeZone &other)
{
    if (d->d->name.isEmpty() || d->d->name != other.d->d->name)
        return false;
    d->d->countryCode = other.d->d->countryCode;
    d->d->comment     = other.d->d->comment;
    d->d->latitude    = other.d->d->latitude;
    d->d->longitude   = other.d->d->longitude;
    return true;
}

bool KTimeZone::parse() const
{
    if (!isValid())
        return false;
    if (d->d->source->useZoneParse())
    {
        delete d->d->data;
        d->d->data = d->d->source->parse(*this);
    }
    return d->d->data;
}

QDateTime KTimeZone::toUtc(const QDateTime &zoneDateTime) const
{
    if (!zoneDateTime.isValid()  ||  zoneDateTime.timeSpec() != Qt::LocalTime)
        return QDateTime();
    const int secs = offsetAtZoneTime(zoneDateTime);
    if (secs == InvalidOffset)
        return QDateTime();
    QDateTime dt = zoneDateTime;
    dt.setTimeSpec(Qt::UTC);
    return dt.addSecs(-secs);
}

QDateTime KTimeZone::toZoneTime(const QDateTime &utcDateTime, bool *secondOccurrence) const
{
    if (secondOccurrence)
        *secondOccurrence = false;
    if (!utcDateTime.isValid()  ||  utcDateTime.timeSpec() != Qt::UTC)    // check for invalid time
        return QDateTime();

    // Convert UTC to local time
    if (!data(true))
    {
        // No data - default to UTC
        QDateTime dt = utcDateTime;
        dt.setTimeSpec(Qt::LocalTime);
        return dt;
    }

    const KTimeZoneData *data = d->d->data;
    const int index = data->transitionIndex(utcDateTime);
    const int secs = (index >= 0) ? data->transitions().at(index).phase().utcOffset() : data->previousUtcOffset();
    QDateTime dt = utcDateTime.addSecs(secs);
    if (secondOccurrence)
    {
        // Check whether the local time occurs twice around a daylight savings time
        // shift, and if so, whether it's the first or second occurrence.
        *secondOccurrence = data->d->isSecondOccurrence(dt, index);
    }
    dt.setTimeSpec(Qt::LocalTime);
    return dt;
}

QDateTime KTimeZone::convert(const KTimeZone &newZone, const QDateTime &zoneDateTime) const
{
    if (newZone == *this)
    {
        if (zoneDateTime.timeSpec() != Qt::LocalTime)
            return QDateTime();
        return zoneDateTime;
    }
    return newZone.toZoneTime(toUtc(zoneDateTime));
}

int KTimeZone::offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset) const
{
    return d->offsetAtZoneTime(this, zoneDateTime, secondOffset);
}

int KTimeZone::offsetAtUtc(const QDateTime &utcDateTime) const
{
    return d->offsetAtUtc(this, utcDateTime);
}

int KTimeZone::offset(time_t t) const
{
    return d->offset(this, t);
}

int KTimeZone::currentOffset(Qt::TimeSpec basis) const
{
    // Get current offset of this time zone to UTC
    const time_t now = time(0);
    const int secs = offset(now);

    switch (basis)
    {
        case Qt::LocalTime:
            // Return the current offset of this time zone to the local system time
            return secs - gmtoff(now);
        case Qt::UTC:
            // Return the current offset of this time zone to UTC
            return secs;

        default:
            break;
    }
    return 0;
}

bool KTimeZone::isDstAtUtc(const QDateTime &utcDateTime) const
{
    return d->isDstAtUtc(this, utcDateTime);
}

bool KTimeZone::isDst(time_t t) const
{
    return d->isDst(this, t);
}

KTimeZone KTimeZone::utc()
{
    static KTimeZone utcZone(KTimeZonePrivate::utcSource(), QLatin1String("UTC"));
    return utcZone;
}

QDateTime KTimeZone::fromTime_t(time_t t)
{
    static const int secondsADay = 86400;
    static const QDate epochDate(1970,1,1);
    static const QTime epochTime(0,0,0);
    int days = t / secondsADay;
    int secs;
    if (t >= 0)
        secs = t % secondsADay;
    else
    {
        secs = secondsADay - (-t % secondsADay);
        --days;
    }
    return QDateTime(epochDate.addDays(days), epochTime.addSecs(secs), Qt::UTC);
}

time_t KTimeZone::toTime_t(const QDateTime &utcDateTime)
{
    static const QDate epochDate(1970,1,1);
    static const QTime epochTime(0,0,0);
    if (utcDateTime.timeSpec() != Qt::UTC)
        return InvalidTime_t;
    const qint64 days = epochDate.daysTo(utcDateTime.date());
    const qint64 secs = epochTime.secsTo(utcDateTime.time());
    const qint64 t64 = days * 86400 + secs;
    const time_t t = static_cast<time_t>(t64);
    if (static_cast<qint64>(t) != t64)
        return InvalidTime_t;
    return t;
}


/******************************************************************************/

class KTimeZoneSourcePrivate
{
public:
    KTimeZoneSourcePrivate()
      : mUseZoneParse(true) {}

    KTimeZoneSourcePrivate(const QString &loc)
      : mUseZoneParse(true), mLocation(loc) {}

    bool mUseZoneParse;
    QString mLocation;
};


KTimeZoneSource::KTimeZoneSource()
  : d(new KTimeZoneSourcePrivate())
{
}

KTimeZoneSource::KTimeZoneSource(const QString &location)
  : d(new KTimeZoneSourcePrivate(location))
{
    if (location.length() > 1 && location.endsWith(QLatin1Char('/')))
        d->mLocation.chop(1);
}

KTimeZoneSource::KTimeZoneSource(bool useZoneParse)
  : d(new KTimeZoneSourcePrivate())
{
    d->mUseZoneParse = useZoneParse;
}

KTimeZoneSource::~KTimeZoneSource()
{
    delete d;
}

// for reference:
// https://man7.org/linux/man-pages/man5/tzfile.5.html
KTimeZoneData *KTimeZoneSource::parse(const KTimeZone &zone) const
{
    Q_ASSERT(d->mUseZoneParse);  // method should never be called if it isn't usable

    quint32 abbrCharCount;     // the number of characters of time zone abbreviation strings
    quint32 ttisgmtcnt;
    quint8  T_, Z_, i_, f_;    // tzfile identifier prefix

    QString path = zone.name();
    if (!path.startsWith(QLatin1Char('/')))
    {
        if (d->mLocation == QLatin1String("/"))
            path.prepend(d->mLocation);
        else
            path = d->mLocation + QLatin1Char('/') + path;
    }
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
    {
        kError() << "Cannot open " << f.fileName();
        return 0;
    }
    QDataStream str(&f);
    str.setByteOrder(QDataStream::BigEndian);

    // Read the file type identifier
    str >> T_ >> Z_ >> i_ >> f_;
    if (T_ != 'T' || Z_ != 'Z' || i_ != 'i' || f_ != 'f')
    {
        kError() << "Not a TZFILE: " << f.fileName();
        return 0;
    }
    // Discard 16 bytes reserved for future use
    unsigned i;
    for (i = 0; i < 4; ++i)
        str >> ttisgmtcnt;

    KTimeZoneData* data = new KTimeZoneData();

    // Read the sizes of arrays held in the file
    quint32 nTransitionTimes;
    quint32 nLocalTimeTypes;
    quint32 nLeapSecondAdjusts;
    quint32 nIsStandard;
    quint32 nIsUtc;
    str >> nIsUtc
        >> nIsStandard
        >> nLeapSecondAdjusts
        >> nTransitionTimes
        >> nLocalTimeTypes
        >> abbrCharCount;
    // kDebug() << "header: " << nIsUtc << ", " << nIsStandard << ", " << nLeapSecondAdjusts << ", " <<
    //    nTransitionTimes << ", " << nLocalTimeTypes << ", " << abbrCharCount;

    // Read the transition times, at which the rules for computing local time change
    struct TransitionTime
    {
        qint32 time;            // time (as returned by time(2)) at which the rules for computing local time change
        quint8 localTimeIndex;  // index into the LocalTimeType array
    };
    // kDebug()<<"Reading zone "<<zone.name();
    TransitionTime *transitionTimes = new TransitionTime[nTransitionTimes];
    for (i = 0;  i < nTransitionTimes;  ++i)
    {
        str >> transitionTimes[i].time;
    }
    for (i = 0;  i < nTransitionTimes;  ++i)
    {
        str >> transitionTimes[i].localTimeIndex;
        // kDebug() << "Transition time "<<i<<": "<<transitionTimes[i].time<<"   lt index="<<(int)transitionTimes[i].localTimeIndex;
    }

    // Read the local time types
    struct LocalTimeType
    {
        qint32 gmtoff;     // number of seconds to be added to UTC
        bool   isdst;      // whether tm_isdst should be set by localtime(3)
        quint8 abbrIndex;  // index into the list of time zone abbreviations
    };
    LocalTimeType *localTimeTypes = new LocalTimeType[nLocalTimeTypes];
    LocalTimeType *ltt = localTimeTypes;
    for (i = 0;  i < nLocalTimeTypes;  ++ltt, ++i)
    {
        str >> ltt->gmtoff;
        str >> ltt->isdst;
        str >> ltt->abbrIndex;
        // kDebug() << "local type: " << ltt->gmtoff << ", " << ltt->isdst << ", " << ltt->abbrIndex;
    }

    // Read the timezone abbreviations. They are stored as null terminated strings in
    // a character array.
    // Make sure we don't fall foul of maliciously coded time zone abbreviations.
    if (abbrCharCount > 64)
    {
        kError() << "excessive length for timezone abbreviations: " << abbrCharCount;
        delete data;
        delete[] transitionTimes;
        delete[] localTimeTypes;
        return 0;
    }
    QByteArray array(abbrCharCount, 0);
    str.readRawData(array.data(), array.size());
    const char *abbrs = array.constData();
    if (abbrs[abbrCharCount - 1] != 0)
    {
        // These abbreviations are corrupt!
        kError() << "timezone abbreviations not null terminated: " << abbrs[abbrCharCount - 1];
        delete data;
        delete[] transitionTimes;
        delete[] localTimeTypes;
        return 0;
    }
    quint8 n = 0;
    QList<QByteArray> abbreviations;
    for (i = 0;  i < abbrCharCount;  ++n, i += strlen(abbrs + i) + 1)
    {
        abbreviations += QByteArray(abbrs + i);
        // Convert the LocalTimeTypes pointer to a sequential index
        ltt = localTimeTypes;
        for (unsigned j = 0;  j < nLocalTimeTypes;  ++ltt, ++j)
        {
            if (ltt->abbrIndex == i)
                ltt->abbrIndex = n;
        }
    }

    // Skip the leap second adjustments, standard/wall and UTC/local time indicators.
    const int skiptotal = (
        (nLeapSecondAdjusts * (sizeof(qint32) + sizeof(quint32))) +
        (nIsStandard * sizeof(qint8)) +
        (nIsUtc * sizeof(qint8))
    );
    str.skipRawData(skiptotal);

    // Find the starting offset from UTC to use before the first transition time.
    // This is first non-daylight savings local time type, or if there is none,
    // the first local time type.
    LocalTimeType* firstLtt = 0;
    ltt = localTimeTypes;
    for (i = 0;  i < nLocalTimeTypes;  ++ltt, ++i)
    {
        if (!ltt->isdst)
        {
            firstLtt = ltt;
            break;
        }
    }

    // Compile the time type data into a list of KTimeZone::Phase instances.
    // Also check for local time types which are identical (this does happen)
    // and use the same Phase instance for each.
    QByteArray abbrev;
    QList<KTimeZone::Phase> phases;
    QList<QByteArray> phaseAbbrevs;
    QVector<int> lttLookup(nLocalTimeTypes);
    ltt = localTimeTypes;
    for (i = 0;  i < nLocalTimeTypes;  ++ltt, ++i)
    {
        if (ltt->abbrIndex >= abbreviations.count())
        {
            kError() << "KTimeZoneSource::parse(): abbreviation index out of range";
            abbrev = "???";
        }
        else
            abbrev = abbreviations[ltt->abbrIndex];
        // Check for an identical Phase
        int phindex = 0;
        for (int j = 0, jend = phases.count();  j < jend;  ++j, ++phindex)
        {
            if (ltt->gmtoff == phases[j].utcOffset()
            &&  (bool)ltt->isdst == phases[j].isDst()
            &&  abbrev == phaseAbbrevs[j])
                break;
        }
        lttLookup[i] = phindex;
        if (phindex == phases.count())
        {
            phases += KTimeZone::Phase(ltt->gmtoff, abbrev, ltt->isdst);
            phaseAbbrevs += abbrev;
        }
    }
    KTimeZone::Phase prePhase(firstLtt->gmtoff,
                              (firstLtt->abbrIndex < abbreviations.count() ? abbreviations[firstLtt->abbrIndex] : ""),
                              false);
    data->setPhases(phases, prePhase);

    // Compile the transition list
    QList<KTimeZone::Transition> transitions;
    TransitionTime *tt = transitionTimes;
    for (i = 0;  i < nTransitionTimes;  ++tt, ++i)
    {
        if (tt->localTimeIndex >= nLocalTimeTypes)
        {
            kError() << "KTimeZoneSource::parse(): transition ignored: local time type out of range: " << (int)tt->localTimeIndex << " > " << nLocalTimeTypes;
            continue;
        }

        // Convert local transition times to UTC
        ltt = &localTimeTypes[tt->localTimeIndex];
        const KTimeZone::Phase phase = phases[lttLookup[tt->localTimeIndex]];
        // kDebug(161) << "Transition time "<<i<<": "<<fromTime_t(tt->time)<<", offset="<<phase.utcOffset()/60;
        transitions += KTimeZone::Transition(fromTime_t(tt->time), phase);
    }
    data->setTransitions(transitions);
    // for(int xxx=1;xxx<data->transitions().count();xxx++)
    // kDebug(161) << "Transition time "<<xxx<<": "<<data->transitions()[xxx].time()<<", offset="<<data->transitions()[xxx].phase().utcOffset()/60;
    delete[] localTimeTypes;
    delete[] transitionTimes;

    return data;
}

bool KTimeZoneSource::useZoneParse() const
{
    return d->mUseZoneParse;
}

QString KTimeZoneSource::location() const
{
    return d->mLocation;
}

/******************************************************************************/


int KTimeZoneDataPrivate::transitionIndex(const QDateTime &dt) const
{
    // Do a binary search to find the last transition before this date/time
    int start = -1;
    int end = transitions.count();
    if (dt.timeSpec() == Qt::UTC)
    {
        while (end - start > 1)
        {
            int i = (start + end) / 2;
            if (dt < transitions[i].time())
                end = i;
            else
                start = i;
        }
    }
    else
    {
        QDateTime dtutc = dt;
        dtutc.setTimeSpec(Qt::UTC);
        while (end - start > 1)
        {
            const int i = (start + end) / 2;
            if (dtutc.addSecs(-transitions[i].phase().utcOffset()) < transitions[i].time())
                end = i;
            else
                start = i;
        }
    }
    return end ? start : -1;
}

// Find the indexes to the transitions at or after start, and before or at end.
// start and end must be UTC.
// Reply = false if none.
bool KTimeZoneDataPrivate::transitionIndexes(const QDateTime &start, const QDateTime &end, int &ixstart, int &ixend) const
{
    ixstart = 0;
    if (start.isValid() && start.timeSpec() == Qt::UTC)
    {
        ixstart = transitionIndex(start);
        if (ixstart < 0)
            ixstart = 0;
        else if (transitions[ixstart].time() < start)
        {
            if (++ixstart >= transitions.count())
                return false;   // there are no transitions at/after 'start'
        }
    }
    ixend = -1;
    if (end.isValid() && end.timeSpec() == Qt::UTC)
    {
        ixend = transitionIndex(end);
        if (ixend < 0)
            return false;   // there are no transitions at/before 'end'
    }
    return true;
}

/* Check if it's a local time which occurs both before and after the specified
 * transition (for which it has to span a daylight saving to standard time change).
 * @param utcLocalTime local time set to Qt::UTC
 */
bool KTimeZoneDataPrivate::isSecondOccurrence(const QDateTime &utcLocalTime, int transitionIndex) const
{
    if (transitionIndex < 0)
        return false;
    const int offset = transitions[transitionIndex].phase().utcOffset();
    const int prevoffset = (transitionIndex > 0) ? transitions[transitionIndex-1].phase().utcOffset() : prePhase.utcOffset();
    const int phaseDiff = prevoffset - offset;
    if (phaseDiff <= 0)
        return false;
    // Find how long after the start of the latest phase 'dt' is
    const qint64 afterStart = transitions[transitionIndex].time().msecsTo(utcLocalTime)/1000 - offset;
    return (afterStart < phaseDiff);
}



KTimeZoneData::KTimeZoneData()
  : d(new KTimeZoneDataPrivate)
{ }

KTimeZoneData::KTimeZoneData(const KTimeZoneData &c)
  : d(new KTimeZoneDataPrivate)
{
    d->phases        = c.d->phases;
    d->transitions   = c.d->transitions;
    d->utcOffsets    = c.d->utcOffsets;
    d->abbreviations = c.d->abbreviations;
    d->prePhase      = c.d->prePhase;
}

KTimeZoneData::~KTimeZoneData()
{
    delete d;
}

KTimeZoneData &KTimeZoneData::operator=(const KTimeZoneData &c)
{
    d->phases        = c.d->phases;
    d->transitions   = c.d->transitions;
    d->utcOffsets    = c.d->utcOffsets;
    d->abbreviations = c.d->abbreviations;
    d->prePhase      = c.d->prePhase;
    return *this;
}

KTimeZoneData *KTimeZoneData::clone() const
{
    return new KTimeZoneData(*this);
}

QList<QByteArray> KTimeZoneData::abbreviations() const
{
    if (d->abbreviations.isEmpty())
    {
        for (int i = 0, end = d->phases.count();  i < end;  ++i)
        {
            const QList<QByteArray> abbrevs = d->phases[i].abbreviations();
            for (int j = 0, jend = abbrevs.count();  j < jend;  ++j)
                if (!d->abbreviations.contains(abbrevs[j]))
                    d->abbreviations.append(abbrevs[j]);
        }
        if (d->abbreviations.isEmpty())
            d->abbreviations += "UTC";
    }
    return d->abbreviations;
}

QByteArray KTimeZoneData::abbreviation(const QDateTime &utcDateTime) const
{
    if (d->phases.isEmpty())
        return QByteArray("UTC");
    const KTimeZone::Transition *tr = transition(utcDateTime);
    const QList<QByteArray> abbrevs = tr ? tr->phase().abbreviations()
                                         : d->prePhase.abbreviations();
    if (abbrevs.isEmpty())
        return QByteArray();
    return abbrevs[0];
}

QList<int> KTimeZoneData::utcOffsets() const
{
    if (d->utcOffsets.isEmpty())
    {
        for (int i = 0, end = d->phases.count();  i < end;  ++i)
        {
            const int offset = d->phases[i].utcOffset();
            if (!d->utcOffsets.contains(offset))
                d->utcOffsets.append(offset);
        }
        if (d->utcOffsets.isEmpty())
            d->utcOffsets += 0;
        else
            qSort(d->utcOffsets);
    }
    return d->utcOffsets;
}

QList<KTimeZone::Phase> KTimeZoneData::phases() const
{
    return d->phases;
}

void KTimeZoneData::setPhases(const QList<KTimeZone::Phase> &phases, const KTimeZone::Phase& previousPhase)
{
    d->phases   = phases;
    d->prePhase = previousPhase;
}

void KTimeZoneData::setPhases(const QList<KTimeZone::Phase> &phases, int previousUtcOffset)
{
    d->phases   = phases;
    d->prePhase = KTimeZone::Phase(previousUtcOffset, QByteArray(), false);
}

QList<KTimeZone::Transition> KTimeZoneData::transitions(const QDateTime &start, const QDateTime &end) const
{
    int ixstart, ixend;
    if (!d->transitionIndexes(start, end, ixstart, ixend))
        return QList<KTimeZone::Transition>();   // there are no transitions within the time period
    if (ixend >= 0)
        return d->transitions.mid(ixstart, ixend - ixstart + 1);
    if (ixstart > 0)
        return d->transitions.mid(ixstart);
    return d->transitions;
}

void KTimeZoneData::setTransitions(const QList<KTimeZone::Transition> &transitions)
{
    d->transitions = transitions;
}

int KTimeZoneData::previousUtcOffset() const
{
    return d->prePhase.utcOffset();
}

const KTimeZone::Transition *KTimeZoneData::transition(const QDateTime &dt, const KTimeZone::Transition **secondTransition,
                                                       bool *validTime) const
{
    int secondIndex;
    const int index = transitionIndex(dt, (secondTransition ? &secondIndex : 0), validTime);
    if (secondTransition)
        *secondTransition = (secondIndex >= 0) ? &d->transitions[secondIndex] : 0;
    return (index >= 0) ? &d->transitions[index] : 0;
}

int KTimeZoneData::transitionIndex(const QDateTime &dt, int *secondIndex, bool *validTime) const
{
    if (validTime)
        *validTime = true;

    // Find the last transition before this date/time
    int index = d->transitionIndex(dt);
    if (dt.timeSpec() == Qt::UTC)
    {
        if (secondIndex)
            *secondIndex = index;
        return index;
    }
    else
    {
        /* Check whether the specified local time actually occurs.
         * Find the start of the next phase, and check if it falls in the gap
         * between the two phases.
         */
        QDateTime dtutc = dt;
        dtutc.setTimeSpec(Qt::UTC);
        const int count = d->transitions.count();
        const int next = (index >= 0) ? index + 1 : 0;
        if (next < count)
        {
            KTimeZone::Phase nextPhase = d->transitions[next].phase();
            const int offset = (index >= 0) ? d->transitions[index].phase().utcOffset() : d->prePhase.utcOffset();
            const int phaseDiff = nextPhase.utcOffset() - offset;
            if (phaseDiff > 0)
            {
                // Get UTC equivalent as if 'dt' was in the next phase
                if (dtutc.msecsTo(d->transitions[next].time())/1000 + nextPhase.utcOffset() <= phaseDiff)
                {
                    // The time falls in the gap between the two phases,
                    // so return an invalid value.
                    if (validTime)
                        *validTime = false;
                    if (secondIndex)
                        *secondIndex = -1;
                    return -1;
                }
            }
        }

        if (index < 0)
        {
            // The specified time is before the first phase
            if (secondIndex)
                *secondIndex = -1;
            return -1;
        }

        /* Check if it's a local time which occurs both before and after the 'latest'
         * phase start time (for which it has to span a daylight saving to standard
         * time change).
         */
        bool duplicate = true;
        if (d->isSecondOccurrence(dtutc, index))
        {
            // 'dt' occurs twice
            if (secondIndex)
            {
                *secondIndex = index;
                duplicate = false;
            }
            // Get the transition containing the first occurrence of 'dt'
            if (index <= 0)
                return -1;   // first occurrence of 'dt' is just before the first transition
            --index;
        }

        if (secondIndex  &&  duplicate)
            *secondIndex = index;
        return index;
    }
}

QList<QDateTime> KTimeZoneData::transitionTimes(const KTimeZone::Phase &phase, const QDateTime &start, const QDateTime &end) const
{
    QList<QDateTime> times;
    int ixstart, ixend;
    if (d->transitionIndexes(start, end, ixstart, ixend))
    {
        if (ixend < 0)
            ixend = d->transitions.count() - 1;
        while (ixstart <= ixend)
        {
            if (d->transitions[ixstart].phase() == phase)
                times += d->transitions[ixstart].time();
        }
    }
    return times;
}
