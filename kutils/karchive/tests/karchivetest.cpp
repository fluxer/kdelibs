/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qtest_kde.h"
#include "karchive.h"
#include "ktemporaryfile.h"
#include "ktempdir.h"
#include "kdebug.h"

#include <sys/stat.h>

static QString tmpName(const QString &archiveext)
{
    const QString tmptemplate = QString::fromLatin1("XXXXXXXXXX%1").arg(archiveext);
    return KTemporaryFile::filePath(tmptemplate);
}

static QString tmpCopy(const QString &archivepath)
{
    const QString tmpdir = QFile::encodeName(KDEBINDIR);
    const QString tmpbase = QFileInfo(archivepath).fileName();
    const QString tmpcopy = QString::fromLatin1("%1/%2").arg(tmpdir).arg(tmpbase);
    QFile::remove(tmpcopy);
    QFile::copy(archivepath, tmpcopy);
    return tmpcopy;
}

class KArchiveTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void list_data();
    void list();

    void add_data();
    void add();

    void remove_data();
    void remove();

    // TODO: extract tests

    void error_data();
    void error();

    void encryption();

    void progress();
};

QTEST_KDEMAIN_CORE(KArchiveTest)

void KArchiveTest::initTestCase()
{
    if (!KArchive::isSupported()) {
        QSKIP("Built without LibArchive", SkipAll);
    }
    // qDebug() << KArchive::readableMimeTypes();
    // qDebug() << KArchive::writableMimeTypes();
}

void KArchiveTest::cleanupTestCase()
{
}

void KArchiveTest::list_data()
{
    QTest::addColumn<QString>("archivepath");
    QTest::newRow(".tar.gz") << QFile::decodeName(KDESRCDIR "/tests.tar.gz");
    QTest::newRow(".zip") << QFile::decodeName(KDESRCDIR "/tests.zip");
}

void KArchiveTest::list()
{
    QFETCH(QString, archivepath);

    KArchive karchive(archivepath);
    QVERIFY(karchive.isReadable());
    QList<KArchiveEntry> karchiveentries = karchive.list();
    QCOMPARE(karchiveentries.size(), 3);
    QCOMPARE(karchiveentries.at(0).pathname, QByteArray("tests/"));
    QVERIFY(S_ISDIR(karchiveentries.at(0).mode));
    QCOMPARE(karchiveentries.at(1).pathname, QByteArray("tests/CMakeLists.txt"));
    QVERIFY(S_ISREG(karchiveentries.at(1).mode));
}

void KArchiveTest::add_data()
{
    QTest::addColumn<QString>("archiveext");
    QTest::newRow(".tar.gz") << ".tar.gz";
    QTest::newRow(".zip") << ".zip";
}

void KArchiveTest::add()
{
    QFETCH(QString, archiveext);

    {
        KArchive karchive(tmpName(archiveext));
        QVERIFY(karchive.isWritable());
        QStringList toadd = QStringList()
            << QFile::decodeName(KDESRCDIR "/CMakeLists.txt");
        QVERIFY(karchive.add(toadd));
        QList<KArchiveEntry> karchiveentries = karchive.list();
        QCOMPARE(karchiveentries.size(), 1);
        QCOMPARE(karchiveentries.at(0).pathname, QByteArray(KDESRCDIR "CMakeLists.txt").mid(1));
        QVERIFY(S_ISREG(karchiveentries.at(0).mode));
    }

    {
        KArchive karchive(tmpName(archiveext));
        QVERIFY(karchive.isWritable());
        QStringList toadd = QStringList()
            << QFile::decodeName(KDESRCDIR "/CMakeLists.txt");
        QVERIFY(karchive.add(toadd, QByteArray(), QByteArray("dir")));
        QList<KArchiveEntry> karchiveentries = karchive.list();
        QCOMPARE(karchiveentries.size(), 1);
        QCOMPARE(karchiveentries.at(0).pathname, QByteArray("dir" KDESRCDIR "CMakeLists.txt"));
        QVERIFY(S_ISREG(karchiveentries.at(0).mode));
    }
}

void KArchiveTest::remove_data()
{
    QTest::addColumn<QString>("archivepath");
    QTest::newRow(".tar.gz") << QFile::decodeName(KDESRCDIR "/tests.tar.gz");
    QTest::newRow(".zip") << QFile::decodeName(KDESRCDIR "/tests.zip");
}

void KArchiveTest::remove()
{
    QFETCH(QString, archivepath);

    KArchive karchive(tmpCopy(archivepath));
    QVERIFY(karchive.isReadable());
    QVERIFY(karchive.isWritable());
    QList<KArchiveEntry> karchiveentries = karchive.list();
    QCOMPARE(karchiveentries.size(), 3);

    {
        QStringList toremove = QStringList()
            << QFile::decodeName("tests/CMakeLists.txt");
        QVERIFY(karchive.remove(toremove));
        QList<KArchiveEntry> karchiveentries2 = karchive.list();
        QCOMPARE(karchiveentries2.size(), 2);
        QCOMPARE(karchiveentries2.at(0).pathname, QByteArray("tests/"));
        QVERIFY(S_ISDIR(karchiveentries2.at(0).mode));
        QCOMPARE(karchiveentries2.at(1).pathname, QByteArray("tests/karchivetest.cpp"));
        QVERIFY(S_ISREG(karchiveentries2.at(1).mode));
    }

    {
        QStringList toremove = QStringList()
            << QFile::decodeName("tests/");
        QVERIFY(karchive.remove(toremove));
        QList<KArchiveEntry> karchiveentries3 = karchive.list();
        QCOMPARE(karchiveentries3.size(), 0);
    }
}

