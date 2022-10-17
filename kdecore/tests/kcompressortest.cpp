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
#include "kcompressor.h"
#include "kdebug.h"

static const QByteArray s_testdata = QByteArray("foobar");

class KCompressorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void process_data();
    void process();
};

QTEST_KDEMAIN_CORE(KCompressorTest)

void KCompressorTest::initTestCase()
{
}

void KCompressorTest::cleanupTestCase()
{
}

void KCompressorTest::process_data()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<int>("level");
    QTest::addColumn<bool>("success");
    QTest::addColumn<QString>("errorstring");

    static const char* kcompressortypestr[] = {
        "KCompressor::TypeUnknown",
        "KCompressor::TypeDeflate",
        "KCompressor::TypeZlib",
        "KCompressor::TypeGZip",
        "KCompressor::TypeBZip2",
        "KCompressor::TypeXZ"
    };
    for (int itype = 1; itype < int(KCompressor::TypeXZ + 1); itype++) {
        for (int ilevel = 0; ilevel < 10; ilevel++) {
            if (itype == int(KCompressor::TypeBZip2) && ilevel == 0) {
                // compression level 0 is not valid for Bzip2
                continue;
            }
            QTest::newRow(kcompressortypestr[itype]) << itype << ilevel;
        }
    }
}

void KCompressorTest::process()
{
    QFETCH(int, type);
    QFETCH(int, level);

    KCompressor::KCompressorType kcompressortype = static_cast<KCompressor::KCompressorType>(type);
    KCompressor kcompressor;
    kcompressor.setType(kcompressortype);
    QCOMPARE(kcompressor.type(), kcompressortype);
    kcompressor.setLevel(level);
    QCOMPARE(kcompressor.level(), level);
    QCOMPARE(kcompressor.process(s_testdata), true);
    QCOMPARE(kcompressor.errorString(), QString());
}

#include "kcompressortest.moc"
