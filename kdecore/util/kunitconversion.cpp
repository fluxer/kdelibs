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

#include "kunitconversion.h"
#include "kdebug.h"

#include <qmath.h>

class KTemperaturePrivate {
public:
    KTemperaturePrivate(const double number, const QString &unit);

    double m_number;
    QString m_unit;
    KTemperature::KTempUnit m_tempunit;
};

KTemperaturePrivate::KTemperaturePrivate(const double number, const QString &unit)
    : m_number(number),
    m_tempunit(KTemperature::Invalid)
{
    if (unit == QString::fromUtf8("°C") || unit == QLatin1String("C") || unit == QLatin1String("Celsius")) {
        m_tempunit = KTemperature::Celsius;
        m_unit = QString::fromUtf8("°C");
    } else if (unit == QString::fromUtf8("°F") || unit == QLatin1String("F") || unit == QLatin1String("Fahrenheit")) {
        m_tempunit = KTemperature::Fahrenheit;
        m_unit = QString::fromUtf8("°F");
    // kelvin and kelvins are for KUnitConversion compatibility
    } else if (unit == QLatin1String("K") || unit == QLatin1String("Kelvin")
        || unit == QLatin1String("kelvin") || unit == QLatin1String("kelvins")) {
        m_tempunit = KTemperature::Kelvin;
        m_unit = QLatin1String("K");
    } else {
        kDebug() << "invalid unit" << unit;
        m_unit = QLatin1String("Unknown");
    }
}

KTemperature::KTemperature(const double number, const QString &unit)
    : d(new KTemperaturePrivate(number, unit))
{
}

KTemperature::~KTemperature()
{
    delete d;
}

double KTemperature::number() const
{
    return d->m_number;
}

QString KTemperature::unit() const
{
    return d->m_unit;
}

QString KTemperature::toString() const
{
    return QString::fromLatin1("%1 %2").arg(QString::number(d->m_number), d->m_unit);
}

double KTemperature::convertTo(const KTempUnit unit) const
{
    if (unit == d->m_tempunit) {
        return d->m_number;
    }

    // for reference:
    // https://www.rapidtables.com/convert/temperature/celsius-to-fahrenheit.html
    // https://www.rapidtables.com/convert/temperature/celsius-to-kelvin.html
    // https://www.rapidtables.com/convert/temperature/fahrenheit-to-celsius.html
    // https://www.rapidtables.com/convert/temperature/fahrenheit-to-kelvin.html
    // https://www.rapidtables.com/convert/temperature/kelvin-to-celsius.html
    // https://www.rapidtables.com/convert/temperature/kelvin-to-fahrenheit.html
    if (d->m_tempunit == KTemperature::Celsius && unit == KTemperature::Fahrenheit) {
        return (d->m_number * 1.8 + 32);
    } else if (d->m_tempunit == KTemperature::Celsius && unit == KTemperature::Kelvin) {
        return (d->m_number + 273.15);
    } else if (d->m_tempunit == KTemperature::Fahrenheit && unit == KTemperature::Celsius) {
        return ((d->m_number - 32) / 1.8);
    } else if (d->m_tempunit == KTemperature::Fahrenheit && unit == KTemperature::Kelvin) {
        return ((d->m_number + 459.67) * 0.5);
    } else if (d->m_tempunit == KTemperature::Kelvin && unit == KTemperature::Celsius) {
        return (d->m_number - 273.15);
    } else if (d->m_tempunit == KTemperature::Kelvin && unit == KTemperature::Fahrenheit) {
        return (d->m_number * 1.8 - 459.67);
    }
    return 0.0;
}

double KTemperature::round(const double number, const uint digits)
{
    const double poweroften = qPow(10, digits);
    return qRound(number * poweroften) / poweroften;
}
