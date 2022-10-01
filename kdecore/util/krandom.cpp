/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "config.h"
#include "krandom.h"
#include "kdebug.h"

#if defined(HAVE_ARC4RANDOM_UNIFORM)
#  include <stdlib.h>
#endif

int KRandom::randomMax(int max)
{
    if (Q_UNLIKELY(max <= 0)) {
        kWarning() << "Maximum value must be greater than zero";
        return 0;
    }

#if defined(HAVE_ARC4RANDOM_UNIFORM)
    return ::arc4random_uniform(max);
#else
    return KRandom::random() % max;
#endif
}

QString KRandom::randomString(int length)
{
    if (length <= 0) {
        return QString();
    }

    QString str(length, Qt::Uninitialized);
    static const char rndstrchars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < length; i++) {
        str[i] = rndstrchars[KRandom::randomMax(62)];
    }
    return str;
}
