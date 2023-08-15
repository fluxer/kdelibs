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

#include "config-date.h"
#include "config-prefix.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QFileSystemWatcher>
#include <QElapsedTimer>

#include "kglobal.h"
#include "kdebug.h"
#include "ksystemtimezone.h"
#include "kde_file.h"

#include <sys/time.h>
#include <time.h>

typedef QPair<float, float> KZoneCoordinates;

static const QString s_localtime = QString::fromLatin1("/etc/localtime");

QString zoneinfoDir()
{
    static const QStringList zoneinfodirs = QStringList()
        << QString::fromLatin1("/share/zoneinfo")
        << QString::fromLatin1("/lib/zoneinfo")
        << QString::fromLatin1("/usr/share/zoneinfo")
        << QString::fromLatin1("/usr/lib/zoneinfo")
        << QString::fromLatin1("/usr/local/share/zoneinfo")
        << QString::fromLatin1("/usr/local/lib/zoneinfo")
        << (QString::fromLatin1(KDEDIR) + QLatin1String("/share/zoneinfo"))
        << (QString::fromLatin1(KDEDIR) + QLatin1String("/lib/zoneinfo"));
    foreach (const QString &zoneinfodir, zoneinfodirs) {
        KDE_struct_stat statbuf;
        if (KDE::stat(zoneinfodir, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            return zoneinfodir;
        }
    }
    return zoneinfodirs.last();
}

// Convert sHHMM or sHHMMSS to a floating point number of degrees.
static float convertCoordinate(const QByteArray &coordinate)
{
    int value = coordinate.toInt();
    int degrees = 0;
    int minutes = 0;
    int seconds = 0;

    if (coordinate.length() > 6) {
        degrees = value / 10000;
        value -= degrees * 10000;
        minutes = value / 100;
        value -= minutes * 100;
        seconds = value;
    } else {
        degrees = value / 100;
        value -= degrees * 100;
        minutes = value;
    }
    value = degrees * 3600 + minutes * 60 + seconds;
    return value / 3600.0;
}

static KZoneCoordinates splitZoneTabCoordinates(const QByteArray &zonetabcoordinates)
{
    KZoneCoordinates result = qMakePair(KTimeZone::UNKNOWN, KTimeZone::UNKNOWN);
    int startindex = 0;
    if (zonetabcoordinates.startsWith('+') || zonetabcoordinates.startsWith('-')) {
        startindex = 1;
    }
    int signindex = zonetabcoordinates.indexOf('+', startindex);
    if (signindex < startindex) {
        signindex = zonetabcoordinates.indexOf('-', startindex);
    }
    if (signindex < startindex) {
        return result;
    }
    const float latitude = convertCoordinate(zonetabcoordinates.mid(0, signindex));
    const float longitude = convertCoordinate(zonetabcoordinates.mid(signindex, zonetabcoordinates.size() - signindex));
    result = qMakePair(latitude, longitude);
    return result;
}

class KSystemTimeZonesPrivate : public QObject
{
    Q_OBJECT
public:
    KSystemTimeZonesPrivate();

    KTimeZone findZone(const QString &name) const;

    KTimeZoneList m_zones;
    KTimeZone m_localtz;
    QString m_zoneinfoDir;

private Q_SLOTS:
    void update(const QString &path);

private:
    QFileSystemWatcher *m_watcher;
};

KSystemTimeZonesPrivate::KSystemTimeZonesPrivate()
    : m_watcher(nullptr)
{
    update(QString());
}

KTimeZone KSystemTimeZonesPrivate::findZone(const QString &name) const
{
    foreach (const KTimeZone &zone, m_zones) {
        if (zone.name() == name) {
            return zone;
        }
    }
    return KTimeZone();
}

void KSystemTimeZonesPrivate::update(const QString &path)
{
    Q_UNUSED(path);

#ifndef NDEBUG
    QElapsedTimer updatetimer;
    updatetimer.start();
#endif

    m_localtz = KTimeZone::utc();
    static const QByteArray kdezoneinfodir = qgetenv("KDE_ZONEINFO_DIR");
    m_zoneinfoDir = QFile::decodeName(kdezoneinfodir);
    if (m_zoneinfoDir.isEmpty()) {
        m_zoneinfoDir = zoneinfoDir();
    }

    m_zones.clear();
    // UTC is used as fallback, should be there in any case
    m_zones.append(m_localtz);

    const QString zonetab = m_zoneinfoDir + QLatin1String("/zone.tab");
    QFile zonetabfile(zonetab);
    if (!zonetabfile.open(QFile::ReadOnly)) {
        kWarning() << "Could not open zone.tab" << zonetab;
        return;
    }

    QStringList watchlist = QStringList()
        << zonetab
        << s_localtime;
    QString reallocaltime(s_localtime);
    QFileInfo localtimeinfo(s_localtime);
    if (localtimeinfo.isSymLink()) {
        reallocaltime = localtimeinfo.readLink();
        watchlist.append(reallocaltime);
    }
    if (m_watcher) {
        m_watcher->deleteLater();
    }
    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPaths(watchlist);
    connect(m_watcher, SIGNAL(fileChanged(QString)), this, SLOT(update(QString)));

    kDebug() << "Parsing" << zonetab;
    char zonecode[4];
    char zonecoordinates[32];
    char zonename[128];
    char zonecomment[1024];
    while (!zonetabfile.atEnd()) {
        const QByteArray zonetabline = zonetabfile.readLine().trimmed();
        if (zonetabline.isEmpty() || zonetabline.startsWith('#')) {
            continue;
        }

        ::memset(zonecode, '\0', sizeof(zonecode));
        ::memset(zonecoordinates, '\0', sizeof(zonecode));
        ::memset(zonename, '\0', sizeof(zonename));
        ::memset(zonecomment, '\0', sizeof(zonecomment));
        const int sscanfresult = sscanf(
            zonetabline.constData(),
            "%3s %31s %127s %1023[^\n]",
            zonecode, zonecoordinates, zonename, zonecomment
        );

        if (Q_UNLIKELY(sscanfresult < 3 || sscanfresult > 4)) {
            kWarning() << "Invalid zone.tab entry" << zonetabline;
            continue;
        }

        const KZoneCoordinates zonetabcoordinates = splitZoneTabCoordinates(
            QByteArray::fromRawData(zonecoordinates, qstrlen(zonecoordinates))
        );
        if (Q_UNLIKELY(zonetabcoordinates.first == KTimeZone::UNKNOWN
            || zonetabcoordinates.second == KTimeZone::UNKNOWN)) {
            kWarning() << "Invalid zone.tab coordinates" << zonetabline;
            continue;
        }

        const KTimeZone ktimezone(
            QString::fromLatin1(zonename),
            QString::fromLatin1(zonecode),
            zonetabcoordinates.first, zonetabcoordinates.second,
            QString::fromLatin1(zonecomment)
        );
        m_zones.append(ktimezone);
    }

    if (localtimeinfo.isSymLink()) {
        const int zonediroffset = (m_zoneinfoDir.size() + 1);
        const QString localtz = reallocaltime.mid(zonediroffset, reallocaltime.size() - zonediroffset);
        m_localtz = findZone(localtz);
#ifndef NDEBUG
        kDebug() << "Zones update took" << updatetimer.elapsed() << "ms";
#endif
        return;
    }

    time_t ltime;
    ::time(&ltime);
    ::tzset();
    struct tm res;
    struct tm *t = ::localtime_r(&ltime, &res);
#if defined(HAVE_STRUCT_TM_TM_ZONE)
    const QByteArray localtz(t->tm_zone);
#else
    const QByteArray localtz(tzname[t->tm_isdst]);
#endif
    foreach (const KTimeZone &zone, m_zones) {
        if (zone.abbreviations().contains(localtz)) {
            m_localtz = zone;
#ifndef NDEBUG
            kDebug() << "Zones update took" << updatetimer.elapsed() << "ms";
#endif
            break;
        }
    }
}

K_GLOBAL_STATIC(KSystemTimeZonesPrivate, s_systemzones);

/******************************************************************************/

KTimeZone KSystemTimeZones::local()
{
    const QByteArray envtz = qgetenv("TZ");
    if (!envtz.isEmpty()) {
        if (envtz.at(0) == ':') {
            return KSystemTimeZones::zone(QString::fromLocal8Bit(envtz.constData() + 1, envtz.size() - 1));
        }
        return KSystemTimeZones::zone(QString::fromLocal8Bit(envtz.constData(), envtz.size()));
    }

    return s_systemzones->m_localtz;
}

QString KSystemTimeZones::zoneinfoDir()
{
    return s_systemzones->m_zoneinfoDir;
}

const KTimeZoneList KSystemTimeZones::zones()
{
    return s_systemzones->m_zones;
}

KTimeZone KSystemTimeZones::zone(const QString &name)
{
    return s_systemzones->findZone(name);
}

#include "ksystemtimezone.moc"
