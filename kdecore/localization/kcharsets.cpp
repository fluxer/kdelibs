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

static const char* kSystemEncoding = "System";

class KCharsetsPrivate
{
public:
    KCharsetsPrivate()
    {
        codecForNameDict.reserve( 43 );
        kOtherGroup = i18nc( "@item", "Other");
    }

    // Hash for the encoding names (sensitive case)
    QHash<QByteArray,QTextCodec*> codecForNameDict;

    //Cache list so QStrings can be implicitly shared
    QList<QStringList> encodingsByScript;

    QString encodingGroup(const QByteArray &encoding) const;

    QString kOtherGroup;
};


QString KCharsetsPrivate::encodingGroup(const QByteArray &encoding) const
{
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
    return kOtherGroup;
}


// --------------------------------------------------------------------------

KCharsets::KCharsets()
    :d(new KCharsetsPrivate())
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

    if(!e) {
        //kDebug() << "unknown entity " << str <<", len = " << str.length();
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
    while(len > 0) {
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
        available.append( QString::fromLatin1( encoding.constData(), encoding.size() ) );
    }
    available.sort();
    return available;
}


QString KCharsets::descriptionForEncoding( const QString& encoding ) const
{
    const QString group = d->encodingGroup(encoding.toUtf8().trimmed());
    return i18nc( "@item %1 character set, %2 encoding", "%1 ( %2 )", group, encoding.trimmed() );
}

QString KCharsets::encodingForName( const QString &descriptiveName ) const
{
    QString name = descriptiveName.trimmed();
    name.remove( QLatin1Char('&') ); // QAction shortcut, how does that even happen?

    const int left = name.lastIndexOf( QLatin1Char('(') );

    // Parenthesis, remove text before it
    if (left >= 0 ) {
        name = name.mid(left+1);
    }

    const int right = name.lastIndexOf( QLatin1Char(')') );
    if (right >= 0) {
        name = name.left(right);
    }

    return name;
}

QStringList KCharsets::descriptiveEncodingNames() const
{
    QStringList encodings;

    QMap<QString, QStringList> encodingGroups;
    foreach (const QByteArray &encoding, QTextCodec::availableCodecs()) {
        const QString group = d->encodingGroup(encoding);
        encodingGroups[group].append( QString::fromLatin1(encoding.constData(), encoding.size()) );
    }

    // remove groups with only one entry and move their entry to Other group
    QMapIterator<QString, QStringList> iter(encodingGroups);
    while (iter.hasNext()) {
        iter.next();
        const QStringList value(iter.value());
        if (value.size() == 1) {
            QString group(iter.key());
            encodingGroups[d->kOtherGroup].append(value.at(0));
            encodingGroups.remove(group);
        }
    }

    QMapIterator<QString, QStringList> iter2(encodingGroups);
    while (iter2.hasNext()) {
        iter2.next();
        foreach (const QString &encoding, iter2.value()) {
            encodings.append(i18nc( "@item %1 character set, %2 encoding", "%1 ( %2 )", iter2.key(), encoding ));
        }
    }

    encodings.sort();

    return encodings;
}

QList<QStringList> KCharsets::encodingsByScript() const
{
    if (!d->encodingsByScript.isEmpty())
        return d->encodingsByScript;

    foreach (const QByteArray &encoding, QTextCodec::availableCodecs()) {
        QString group = d->encodingGroup(encoding);

        int i = 0;
        const QString encodingstring = QString::fromLatin1(encoding.constData(), encoding.size());
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

    // almost the same kung-fu as in descriptiveEncodingNames()
    QList<QStringList>::iterator it = d->encodingsByScript.begin();
    while (it != d->encodingsByScript.end()) {
        const QStringList &list = *it;
        if (list.size() == 2) {
            int i = 0;
            const QString encoding = list.at(1);
            d->encodingsByScript.removeAll(list);

            for (i = 0; i < d->encodingsByScript.size(); i++) {
                if (d->encodingsByScript.at(i).at(0) == d->kOtherGroup) {
                    d->encodingsByScript[i].append(encoding);
                    break;
                }
            }
            if (i == d->encodingsByScript.size()) {
                d->encodingsByScript.append(QStringList() << d->kOtherGroup << encoding);
            }

            it = d->encodingsByScript.begin();
        } else {
            it++;
        }
    }

    return d->encodingsByScript;
}

QTextCodec* KCharsets::codecForName(const QString &n) const
{
    const QByteArray name( n.toLatin1() );
    QTextCodec* codec = codecForNameOrNull( name );
    if ( codec ) {
        return codec;
    }
    return QTextCodec::codecForName( "iso-8859-1" );
}

QTextCodec* KCharsets::codecForName(const QString &n, bool &ok) const
{
    const QByteArray name( n.toLatin1() );
    QTextCodec* codec = codecForNameOrNull( name );
    if ( codec ) {
        ok = true;
        return codec;
    } else {
        ok = false;
        return QTextCodec::codecForName( "iso-8859-1" );
    }
}

QTextCodec *KCharsets::codecForNameOrNull( const QByteArray& n ) const
{
    const QByteArray dn = encodingForName( QString::fromLatin1( n.constData(), n.size()) ).toLatin1();

    if (dn.isEmpty() || qstrnicmp(dn.constData(), kSystemEncoding, dn.size()) == 0) {
        // No name, assume locale (KDE's, not Qt's)
        if ( d->codecForNameDict.contains( kSystemEncoding ) )
            return d->codecForNameDict.value( kSystemEncoding );
        QTextCodec* codec = QTextCodec::codecForLocale();
        d->codecForNameDict.insert(kSystemEncoding, codec);
        return codec;
    }

    // For a non-empty name, lookup the "dictionnary", in a case-sensitive way.
    if ( d->codecForNameDict.contains( n ) ) {
        return d->codecForNameDict.value( n );
    }

    // If the name is not in the hash table, call directly QTextCoded::codecForName.
    // We assume that QTextCodec is smarter and more maintained than this code.
    QTextCodec* codec = QTextCodec::codecForName( dn );
    if ( codec ) {
        d->codecForNameDict.insert( dn, codec );
        return codec;
    }

    // we could not assign a codec, therefore return NULL
    return 0;
}
