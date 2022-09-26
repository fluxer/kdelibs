/* This file is part of the KDE libraries
    Copyright (c) 2002-2003 KDE Team

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

/**
 * @file kdemacros.h
 *
 * This header defines several compiler-independent macros which are used
 * throughout KDE. Most of these macros make use of GCC extensions; on other
 * compilers, they don't have any effect.
 */

#ifndef _KDE_MACROS_H_
#define _KDE_MACROS_H_

#include <QtCore/qglobal.h>

/**
 * @def KDE_EXPORT
 * @ingroup KDEMacros
 *
 * The KDE_EXPORT macro marks the symbol of the given variable
 * to be visible, so it can be used from outside the resulting library.
 *
 * \code
 * int KDE_EXPORT bar;
 * \endcode
 */

/**
 * @def KDE_IMPORT
 * @ingroup KDEMacros
 */

#define KDE_EXPORT Q_DECL_EXPORT

#define KDE_IMPORT Q_DECL_IMPORT

#endif /* _KDE_MACROS_H_ */
