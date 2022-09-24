/*
    Copyright (c) 2009 John Layt <john@layt.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KCURRENCYCODE_H
#define KCURRENCYCODE_H

#include <kdecore_export.h>

#include <QSharedDataPointer>
#include <QStringList>

class KCurrencyCodePrivate;

/**
 * @since 4.4
 *
 * This is a class to implement the ISO 4217 Currency Code standard
 *
 * @b license GNU-LGPL v.2 or later
 *
 * @see KLocale
 *
 * @author John Layt <john@layt.net>
 */
class KDECORE_EXPORT KCurrencyCode
{
public:
    /**
     * Constructs a KCurrencyCode for a given ISO Currency Code.
     *
     * If the supplied Currency Code is not known then the KCurrencyCode will return isValid() == false
     *
     * @param isoCurrencyCode the ISO Currency Code to construct, defaults to USD
     * @param language the language to use for translations, default to the Locale language
     *
     */
    explicit KCurrencyCode( const QString &isoCurrencyCode, const QString &language = QString() );

    /**
     * Copy Constructor
     *
     * @param rhs KCurrencyCode to copy
     *
     */
    KCurrencyCode( const KCurrencyCode &rhs );

    /**
     * Destructor.
     */
    ~KCurrencyCode();

    /**
     * Assignment operator
     *
     * @param rhs KCurrencyCode to assign
     *
     */
    KCurrencyCode& operator=( const KCurrencyCode &rhs );

    /**
     * Return the ISO 4217 Currency Code in Alpha 3 format, e.g. USD
     *
     * @return the ISO Currency Code
     *
     * @see isoCurrencyCodeNumeric()
     */
    QString isoCurrencyCode() const;

    /**
     * Return the ISO 4217 Currency Code in Numeric 3 format, e.g. 840
     *
     * @return the ISO Currency Code
     *
     * @see isoCurrencyCode()
     */
    QString isoCurrencyCodeNumeric() const;

    /**
     * Return translated Currency Code Name in a standard display format
     * e.g. United States Dollar
     *
     * @return the display Currency Code Name
     *
     * @see isoName()
     */
    QString name() const;

    /**
     * Return untranslated official ISO Currency Code Name
     *
     * This name is not translated and should only be used where appropriate.
     * For displaying the name to a user, use name() instead.
     *
     * @return the official ISO Currency Code Name
     *
     * @see name()
     */
    QString isoName() const;

    /**
     * Return the default Symbol for the Currency, e.g. $ or £
     *
     * @return the default Currency Symbol
     *
     * @see symbols()
     */
    QString defaultSymbol() const;

    /**
     * Return the number of decimal places required to display the currency subunits
     *
     * @return number of decimal places
     */
    int decimalPlaces() const;

    /**
     * Return if the currency object loaded/initialised correctly
     *
     * @return true if valid KCurrencyCode object
     */
    bool isValid() const;

    /**
     * Return if a given Currency Code is supported in KDE.
     *
     * @param currencyCode the Currency Code to validate
     *
     * @return true if valid currency code
     */
    static bool isValid( const QString &currencyCode );

    /**
     * Provides list of all known ISO Currency Codes.
     *
     * Use currencyCodeToName(currencyCode) to get human readable, localized currency names.
     *
     * @return a list of all active ISO Currency Codes
     *
     * @see currencyCodeToName
     */
    static QStringList allCurrencyCodesList( );

    /**
     * Convert a known ISO Currency Code to a human readable, localized form.
     *
     * If an unknown Currency Code is supplied, empty string is returned;
     * this will never happen if the code has been obtained by one of the
     * KCurrencyCode methods.
     *
     * @param currencyCode the ISO Currency Code
     * @param language the language to use for translations, default to the Locale language
     *
     * @return the human readable and localized form of the Currency name
     *
     * @see currencyCode
     * @see allCurrencyCodesList
     */
    static QString currencyCodeToName( const QString &currencyCode, const QString &language = QString() );


private:
    QSharedDataPointer<KCurrencyCodePrivate> d;
};

#endif // KCURRENCYCODE_H
