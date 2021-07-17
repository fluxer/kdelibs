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
#include "klocale.h"

#include <qmath.h>

double KUnitConversion::round(const double number, const uint digits)
{
    const double poweroften = qPow(10, digits);
    return qRound(number * poweroften) / poweroften;
}

class KTemperaturePrivate {
public:
    KTemperaturePrivate(const double number, const KTemperature::KTempUnit unit);
    KTemperaturePrivate(const double number, const QString &unit);

    const double m_number;
    KTemperature::KTempUnit m_unitenum;
};

KTemperaturePrivate::KTemperaturePrivate(const double number, const KTemperature::KTempUnit unit)
    : m_number(number),
    m_unitenum(unit)
{
}

KTemperaturePrivate::KTemperaturePrivate(const double number, const QString &unit)
    : m_number(number),
    m_unitenum(KTemperature::Invalid)
{
    // any unit other than the string representation of the enum is for KUnitConversion compatibility
    if (unit == QLatin1String("Celsius")
        || unit == QString::fromUtf8("°C") || unit == QLatin1String("C")) {
        m_unitenum = KTemperature::Celsius;
    } else if (unit == QLatin1String("Fahrenheit")
        || unit == QString::fromUtf8("°F") || unit == QLatin1String("F")) {
        m_unitenum = KTemperature::Fahrenheit;
    } else if (unit == QLatin1String("Kelvin")
        || unit == QLatin1String("K") || unit == QLatin1String("kelvin")
        || unit == QLatin1String("kelvins")) {
        m_unitenum = KTemperature::Kelvin;
    } else {
        kDebug() << "invalid temperature unit" << unit;
    }
}

KTemperature::KTemperature(const double number, const KTempUnit unit)
    : d(new KTemperaturePrivate(number, unit))
{
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
    switch (d->m_unitenum) {
        case KTemperature::Celsius:
            return QString::fromUtf8("°C");
        case KTemperature::Fahrenheit:
            return QString::fromUtf8("°F");
        case KTemperature::Kelvin:
            return QString::fromUtf8("K");
        case KTemperature::Invalid:
        case KTemperature::UnitCount:
            break;
    }
    return QLatin1String("Unknown");
}

KTemperature::KTempUnit KTemperature::unitEnum() const
{
    return d->m_unitenum;
}

QString KTemperature::toString() const
{
    return QString::fromLatin1("%1 %2").arg(QString::number(d->m_number), KTemperature::unit());
}

double KTemperature::convertTo(const KTempUnit unit) const
{
    if (unit == d->m_unitenum) {
        return d->m_number;
    }

    // for reference:
    // https://www.rapidtables.com/convert/temperature/celsius-to-fahrenheit.html
    // https://www.rapidtables.com/convert/temperature/celsius-to-kelvin.html
    // https://www.rapidtables.com/convert/temperature/fahrenheit-to-celsius.html
    // https://www.rapidtables.com/convert/temperature/fahrenheit-to-kelvin.html
    // https://www.rapidtables.com/convert/temperature/kelvin-to-celsius.html
    // https://www.rapidtables.com/convert/temperature/kelvin-to-fahrenheit.html
    if (d->m_unitenum == KTemperature::Celsius && unit == KTemperature::Fahrenheit) {
        return (d->m_number * 1.8 + 32);
    } else if (d->m_unitenum == KTemperature::Celsius && unit == KTemperature::Kelvin) {
        return (d->m_number + 273.15);
    } else if (d->m_unitenum == KTemperature::Fahrenheit && unit == KTemperature::Celsius) {
        return ((d->m_number - 32) / 1.8);
    } else if (d->m_unitenum == KTemperature::Fahrenheit && unit == KTemperature::Kelvin) {
        return ((d->m_number + 459.67) * 0.5);
    } else if (d->m_unitenum == KTemperature::Kelvin && unit == KTemperature::Celsius) {
        return (d->m_number - 273.15);
    } else if (d->m_unitenum == KTemperature::Kelvin && unit == KTemperature::Fahrenheit) {
        return (d->m_number * 1.8 - 459.67);
    }
    return 0.0;
}

