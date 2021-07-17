/*  This file is part of the KDE libraries
    Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>

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

#include "kunitconversiontest.h"
#include "kunitconversion.h"
#include "qtest_kde.h"

QTEST_KDEMAIN_CORE(KUnitConversionTest)

void KUnitConversionTest::initTestCase()
{
}

void KUnitConversionTest::testRound()
{
    static const double toround = 1.23456789;
    QCOMPARE(KUnitConversion::round(toround, 1), 1.2);
    QCOMPARE(KUnitConversion::round(toround, 2), 1.23);
    QCOMPARE(KUnitConversion::round(toround, 3), 1.235);
}

void KUnitConversionTest::testTemperature()
{
    KTemperature invalidtemp(12, "");
    QCOMPARE(invalidtemp.unitEnum(), KTemperature::Invalid);

    KTemperature utf8celsiustemp(12, "°C");
    QCOMPARE(utf8celsiustemp.unitEnum(), KTemperature::Celsius);

    KTemperature ccelsiustemp(12, "C");
    QCOMPARE(ccelsiustemp.unitEnum(), KTemperature::Celsius);

    KTemperature tostringtemp(123.4, "°F");
    QCOMPARE(tostringtemp.toString(), QString::fromUtf8("123.4 °F"));

    KTemperature converttotemp(123.4, "°F");
    QCOMPARE(KUnitConversion::round(converttotemp.convertTo(KTemperature::Celsius), 1), 50.8);
}

#include "moc_kunitconversiontest.cpp"
