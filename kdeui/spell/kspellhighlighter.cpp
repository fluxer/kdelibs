/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "kspellhighlighter.h"
#include "kspeller.h"
#include "kcolorscheme.h"
#include "kdebug.h"

#include <QTextBoundaryFinder>

class KSpellHighlighterPrivate
{
public:
    KSpellHighlighterPrivate(KConfig *config);

    KSpeller speller;
    QTextCharFormat charformat;
};

KSpellHighlighterPrivate::KSpellHighlighterPrivate(KConfig *config)
    : speller(config)
{
    charformat.setFontUnderline(true);
    charformat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    // NOTE: same as the default of Kate
    charformat.setUnderlineColor(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::NegativeText).color());
}

KSpellHighlighter::KSpellHighlighter(KConfig *config, QTextEdit *parent)
    : QSyntaxHighlighter(parent),
    d(new KSpellHighlighterPrivate(config))
{
}

KSpellHighlighter::~KSpellHighlighter()
{
    delete d;
}

QString KSpellHighlighter::currentLanguage() const
{
    return d->speller.dictionary();
}

void KSpellHighlighter::setCurrentLanguage(const QString &lang)
{
    d->speller.setDictionary(lang);
}

void KSpellHighlighter::addWordToDictionary(const QString &word)
{
    d->speller.addToPersonal(word);
}

void KSpellHighlighter::ignoreWord(const QString &word)
{
    d->speller.addToSession(word);
}

QStringList KSpellHighlighter::suggestionsForWord(const QString &word, int max)
{
    QStringList result = d->speller.suggest(word);
    // qDebug() << Q_FUNC_INFO << result << max;
    while (result.size() > max) {
        result.removeLast();
    }
    return result;
}

bool KSpellHighlighter::isWordMisspelled(const QString &word)
{
    return !d->speller.check(word);
}

void KSpellHighlighter::highlightBlock(const QString &text)
{
    // qDebug() << Q_FUNC_INFO << text.size() << d->speller.dictionary();
    if (text.isEmpty() || d->speller.dictionary().isEmpty()) {
        return;
    }
    int wordstart = 0;
    QTextBoundaryFinder finder(QTextBoundaryFinder::Word, text);
    while (finder.toNextBoundary() >= 0) {
        const QTextBoundaryFinder::BoundaryReasons boundary = finder.boundaryReasons();
        if (boundary & QTextBoundaryFinder::StartWord) {
            wordstart = finder.position();
        }
        if (boundary & QTextBoundaryFinder::EndWord) {
            QString word = text.mid(wordstart, finder.position() - wordstart);

            // remove whitespace at the start and end
            while (!word.isEmpty() && word.at(0).isSpace()) {
                word = word.mid(1, word.size() - 1);
                wordstart++;
            }
            while (!word.isEmpty() && word.at(word.size() - 1).isSpace()) {
                word = word.mid(0, word.size() - 1);
            }

            // chop punctuation
            if (word.at(word.size() - 1).isPunct()) {
                word = word.mid(0, word.size() - 1);
            }

            // not worth checking if it is less than two characters
            if (word.size() < 2) {
                continue;
            }

            if (!d->speller.check(word)) {
                setFormat(wordstart, word.size(), d->charformat);
            } else {
                setFormat(wordstart, word.size(), QTextCharFormat());
            }
        }
    }
}

#include "moc_kspellhighlighter.cpp"
