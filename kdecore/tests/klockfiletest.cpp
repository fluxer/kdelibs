/* This file is part of the KDE libraries
    Copyright (c) 2005 Thomas Braxton <brax108@cox.net>

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

#include "klockfiletest.h"
#include "moc_klockfiletest.cpp"

#include <kdebug.h>
#include <unistd.h>

#include <QProcess>
#include <QHostInfo>

// TODO test locking from two different threads

QT_BEGIN_NAMESPACE
namespace QTest {
template<>
char* toString(const KLockFile::LockResult &result)
    {
        static const char *const strings[] = {
                "LockOK", "LockFail", "LockError", "LockStale"
        };
        return qstrdup(strings[result]);
    }
}
QT_END_NAMESPACE

static const QString lockName = "klockfiletest";
static const char *const lockNameFull = "klockfiletest.klockfile";

static bool seenWarningMessage = false;
static QtMsgHandler msgHandler = nullptr;

void kMessageHandler(QtMsgType type, const char *msg)
{
    QByteArray expectedMsg = QByteArray("Deleting stale lock file \"") + lockNameFull + "\"";
    if (QByteArray(msg).contains(expectedMsg)) {
        seenWarningMessage = true;
    }
    msgHandler(type, msg);
}

void Test_KLockFile::initTestCase()
{
    msgHandler = qInstallMsgHandler(kMessageHandler);
    QFile::remove(QString::fromLatin1(lockNameFull));
    lockFile = new KLockFile(lockName);
}

void Test_KLockFile::cleanupTestCase()
{
    delete lockFile;
    lockFile = nullptr;
    qInstallMsgHandler(nullptr);
}

static KLockFile::LockResult testLockFromProcess(const QString& lockName)
{
    const int ret = QProcess::execute(KDEBINDIR "/kdecore-klockfile_testlock", QStringList() << lockName);
    return KLockFile::LockResult(ret);
}

void Test_KLockFile::testLock()
{
    QVERIFY(!lockFile->isLocked());
    QCOMPARE(lockFile->lock(), KLockFile::LockOK);
    QVERIFY(lockFile->isLocked());

    // Try to lock it again, should fail
    KLockFile *lockFile2 = new KLockFile(lockName);
    QVERIFY(!lockFile2->isLocked());
    QCOMPARE(lockFile2->lock(KLockFile::NoBlockFlag), KLockFile::LockFail);
    QVERIFY(!lockFile2->isLocked());
    delete lockFile2;

    // Also try from a different process.
    QCOMPARE(testLockFromProcess(lockName), KLockFile::LockFail);
}

void Test_KLockFile::testLockInfo()
{
    QVERIFY(lockFile->isLocked());

    KLockFile lf(lockName);
    QCOMPARE(lf.lock(KLockFile::NoBlockFlag), KLockFile::LockFail);
    QVERIFY(!lf.isLocked());

    qint64 pid = -1;
    QString host;
    QVERIFY(!lf.getLockInfo(pid, host));
    QCOMPARE(pid, qint64(-1));
    QCOMPARE(host, QString());

    pid = -1;
    host.clear();
    QVERIFY(lockFile->getLockInfo(pid, host));
    QCOMPARE(pid, static_cast<qint64>(::getpid()));
    QCOMPARE(host, QHostInfo::localHostName());
}

void Test_KLockFile::testUnlock()
{
    QVERIFY(lockFile->isLocked());
    lockFile->unlock();
    QVERIFY(!lockFile->isLocked());
}


void Test_KLockFile::testStaleNoBlockFlag()
{
    QFile f(QString::fromLatin1(lockNameFull));
    f.open(QIODevice::WriteOnly);
    QTextStream stream(&f);
    stream << (QString::number(111222) + QLatin1Char('\t') + QHostInfo::localHostName());
    stream.flush();
    f.close();

    KLockFile* lockFile2 = new KLockFile(lockName);
    QVERIFY(!lockFile2->isLocked());
    QCOMPARE(lockFile2->lock(KLockFile::NoBlockFlag), KLockFile::LockStale);
    seenWarningMessage = false;
    QCOMPARE(lockFile2->lock(KLockFile::NoBlockFlag|KLockFile::ForceFlag), KLockFile::LockOK);
    QTest::qWait(2000);
    QVERIFY(seenWarningMessage);

    QVERIFY(lockFile2->isLocked());
    delete lockFile2;
}

QTEST_KDEMAIN_CORE(Test_KLockFile)
