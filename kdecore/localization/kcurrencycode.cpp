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

#include "kcurrencycode.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kglobal.h"
#include "kstandarddirs.h"
#include "kdebug.h"

class KCurrencyCodePrivate : public QSharedData
{
public:
    KCurrencyCodePrivate( const QString &isoCurrencyCode, const QString &language );
    KCurrencyCodePrivate( const KCurrencyCodePrivate& other );

    QString     m_currencyCodeIsoAlpha3;
    QString     m_currencyCodeIsoNumeric3;
    QString     m_currencyNameIso;
    QString     m_currencyNameDisplay;
    QString     m_currencyUnitSymbolDefault;
    int         m_currencyDecimalPlacesDisplay;
};

KCurrencyCodePrivate::KCurrencyCodePrivate( const QString &isoCurrencyCode, const QString &language )
{
    QString file = KStandardDirs::locate( "locale", QString::fromLatin1( "currency/%1.desktop" ).arg( isoCurrencyCode.toLower() ) );

    KConfig cgFile( file );

    // If language is empty, means to stick with the global default, which is the default for any new KConfig
    if ( !language.isEmpty() ) {
        cgFile.setLocale( language );
    }

    KConfigGroup cg( &cgFile, "Currency Code" );

    m_currencyCodeIsoAlpha3         = cg.readEntry( "CurrencyCodeIsoAlpha3",         QString() );
    m_currencyCodeIsoNumeric3       = cg.readEntry( "CurrencyCodeIsoNumeric3",       QString() );
    m_currencyNameIso               = cg.readEntry( "CurrencyNameIso",               QString() );
    m_currencyNameDisplay           = cg.readEntry( "Name",                          QString() );
    m_currencyUnitSymbolDefault     = cg.readEntry( "CurrencyUnitSymbolDefault",     QString() );
    m_currencyDecimalPlacesDisplay  = cg.readEntry( "CurrencyDecimalPlacesDisplay",  2 );
}

KCurrencyCodePrivate::KCurrencyCodePrivate( const KCurrencyCodePrivate& other )
    : QSharedData( other ),
      m_currencyCodeIsoAlpha3( other.m_currencyCodeIsoAlpha3 ),
      m_currencyCodeIsoNumeric3( other.m_currencyCodeIsoNumeric3 ),
      m_currencyNameIso( other.m_currencyNameIso ),
      m_currencyNameDisplay( other.m_currencyNameDisplay ),
      m_currencyUnitSymbolDefault( other.m_currencyUnitSymbolDefault ),
      m_currencyDecimalPlacesDisplay( other.m_currencyDecimalPlacesDisplay )
{
}

KCurrencyCode::KCurrencyCode( const QString &isoCurrencyCode, const QString &language )
    :d( new KCurrencyCodePrivate( isoCurrencyCode, language ) )
{
}

KCurrencyCode::KCurrencyCode( const KCurrencyCode &rhs )
    :d( rhs.d )
{
}

KCurrencyCode& KCurrencyCode::operator=( const KCurrencyCode &rhs )
{
    if (&rhs != this) {
        d = rhs.d;
    }
    return *this;
}

KCurrencyCode::~KCurrencyCode()
{
}

QString KCurrencyCode::isoCurrencyCode() const
{
    return d->m_currencyCodeIsoAlpha3;
}

QString KCurrencyCode::isoCurrencyCodeNumeric() const
{
    return d->m_currencyCodeIsoNumeric3;
}

QString KCurrencyCode::name() const
{
    return d->m_currencyNameDisplay;
}

QString KCurrencyCode::isoName() const
{
    return d->m_currencyNameIso;
}

QString KCurrencyCode::defaultSymbol() const
{
    return d->m_currencyUnitSymbolDefault;
}

int KCurrencyCode::decimalPlaces() const
{
    return d->m_currencyDecimalPlacesDisplay;
}

bool KCurrencyCode::isValid() const
{
    return !d->m_currencyCodeIsoAlpha3.isEmpty();
}

bool KCurrencyCode::isValid( const QString &isoCurrencyCode )
{
    KCurrencyCode test = KCurrencyCode( isoCurrencyCode );
    return test.isValid();
}

QStringList KCurrencyCode::allCurrencyCodesList( )
{
    QStringList currencyCodes;
    const QStringList paths = KGlobal::dirs()->findAllResources( "locale", QLatin1String("currency/*.desktop") );
    foreach( const QString &path, paths ) {
        QString code = path.mid( path.length()-11, 3 ).toUpper();
        if ( KCurrencyCode::isValid( code ) ) {
            currencyCodes.append( code );
        }
    }
    return currencyCodes;
}

QString KCurrencyCode::currencyCodeToName( const QString &isoCurrencyCode, const QString &language )
{
    KCurrencyCode temp = KCurrencyCode( isoCurrencyCode, language );
    if ( temp.isValid() ) {
        return temp.name();
    }
    return QString();
}
