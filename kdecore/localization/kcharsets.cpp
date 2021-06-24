/* This file is part of the KDE libraries
    Copyright (C) 1999 Lars Knoll (knoll@kde.org)
    Copyright (C) 2001, 2003, 2004, 2005, 2006 Nicolas GOUTTE <goutte@kde.org>
    Copyright (C) 2007 Nick Shaforostoff <shafff@ukr.net>

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
#include "kcharsets.h"

#include "kfilterdev.h"
#include "kentities.cpp"

#include "kconfig.h"
#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"

#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/QTextCodec>

#include <assert.h>
#include <QHash>

static const QLatin1String kOtherEncoding = QLatin1String("Other");

static QString encodingGroup(const QByteArray &encoding) {
    int separatorindex = 0;
    const char *data = encoding.constData();
    for (int i = 0; i < encoding.size(); i++) {
        if (data[i] == ' ' || data[i] == '-' || data[i] == '_') {
            separatorindex = i;
            break;
        }
    }
    if (separatorindex > 1) {
        return QString::fromLatin1(encoding.mid(0, separatorindex));
    }
    return kOtherEncoding;
}

class KCharsetsPrivate
{
public:
    KCharsetsPrivate(KCharsets* _kc)
    {
        kc = _kc;
        codecForNameDict.reserve( 43 );
    }
    // Hash for the encoding names (sensitive case)
    QHash<QByteArray,QTextCodec*> codecForNameDict;
    KCharsets* kc;

    //Cache list so QStrings can be implicitly shared
    QList<QStringList> encodingsByScript;
};

// --------------------------------------------------------------------------

KCharsets::KCharsets()
    :d(new KCharsetsPrivate(this))
{
}

KCharsets::~KCharsets()
{
    delete d;
}

QChar KCharsets::fromEntity(const QString &str)
{
    QChar res;

    if ( str.isEmpty() )
        return res;

    int pos = 0;
    if(str[pos] == QLatin1Char('&')) pos++;

    // Check for '&#000' or '&#x0000' sequence
    if (str[pos] == QLatin1Char('#') && str.length()-pos > 1) {
        bool ok;
        pos++;
        if (str[pos] == QLatin1Char('x') || str[pos] == QLatin1Char('X')) {
            pos++;
            // '&#x0000', hexadecimal character reference
            const QString tmp( str.mid( pos ) );
            res = tmp.toInt(&ok, 16);
        } else {
            //  '&#0000', decimal character reference
            const QString tmp( str.mid( pos ) );
            res = tmp.toInt(&ok, 10);
        }
        if ( ok )
            return res;
        else
            return QChar();
    }

    const QByteArray raw ( str.toLatin1() );
    const entity *e = EntitiesHash::kde_findEntity( raw, raw.length() );

    if(!e)
    {
        //kDebug( 0 ) << "unknown entity " << str <<", len = " << str.length();
        return QChar();
    }
    //kDebug() << "got entity " << str << " = " << e->code;

    return QChar(e->code);
}

QChar KCharsets::fromEntity(const QString &str, int &len)
{
    // entities are never longer than 8 chars... we start from
    // that length and work backwards...
    len = 8;
    while(len > 0)
    {
        QString tmp = str.left(len);
        QChar res = fromEntity(tmp);
        if( !res.isNull() ) return res;
        len--;
    }
    return QChar();
}


QString KCharsets::toEntity(const QChar &ch)
{
    QString ent;
    ent.sprintf("&#0x%x;", ch.unicode());
    return ent;
}

QString KCharsets::resolveEntities( const QString &input )
{
    QString text = input;
    const QChar *p = text.unicode();
    const QChar *end = p + text.length();
    const QChar *ampersand = 0;
    bool scanForSemicolon = false;

    for ( ; p < end; ++p ) {
        const QChar ch = *p;

        if ( ch == QLatin1Char('&') ) {
            ampersand = p;
            scanForSemicolon = true;
            continue;
        }

        if ( ch != QLatin1Char(';') || scanForSemicolon == false )
            continue;

        assert( ampersand );

        scanForSemicolon = false;

        const QChar *entityBegin = ampersand + 1;

        const uint entityLength = p - entityBegin;
        if ( entityLength == 0 )
            continue;

        const QChar entityValue = KCharsets::fromEntity( QString( entityBegin, entityLength ) );
        if ( entityValue.isNull() )
            continue;

        const uint ampersandPos = ampersand - text.unicode();

        text[ (int)ampersandPos ] = entityValue;
        text.remove( ampersandPos + 1, entityLength + 1 );
        p = text.unicode() + ampersandPos;
        end = text.unicode() + text.length();
        ampersand = 0;
    }

    return text;
}

QStringList KCharsets::availableEncodingNames() const
{
    QStringList available;
    foreach (const QByteArray &encoding, QTextCodec::availableCodecs()) {
        available.append( QString::fromLatin1( encoding ) );
    }
    available.sort();
    return available;
}


QString KCharsets::descriptionForEncoding( const QString& encoding ) const
{
    QString group = encodingGroup(encoding.toUtf8());

    if ( group != kOtherEncoding )
        return i18nc( "@item %1 character set, %2 encoding",
            "%1 ( %2 )", group, encoding );
    return i18nc( "@item", "Other encoding (%1)", encoding );
}

QString KCharsets::encodingForName( const QString &descriptiveName ) const
{
    const int left = descriptiveName.lastIndexOf( QLatin1Char('(') );

    if (left<0) // No parenthesis, so assume it is a normal encoding name
        return descriptiveName.trimmed();

    QString name(descriptiveName.mid(left+1));

    const int right = name.lastIndexOf( QLatin1Char(')') );

    if (right<0)
        return name;

    return name.left(right).trimmed();
}

QStringList KCharsets::descriptiveEncodingNames() const
{
    QStringList encodings;
    foreach (const QByteArray &encoding, QTextCodec::availableCodecs()) {
        QString group = encodingGroup(encoding);

        encodings.append( i18nc( "@item Text encoding: %1 character set, %2 encoding",
            "%1 ( %2 )", group, QString::fromLatin1(encoding.constData(), encoding.size()) ) );
    }
    encodings.sort();
    return encodings;
}

QList<QStringList> KCharsets::encodingsByScript() const
{
    if (!d->encodingsByScript.isEmpty())
        return d->encodingsByScript;

    foreach (const QByteArray &encoding, QTextCodec::availableCodecs()) {
        QString group = encodingGroup(encoding);

        int i = 0;
        const QString encodingstring = QString::fromLatin1(encoding);
        for (i = 0; i < d->encodingsByScript.size(); i++) {
            if (d->encodingsByScript.at(i).at(0).toLower() == group.toLower()) {
                d->encodingsByScript[i].append(encodingstring);
                break;
            }
        }
        if (i == d->encodingsByScript.size()) {
            d->encodingsByScript.append(QStringList() << group << encodingstring);
        }
    }

    // remove groups with only one entry and move their entry to Other group
    foreach (const QStringList &list, d->encodingsByScript) {
        if (list.size() == 2) {
            int i = 0;
            const QString encoding = list.at(1);
            d->encodingsByScript.removeAll(list);

            for (i = 0; i < d->encodingsByScript.size(); i++) {
                if (d->encodingsByScript.at(i).at(0) == kOtherEncoding) {
                    d->encodingsByScript[i].append(encoding);
                    break;
                }
            }
            if (i == d->encodingsByScript.size()) {
                d->encodingsByScript.append(QStringList() << kOtherEncoding << encoding);
            }
        }
    }

    return d->encodingsByScript;
}

QTextCodec* KCharsets::codecForName(const QString &n) const
{
    if ( n == QLatin1String("gb2312") || n == QLatin1String("gbk") )
        return QTextCodec::codecForName( "gb18030" );
    const QByteArray name( n.toLatin1() );
    QTextCodec* codec = codecForNameOrNull( name );
    if ( codec )
        return codec;
    else
        return QTextCodec::codecForName( "iso-8859-1" );
}

QTextCodec* KCharsets::codecForName(const QString &n, bool &ok) const
{
    if (n == QLatin1String("gb2312") || n == QLatin1String("gbk")) {
        ok = true;
        return QTextCodec::codecForName( "gb18030" );
    }
    const QByteArray name( n.toLatin1() );
    QTextCodec* codec = codecForNameOrNull( name );
    if ( codec )
    {
        ok = true;
        return codec;
    }
    else
    {
        ok = false;
        return QTextCodec::codecForName( "iso-8859-1" );
    }
}

QTextCodec *KCharsets::codecForNameOrNull( const QByteArray& n ) const
{
    QTextCodec* codec = 0;

    if (n.isEmpty()) {
        // No name, assume locale (KDE's, not Qt's)
        const QByteArray locale = "->locale<-";
        if ( d->codecForNameDict.contains( locale ) )
            return d->codecForNameDict.value( locale );
        codec = KGlobal::locale()->codecForEncoding();
        d->codecForNameDict.insert("->locale<-", codec);
        return codec;
    }
    // For a non-empty name, lookup the "dictionnary", in a case-sensitive way.
    else if ( d->codecForNameDict.contains( n ) ) {
        return d->codecForNameDict.value( n );
    }

    // If the name is not in the hash table, call directly QTextCoded::codecForName.
    // We assume that QTextCodec is smarter and more maintained than this code.
    codec = QTextCodec::codecForName( n );
    if ( codec ) {
        d->codecForNameDict.insert( n, codec );
        return codec;
    }

    // We have had no luck with QTextCodec::codecForName, so we must now process the name, so that QTextCodec::codecForName could work with it.

    QByteArray name = n.toLower();
    bool changed = false;
    if (name.endsWith("_charset")) { // krazy:exclude=strings
       name.chop( 8 );
       changed = true;
    }
    if ( name.startsWith( "x-" ) ) { // krazy:exclude=strings
       name.remove( 0, 2 ); // remove x- at start
       changed = true;
    }

    if (name.isEmpty()) {
      // We have no name anymore, therefore the name is invalid.
      return 0;
    }

    // We only need to check changed names.
    if ( changed ) {
        codec = QTextCodec::codecForName(name);
        if (codec) {
            d->codecForNameDict.insert( n, codec );
            return codec;
        }
    }

    // we could not assign a codec, therefore return NULL
    return 0;
}