QString KTemperature::description()
{
    return i18n("Temperature");
}

QString KTemperature::unitDescription(const KTempUnit unit)
{
    switch (unit) {
        case KTemperature::Celsius:
            return i18n("Celsius (°C)");
        case KTemperature::Fahrenheit:
            return i18n("Fahrenheit (°F)");
        case KTemperature::Kelvin:
            return i18n("Kelvin (K)");
        case KTemperature::Invalid:
        case KTemperature::UnitCount:
            break;
    }
    return i18n("Unknown");
}


class KVelocityPrivate {
public:
    KVelocityPrivate(const double number, const KVelocity::KVeloUnit unit);
    KVelocityPrivate(const double number, const QString &unit);

    const double m_number;
    KVelocity::KVeloUnit m_unitenum;
};

KVelocityPrivate::KVelocityPrivate(const double number, const KVelocity::KVeloUnit unit)
    : m_number(number),
    m_unitenum(unit)
{
}

KVelocityPrivate::KVelocityPrivate(const double number, const QString &unit)
    : m_number(number),
    m_unitenum(KVelocity::Invalid)
{
    if (unit == QLatin1String("MeterPerSecond")
        || unit == QLatin1String("meter per second") || unit == QLatin1String("meters per second")
        || unit == QLatin1String("m/s") || unit == QLatin1String("ms")) {
        m_unitenum = KVelocity::MeterPerSecond;
    } else if (unit == QLatin1String("KilometerPerHour")
        || unit == QLatin1String("kilometer per hour") || unit == QLatin1String("kilometers per hour")
        || unit == QLatin1String("km/h") || unit == QLatin1String("kmh")) {
        m_unitenum = KVelocity::KilometerPerHour;
    } else if (unit == QLatin1String("MilePerHour")
        || unit == QLatin1String("mile per hour") || unit == QLatin1String("miles per hour")
        || unit == QLatin1String("mph")) {
        m_unitenum = KVelocity::MilePerHour;
    } else if (unit == QLatin1String("Knot")
        || unit == QLatin1String("knot") || unit == QLatin1String("knots")
        || unit == QLatin1String("kt") || unit == QLatin1String("nautical miles per hour")) {
        m_unitenum = KVelocity::Knot;
    } else {
        kDebug() << "invalid velocity unit" << unit;
    }
}

KVelocity::KVelocity(const double number, const KVeloUnit unit)
    : d(new KVelocityPrivate(number, unit))
{
}

KVelocity::KVelocity(const double number, const QString &unit)
    : d(new KVelocityPrivate(number, unit))
{
}

KVelocity::~KVelocity()
{
    delete d;
}

double KVelocity::number() const
{
    return d->m_number;
}

QString KVelocity::unit() const
{
    switch (d->m_unitenum) {
        case KVelocity::MeterPerSecond:
            return QLatin1String("m/s");
        case KVelocity::KilometerPerHour:
            return QLatin1String("km/h");
        case KVelocity::MilePerHour:
            return QLatin1String("mph");
        case KVelocity::Knot:
            return QLatin1String("kt");
        case KVelocity::Invalid:
        case KVelocity::UnitCount:
            break;
    }
    return QLatin1String("Unknown");
}

KVelocity::KVeloUnit KVelocity::unitEnum() const
{
    return d->m_unitenum;
}

QString KVelocity::toString() const
{
    return QString::fromLatin1("%1 %2").arg(QString::number(d->m_number), KVelocity::unit());
}

