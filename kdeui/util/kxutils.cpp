/*
    This file is part of the KDE libraries
    Copyright (C) 2008 Lubos Lunak (l.lunak@kde.org)

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

#include "kxutils.h"

#ifdef Q_WS_X11

namespace KXUtils
{


// Functions for X timestamp comparing. For Time being 32bit they're fairly simple
// (the #if 0 part), but on 64bit architectures Time is 64bit unsigned long,
// so there special care needs to be taken to always use only the lower 32bits.
#if 0
int timestampCompare( Time time1, Time time2 ) // like strcmp()
    {
    if( time1 == time2 )
        return 0;
    return ( time1 - time2 ) < 0x7fffffffU ? 1 : -1; // time1 > time2 -> 1, handle wrapping
    }

Time timestampDiff( Time time1, Time time2 ) // returns time2 - time1
    { // no need to handle wrapping?
    return time2 - time1;
    }
#else
int timestampCompare( unsigned long time1_, unsigned long time2_ ) // like strcmp()
    {
    quint32 time1 = time1_;
    quint32 time2 = time2_;
    if( time1 == time2 )
        return 0;
    return quint32( time1 - time2 ) < 0x7fffffffU ? 1 : -1; // time1 > time2 -> 1, handle wrapping
    }

int timestampDiff( unsigned long time1_, unsigned long time2_ ) // returns time2 - time1
    { // no need to handle wrapping?
    quint32 time1 = time1_;
    quint32 time2 = time2_;
    return quint32( time2 - time1 );
    }
#endif


} // namespace

#endif
