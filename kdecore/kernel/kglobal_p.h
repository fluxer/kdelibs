/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KGLOBAL_P_H
#define KGLOBAL_P_H

#include <kdecore_export.h>

class KComponentData;

namespace KGlobal
{
    ///@internal
    void newComponentData(const KComponentData &c);
}

#include "kglobal.h"
#include "klocale.h"

#include <QTranslator>

#ifndef QT_NO_TRANSLATION
class KDETranslator : public QTranslator
{
public:
    QString translate(const char *context, const char *sourceText) const final
    {
        if (isEmpty()) {
            return QString();
        }
        return KGlobal::locale()->translateQt(context, sourceText);
    }

    bool isEmpty() const final
    {
        return !KGlobal::hasLocale();
    }
};
#endif // QT_NO_TRANSLATION

#endif // KGLOBAL_P_H
