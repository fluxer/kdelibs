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

void KUrlTest::testHash_data()
{
    QTest::addColumn<KUrl>("url");

    QTest::newRow("null")
        << KUrl();
    QTest::newRow("empty")
        << KUrl("");
    QTest::newRow("local file 1")
        << KUrl("file:///");
    QTest::newRow("local file 2")
        << KUrl("file:///home/kde/foo?bar=baz#foobar");
    QTest::newRow("local file 3")
        << KUrl("kde//foo?bar=baz#foobar");
    QTest::newRow("ftp url - 3 trailing slashes")
        << KUrl("ftp://ftp.kde.org/foo?bar=baz#foobar");
}

void KUrlTest::testHash()
{
    QFETCH(KUrl, url);

    {
        KUrl testurl;
        testurl.setScheme(url.scheme());
        testurl.setAuthority(url.authority());
        testurl.setPath(url.path());
        testurl.setQuery(url.query());
        testurl.setFragment(url.fragment());
        // qDebug() << Q_FUNC_INFO << url << testurl;
        QCOMPARE(url.url(), testurl.url());
        // qDebug() << qHash(url) << qHash(testurl);
        QCOMPARE(qHash(url), qHash(testurl));
    }

    {
        // changing the port of local files is not going to do what you think, especially if they
        // are not full path
        if (url.isLocalFile()) {
            return;
        }
        KUrl testurl(url);
        QCOMPARE(qHash(testurl), qHash(url));
        QCOMPARE(qHash(testurl), qHash(testurl));

        // change of authorirty
        KUrl testurl2(url);
        testurl2.setPort(url.port() + 10);
        // qDebug() << Q_FUNC_INFO << testurl << testurl2;
        QVERIFY(qHash(testurl) != qHash(testurl2));
    }
}

void KUrlTest::testQueryAndFragment_data()
{
    QTest::addColumn<KUrl>("url");
    QTest::addColumn<QString>("query");
    QTest::addColumn<QString>("fragment");

    QTest::newRow("local file 1")
        << KUrl("file:///")
        << QString()
        << QString();
    QTest::newRow("local file 1 - with query and fragment")
        << KUrl("file:///?foo=bar#baz")
        << QString::fromLatin1("foo=bar")
        << QString::fromLatin1("baz");
    QTest::newRow("local file 2")
        << KUrl("file:///home/kde/?foo=bar#baz")
        << QString::fromLatin1("foo=bar")
        << QString::fromLatin1("baz");
    QTest::newRow("local file 3")
        << KUrl("kde//?foo=bar#baz")
        << QString::fromLatin1("foo=bar")
        << QString::fromLatin1("baz");
    // NOTE: adding query or fragment to local file URLs is not supported and will trigger the
    // fatal message in kCheckLocalFile(). why? because what looks like query and fragment can
    // actually be part of the file name (i.e. not an actual query) so it is passed as-is
    QTest::newRow("local file 4")
        << KUrl("/foo?bar=baz#foobar")
        << QString()
        << QString();
    QTest::newRow("ftp url - 3 trailing slashes")
        << KUrl("ftp://ftp.kde.org///?foo=bar#baz")
        << QString::fromLatin1("foo=bar")
        << QString::fromLatin1("baz");
}

void KUrlTest::testQueryAndFragment()
{
    QFETCH(KUrl, url);
    QFETCH(QString, query);
    QFETCH(QString, fragment);

    QCOMPARE(url.query(), query);
    QCOMPARE(url.fragment(), fragment);
}

void KUrlTest::testcleanPath_data()
{
    QTest::addColumn<KUrl>("url");
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
    QTest::addColumn<KUrl>("url");
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
    QTest::addColumn<KUrl>("url");
    QTest::addColumn<QString>("urlLTS");
    QTest::addColumn<QString>("urlRTS");
    QTest::addColumn<QString>("urlATS");

    QTest::newRow("local file 1")
        << KUrl("file:///")
        << QString::fromLatin1("/")
        << QString::fromLatin1("/")
        << QString::fromLatin1("/");
    QTest::newRow("local file 2")
        << KUrl("file:///home/kde/")
        << QString::fromLatin1("/home/kde/")
        << QString::fromLatin1("/home/kde")
        << QString::fromLatin1("/home/kde/");
    QTest::newRow("local file 3")
        << KUrl("file:///home/kde//")
        << QString::fromLatin1("/home/kde//")
        << QString::fromLatin1("/home/kde")
        << QString::fromLatin1("/home/kde//");

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

    // kDebug() << urls.toStringList(KUrl::LeaveTrailingSlash);
    QCOMPARE(
        urls.toStringList(KUrl::LeaveTrailingSlash),
        QStringList()
            << QLatin1String("/")
            << QLatin1String("/home/kde/")
            << QLatin1String("/home/kde//")
            << QLatin1String("ftp://ftp.kde.org/")
            << QLatin1String("ftp://ftp.kde.org///")
    );

    // kDebug() << urls.toStringList(KUrl::RemoveTrailingSlash);
    QCOMPARE(
        urls.toStringList(KUrl::RemoveTrailingSlash),
        QStringList()
            << QLatin1String("/")
            << QLatin1String("/home/kde")
            << QLatin1String("/home/kde")
            << QLatin1String("ftp://ftp.kde.org")
            << QLatin1String("ftp://ftp.kde.org")
    );

    // kDebug() << urls.toStringList(KUrl::AddTrailingSlash);
    QCOMPARE(
        urls.toStringList(KUrl::AddTrailingSlash),
        QStringList()
            << QLatin1String("/")
            << QLatin1String("/home/kde/")
            << QLatin1String("/home/kde//")
            << QLatin1String("ftp://ftp.kde.org/")
            << QLatin1String("ftp://ftp.kde.org///")
    );
}

#include "moc_kurltest.cpp"