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

#include "kpasswdstoretest.h"
#include "qtest_kde.h"
#include "kpasswdstore.h"

#include <QDebug>

QTEST_KDEMAIN(KPasswdStoreTest, GUI)

/**
 * @see QTest::initTestCase()
 */
void KPasswdStoreTest::initTestCase()
{
}

/**
 * @see QTest::cleanupTestCase()
 */
void KPasswdStoreTest::cleanupTestCase()
{
}

void KPasswdStoreTest::storeAndGet()
{
    const QString testid = QString::number(qrand());
    const QByteArray testkey = QByteArray("mykey");
    const QString testpassword = QString::fromLatin1("c206e94efcffe47a03694f92bc94a392cadb79ec0895e07b71e1e2275dea3463");

    {
        KPasswdStore kpasswdstore;
        kpasswdstore.setStoreID(testid);
        kpasswdstore.openStore();
    }

    QString firstpass;
    {
        KPasswdStore kpasswdstore;
        kpasswdstore.setStoreID(testid);
        QVERIFY(kpasswdstore.storePasswd(testkey, testpassword));
        QVERIFY(kpasswdstore.hasPasswd(testkey));
        firstpass = kpasswdstore.getPasswd(testkey);
    }

    QString secondpass;
    {
        KPasswdStore kpasswdstore;
        kpasswdstore.setStoreID(testid);
        secondpass = kpasswdstore.getPasswd(testkey);
    }

    // qDebug() << firstpass << secondpass;
    QCOMPARE(firstpass, testpassword);
    QCOMPARE(secondpass, testpassword);
    QCOMPARE(firstpass, secondpass);
}

#include "moc_kpasswdstoretest.cpp"
