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

#include "ktimezonestest.h"
#include "ksystemtimezone.h"
#include "qtest_kde.h"
#include "kdebug.h"

#include <QtCore/QDir>
#include <QtCore/QDateTime>

#include <stdlib.h>

QTEST_KDEMAIN_CORE(KTimeZonesTest)

void KTimeZonesTest::initTestCase()
{
}

void KTimeZonesTest::cleanupTestCase()
{
}

///////////////////
// KTimeZone: UTC
///////////////////

void KTimeZonesTest::utc()
{
    KTimeZone utc = KTimeZone::utc();
    QVERIFY(utc.isValid());
    QCOMPARE(utc.name(), QString("UTC"));
    KTimeZone systemUtc = KSystemTimeZones::zone("UTC");
    QCOMPARE(utc, systemUtc);
}

/////////////////////////
// KSystemTimeZones tests
/////////////////////////

void KTimeZonesTest::local()
{
    const QByteArray tz = qgetenv("TZ");
    ::setenv("TZ", "Europe/Paris", 1);
    KTimeZone local = KSystemTimeZones::local();
    QVERIFY(local.isValid());
    QCOMPARE(local.name(), QString::fromLatin1("Europe/Paris"));
    ::setenv("TZ", tz.constData(), 1);
}

void KTimeZonesTest::zone()
{
    KTimeZone utc = KSystemTimeZones::zone("UTC");
    QVERIFY(utc.isValid());

    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    QCOMPARE(london.name(), QString("Europe/London"));
    QCOMPARE(london.countryCode(), QString("GB"));
    QCOMPARE(london.latitude(), float(51.5083));
    QCOMPARE(london.longitude(), float(-0.125278));
    QCOMPARE(london.comment(), QString());

    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY(losAngeles.isValid());
    QCOMPARE(losAngeles.name(), QString("America/Los_Angeles"));
    QCOMPARE(losAngeles.countryCode(), QString("US"));
    QCOMPARE(losAngeles.latitude(), float(34.0522));
    QCOMPARE(losAngeles.longitude(), float(-118.243));
    QCOMPARE(losAngeles.comment(), QString("Pacific"));
}

void KTimeZonesTest::zoneinfoDir()
{
    const QString zoneinfo = KSystemTimeZones::zoneinfoDir();
    QVERIFY(!zoneinfo.isEmpty());
}

void KTimeZonesTest::abbreviation()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    const QDateTime gmt = QDateTime(QDate(2006, 3, 26), QTime(0, 59, 0), Qt::UTC);
    const QDateTime bst = QDateTime(QDate(2006, 3, 26), QTime(1, 0, 0), Qt::UTC);
    QCOMPARE(london.abbreviation(gmt), QByteArray("GMT"));
    QCOMPARE(london.abbreviation(bst), QByteArray("BST"));

    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY(losAngeles.isValid());
    const QDateTime pdt = QDateTime(QDate(2012, 11, 4), QTime(8, 59, 59), Qt::UTC);
    const QDateTime pst = QDateTime(QDate(2012, 11, 4), QTime(9, 0, 0), Qt::UTC);
    QCOMPARE(losAngeles.abbreviation(pdt), QByteArray("PDT"));
    QCOMPARE(losAngeles.abbreviation(pst), QByteArray("PST"));
}

void KTimeZonesTest::fromAndTo()
{
    const QDateTime utc = QDateTime(QDate(2018, 2, 10), QTime(4, 0, 0), Qt::UTC);

    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    QDateTime result = london.toZoneTime(utc);
    // was GMT so same date
    const QDateTime londonDate = QDateTime(QDate(2018, 2, 10), QTime(4, 0, 0), Qt::LocalTime);
    QCOMPARE(result, londonDate);
    QCOMPARE(london.toUtc(result), utc);

    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY(losAngeles.isValid());
    result = losAngeles.toZoneTime(utc);
    // gmtoff=-28800
    const QDateTime losAngelesDate = QDateTime(QDate(2018, 2, 9), QTime(20, 0, 0), Qt::LocalTime);
    QCOMPARE(result, losAngelesDate);
    QCOMPARE(losAngeles.toUtc(result), utc);
}

#include "moc_ktimezonestest.cpp"