double KVelocity::convertTo(const KVeloUnit unit) const
{
    if (unit == d->m_unitenum) {
        return d->m_number;
    }

    // for reference:
    // https://www.metric-conversions.org/speed/meters-per-second-to-kilometers-per-hour.htm
    // https://www.metric-conversions.org/speed/meters-per-second-to-miles-per-hour.htm
    // https://www.metric-conversions.org/speed/meters-per-second-to-knots.htm
    // https://www.metric-conversions.org/speed/kilometers-per-hour-to-meters-per-second.htm
    // https://www.metric-conversions.org/speed/kilometers-per-hour-to-miles-per-hour.htm
    // https://www.metric-conversions.org/speed/kilometers-per-hour-to-knots.htm
    // https://www.metric-conversions.org/speed/miles-per-hour-to-meters-per-second.htm
    // https://www.metric-conversions.org/speed/miles-per-hour-to-kilometers-per-hour.htm
    // https://www.metric-conversions.org/speed/miles-per-hour-to-knots.htm
    // https://www.metric-conversions.org/speed/knots-to-meters-per-second.htm
    // https://www.metric-conversions.org/speed/knots-to-kilometers-per-hour.htm
    // https://www.metric-conversions.org/speed/knots-to-miles-per-hour.htm
    if (d->m_unitenum == KVelocity::MeterPerSecond && unit == KVelocity::KilometerPerHour) {
        return (d->m_number * 3.6);
    } else if (d->m_unitenum == KVelocity::MeterPerSecond && unit == KVelocity::MilePerHour) {
        return (d->m_number * 2.236936);
    } else if (d->m_unitenum == KVelocity::MeterPerSecond && unit == KVelocity::Knot) {
        return (d->m_number * 1.943844);
    } else if (d->m_unitenum == KVelocity::KilometerPerHour && unit == KVelocity::MeterPerSecond) {
        return (d->m_number / 3.6);
    } else if (d->m_unitenum == KVelocity::KilometerPerHour && unit == KVelocity::MilePerHour) {
        return (d->m_number / 1.609344);
    } else if (d->m_unitenum == KVelocity::KilometerPerHour && unit == KVelocity::Knot) {
        return d->m_number / 1.852;
    } else if (d->m_unitenum == KVelocity::MilePerHour && unit == KVelocity::MeterPerSecond) {
        return (d->m_number / 2.236936);
    } else if (d->m_unitenum == KVelocity::MilePerHour && unit == KVelocity::KilometerPerHour) {
        return (d->m_number * 1.609344);
    } else if (d->m_unitenum == KVelocity::MilePerHour && unit == KVelocity::Knot) {
        return (d->m_number / 1.150779);
    } else if (d->m_unitenum == KVelocity::Knot && unit == KVelocity::MeterPerSecond) {
        return (d->m_number / 1.943844);
    } else if (d->m_unitenum == KVelocity::Knot && unit == KVelocity::KilometerPerHour) {
        return (d->m_number * 1.852);
    } else if (d->m_unitenum == KVelocity::Knot && unit == KVelocity::MilePerHour) {
        return (d->m_number * 1.150779);
    }
    return 0.0;
}

QString KVelocity::description()
{
    return i18n("Velocity");
}

QString KVelocity::unitDescription(const KVeloUnit unit)
{
    switch (unit) {
        case KVelocity::MeterPerSecond:
            return i18n("Meter per second (m/s)");
        case KVelocity::KilometerPerHour:
            return i18n("Kilometer per hour (km/h)");
        case KVelocity::MilePerHour:
            return i18n("Mile per hour (mph)");
        case KVelocity::Knot:
            return i18n("Knot (kt)");
        case KVelocity::Invalid:
        case KVelocity::UnitCount:
            break;
    }
    return i18n("Unknown");
}


class KPressurePrivate {
public:
    KPressurePrivate(const double number, const KPressure::KPresUnit unit);
    KPressurePrivate(const double number, const QString &unit);

    const double m_number;
    KPressure::KPresUnit m_unitenum;
};

KPressurePrivate::KPressurePrivate(const double number, const KPressure::KPresUnit unit)
    : m_number(number),
    m_unitenum(unit)
{
}

