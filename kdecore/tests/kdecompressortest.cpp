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
#include "kdecompressor.h"
#include "kdebug.h"

#include <qplatformdefs.h>

static const QByteArray s_shorttestdata = QByteArray("foobar");
static const QByteArray s_longtestdata = s_shorttestdata.repeated(QT_BUFFSIZE);

class KDecompressorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void process_data();
    void process();
};

QTEST_KDEMAIN_CORE(KDecompressorTest)

void KDecompressorTest::initTestCase()
{
}

void KDecompressorTest::cleanupTestCase()
{
}

void KDecompressorTest::process_data()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<int>("level");
    QTest::addColumn<QByteArray>("shortdata");
    QTest::addColumn<QByteArray>("longdata");

    static const char* kdecompressortypestr[] = {
        "KDecompressor::TypeUnknown",
        "KDecompressor::TypeDeflate",
        "KDecompressor::TypeZlib",
        "KDecompressor::TypeGZip",
        "KDecompressor::TypeBZip2",
        "KDecompressor::TypeXZ"
    };
    for (int itype = 1; itype < int(KDecompressor::TypeXZ + 1); itype++) {
        for (int ilevel = 0; ilevel < 10; ilevel++) {
            if (itype == int(KDecompressor::TypeBZip2) && ilevel == 0) {
                // compression level 0 is not valid for Bzip2
                continue;
            }
            QTest::newRow(kdecompressortypestr[itype]) << itype << ilevel << s_shorttestdata << s_longtestdata;
        }
    }
}

void KDecompressorTest::process()
{
    QFETCH(int, type);
    QFETCH(int, level);
    QFETCH(QByteArray, shortdata);
    QFETCH(QByteArray, longdata);

    {
        KCompressor::KCompressorType kcompressortype = static_cast<KCompressor::KCompressorType>(type);
        KCompressor kcompressor;
        kcompressor.setType(kcompressortype);
        QCOMPARE(kcompressor.type(), kcompressortype);
        kcompressor.setLevel(level);
        QCOMPARE(kcompressor.level(), level);
        QCOMPARE(kcompressor.process(shortdata), true);
        const QByteArray compresseddata = kcompressor.result();

        KDecompressor::KDecompressorType kdecompressortype = static_cast<KDecompressor::KDecompressorType>(type);
        KDecompressor kdecompressor;
        kdecompressor.setType(kdecompressortype);
        QCOMPARE(kdecompressor.type(), kdecompressortype);
        QCOMPARE(kdecompressor.process(compresseddata), true);
        QCOMPARE(kdecompressor.errorString(), QString());
        const QByteArray decompresseddata = kdecompressor.result();
        QCOMPARE(decompresseddata, shortdata);
    }

    {
        KCompressor::KCompressorType kcompressortype = static_cast<KCompressor::KCompressorType>(type);
        KCompressor kcompressor;
        kcompressor.setType(kcompressortype);
        QCOMPARE(kcompressor.type(), kcompressortype);
        kcompressor.setLevel(level);
        QCOMPARE(kcompressor.level(), level);
        QCOMPARE(kcompressor.process(longdata), true);
        const QByteArray compresseddata = kcompressor.result();

        KDecompressor::KDecompressorType kdecompressortype = static_cast<KDecompressor::KDecompressorType>(type);
        KDecompressor kdecompressor;
        kdecompressor.setType(kdecompressortype);
        QCOMPARE(kdecompressor.type(), kdecompressortype);
        QCOMPARE(kdecompressor.process(compresseddata), true);
        QCOMPARE(kdecompressor.errorString(), QString());
        const QByteArray decompresseddata = kdecompressor.result();
        QCOMPARE(decompresseddata, longdata);
    }
}

#include "kdecompressortest.moc"
