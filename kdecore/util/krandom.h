/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>
    Copyright (c) 1999 Sean Harmer <sh@astro.keele.ac.uk>

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

#ifndef KRANDOM_H
#define KRANDOM_H

#include <kdecore_export.h>

#include <QtCore/QString>

/**
 * \headerfile krandom.h <KRandom>
 * 
 * @short Helper class to create random data
 *
 * This namespace provides methods which generate random data.
 */
namespace KRandom {
    /**
     * Generates a uniform random number.
     * @return A random number in the range [0, RAND_MAX]
     */
    inline int random() { return qrand(); };

    /**
     * Generates a uniform random number.
     * @param max Maximum value for the returned number.
     * @return A random number in the range [0, @p max]
     */
    KDECORE_EXPORT int randomMax(int max);

    /**
     * Generates a random string. It operates in the range [A-Za-z0-9]
     * @param length Generate a string of this length.
     * @return the random string
     */
    KDECORE_EXPORT QString randomString(int length);

    /**
    * Put a list in random order. Since KDE 4.11, this function uses a more
    * efficient algorithm (Fisher-Yates). Therefore, the order of the items
    * in the randomized list is different from the one in earlier versions
    * if the same seed value is used for the random sequence.
    *
    * @param list the list whose order will be modified
    * @note modifies the list in place
    * @author Sean Harmer <sh@astro.keele.ac.uk>
    */
    template<typename T> void randomize(QList<T>& list) {
        // Fisher-Yates algorithm
        for (int index = list.count() - 1; index > 0; --index) {
            const int swapIndex = randomMax(index + 1);
            qSwap(list[index], list[swapIndex]);
        }
    }
}

#endif // KRANDOM_H
