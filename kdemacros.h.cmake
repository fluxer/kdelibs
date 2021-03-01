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
 * @def KDE_NO_EXPORT
 * @ingroup KDEMacros
 *
 * The KDE_NO_EXPORT macro marks the symbol of the given variable
 * to be hidden. A hidden symbol is stripped during the linking step,
 * so it can't be used from outside the resulting library, which is similar
 * to static. However, static limits the visibility to the current
 * compilation unit. Hidden symbols can still be used in multiple compilation
 * units.
 *
 * \code
 * int KDE_NO_EXPORT foo;
 * int KDE_EXPORT bar;
 * \endcode
 *
 * @sa KDE_EXPORT
 */

/**
 * @def KDE_EXPORT
 * @ingroup KDEMacros
 *
 * The KDE_EXPORT macro marks the symbol of the given variable
 * to be visible, so it can be used from outside the resulting library.
 *
 * \code
 * int KDE_NO_EXPORT foo;
 * int KDE_EXPORT bar;
 * \endcode
 *
 * @sa KDE_NO_EXPORT
 */

/**
 * @def KDE_IMPORT
 * @ingroup KDEMacros
 */

#define KDE_NO_EXPORT Q_DECL_HIDDEN

#define KDE_EXPORT Q_DECL_EXPORT

#define KDE_IMPORT Q_DECL_IMPORT

/**
 * @def KDE_DEPRECATED
 * @ingroup KDEMacros
 *
 * The KDE_DEPRECATED macro can be used to trigger compile-time warnings
 * with newer compilers when deprecated functions are used.
 *
 * For non-inline functions, the macro gets inserted at front of the
 * function declaration, right before the return type:
 *
 * \code
 * KDE_DEPRECATED void deprecatedFunctionA();
 * KDE_DEPRECATED int deprecatedFunctionB() const;
 * \endcode
 *
 * For functions which are implemented inline,
 * the KDE_DEPRECATED macro is inserted at the front, right before the return
 * type, but after "static", "inline" or "virtual":
 *
 * \code
 * KDE_DEPRECATED void deprecatedInlineFunctionA() { .. }
 * virtual KDE_DEPRECATED int deprecatedInlineFunctionB() { .. }
 * static KDE_DEPRECATED bool deprecatedInlineFunctionC() { .. }
 * inline KDE_DEPRECATED bool deprecatedInlineFunctionD() { .. }
 * \endcode
 *
 * You can also mark whole structs or classes as deprecated, by inserting the
 * KDE_DEPRECATED macro after the struct/class keyword, but before the
 * name of the struct/class:
 *
 * \code
 * class KDE_DEPRECATED DeprecatedClass { };
 * struct KDE_DEPRECATED DeprecatedStruct { };
 * \endcode
 *
 * \note
 * It does not make much sense to use the KDE_DEPRECATED keyword for a Qt signal;
 * this is because usually get called by the class which they belong to,
 * and one would assume that a class author does not use deprecated methods of
 * his own class. The only exception to this are signals which are connected to
 * other signals; they get invoked from moc-generated code. In any case,
 * printing a warning message in either case is not useful.
 * For slots, it can make sense (since slots can be invoked directly) but be
 * aware that if the slots get triggered by a signal, the will get called from
 * moc code as well and thus the warnings are useless.
 *
 * \par
 * Also note that it is not possible to use KDE_DEPRECATED for classes which
 * use the k_dcop keyword (to indicate a DCOP interface declaration); this is
 * because the dcopidl program would choke on the unexpected declaration
 * syntax.
 *
 * \note
 * KDE_DEPRECATED cannot be used at the end of the declaration
 *
 * \note
 * KDE_DEPRECATED cannot be used for constructors, 
 * use KDE_CONSTRUCTOR_DEPRECATED instead.
 */

#define KDE_DEPRECATED Q_DECL_DEPRECATED

/**
 * @def KDE_CONSTRUCTOR_DEPRECATED
 * @ingroup KDEMacros
 *
 * The KDE_CONSTRUCTOR_DEPRECATED macro can be used to trigger compile-time
 * warnings with newer compilers when deprecated constructors are used.
 *
 * For non-inline constructors, the macro gets inserted at front of the
 * constructor declaration, right before the return type:
 *
 * \code
 * KDE_CONSTRUCTOR_DEPRECATED classA();
 * \endcode
 *
 * For constructors which are implemented inline,
 * the KDE_CONSTRUCTOR_DEPRECATED macro is inserted at the front,
 * but after the "inline" keyword:
 *
 * \code
 * KDE_CONSTRUCTOR_DEPRECATED classA() { .. }
 * \endcode
 *
 * \note Do not forget that inlined constructors are not allowed in public
 * headers for KDE.
 */

#define KDE_CONSTRUCTOR_DEPRECATED Q_DECL_CONSTRUCTOR_DEPRECATED

/**
 * @def KDE_NO_DEPRECATED
 * @ingroup KDEMacros
 *
 * The KDE_NO_DEPRECATED indicates if the deprecated symbols of the platform
 * have been compiled out.
 */
#cmakedefine KDE_NO_DEPRECATED

#endif /* _KDE_MACROS_H_ */
