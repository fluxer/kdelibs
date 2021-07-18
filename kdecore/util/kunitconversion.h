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

#ifndef KUNITCONVERSION_H
#define KUNITCONVERSION_H

#include <QString>

class KTemperaturePrivate;
class KVelocityPrivate;
class KPressurePrivate;
class KLengthPrivate;

/*!
    Base class for all other conversion classes

    @since 4.20
    @see KTemperature, KVelocity, KPressure, KLength
*/
class KUnitConversion {
public:
    /*!
        @return Rounded number up to number of decimal digits specified by @p digits
    */
    static double round(const double number, const uint digits);
};

/*!
    Temperature conversion class

    @since 4.20
*/
class KTemperature : public KUnitConversion {
public:
    enum KTempUnit {
        Invalid = -1,
        Celsius = 0,
        Fahrenheit = 1,
        Kelvin = 2,
        UnitCount = 3
    };

    /*!
        @brief Constructs convertor
        @param number value of the unit
        @param unit unit enum, one of: Invalid, Celsius, Fahrenheit or Kelvin
    */
    KTemperature(const double number, const KTempUnit unit);
    /*!
        @brief Constructs convertor
        @param number value of the unit
        @param unit string representation of the unit, one of: "°C", "C", "Celsius", "°F", "F",
        "Fahrenheit", "Kelvin" or "K"
    */
    KTemperature(const double number, const QString &unit);
    ~KTemperature();

    /*!
        @return Same number as the value passed to the constructor
    */
    double number() const;
    /*!
        @return Short string representing the unit passed to the constructor, e.g. "Unknown" (for
        @p KTempUnit::Invalid), "°F" or "K"
    */
    QString unit() const;
    /*!
        @return Enum representing the unit passed to the constructor, e.g. @p KTempUnit::Invalid,
        @p KTempUnit::Fahrenheit or @p KTempUnit::Kelvin
    */
    KTempUnit unitEnum() const;
    /*!
        @return Combination of the number and short unit as string, e.g. "12 °C", "123 °F" or
        "123 K"
    */
    QString toString() const;
    /*!
        @return Number converted to different unit
    */
    double convertTo(const KTempUnit unit) const;
    /*!
        @return Translated convertor description, e.g. "Температура"
    */
    static QString description();
    /*!
        @return Translated unit description, e.g. "Unknown" (for @p KTempUnit::Invalid),
        "Fahrenheitia (°F)" or "Келвин (K)"
    */
    static QString unitDescription(const KTempUnit unit);

private:
    Q_DISABLE_COPY(KTemperature);
    KTemperaturePrivate * const d;
};

/*!
    Velocity conversion class

    @since 4.20
*/
class KVelocity : public KUnitConversion {
public:
    enum KVeloUnit {
        Invalid = -1,
        KilometerPerHour = 0,
        Knot = 1,
        MeterPerSecond = 2,
        MilePerHour = 3,
        UnitCount = 4
    };

    KVelocity(const double number, const KVeloUnit unit);
    KVelocity(const double number, const QString &unit);
    ~KVelocity();

    double number() const;
    QString unit() const;
    KVeloUnit unitEnum() const;
    QString toString() const;
    double convertTo(const KVeloUnit unit) const;
    static QString description();
    static QString unitDescription(const KVeloUnit unit);

private:
    Q_DISABLE_COPY(KVelocity);
    KVelocityPrivate * const d;
};

/*!
    Pressure conversion class

    @since 4.20
*/
class KPressure : public KUnitConversion {
public:
    enum KPresUnit {
        Invalid = -1,
        Hectopascal = 0,
        InchesOfMercury = 1,
        Kilopascal = 2,
        Millibar = 3,
        UnitCount = 4
    };

    KPressure(const double number, const KPresUnit unit);
    KPressure(const double number, const QString &unit);
    ~KPressure();

    double number() const;
    QString unit() const;
    KPresUnit unitEnum() const;
    QString toString() const;
    double convertTo(const KPresUnit unit) const;
    static QString description();
    static QString unitDescription(const KPresUnit unit);

private:
    Q_DISABLE_COPY(KPressure);
    KPressurePrivate * const d;
};

/*!
    Length conversion class

    @since 4.20
*/
class KLength : public KUnitConversion {
public:
    enum KLengUnit {
        Invalid = -1,
        Kilometer = 0,
        Mile = 1,
        UnitCount = 2
    };

    KLength(const double number, const KLengUnit unit);
    KLength(const double number, const QString &unit);
    ~KLength();

    double number() const;
    QString unit() const;
    KLengUnit unitEnum() const;
    QString toString() const;
    double convertTo(const KLengUnit unit) const;
    static QString description();
    static QString unitDescription(const KLengUnit unit);

private:
    Q_DISABLE_COPY(KLength);
    KLengthPrivate * const d;
};

#endif // KUNITCONVERSION_H
