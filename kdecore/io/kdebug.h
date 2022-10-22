/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2000-2002 Stephan Kulow (coolo@kde.org)
                  2002 Holger Freyther (freyther@kde.org)

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

#ifndef KDEBUG_H
#define KDEBUG_H

#include <kdecore_export.h>

#include <QtCore/qdebug.h>

/**
 * \addtogroup kdebug Debug message generators
 *  @{
 * KDE debug message streams let you and the user control just how many debug
 * messages you see. Debug message can be controled by editing kdebugrc.
 *
 * You can also control what you see: process name, area name, method name,
 * file and line number, timestamp, etc. using environment variables:
 * KDE_DEBUG_TIMESTAMP - adds timestamp to the message
 * KDE_DEBUG_METHODNAME - adds the method to the message
 */

/**
 * @internal
 * Returns a debug stream that may or may not output anything.
 */
KDECORE_EXPORT QDebug kDebugStream(QtMsgType level, int area, const char *file = 0,
                                   int line = -1, const char *funcinfo = 0);

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
 * @param area an id to identify the output, KDE_DEFAULT_DEBUG_AREA for default
 */
static inline QDebug kDebug(int area = KDE_DEFAULT_DEBUG_AREA)
{ return kDebugStream(QtDebugMsg, area); }

/**
 * \relates KGlobal
 * Returns a warning stream. You can use it to print warning
 * information.
 * @param area an id to identify the output, KDE_DEFAULT_DEBUG_AREA for default
 */
static inline QDebug kWarning(int area = KDE_DEFAULT_DEBUG_AREA)
{ return kDebugStream(QtWarningMsg, area); }

/**
 * \relates KGlobal
 * Returns an error stream. You can use it to print error
 * information.
 * @param area an id to identify the output, KDE_DEFAULT_DEBUG_AREA for default
 */
static inline QDebug kError(int area = KDE_DEFAULT_DEBUG_AREA)
{ return kDebugStream(QtCriticalMsg, area); }

/**
 * \relates KGlobal
 * Returns a fatal error stream. You can use it to print fatal error
 * information.
 * @param area an id to identify the output, KDE_DEFAULT_DEBUG_AREA for default
 */
static inline QDebug kFatal(int area = KDE_DEFAULT_DEBUG_AREA)
{ return kDebugStream(QtFatalMsg, area); }

// operators for KDE types
class KUrl;
class KDateTime;
KDECORE_EXPORT QDebug operator<<(QDebug s, const KUrl &url);
KDECORE_EXPORT QDebug operator<<(QDebug s, const KDateTime &time);

/**
 * @internal
 * A class for using operator()
 */
class KDebug                    //krazy= ?
{
    const char* const file;
    const char* const funcinfo;
    const int line;
    const QtMsgType level;
public:
    explicit inline KDebug(QtMsgType type, const char* const f = 0, int l = -1, const char* const info = 0)
        : file(f), funcinfo(info), line(l), level(type)
        {
        }

    inline QDebug operator()(int area = KDE_DEFAULT_DEBUG_AREA)
        { return kDebugStream(level, area, file, line, funcinfo); }
};


#define kDebug     KDebug(QtDebugMsg, __FILE__, __LINE__, Q_FUNC_INFO)
#define kWarning   KDebug(QtWarningMsg, __FILE__, __LINE__, Q_FUNC_INFO)
#define kError     KDebug(QtCriticalMsg, __FILE__, __LINE__, Q_FUNC_INFO)
#define kFatal     KDebug(QtFatalMsg, __FILE__, __LINE__, Q_FUNC_INFO)

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
