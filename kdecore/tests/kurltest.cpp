// krazy:excludeall=qclasses
/* This file is part of the KDE libraries
    Copyright (c) 1999-2005 Waldo Bastian <bastian@kde.org>
    Copyright (c) 2000-2005 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kurltest.h"

#include "qtest_kde.h"

QTEST_KDEMAIN_CORE(KUrlTest)

#include "kurl.h"
#include <kdebug.h>

Q_DECLARE_METATYPE(KUrl::EqualsOptions);

void KUrlTest::testcleanPath_data()
{
    QTest::addColumn<KUrl>("url" );
    QTest::addColumn<KUrl>("url2");

    QTest::newRow("local file 1")
        << KUrl("file:///")
        << KUrl("file:///");
    QTest::newRow("local file 2")
        << KUrl("file:///home/kde/")
        << KUrl("file:///home/kde");
    QTest::newRow("local file 3")
        << KUrl("kde//")
        << KUrl("kde");
    QTest::newRow("ftp url - 3 trailing slashes")
        << KUrl("ftp://ftp.kde.org///")
        << KUrl("ftp://ftp.kde.org/");
}

void KUrlTest::testcleanPath()
{
    QFETCH(KUrl, url);
    QFETCH(KUrl, url2);

    KUrl copy(url);
    copy.cleanPath();
    QCOMPARE(copy, url2);
}

void KUrlTest::testEquals_data()
{
    QTest::addColumn<KUrl>("url" );
    QTest::addColumn<KUrl>("url2");
    QTest::addColumn<KUrl::EqualsOptions>("options");
    QTest::addColumn<bool>("equals");

    QTest::newRow("local file 1")
        << KUrl("file:///")
        << KUrl("file:///")
        << KUrl::EqualsOptions(KUrl::CompareWithoutTrailingSlash)
        << true;
    QTest::newRow("local file 2")
        << KUrl("file:///home/kde/")
        << KUrl("file:///home/kde")
        << KUrl::EqualsOptions(KUrl::CompareWithoutTrailingSlash)
        << true;
    QTest::newRow("local file 3")
        << KUrl("file:///home/kde//")
        << KUrl("file:///home/kde")
        << KUrl::EqualsOptions(KUrl::CompareWithoutTrailingSlash)
        << true;
    QTest::newRow("ftp url - 3 trailing slashes")
        << KUrl("ftp://ftp.kde.org///")
        << KUrl("ftp://ftp.kde.org/")
        << KUrl::EqualsOptions(KUrl::CompareWithoutTrailingSlash)
        << true;
}

void KUrlTest::testEquals()
{
    QFETCH(KUrl, url);
    QFETCH(KUrl, url2);
    QFETCH(KUrl::EqualsOptions, options);
    QFETCH(bool, equals);

    QCOMPARE(url.equals(url2, options), equals);
}

void KUrlTest::testUriMode()
{
    KUrl url1;
    url1 = "mailto:User@Host.COM?subject=Hello";
    QCOMPARE(url1.path(), QString("User@Host.COM"));
}

void KUrlTest::testToLocalFile()
{
    const QString localFile("/tmp/print.pdf");

    const KUrl urlWithHost("file://localhost/tmp/print.pdf");
    const KUrl urlWithoutHost("file:///tmp/print.pdf");

    QCOMPARE(urlWithHost.toLocalFile(), localFile);
    QCOMPARE(urlWithoutHost.toLocalFile(), localFile );
}

void KUrlTest::testUrl_data()
{
    QTest::addColumn<KUrl>("url" );
    QTest::addColumn<QString>("urlLTS");
    QTest::addColumn<QString>("urlRTS");
    QTest::addColumn<QString>("urlATS");

    QTest::newRow("local file 1")
        << KUrl("file:///")
        << QString::fromLatin1("file:///")
        << QString::fromLatin1("file:///")
        << QString::fromLatin1("file:///");
    QTest::newRow("local file 2")
        << KUrl("file:///home/kde/")
        << QString::fromLatin1("file:///home/kde/")
        << QString::fromLatin1("file:///home/kde")
        << QString::fromLatin1("file:///home/kde/");
    QTest::newRow("local file 3")
        << KUrl("file:///home/kde//")
        << QString::fromLatin1("file:///home/kde//")
        << QString::fromLatin1("file:///home/kde")
        << QString::fromLatin1("file:///home/kde//");

    QTest::newRow("ftp url")
        << KUrl("ftp://ftp.kde.org/")
        << QString::fromLatin1("ftp://ftp.kde.org/")
        << QString::fromLatin1("ftp://ftp.kde.org")
        << QString::fromLatin1("ftp://ftp.kde.org/");
    QTest::newRow("ftp url - 3 trailing slashes")
        << KUrl("ftp://ftp.kde.org///")
        << QString::fromLatin1("ftp://ftp.kde.org///")
        << QString::fromLatin1("ftp://ftp.kde.org")
        << QString::fromLatin1("ftp://ftp.kde.org///");
}

void KUrlTest::testUrl()
{
    QFETCH(KUrl, url);
    QFETCH(QString, urlLTS);
    QFETCH(QString, urlRTS);
    QFETCH(QString, urlATS);

    QCOMPARE(url.url(KUrl::LeaveTrailingSlash), urlLTS);
    QCOMPARE(url.url(KUrl::RemoveTrailingSlash), urlRTS);
    QCOMPARE(url.url(KUrl::AddTrailingSlash), urlATS);
}

void KUrlTest::testToStringList()
{
    KUrl::List urls;
    urls << KUrl("file:///")
         << KUrl("file:///home/kde/")
         << KUrl("file:///home/kde//")
         << KUrl("ftp://ftp.kde.org/")
         << KUrl("ftp://ftp.kde.org///");

    //kDebug() << urls.toStringList(KUrl::LeaveTrailingSlash);
    QCOMPARE(
        urls.toStringList(KUrl::LeaveTrailingSlash),
        QStringList()
            << QLatin1String("file:///")
            << QLatin1String("file:///home/kde/")
            << QLatin1String("file:///home/kde//")
            << QLatin1String("ftp://ftp.kde.org/")
            << QLatin1String("ftp://ftp.kde.org///")
    );

    //kDebug() << urls.toStringList(KUrl::RemoveTrailingSlash);
    QCOMPARE(
        urls.toStringList(KUrl::RemoveTrailingSlash),
        QStringList()
            << QLatin1String("file:///")
            << QLatin1String("file:///home/kde")
            << QLatin1String("file:///home/kde")
            << QLatin1String("ftp://ftp.kde.org")
            << QLatin1String("ftp://ftp.kde.org")
    );

    //kDebug() << urls.toStringList(KUrl::AddTrailingSlash);
    QCOMPARE(
        urls.toStringList(KUrl::AddTrailingSlash),
        QStringList()
            << QLatin1String("file:///")
            << QLatin1String("file:///home/kde/")
            << QLatin1String("file:///home/kde//")
            << QLatin1String("ftp://ftp.kde.org/")
            << QLatin1String("ftp://ftp.kde.org///")
    );
}

#include "moc_kurltest.cpp"