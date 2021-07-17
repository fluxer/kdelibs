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

    KTemperature cconverttotemp(12.3, "°C");
    QCOMPARE(KUnitConversion::round(cconverttotemp.convertTo(KTemperature::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(cconverttotemp.convertTo(KTemperature::Fahrenheit), 1), 54.1);
    QCOMPARE(KUnitConversion::round(cconverttotemp.convertTo(KTemperature::Celsius), 1), 12.3);
    QCOMPARE(KUnitConversion::round(cconverttotemp.convertTo(KTemperature::Kelvin), 1), 285.5);
    QCOMPARE(KUnitConversion::round(cconverttotemp.convertTo(KTemperature::UnitCount), 1), 0.0);

    KTemperature fconverttotemp(123.4, "°F");
    QCOMPARE(KUnitConversion::round(fconverttotemp.convertTo(KTemperature::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(fconverttotemp.convertTo(KTemperature::Fahrenheit), 1), 123.4);
    QCOMPARE(KUnitConversion::round(fconverttotemp.convertTo(KTemperature::Celsius), 1), 50.8);
    QCOMPARE(KUnitConversion::round(fconverttotemp.convertTo(KTemperature::Kelvin), 1), 323.9);
    QCOMPARE(KUnitConversion::round(fconverttotemp.convertTo(KTemperature::UnitCount), 1), 0.0);

    KTemperature kconverttotemp(1234.5, "K");
    QCOMPARE(KUnitConversion::round(kconverttotemp.convertTo(KTemperature::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(kconverttotemp.convertTo(KTemperature::Fahrenheit), 1), 1762.4);
    QCOMPARE(KUnitConversion::round(kconverttotemp.convertTo(KTemperature::Celsius), 1), 961.4);
    QCOMPARE(KUnitConversion::round(kconverttotemp.convertTo(KTemperature::Kelvin), 1), 1234.5);
    QCOMPARE(KUnitConversion::round(kconverttotemp.convertTo(KTemperature::UnitCount), 1), 0.0);
}

#include "moc_kunitconversiontest.cpp"
