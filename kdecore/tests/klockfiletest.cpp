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

#include <QProcess>
#include <QTextStream>

#include <unistd.h>

// TODO test locking from two different threads

static const QString lockName = "klockfiletest";
static const char *const lockNameFull = "klockfiletest.klockfile";

void Test_KLockFile::initTestCase()
{
    QFile::remove(QString::fromLatin1(lockNameFull));
    lockFile = new KLockFile(lockName);
}

void Test_KLockFile::cleanupTestCase()
{
    delete lockFile;
    lockFile = nullptr;
}

static bool testLockFromProcess(const QString& lockName)
{
    const int ret = QProcess::execute(KDEBINDIR "/kdecore-klockfile_testlock", QStringList() << lockName);
    return bool(ret);
}

void Test_KLockFile::testLock()
{
    QVERIFY(!lockFile->isLocked());
    QCOMPARE(lockFile->tryLock(), true);
    QVERIFY(lockFile->isLocked());

    // Try to lock it again, should fail
    KLockFile *lockFile2 = new KLockFile(lockName);
    QVERIFY(!lockFile2->isLocked());
    QCOMPARE(lockFile2->tryLock(), false);
    QVERIFY(!lockFile2->isLocked());
    delete lockFile2;

    // Also try from a different process.
    QCOMPARE(testLockFromProcess(lockName), false);
}

void Test_KLockFile::testUnlock()
{
    QVERIFY(lockFile->isLocked());
    lockFile->unlock();
    QVERIFY(!lockFile->isLocked());
}

QTEST_KDEMAIN_CORE(Test_KLockFile)
