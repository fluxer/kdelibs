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

/*!
    Temperature conversion class
    @since 4.20
    @todo implement other classes to substitute KUnitConversion
*/
class KTemperature {
public:
    enum KTempUnit {
        Invalid,
        Celsius,
        Fahrenheit,
        Kelvin
    };

    /*!
        @brief Constructs convertor
        @param number value of the unit
        @param unit string representation of the unit one of: "°C", "C", "Celsius", "°F", "F", "Fahrenheit", "Kelvin" or "K"
    */
    KTemperature(const double number, const QString &unit);
    ~KTemperature();

    /*!
        @return Same number as the value passed to the constructor
    */
    double number() const;
    /*!
        @return Short string representing the unit passed to the constructor, e.g. "°C", "°F" or "K"
    */
    QString unit() const;
    /*!
        @return Combination of the number and unit as string, e.g. "12 °C", "123 °F" or "123 K"
    */
    QString toString() const;
    /*!
        @return Number converted to different unit
    */
    double convertTo(const KTempUnit unit) const;

    /*!
        @return Rounded number up to number of decimal digits specified by @p digits
    */
    static double round(const double number, const uint digits);

private:
    Q_DISABLE_COPY(KTemperature);
    KTemperaturePrivate * const d;
};

#endif
