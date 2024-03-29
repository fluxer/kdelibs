/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "kdirlistertest.h"
#include "kdebug.h"
#include <qtest_kde.h>
#include "kiotesthelper.h"

QTEST_KDEMAIN(KDirListerTest, NoGUI)

void KDirListerTest::initTestCase()
{
    m_tempDir = new KTempDir();
    m_dirLister = new KDirLister();
}

void KDirListerTest::cleanupTestCase()
{
    delete m_dirLister;
    m_dirLister = nullptr;
    delete m_tempDir;
    m_tempDir = nullptr;
}

void KDirListerTest::testOpenUrl()
{
    m_dirLister->openUrl(KUrl(m_tempDir->name()));
    QTest::kWaitForSignal(m_dirLister, SIGNAL(completed()), 5000);
}

void KDirListerTest::testItems()
{
    const QString testfile = QString::fromLatin1("%1/foo.bar").arg(m_tempDir->name());
    const KUrl testfileurl(testfile);
    createTestFile(testfile);

    m_dirLister->openUrl(KUrl(m_tempDir->name()));
    QTest::kWaitForSignal(m_dirLister, SIGNAL(completed()), 5000);
    const KFileItemList diritems = m_dirLister->items();
    QVERIFY(!diritems.isEmpty());
    KFileItem foobaritem = diritems.findByName("foo.bar");
    QVERIFY(!foobaritem.isNull());
    foobaritem = diritems.findByUrl(testfileurl);
    QVERIFY(!foobaritem.isNull());
    foobaritem = m_dirLister->findByName("foo.bar");
    QVERIFY(!foobaritem.isNull());
    foobaritem = m_dirLister->findByUrl(testfileurl);
    QVERIFY(!foobaritem.isNull());
}


void KDirListerTest::testIsFinished()
{
    QCOMPARE(m_dirLister->isFinished(), true);
    m_dirLister->openUrl(KUrl(m_tempDir->name()));
    QTest::kWaitForSignal(m_dirLister, SIGNAL(completed()), 5000);
    QCOMPARE(m_dirLister->isFinished(), true);
}

#include "moc_kdirlistertest.cpp"
