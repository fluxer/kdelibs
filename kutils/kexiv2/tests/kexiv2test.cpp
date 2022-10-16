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
#include "kexiv2.h"
#include "kdebug.h"

class KExiv2Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void orientation_data();
    void orientation();
};

QTEST_KDEMAIN_CORE(KExiv2Test)

void KExiv2Test::initTestCase()
{
    if (!KExiv2::isSupported()) {
        QSKIP("Built without Exiv2", SkipAll);
    }
}

void KExiv2Test::cleanupTestCase()
{
}

void KExiv2Test::orientation_data()
{
    QTest::addColumn<QString>("imagepath");
    QTest::addColumn<bool>("different");

    QTest::newRow("landscape_1.jpg") << QFile::decodeName(KDESRCDIR "/orientation/landscape_1.jpg") << false;
    QTest::newRow("landscape_2.jpg") << QFile::decodeName(KDESRCDIR "/orientation/landscape_2.jpg") << true;
    QTest::newRow("landscape_3.jpg") << QFile::decodeName(KDESRCDIR "/orientation/landscape_3.jpg") << true;
    QTest::newRow("landscape_4.jpg") << QFile::decodeName(KDESRCDIR "/orientation/landscape_4.jpg") << true;
    QTest::newRow("landscape_5.jpg") << QFile::decodeName(KDESRCDIR "/orientation/landscape_5.jpg") << true;
    QTest::newRow("landscape_6.jpg") << QFile::decodeName(KDESRCDIR "/orientation/landscape_6.jpg") << true;
    QTest::newRow("landscape_7.jpg") << QFile::decodeName(KDESRCDIR "/orientation/landscape_7.jpg") << true;
    QTest::newRow("landscape_8.jpg") << QFile::decodeName(KDESRCDIR "/orientation/landscape_8.jpg") << true;

    QTest::newRow("portrait_1.jpg") << QFile::decodeName(KDESRCDIR "/orientation/portrait_1.jpg") << false;
    QTest::newRow("portrait_2.jpg") << QFile::decodeName(KDESRCDIR "/orientation/portrait_2.jpg") << true;
    QTest::newRow("portrait_3.jpg") << QFile::decodeName(KDESRCDIR "/orientation/portrait_3.jpg") << true;
    QTest::newRow("portrait_4.jpg") << QFile::decodeName(KDESRCDIR "/orientation/portrait_4.jpg") << true;
    QTest::newRow("portrait_5.jpg") << QFile::decodeName(KDESRCDIR "/orientation/portrait_5.jpg") << true;
    QTest::newRow("portrait_6.jpg") << QFile::decodeName(KDESRCDIR "/orientation/portrait_6.jpg") << true;
    QTest::newRow("portrait_7.jpg") << QFile::decodeName(KDESRCDIR "/orientation/portrait_7.jpg") << true;
    QTest::newRow("portrait_8.jpg") << QFile::decodeName(KDESRCDIR "/orientation/portrait_8.jpg") << true;
}

void KExiv2Test::orientation()
{
    QFETCH(QString, imagepath);
    QFETCH(bool, different);

    QImage qimage(imagepath);
    QImage qimagecopy(qimage);
    QVERIFY(qimage == qimagecopy);
    KExiv2 kexiv2(imagepath);
    QVERIFY(kexiv2.rotateImage(qimage));
    QCOMPARE(qimage != qimagecopy, different);
}

#include "kexiv2test.moc"
