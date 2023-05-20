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

    KTemperature tostringtemp(123.4, QString::fromUtf8("°F"));
    QCOMPARE(tostringtemp.toString(), QString::fromUtf8("123.4 °F"));

    KTemperature ctemp(12.3, QString::fromUtf8("°C"));
    QCOMPARE(ctemp.unitEnum(), KTemperature::Celsius);
    QCOMPARE(KUnitConversion::round(ctemp.convertTo(KTemperature::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(ctemp.convertTo(KTemperature::Fahrenheit), 1), 54.1);
    QCOMPARE(KUnitConversion::round(ctemp.convertTo(KTemperature::Celsius), 1), 12.3);
    QCOMPARE(KUnitConversion::round(ctemp.convertTo(KTemperature::Kelvin), 1), 285.5);
    QCOMPARE(KUnitConversion::round(ctemp.convertTo(KTemperature::UnitCount), 1), 0.0);

    KTemperature ftemp(123.4, QString::fromUtf8("°F"));
    QCOMPARE(ftemp.unitEnum(), KTemperature::Fahrenheit);
    QCOMPARE(KUnitConversion::round(ftemp.convertTo(KTemperature::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(ftemp.convertTo(KTemperature::Fahrenheit), 1), 123.4);
    QCOMPARE(KUnitConversion::round(ftemp.convertTo(KTemperature::Celsius), 1), 50.8);
    QCOMPARE(KUnitConversion::round(ftemp.convertTo(KTemperature::Kelvin), 1), 323.9);
    QCOMPARE(KUnitConversion::round(ftemp.convertTo(KTemperature::UnitCount), 1), 0.0);

    KTemperature ktemp(1234.5, "K");
    QCOMPARE(ktemp.unitEnum(), KTemperature::Kelvin);
    QCOMPARE(KUnitConversion::round(ktemp.convertTo(KTemperature::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(ktemp.convertTo(KTemperature::Fahrenheit), 1), 1762.4);
    QCOMPARE(KUnitConversion::round(ktemp.convertTo(KTemperature::Celsius), 1), 961.4);
    QCOMPARE(KUnitConversion::round(ktemp.convertTo(KTemperature::Kelvin), 1), 1234.5);
    QCOMPARE(KUnitConversion::round(ktemp.convertTo(KTemperature::UnitCount), 1), 0.0);
}


void KUnitConversionTest::testVelocity()
{
    KVelocity invalidvelo(12, "");
    QCOMPARE(invalidvelo.unitEnum(), KVelocity::Invalid);

    KVelocity kmhvelo(12.3, "km/h");
    QCOMPARE(kmhvelo.unitEnum(), KVelocity::KilometerPerHour);
    QCOMPARE(KUnitConversion::round(kmhvelo.convertTo(KVelocity::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(kmhvelo.convertTo(KVelocity::MeterPerSecond), 1), 3.4);
    QCOMPARE(KUnitConversion::round(kmhvelo.convertTo(KVelocity::KilometerPerHour), 1), 12.3);
    QCOMPARE(KUnitConversion::round(kmhvelo.convertTo(KVelocity::MilePerHour), 1), 7.6);
    QCOMPARE(KUnitConversion::round(kmhvelo.convertTo(KVelocity::Knot), 1), 6.6);
    QCOMPARE(KUnitConversion::round(kmhvelo.convertTo(KVelocity::UnitCount), 1), 0.0);

    KVelocity ktvelo(12.3, "kt");
    QCOMPARE(ktvelo.unitEnum(), KVelocity::Knot);
    QCOMPARE(KUnitConversion::round(ktvelo.convertTo(KVelocity::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(ktvelo.convertTo(KVelocity::MeterPerSecond), 1), 6.3);
    QCOMPARE(KUnitConversion::round(ktvelo.convertTo(KVelocity::KilometerPerHour), 1), 22.8);
    QCOMPARE(KUnitConversion::round(ktvelo.convertTo(KVelocity::MilePerHour), 1), 14.2);
    QCOMPARE(KUnitConversion::round(ktvelo.convertTo(KVelocity::Knot), 1), 12.3);
    QCOMPARE(KUnitConversion::round(ktvelo.convertTo(KVelocity::UnitCount), 1), 0.0);

    KVelocity msvelo(12.3, "m/s");
    QCOMPARE(msvelo.unitEnum(), KVelocity::MeterPerSecond);
    QCOMPARE(KUnitConversion::round(msvelo.convertTo(KVelocity::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(msvelo.convertTo(KVelocity::MeterPerSecond), 1), 12.3);
    QCOMPARE(KUnitConversion::round(msvelo.convertTo(KVelocity::KilometerPerHour), 1), 44.3);
    QCOMPARE(KUnitConversion::round(msvelo.convertTo(KVelocity::MilePerHour), 1), 27.5);
    QCOMPARE(KUnitConversion::round(msvelo.convertTo(KVelocity::Knot), 1), 23.9);
    QCOMPARE(KUnitConversion::round(msvelo.convertTo(KVelocity::UnitCount), 1), 0.0);

    KVelocity mphvelo(12.3, "mph");
    QCOMPARE(mphvelo.unitEnum(), KVelocity::MilePerHour);
    QCOMPARE(KUnitConversion::round(mphvelo.convertTo(KVelocity::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(mphvelo.convertTo(KVelocity::MeterPerSecond), 1), 5.5);
    QCOMPARE(KUnitConversion::round(mphvelo.convertTo(KVelocity::KilometerPerHour), 1), 19.8);
    QCOMPARE(KUnitConversion::round(mphvelo.convertTo(KVelocity::MilePerHour), 1), 12.3);
    QCOMPARE(KUnitConversion::round(mphvelo.convertTo(KVelocity::Knot), 1), 10.7);
    QCOMPARE(KUnitConversion::round(mphvelo.convertTo(KVelocity::UnitCount), 1), 0.0);
}

void KUnitConversionTest::testPressure()
{
    KPressure invalidpres(12, "");
    QCOMPARE(invalidpres.unitEnum(), KPressure::Invalid);

    KPressure hpapres(12.3, "hectopascal");
    QCOMPARE(hpapres.unitEnum(), KPressure::Hectopascal);
    QCOMPARE(KUnitConversion::round(hpapres.convertTo(KPressure::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(hpapres.convertTo(KPressure::Kilopascal), 1), 1.2);
    QCOMPARE(KUnitConversion::round(hpapres.convertTo(KPressure::Hectopascal), 1), 12.3);
    QCOMPARE(KUnitConversion::round(hpapres.convertTo(KPressure::Millibar), 1), 12.3);
    QCOMPARE(KUnitConversion::round(hpapres.convertTo(KPressure::InchesOfMercury), 1), 0.4);
    QCOMPARE(KUnitConversion::round(hpapres.convertTo(KPressure::UnitCount), 1), 0.0);

    KPressure inhpres(12.3, "inch of mercury");
    QCOMPARE(inhpres.unitEnum(), KPressure::InchesOfMercury);
    QCOMPARE(KUnitConversion::round(inhpres.convertTo(KPressure::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(inhpres.convertTo(KPressure::Kilopascal), 1), 41.7);
    QCOMPARE(KUnitConversion::round(inhpres.convertTo(KPressure::Hectopascal), 1), 416.5);
    QCOMPARE(KUnitConversion::round(inhpres.convertTo(KPressure::Millibar), 1), 416.5);
    QCOMPARE(KUnitConversion::round(inhpres.convertTo(KPressure::InchesOfMercury), 1), 12.3);
    QCOMPARE(KUnitConversion::round(inhpres.convertTo(KPressure::UnitCount), 1), 0.0);

    KPressure kpapres(12.3, "kilopascal");
    QCOMPARE(kpapres.unitEnum(), KPressure::Kilopascal);
    QCOMPARE(KUnitConversion::round(kpapres.convertTo(KPressure::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(kpapres.convertTo(KPressure::Kilopascal), 1), 12.3);
    QCOMPARE(KUnitConversion::round(kpapres.convertTo(KPressure::Hectopascal), 1), 123.0);
    QCOMPARE(KUnitConversion::round(kpapres.convertTo(KPressure::Millibar), 1), 123.0);
    QCOMPARE(KUnitConversion::round(kpapres.convertTo(KPressure::InchesOfMercury), 1), 3.6);
    QCOMPARE(KUnitConversion::round(kpapres.convertTo(KPressure::UnitCount), 1), 0.0);

    KPressure mbarpres(12.3, "millibar");
    QCOMPARE(mbarpres.unitEnum(), KPressure::Millibar);
    QCOMPARE(KUnitConversion::round(mbarpres.convertTo(KPressure::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(mbarpres.convertTo(KPressure::Kilopascal), 1), 1.2);
    QCOMPARE(KUnitConversion::round(mbarpres.convertTo(KPressure::Hectopascal), 1), 12.3);
    QCOMPARE(KUnitConversion::round(mbarpres.convertTo(KPressure::Millibar), 1), 12.3);
    QCOMPARE(KUnitConversion::round(mbarpres.convertTo(KPressure::InchesOfMercury), 1), 0.4);
    QCOMPARE(KUnitConversion::round(mbarpres.convertTo(KPressure::UnitCount), 1), 0.0);
}

void KUnitConversionTest::testLength()
{
    KLength invalidleng(12, "");
    QCOMPARE(invalidleng.unitEnum(), KLength::Invalid);

    KLength kmleng(12.3, "kilometer");
    QCOMPARE(kmleng.unitEnum(), KLength::Kilometer);
    QCOMPARE(KUnitConversion::round(kmleng.convertTo(KLength::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(kmleng.convertTo(KLength::Kilometer), 1), 12.3);
    QCOMPARE(KUnitConversion::round(kmleng.convertTo(KLength::Foot), 1), 40354.3);
    QCOMPARE(KUnitConversion::round(kmleng.convertTo(KLength::Mile), 1), 7.6);
    QCOMPARE(KUnitConversion::round(kmleng.convertTo(KLength::UnitCount), 1), 0.0);

    KLength ftleng(12.3, "foot");
    QCOMPARE(ftleng.unitEnum(), KLength::Foot);
    QCOMPARE(KUnitConversion::round(ftleng.convertTo(KLength::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(ftleng.convertTo(KLength::Kilometer), 8), 0.00374904);
    QCOMPARE(KUnitConversion::round(ftleng.convertTo(KLength::Foot), 1), 12.3);
    QCOMPARE(KUnitConversion::round(ftleng.convertTo(KLength::Mile), 8), 0.00232955);
    QCOMPARE(KUnitConversion::round(ftleng.convertTo(KLength::UnitCount), 1), 0.0);

    KLength mileng(12.3, "mile");
    QCOMPARE(mileng.unitEnum(), KLength::Mile);
    QCOMPARE(KUnitConversion::round(mileng.convertTo(KLength::Invalid), 1), 0.0);
    QCOMPARE(KUnitConversion::round(mileng.convertTo(KLength::Kilometer), 1), 19.8);
    QCOMPARE(KUnitConversion::round(mileng.convertTo(KLength::Foot), 1), 64944.0);
    QCOMPARE(KUnitConversion::round(mileng.convertTo(KLength::Mile), 1), 12.3);
    QCOMPARE(KUnitConversion::round(mileng.convertTo(KLength::UnitCount), 1), 0.0);
}

#include "moc_kunitconversiontest.cpp"
