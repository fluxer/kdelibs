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

#ifndef KSPELLHIGHLIGHTER_H
#define KSPELLHIGHLIGHTER_H

#include "kdeui_export.h"
#include "kconfig.h"

#include <QSyntaxHighlighter>

class KSpellHighlighterPrivate;

/*!
    Class to check spelling and highlight misspelled words.

    @since 4.23
*/
class KDEUI_EXPORT KSpellHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    KSpellHighlighter(KConfig *config, QTextEdit *parent);
    ~KSpellHighlighter();

    QString currentLanguage() const;
    void setCurrentLanguage(const QString &lang);

    void addWordToDictionary(const QString &word);
    void ignoreWord(const QString &word);
    QStringList suggestionsForWord(const QString &word, int max = 10);
    bool isWordMisspelled(const QString &word);

protected:
    // reimplementation
    virtual void highlightBlock(const QString &text);

private:
    KSpellHighlighterPrivate *d;
    Q_DISABLE_COPY(KSpellHighlighter);
};

#endif // KSPELLHIGHLIGHTER_H 
