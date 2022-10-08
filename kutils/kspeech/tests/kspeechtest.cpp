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
#include "kspeech.h"
#include "kdebug.h"

class KSpeechTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void say_data();
    void say();

    void setters_and_getters_data();
    void setters_and_getters();
};

QTEST_KDEMAIN_CORE(KSpeechTest)

void KSpeechTest::initTestCase()
{
    if (!KSpeech::isSupported()) {
        QSKIP("Built without speech-dispatcher", SkipAll);
    }
}

void KSpeechTest::cleanupTestCase()
{
}

void KSpeechTest::say_data()
{
    QTest::addColumn<QString>("saystr");
    QTest::newRow("US-ASCII") << QString::fromAscii("foobar");
    QTest::newRow("UTF-8") << QString::fromUtf8("лорем ипсум");
}

void KSpeechTest::say()
{
    QFETCH(QString, saystr);

    KSpeech kspeech(this);
    QVERIFY(kspeech.say(saystr) != 0);
}

void KSpeechTest::setters_and_getters_data()
{
    QTest::addColumn<int>("volume");
    QTest::addColumn<int>("pitch");
    QTest::addColumn<QByteArray>("voice");
    QTest::addColumn<bool>("valid");
    // check `spd-say -L` for valid voice
    QTest::newRow("valid") << int(10) << int(2) << QByteArray("Slovak") << bool(true);
    QTest::newRow("invalid") << int(-123) << int(321) << QByteArray("foobar") << bool(false);
}

void KSpeechTest::setters_and_getters()
{
    QFETCH(int, volume);
    QFETCH(int, pitch);
    QFETCH(QByteArray, voice);
    QFETCH(bool, valid);

    KSpeech kspeech(this);
    QCOMPARE(kspeech.setVolume(volume), valid);
    QCOMPARE(kspeech.setPitch(pitch), valid);
    QCOMPARE(kspeech.setVoice(voice), valid);
    if (valid) {
        QCOMPARE(kspeech.volume(), volume);
        QCOMPARE(kspeech.pitch(), pitch);
        QCOMPARE(kspeech.voice(), voice);
    } else {
        QCOMPARE(kspeech.volume(), 100);
        QCOMPARE(kspeech.pitch(), 0);
        QCOMPARE(kspeech.voice(), QByteArray());
    }
}

#include "kspeechtest.moc"