KPressurePrivate::KPressurePrivate(const double number, const QString &unit)
    : m_number(number),
    m_unitenum(KPressure::Invalid)
{
    if (unit == QLatin1String("Kilopascal")
        || unit == QLatin1String("kilopascal") || unit == QLatin1String("kilopascals")
        || unit == QLatin1String("kPa")) {
        m_unitenum = KPressure::Kilopascal;
    } else if (unit == QLatin1String("Hectopascal")
        || unit == QLatin1String("hectopascal") || unit == QLatin1String("hectopascals")
        || unit == QLatin1String("hPa")) {
        m_unitenum = KPressure::Hectopascal;
    } else if (unit == QLatin1String("Millibar")
        || unit == QLatin1String("millibar") || unit == QLatin1String("millibars")
        || unit == QLatin1String("mbar") || unit == QLatin1String("mb")) {
        m_unitenum = KPressure::Millibar;
    } else if (unit == QLatin1String("InchesOfMercury")
        || unit == QLatin1String("inch of mercury") || unit == QLatin1String("inches of mercury")
        || unit == QLatin1String("inHg")) {
        m_unitenum = KPressure::InchesOfMercury;
    } else {
        kDebug() << "invalid pressure unit" << unit;
    }
}

KPressure::KPressure(const double number, const KPresUnit unit)
    : d(new KPressurePrivate(number, unit))
{
}

KPressure::KPressure(const double number, const QString &unit)
    : d(new KPressurePrivate(number, unit))
{
}

KPressure::~KPressure()
{
    delete d;
}

double KPressure::number() const
{
    return d->m_number;
}

QString KPressure::unit() const
{
    switch (d->m_unitenum) {
        case KPressure::Kilopascal:
            return QLatin1String("kPa");
        case KPressure::Hectopascal:
            return QLatin1String("hPa");
        case KPressure::Millibar:
            return QLatin1String("mbar");
        case KPressure::InchesOfMercury:
            return QLatin1String("inHg");
        case KPressure::Invalid:
        case KPressure::UnitCount:
            break;
    }
    return QLatin1String("Unknown");
}

KPressure::KPresUnit KPressure::unitEnum() const
{
    return d->m_unitenum;
}

QString KPressure::toString() const
{
    return QString::fromLatin1("%1 %2").arg(QString::number(d->m_number), KPressure::unit());
}

double KPressure::convertTo(const KPresUnit unit) const
{
    if (unit == d->m_unitenum) {
        return d->m_number;
    }

    if (d->m_unitenum == KPressure::Kilopascal && unit == KPressure::Hectopascal) {
        return (d->m_number * 10.0);
    } else if (d->m_unitenum == KPressure::Kilopascal && unit == KPressure::Millibar) {
        return (d->m_number * 10.0);
    } else if (d->m_unitenum == KPressure::Kilopascal && unit == KPressure::InchesOfMercury) {
        return (d->m_number * 3.386398);
    } else if (d->m_unitenum == KPressure::Hectopascal && unit == KPressure::Kilopascal) {
        return (d->m_number / 10.0);
    } else if (d->m_unitenum == KPressure::Hectopascal && unit == KPressure::Millibar) {
        return (d->m_number * 1.0);
    } else if (d->m_unitenum == KPressure::Hectopascal && unit == KPressure::InchesOfMercury) {
        return (d->m_number / 3.386398);
    } else if (d->m_unitenum == KPressure::Millibar && unit == KPressure::Kilopascal) {
        return (d->m_number / 0.1);
    } else if (d->m_unitenum == KPressure::Millibar && unit == KPressure::Hectopascal) {
        return (d->m_number * 1.0);
    } else if (d->m_unitenum == KPressure::Millibar && unit == KPressure::InchesOfMercury) {
        return (d->m_number / 33.8639);
    } else if (d->m_unitenum == KPressure::InchesOfMercury && unit == KPressure::Kilopascal) {
        return (d->m_number * 3.386398);
    } else if (d->m_unitenum == KPressure::InchesOfMercury && unit == KPressure::Hectopascal) {
        return (d->m_number * 33.8639);
    } else if (d->m_unitenum == KPressure::InchesOfMercury && unit == KPressure::Millibar) {
        return (d->m_number * 33.8639);
    }

    return d->m_number;
}