void KArchiveTest::error_data()
{
    QTest::addColumn<QString>("archivepath");
    QTest::addColumn<QString>("expectederror");
    QTest::addColumn<bool>("add");
    QTest::addColumn<bool>("remove");
    QTest::addColumn<bool>("extract");
    QTest::addColumn<QStringList>("pathslist");
    QTest::newRow("empty")
        << QFile::decodeName(KDESRCDIR "/tests.tar.gz")
        << QString()
        << true << false << false
        << QStringList();
    QTest::newRow("add_does_not_exist")
        << QFile::decodeName(KDESRCDIR "/tests123.zip")
        << QString::fromLatin1("lstat: No such file or directory")
        << true << false << false
        << (QStringList() << "does_not_exist");
    // TODO: test remove and extract
}

void KArchiveTest::error()
{
    QFETCH(QString, archivepath);
    QFETCH(QString, expectederror);
    QFETCH(bool, add);
    QFETCH(bool, remove);
    QFETCH(bool, extract);
    QFETCH(QStringList, pathslist);

    KArchive karchive(archivepath);
    QCOMPARE(karchive.errorString(), QString());
    if (add) {
        karchive.add(pathslist);
        QCOMPARE(karchive.errorString(), expectederror);
    } else if (remove) {
        karchive.remove(pathslist);
        QCOMPARE(karchive.errorString(), expectederror);
    } else if (extract) {
        KTempDir ktempdir;
        QVERIFY(ktempdir.exists());
        karchive.extract(pathslist, ktempdir.name());
        QCOMPARE(karchive.errorString(), expectederror);
    }
}

void KArchiveTest::encryption()
{
    KArchive karchive(QFile::decodeName(KDESRCDIR "/tests_encryption.zip"));
    QVERIFY(karchive.isReadable());
    QVERIFY(karchive.requiresPassphrase());
    QCOMPARE(karchive.list().size(), 1);

    QStringList toextract = QStringList()
        << QFile::decodeName("tests/CMakeLists.txt");
    {
        KTempDir ktempdir;
        QVERIFY(ktempdir.exists());
        QVERIFY(!karchive.extract(toextract, ktempdir.name()));
        QCOMPARE(karchive.errorString(), QString::fromLatin1("Passphrase required for this entry"));
    }

    {
        KTempDir ktempdir;
        QVERIFY(ktempdir.exists());
        karchive.setReadPassphrase("foobar");
        QVERIFY(karchive.extract(toextract, ktempdir.name()));
        QCOMPARE(karchive.errorString(), QString());
    }
}

void KArchiveTest::progress()
{

    {
        QStringList toadd = QStringList()
            << QFile::decodeName(KDESRCDIR "/CMakeLists.txt")
            << QFile::decodeName(KDESRCDIR "/karchivetest.cpp");

        KArchive karchive(tmpName(QString::fromLatin1(".tar.gz")));
        QVERIFY(karchive.isWritable());

        QSignalSpy signalspy(&karchive, SIGNAL(progress(qreal)));
        QVERIFY(signalspy.isValid());

        QVERIFY(karchive.add(toadd));
        QCOMPARE(karchive.errorString(), QString());

        QCOMPARE(signalspy.size(), 2);
        QCOMPARE(signalspy[0][0].toReal(), qreal(0.5));
        QCOMPARE(signalspy[1][0].toReal(), qreal(1.0));
    }

    {
        QStringList toremove = QStringList()
            << QFile::decodeName("CMakeLists.txt")
            << QFile::decodeName("karchivetest.cpp");

        KArchive karchive(tmpCopy(QFile::decodeName(KDESRCDIR "/tests_progress.tar.gz")));
        QVERIFY(karchive.isReadable());
        QCOMPARE(karchive.list().size(), 7);

        QSignalSpy signalspy(&karchive, SIGNAL(progress(qreal)));
        QVERIFY(signalspy.isValid());

        QVERIFY(karchive.remove(toremove));
        QCOMPARE(karchive.errorString(), QString());

        QCOMPARE(signalspy.size(), 2);
        QCOMPARE(signalspy[0][0].toReal(), qreal(0.5));
        QCOMPARE(signalspy[1][0].toReal(), qreal(1.0));
    }

    {
        QStringList toextract = QStringList()
            << QFile::decodeName("CMakeLists.txt")
            << QFile::decodeName("karchivetest.cpp");

        KArchive karchive(QFile::decodeName(KDESRCDIR "/tests_progress.tar.gz"));
        QVERIFY(karchive.isReadable());
        QCOMPARE(karchive.list().size(), 7);

        QSignalSpy signalspy(&karchive, SIGNAL(progress(qreal)));
        QVERIFY(signalspy.isValid());

        KTempDir ktempdir;
        QVERIFY(ktempdir.exists());
        QVERIFY(karchive.extract(toextract, ktempdir.name()));
        QCOMPARE(karchive.errorString(), QString());

        QCOMPARE(signalspy.size(), 2);
        QCOMPARE(signalspy[0][0].toReal(), qreal(0.5));
        QCOMPARE(signalspy[1][0].toReal(), qreal(1.0));
    }
}

#include "karchivetest.moc"
