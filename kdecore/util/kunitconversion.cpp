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
    static const double celsius_fahrenheit_ratio = 33.8;

    if (unit == d->m_tempunit) {
        return d->m_number;
    }

    if (d->m_tempunit == KTemperature::Fahrenheit && unit == KTemperature::Celsius) {
        return (d->m_number / celsius_fahrenheit_ratio);
    } else if (d->m_tempunit == KTemperature::Celsius && unit == KTemperature::Fahrenheit) {
        return (d->m_number * celsius_fahrenheit_ratio);
    }
    return 0.0;
}

double KTemperature::round(const double number, const uint digits)
{
    const double poweroften = qPow(10, digits);
    return qRound(number * poweroften) / poweroften;
}