QString KPressure::description()
{
    return i18n("Pressure");
}

QString KPressure::unitDescription(const KPresUnit unit)
{
    switch (unit) {
        case KPressure::Kilopascal:
            return i18n("Kilopascal (kPa)");
        case KPressure::Hectopascal:
            return i18n("Hectopascal (hPa)");
        case KPressure::Millibar:
            return i18n("Millibar (mbar)");
        case KPressure::InchesOfMercury:
            return i18n("Inch of mercury (inHg)");
        case KPressure::Invalid:
        case KPressure::UnitCount:
            break;
    }
    return i18n("Unknown");
}


class KLengthPrivate {
public:
    KLengthPrivate(const double number, const KLength::KLengUnit unit);
    KLengthPrivate(const double number, const QString &unit);

    const double m_number;
    KLength::KLengUnit m_unitenum;
};

KLengthPrivate::KLengthPrivate(const double number, const KLength::KLengUnit unit)
    : m_number(number),
    m_unitenum(unit)
{
}

KLengthPrivate::KLengthPrivate(const double number, const QString &unit)
    : m_number(number),
    m_unitenum(KLength::Invalid)
{
    if (unit == QLatin1String("Mile")
        || unit == QLatin1String("mile") || unit == QLatin1String("miles")
        || unit == QLatin1String("mi")) {
        m_unitenum = KLength::Mile;
    } else if (unit == QLatin1String("Kilometer")
        || unit == QLatin1String("kilometer") || unit == QLatin1String("kilometers")
        || unit == QLatin1String("km")) {
        m_unitenum = KLength::Kilometer;
    } else {
        kDebug() << "invalid length unit" << unit;
    }
}

KLength::KLength(const double number, const QString &unit)
    : d(new KLengthPrivate(number, unit))
{
}

KLength::KLength(const double number, const KLengUnit unit)
    : d(new KLengthPrivate(number, unit))
{
}

KLength::~KLength()
{
    delete d;
}

double KLength::number() const
{
    return d->m_number;
}

QString KLength::unit() const
{
    switch (d->m_unitenum) {
        case KLength::Mile:
            return QLatin1String("mi");
        case KLength::Kilometer:
            return QLatin1String("km");
        case KLength::Invalid:
        case KLength::UnitCount:
            break;
    }
    return QLatin1String("Unknown");
}

KLength::KLengUnit KLength::unitEnum() const
{
    return d->m_unitenum;
}

QString KLength::toString() const
{
    return QString::fromLatin1("%1 %2").arg(QString::number(d->m_number), KLength::unit());
}

double KLength::convertTo(const KLengUnit unit) const
{
    if (unit == d->m_unitenum) {
        return d->m_number;
    }

    // for reference:
    // https://www.rapidtables.com/convert/length/mile-to-km.html
    // https://www.rapidtables.com/convert/length/km-to-mile.html
    if (d->m_unitenum == KLength::Mile && unit == KLength::Kilometer) {
        return (d->m_number * 1.609344);
    } else if (d->m_unitenum == KLength::Kilometer && unit == KLength::Mile) {
        return (d->m_number / 1.609344);
    }
    return 0.0;
}

QString KLength::description()
{
    return i18n("Length");
}

QString KLength::unitDescription(const KLengUnit unit)
{
    switch (unit) {
        case KLength::Mile:
            return i18n("Mile (mi)");
        case KLength::Kilometer:
            return i18n("Kilometer (km)");
        case KLength::Invalid:
        case KLength::UnitCount:
            break;
    }
    return i18n("Unknown");
}
