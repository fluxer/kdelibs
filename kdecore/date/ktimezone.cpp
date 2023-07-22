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

extern QString zoneinfoDir(); // in ksystemtimezone.cpp

// for reference:
// https://man7.org/linux/man-pages/man5/tzfile.5.html

/******************************************************************************/

class KTimeZonePrivate
{
public:
    KTimeZonePrivate();
    KTimeZonePrivate(const QString &nam,
                     const QString &country, float lat, float lon, const QString &cmnt);
    KTimeZonePrivate(const KTimeZonePrivate &rhs);

    QString name;
    QString countryCode;
    QString comment;
    float latitude;
    float longitude;
    int currentOffset;
    QList<QByteArray> abbreviations;
};

KTimeZonePrivate::KTimeZonePrivate()
    : latitude(KTimeZone::UNKNOWN),
    longitude(KTimeZone::UNKNOWN),
    currentOffset(KTimeZone::InvalidOffset)
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

    // NOTE: should not be less than or or equal to zero
    if (Q_UNLIKELY(tzh_typecnt <= 0)) {
        kWarning() << "Invalid number of local time types" << tzfile.fileName();
        return;
    }

    // skip transitions
    const int toskip = (
        (tzh_timecnt * (sizeof(quint32) + sizeof(quint8)))
    );
    tzstream.skipRawData(toskip);

    // get local time
    qint32 tt_utoff = 0;
    quint8 tt_isdst = 0;
    quint8 tt_desigidx = 0;
    quint8 abbreviationindex = 0;
    for (quint32 i = 0; i < tzh_typecnt; i++) {
        tzstream >> tt_utoff >> tt_isdst >> tt_desigidx;
        if (!tt_isdst) {
            currentOffset = tt_utoff;
            abbreviationindex = tt_desigidx;
        }
    }

    // get the zone abbreviations
    QByteArray abbreviationsbuffer(tzh_charcnt + 1, '\0');
    tzstream.readRawData(abbreviationsbuffer.data(), abbreviationsbuffer.size());
    foreach (const QByteArray &abbreviation, abbreviationsbuffer.split('\0')) {
        if (abbreviation.isEmpty()) {
            continue;
        }
        abbreviations.append(abbreviation);
    }
    if (abbreviationindex >= 0 && abbreviationindex < abbreviationsbuffer.size()) {
        QByteArray timetabbreviation = abbreviationsbuffer.mid(
            abbreviationindex, qstrlen(abbreviationsbuffer.constData() + abbreviationindex)
        );
        // move the chosen local time abbreviation to the front, if not there already
        for (int i = 1; i < abbreviations.size(); i++) {
            if (abbreviations.at(i) == timetabbreviation) {
                abbreviations.move(i, 0);
                break;
            }
        }
    } else {
        kWarning() << "Invalid abbreviation index" << tzfile.fileName() << abbreviationindex << abbreviationsbuffer.size();
    }
    // qDebug() << Q_FUNC_INFO << tzfile.fileName() << currentOffset << abbreviationindex << abbreviations;
}

KTimeZonePrivate::KTimeZonePrivate(const KTimeZonePrivate &rhs)
    : name(rhs.name),
    countryCode(rhs.countryCode),
    comment(rhs.comment),
    latitude(rhs.latitude),
    longitude(rhs.longitude),
    currentOffset(rhs.currentOffset),
    abbreviations(rhs.abbreviations)
{
}

/******************************************************************************/

const int    KTimeZone::InvalidOffset = 0x80000000;
const float  KTimeZone::UNKNOWN = 1000.0;

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
        d->currentOffset = tz.currentOffset();
        d->abbreviations = tz.abbreviations();
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
        && currentOffset() == rhs.currentOffset()
        && abbreviations() == rhs.abbreviations()
    );
}

bool KTimeZone::isValid() const
{
    // TODO: just because the name is not empty does not mean the time zone is valid
    return !d->name.isEmpty();
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
    return d->abbreviations;
}

QByteArray KTimeZone::abbreviation(const QDateTime &utcDateTime) const
{
    if (utcDateTime.timeSpec() != Qt::UTC) {
        return QByteArray();
    }
    // TODO: actually check the date
    return d->abbreviations[0];
}

QDateTime KTimeZone::toUtc(const QDateTime &zoneDateTime) const
{
    if (!zoneDateTime.isValid() || zoneDateTime.timeSpec() != Qt::LocalTime) {
        return QDateTime();
    }
    const int offset = currentOffset();
    if (offset == KTimeZone::InvalidOffset) {
        return QDateTime();
    }
    QDateTime dt = zoneDateTime.addSecs(-offset);
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
    QDateTime dt = utcDateTime.addSecs(currentOffset());
    dt.setTimeSpec(Qt::LocalTime);
    return dt;
}

int KTimeZone::currentOffset() const
{
    return d->currentOffset;
}

KTimeZone KTimeZone::utc()
{
    static KTimeZone utcZone(QLatin1String("UTC"));
    return utcZone;
}
