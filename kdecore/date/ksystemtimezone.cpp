/*
   This file is part of the KDE libraries
   Copyright (c) 2005-2010 David Jarvie <djarvie@kde.org>
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

#include "config-date.h"
#include "config-prefix.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QFileSystemWatcher>

#include "kglobal.h"
#include "kdebug.h"
#include "ksystemtimezone.h"
#include "kde_file.h"

#include <sys/time.h>
#include <time.h>

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

static QList<float> splitZoneTabCoordinates(const QByteArray &zonetabcoordinates)
{
    QList<float> result;
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
    const float latitude = convertCoordinate(
        QByteArray(zonetabcoordinates.mid(0, signindex))
    );
    const float longitude = convertCoordinate(
        QByteArray(zonetabcoordinates.mid(signindex, zonetabcoordinates.size() - signindex))
    );
    result.append(latitude);
    result.append(longitude);
    return result;
}

class KSystemTimeZonesPrivate : public QObject, public KTimeZones
{
    Q_OBJECT
public:
    KSystemTimeZonesPrivate();

    KTimeZone m_localtz;
    QString m_zoneinfoDir;
    KTimeZoneSource* m_tzfileSource;

private Q_SLOTS:
    void update(const QString &path);

private:
    QFileSystemWatcher *m_watcher;
};

KSystemTimeZonesPrivate::KSystemTimeZonesPrivate()
    : m_tzfileSource(nullptr),
    m_watcher(nullptr)
{
    update(QString());
}

void KSystemTimeZonesPrivate::update(const QString &path)
{
    Q_UNUSED(path);

    m_localtz = KTimeZone::utc();
    static const QByteArray kdezoneinfodir = qgetenv("KDE_ZONEINFO_DIR");
    m_zoneinfoDir = QFile::decodeName(kdezoneinfodir);
    if (m_zoneinfoDir.isEmpty()) {
        m_zoneinfoDir = zoneinfoDir();
    }
    delete m_tzfileSource;
    m_tzfileSource = new KTimeZoneSource(m_zoneinfoDir);

    KTimeZones::clear();

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
    while (!zonetabfile.atEnd()) {
        const QByteArray zonetabline = zonetabfile.readLine().trimmed();
        if (zonetabline.isEmpty() || zonetabline.startsWith('#')) {
            continue;
        }
        const QList<QByteArray> zonetabparts = zonetabline.split('\t');
        if (zonetabparts.size() < 3 || zonetabparts.size() > 4) {
            kWarning() << "Invalid zone.tab entry" << zonetabline;
            continue;
        }
        const QList<float> zonetabcoordinates = splitZoneTabCoordinates(zonetabparts.at(1));
        if (zonetabcoordinates.size() != 2) {
            kWarning() << "Invalid zone.tab coordinates" << zonetabline;
            continue;
        }

        const QString zonecode = QString::fromLatin1(zonetabparts.at(0).constData(), zonetabparts.at(0).size());
        const QString zonename = QString::fromLatin1(zonetabparts.at(2).constData(), zonetabparts.at(2).size());
        QString zonecomment;
        if (zonetabparts.size() == 4) {
            zonecomment = QString::fromLatin1(zonetabparts.at(3).constData(), zonetabparts.at(3).size());
        }
        const float zonelatitude = zonetabcoordinates.at(0);
        const float zonelongitude = zonetabcoordinates.at(1);

        const KTimeZone ktimezone(
            m_tzfileSource,
            zonename, zonecode, zonelatitude, zonelongitude, zonecomment
        );
        KTimeZones::add(ktimezone);
    }

    if (localtimeinfo.isSymLink()) {
        const int zonediroffset = (m_zoneinfoDir.size() + 1);
        const QString localtz = reallocaltime.mid(zonediroffset, reallocaltime.size() - zonediroffset);
        m_localtz = KTimeZones::zone(localtz);
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
    const KTimeZones::ZoneMap allzones = KTimeZones::zones();
    KTimeZones::ZoneMap::const_iterator it = allzones.constBegin();
    while (it != allzones.constEnd()) {
        if (it.value().abbreviations().contains(localtz)) {
            m_localtz = KTimeZones::zone(it.key());
            break;
        }
        it++;
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

KTimeZones *KSystemTimeZones::timeZones()
{
    return s_systemzones;
}

KTimeZone KSystemTimeZones::readZone(const QString &name)
{
    return KTimeZone(s_systemzones->m_tzfileSource, name);
}

const KTimeZones::ZoneMap KSystemTimeZones::zones()
{
    return s_systemzones->zones();
}

KTimeZone KSystemTimeZones::zone(const QString &name)
{
    return s_systemzones->zone(name);
}

#include "ksystemtimezone.moc"
