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

#ifndef KDEBUG_H
#define KDEBUG_H

#include <kdecore_export.h>

#include <QtCore/qdebug.h>

/**
 * \addtogroup kdebug Debug message generators
 *  @{
 * KDE debug message streams let you and the user control just how many debug
 * messages you see. Debug message can be controled by editing kdebugrc and
 * by setting or unsetting environment variables:
 *  KDE_DEBUG_TIMESTAMP - adds timestamp to the message
 *  KDE_DEBUG_METHODNAME - adds the method to the message
 *  KDE_DEBUG_COLOR - colorizes the message, valid only for shell type of
 *                    output and when it is TTY
 */

/**
 * \relates KGlobal
 * Returns a backtrace.
 * Note: Hidden symbol visibility may negatively affect the information provided
 * by kBacktrace - you may want to pass -DCXX_VISIBILITY_PRESET=FALSE to CMake
 * to turn hidden symbol visibility off.
 * @param levels the number of levels of the backtrace
 * @return a backtrace
 */
KDECORE_EXPORT QString kBacktrace(int levels = -1);

/**
 * \relates KGlobal
 * Deletes the kdebugrc cache and therefore forces KDebug to reread the
 * config file
 */
KDECORE_EXPORT void kClearDebugConfig();

#ifndef KDE_DEFAULT_DEBUG_AREA
# define KDE_DEFAULT_DEBUG_AREA 0
#endif

/*!
  \macro KDE_DEFAULT_DEBUG_AREA
  \relates KGlobal

  Denotes the debug area to use in kDebug/kWarning etc when not
  explicitly specified. The default is 0 (zero).

  Define this macro to the debug area of your application/component
  before including any KDE headers. Usually, you want to add code like
  this to your \c CMakeLists.txt:

  \code
  ...
  add_definitions( -DKDE_DEFAULT_DEBUG_AREA=1234 )
  ...
  \endcode

  This way, you save repeating the debug area all over your source
  code, in each debug/warning statement.
*/

/**
 * \relates KGlobal
 * Returns a debug stream. You can use it to print debug
 * information.
 * @param type type of message
 * @param funcinfo caller of KDebug
 * @param area an id to identify the output
 */
KDECORE_EXPORT QDebug KDebug(const QtMsgType type, const char* const funcinfo, const int area = KDE_DEFAULT_DEBUG_AREA);

// operators for KDE types
class KUrl;
class KDateTime;
KDECORE_EXPORT QDebug operator<<(QDebug s, const KUrl &url);
KDECORE_EXPORT QDebug operator<<(QDebug s, const KDateTime &time);

#define kDebug(...)     KDebug(QtDebugMsg, Q_FUNC_INFO, ##__VA_ARGS__)
#define kWarning(...)   KDebug(QtWarningMsg, Q_FUNC_INFO, ##__VA_ARGS__)
#define kError(...)     KDebug(QtCriticalMsg, Q_FUNC_INFO, ##__VA_ARGS__)
#define kFatal(...)     KDebug(QtFatalMsg, Q_FUNC_INFO, ##__VA_ARGS__)

/**
 * Convenience macro, use this to remind yourself to finish the implementation of a function
 * @since 4.6
 */
#define KWARNING_NOTIMPLEMENTED kWarning() << "NOT-IMPLEMENTED" << Q_FUNC_INFO;

/**
 * Convenience macro, use this to alert other developers to stop using a function
 * @since 4.6
 */
#define KWARNING_DEPRECATED kWarning() << "DEPRECATED" << Q_FUNC_INFO;

/** @} */

#endif // KDEBUG_H
