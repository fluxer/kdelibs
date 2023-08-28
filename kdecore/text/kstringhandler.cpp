/* This file is part of the KDE libraries
   Copyright (C) 1999 Ian Zepp (icszepp@islc.net)
   Copyright (C) 2006 by Dominic Battre <dominic@battre.de>
   Copyright (C) 2006 by Martin Pool <mbp@canonical.com>

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

#include "kstringhandler.h"

#include <kglobal.h>

#include <QtCore/QRegExp>            // for the word ranges
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

//
// Capitalization routines
//
QString KStringHandler::capwords(const QString &text)
{
    if (text.isEmpty()) {
        return text;
    }

    const QString strippedText = text.trimmed();
    const QString space = QString(QLatin1Char(' '));
    const QStringList words = capwords(strippedText.split(space));

    QString result = text;
    result.replace(strippedText, words.join(space));
    return result;
}

QStringList KStringHandler::capwords(const QStringList &list)
{
    QStringList tmp = list;
    for (QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it) {
        *it = (*it)[0].toUpper() + (*it).mid(1);
    }
    return tmp;
}


QString KStringHandler::lsqueeze(const QString &str, int maxlen)
{
    if (str.length() > maxlen) {
        const int part = (maxlen - 3);
        return QString::fromLatin1("...") + str.right(part);
    }
    return str;
}

QString KStringHandler::csqueeze(const QString &str, int maxlen)
{
    if (str.length() > maxlen && maxlen > 3) {
        const int part = ((maxlen - 3) / 2);
        return str.left(part) + QLatin1String("...") + str.right(part);
    }
    return str;
}

QString KStringHandler::rsqueeze(const QString &str, int maxlen)
{
    if (str.length() > maxlen) {
        const int part = maxlen - 3;
        return str.left(part) + QLatin1String("...");
    }
    return str;
}

QStringList KStringHandler::perlSplit(const QString & sep, const QString & s, int max)
{
    bool ignoreMax = (max == 0);
    int searchStart = 0;
    int tokenStart = s.indexOf(sep, searchStart);
    QStringList l;

    while (-1 != tokenStart && (ignoreMax || l.count() < max - 1)) {
        if (!s.mid(searchStart, tokenStart - searchStart).isEmpty()) {
            l << s.mid(searchStart, tokenStart - searchStart);
        }

        searchStart = tokenStart + sep.length();
        tokenStart = s.indexOf(sep, searchStart);
    }

    if (!s.mid(searchStart, s.length() - searchStart).isEmpty()) {
        l << s.mid(searchStart, s.length() - searchStart);
    }

    return l;
}

QStringList KStringHandler::perlSplit(const QChar & sep, const QString & s, int max)
{
    bool ignoreMax = (max == 0);
    int searchStart = 0;
    int tokenStart = s.indexOf(sep, searchStart);
    QStringList l;

    while (-1 != tokenStart && (ignoreMax || l.count() < max - 1)) {
        if (!s.mid(searchStart, tokenStart - searchStart).isEmpty()) {
            l << s.mid(searchStart, tokenStart - searchStart);
        }

        searchStart = tokenStart + 1;
        tokenStart = s.indexOf(sep, searchStart);
    }

    if (!s.mid(searchStart, s.length() - searchStart).isEmpty()) {
        l << s.mid(searchStart, s.length() - searchStart);
    }

    return l;
}

QStringList KStringHandler::perlSplit(const QRegExp &sep, const QString &s, int max)
{
    bool ignoreMax = (max == 0);
    int searchStart = 0;
    int tokenStart = sep.indexIn(s, searchStart);
    int len = sep.matchedLength();
    QStringList l;

    while (-1 != tokenStart && (ignoreMax || l.count() < max - 1)) {
        if (!s.mid(searchStart, tokenStart - searchStart).isEmpty()) {
            l << s.mid(searchStart, tokenStart - searchStart);
        }

        searchStart = tokenStart + len;
        tokenStart = sep.indexIn(s, searchStart);
        len = sep.matchedLength();
    }

    if (!s.mid(searchStart, s.length() - searchStart).isEmpty()) {
        l << s.mid(searchStart, s.length() - searchStart);
    }

    return l;
}

QString KStringHandler::tagUrls(const QString &text)
{
    QRegExp urlEx(QLatin1String("(www\\.(?!\\.)|(fish|(f|ht)tp(|s))://)[\\d\\w\\./,:_~\\?=&;#@\\-\\+\\%\\$]+[\\d\\w/]"));

    QString richText(text);
    int urlPos = 0, urlLen;
    while ((urlPos = urlEx.indexIn(richText, urlPos)) >= 0) {
        urlLen = urlEx.matchedLength();
        QString href = richText.mid( urlPos, urlLen );
        // Katie doesn't support (?<=pattern) so do it here
        if ((urlPos > 0) && richText[urlPos-1].isLetterOrNumber()){
            urlPos++;
            continue;
        }
        // Don't use QString::arg since %01, %20, etc could be in the string
        QString anchor = QString::fromLatin1("<a href=\"") + href + QLatin1String("\">") + href + QLatin1String("</a>");
        richText.replace(urlPos, urlLen, anchor);

        urlPos += anchor.length();
    }
    return richText;
}

int KStringHandler::naturalCompare(const QString &_a, const QString &_b, Qt::CaseSensitivity caseSensitivity)
{
    // This method chops the input a and b into pieces of
    // digits and non-digits (a1.05 becomes a | 1 | . | 05)
    // and compares these pieces of a and b to each other
    // (first with first, second with second, ...).
    //
    // This is based on the natural sort order code code by Martin Pool
    // http://sourcefrog.net/projects/natsort/
    // Martin Pool agreed to license this under LGPL or GPL.

    // FIXME: Using toLower() to implement case insensitive comparison is
    // sub-optimal, but is needed because we compare strings with
    // localeAwareCompare(), which does not know about case sensitivity.
    // A task has been filled for this in Qt Task Tracker with ID 205990.
    // http://trolltech.com/developer/task-tracker/index_html?method=entry&id=205990
    QString a;
    QString b;
    if (caseSensitivity == Qt::CaseSensitive) {
        a = _a;
        b = _b;
    } else {
        a = _a.toLower();
        b = _b.toLower();
    }

    const QChar* currA = a.unicode(); // iterator over a
    const QChar* currB = b.unicode(); // iterator over b

    if (currA == currB) {
        return 0;
    }

    while (!currA->isNull() && !currB->isNull()) {
        const QChar* begSeqA = currA; // beginning of a new character sequence of a
        const QChar* begSeqB = currB;
        if (currA->unicode() == QChar::ObjectReplacementCharacter) {
            return 1;
        }

        if (currB->unicode() == QChar::ObjectReplacementCharacter) {
            return -1;
        }

        if (currA->unicode() == QChar::ReplacementCharacter) {
            return 1;
        }

        if (currB->unicode() == QChar::ReplacementCharacter) {
            return -1;
        }

        // find sequence of characters ending at the first non-character
        while (!currA->isNull() && !currA->isDigit() && !currA->isPunct() && !currA->isSpace()) {
            ++currA;
        }

        while (!currB->isNull() && !currB->isDigit() && !currB->isPunct() && !currB->isSpace()) {
            ++currB;
        }

        // compare these sequences
        const QStringRef& subA(a.midRef(begSeqA - a.unicode(), currA - begSeqA));
        const QStringRef& subB(b.midRef(begSeqB - b.unicode(), currB - begSeqB));
        const int cmp = QStringRef::localeAwareCompare(subA, subB);
        if (cmp != 0) {
            return cmp < 0 ? -1 : +1;
        }

        if (currA->isNull() || currB->isNull()) {
            break;
        }

        // find sequence of characters ending at the first non-character
        while ((currA->isPunct() || currA->isSpace()) && (currB->isPunct() || currB->isSpace())) {
            if (*currA != *currB) {
                return (*currA < *currB) ? -1 : +1;
            }
            ++currA;
            ++currB;
            if (currA->isNull() || currB->isNull()) {
                break;
            }
        }

        // now some digits follow...
        if ((*currA == QLatin1Char('0')) || (*currB == QLatin1Char('0'))) {
            // one digit-sequence starts with 0 -> assume we are in a fraction part
            // do left aligned comparison (numbers are considered left aligned)
            while (1) {
                if (!currA->isDigit() && !currB->isDigit()) {
                    break;
                } else if (!currA->isDigit()) {
                    return +1;
                } else if (!currB->isDigit()) {
                    return -1;
                } else if (*currA < *currB) {
                    return -1;
                } else if (*currA > *currB) {
                    return + 1;
                }
                ++currA;
                ++currB;
            }
        } else {
            // No digit-sequence starts with 0 -> assume we are looking at some integer
            // do right aligned comparison.
            //
            // The longest run of digits wins. That aside, the greatest
            // value wins, but we can't know that it will until we've scanned
            // both numbers to know that they have the same magnitude.

            bool isFirstRun = true;
            int weight = 0;
            while (1) {
                if (!currA->isDigit() && !currB->isDigit()) {
                    if (weight != 0) {
                        return weight;
                    }
                    break;
                } else if (!currA->isDigit()) {
                    if (isFirstRun) {
                        return *currA < *currB ? -1 : +1;
                    } else {
                        return -1;
                    }
                } else if (!currB->isDigit()) {
                    if (isFirstRun) {
                        return *currA < *currB ? -1 : +1;
                    } else {
                        return +1;
                    }
                } else if ((*currA < *currB) && (weight == 0)) {
                    weight = -1;
                } else if ((*currA > *currB) && (weight == 0)) {
                    weight = + 1;
                }
                ++currA;
                ++currB;
                isFirstRun = false;
            }
        }
    }

    if (currA->isNull() && currB->isNull()) {
        return 0;
    }

    return currA->isNull() ? -1 : + 1;
}

QString KStringHandler::preProcessWrap(const QString &text)
{
    const QChar zwsp(0x200b);

    QString result;
    result.reserve(text.length());

    for (int i = 0; i < text.length(); i++) {
        const QChar c = text[i];
        bool openingParens = (c == QLatin1Char('(') || c == QLatin1Char('{') || c == QLatin1Char('['));
        bool singleQuote = (c == QLatin1Char('\'') );
        bool closingParens = (c == QLatin1Char(')') || c == QLatin1Char('}') || c == QLatin1Char(']'));
        bool breakAfter   = (closingParens || c.isPunct() || c.isSymbol());
        bool nextIsSpace  = (i == (text.length() - 1) || text[i + 1].isSpace());
        bool prevIsSpace  = (i == 0 || text[i - 1].isSpace() || result[result.length() - 1] == zwsp);

        // Provide a breaking opportunity before opening parenthesis
        if (openingParens && !prevIsSpace)
            result += zwsp;
        
        // Provide a word joiner before the single quote
        if (singleQuote && !prevIsSpace)
            result += QChar(0x2060);

        result += c;

        if (breakAfter && !openingParens && !nextIsSpace && !singleQuote) 
            result += zwsp;
    }

    return result;
}

