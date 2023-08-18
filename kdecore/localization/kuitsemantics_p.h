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

#ifndef KUITSEMANTICS_P_H
#define KUITSEMANTICS_P_H

#include "kcatalog_p.h"

#include <QString>
#include <QList>

struct KuitFormat
{
    QString tag;
    QString plain;
    QString rich;
};

/**
  * @internal
  * (used by KLocalizedString)
  *
  * KuitSemantics resolves semantic markup in user interface text into appropriate visual
  * formatting.
  *
  * @author Ivailo Monev <xakepa10@gmail.com>
  * @short class for formatting semantic markup in UI messages
  */
class KuitSemantics
{
public:
    /**
     * Constructor.
     *
     * @param lang language to create the formatter for
     */
    KuitSemantics(const QString &lang);

    /**
     * Transforms the semantic markup in the given text into visual formatting.
     * The appropriate visual formatting is decided based on the semantic
     * context marker provided in the context string.
     *
     * @param text text containing the semantic markup
     * @param ctxt context of the text
     */
    QString format(const QString &text, const QString &ctxt) const;

private:
    KCatalog m_catalog;
    QList<KuitFormat> m_formats;
    static const QLatin1String s_numargs;
    static const QLatin1String s_numintg;
    static const QLatin1String s_numreal;
    static const QLatin1String s_title;
};

// Some stuff needed in klocalizedstring.cpp too.
#define KUIT_NUMARGS "numargs"
#define KUIT_NUMINTG "numintg"
#define KUIT_NUMREAL "numreal"

#endif
