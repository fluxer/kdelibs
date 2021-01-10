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

#ifndef _KDEBUG_H_
#define _KDEBUG_H_

#include <kdecore_export.h>

#include <QtCore/qdebug.h>

/**
 * \addtogroup kdebug Debug message generators
 *  @{
 * KDE debug message streams let you and the user control just how many debug
 * messages you see. Debug message printing is controlled by (un)defining
 * QT_NO_DEBUG when compiling your source. If QT_NO_DEBUG is defined then debug
 * messages are not printed by default but can still be enabled by runtime
 * configuration, e.g. via kdebugdialog or by editing kdebugrc.
 *
 * You can also control what you see: process name, area name, method name,
 * file and line number, timestamp, etc. using environment variables.
 * See http://techbase.kde.org/SysAdmin/Environment_Variables#KDE_DEBUG_NOPROCESSINFO
 */

#if !defined(KDE_NO_DEBUG_OUTPUT)
# if defined(QT_NO_DEBUG_OUTPUT) || defined(QT_NO_DEBUG_STREAM)
#  define KDE_NO_DEBUG_OUTPUT
# endif
#endif

#if !defined(KDE_NO_WARNING_OUTPUT)
# if defined(QT_NO_WARNING_OUTPUT)
#  define KDE_NO_WARNING_OUTPUT
# endif
#endif

#ifdef QT_NO_DEBUG /* The application is compiled in release mode */
# define KDE_DEBUG_ENABLED_BY_DEFAULT false
#else
# define KDE_DEBUG_ENABLED_BY_DEFAULT true
#endif

/**
 * @internal
 * Returns a debug stream that may or may not output anything.
 */
KDECORE_EXPORT QDebug kDebugStream(QtMsgType level, int area, const char *file = 0,
                                   int line = -1, const char *funcinfo = 0);

/**
 * @internal
 * Returns a debug stream that goes the way of the blackhole.
 */
KDECORE_EXPORT QDebug kDebugDevNull();

/**
 * \relates KGlobal
 * Returns a backtrace.
 * Note: Hidden symbol visibility may negatively affect the information provided
 * by kBacktrace - you may want to pass -DCXX_VISIBILITY_PRESET=FALSE to CMake
 * to turn hidden symbol visibility off.
 * @param levels the number of levels of the backtrace
 * @return a backtrace
 */
#if !defined(KDE_NO_DEBUG_OUTPUT)
KDECORE_EXPORT QString kBacktrace(int levels=-1);
#else
inline QString kBacktrace(int=-1) { return QString(); };
#endif

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

#if !defined(KDE_NO_DEBUG_OUTPUT)
/**
 * \relates KGlobal
 * Returns a debug stream. You can use it to print debug
 * information.
 * @param area an id to identify the output, KDE_DEFAULT_DEBUG_AREA for default
 */
static inline QDebug kDebug(int area = KDE_DEFAULT_DEBUG_AREA)
{ return kDebugStream(QtDebugMsg, area); }
static inline QDebug kDebug(bool cond, int area = KDE_DEFAULT_DEBUG_AREA)
{ return cond ? kDebug(area) : kDebugDevNull(); }

#else  // KDE_NO_DEBUG_OUTPUT
static inline QDebug kDebug(int = KDE_DEFAULT_DEBUG_AREA) { return kDebugDevNull(); }
static inline QDebug kDebug(bool, int = KDE_DEFAULT_DEBUG_AREA) { return kDebugDevNull(); }
#endif

#if !defined(KDE_NO_WARNING_OUTPUT)
/**
 * \relates KGlobal
 * Returns a warning stream. You can use it to print warning
 * information.
 * @param area an id to identify the output, KDE_DEFAULT_DEBUG_AREA for default
 */
static inline QDebug kWarning(int area = KDE_DEFAULT_DEBUG_AREA)
{ return kDebugStream(QtWarningMsg, area); }
static inline QDebug kWarning(bool cond, int area = KDE_DEFAULT_DEBUG_AREA)
{ return cond ? kWarning(area) : kDebugDevNull(); }

#else  // KDE_NO_WARNING_OUTPUT
static inline QDebug kWarning(int = KDE_DEFAULT_DEBUG_AREA) { return kDebugDevNull(); }
static inline QDebug kWarning(bool, int = KDE_DEFAULT_DEBUG_AREA) { return kDebugDevNull(); }
#endif

/**
 * \relates KGlobal
 * Returns an error stream. You can use it to print error
 * information.
 * @param area an id to identify the output, KDE_DEFAULT_DEBUG_AREA for default
 */
static inline QDebug kError(int area = KDE_DEFAULT_DEBUG_AREA)
{ return kDebugStream(QtCriticalMsg, area); }
static inline QDebug kError(bool cond, int area = KDE_DEFAULT_DEBUG_AREA)
{ return cond ? kError(area) : kDebugDevNull(); }

/**
 * \relates KGlobal
 * Returns a fatal error stream. You can use it to print fatal error
 * information.
 * @param area an id to identify the output, KDE_DEFAULT_DEBUG_AREA for default
 */
static inline QDebug kFatal(int area = KDE_DEFAULT_DEBUG_AREA)
{ return kDebugStream(QtFatalMsg, area); }
static inline QDebug kFatal(bool cond, int area = KDE_DEFAULT_DEBUG_AREA)
{ return cond ? kFatal(area) : kDebugDevNull(); }

struct KDebugTag { }; ///! @internal just a tag class
typedef QDebug (*KDebugStreamFunction)(QDebug, KDebugTag); ///< @internal
inline QDebug operator<<(QDebug s, KDebugStreamFunction f)
{ return (*f)(s, KDebugTag()); }

/**
 * \relates KGlobal
 * Print a message describing the last system error.
 * @param s the debug stream to write to
 * @return the debug stream (@p s)
 * @see perror(3)
 */
KDECORE_EXPORT QDebug perror(QDebug, KDebugTag);

// operators for KDE types
class KUrl;
class KDateTime;
#include <QObject>
KDECORE_EXPORT QDebug operator<<(QDebug s, const KUrl &url);
KDECORE_EXPORT QDebug operator<<(QDebug s, const KDateTime &time);

/**
 * @internal
 * A class for using operator()
 */
class KDebug                    //krazy= ?
{
    const char *file;
    const char *funcinfo;
    int line;
    QtMsgType level;
public:
    class Block;
    explicit inline KDebug(QtMsgType type, const char *f = 0, int l = -1, const char *info = 0)
        : file(f), funcinfo(info), line(l), level(type)
        {
#ifdef KDE4_CMAKE_TOPLEVEL_DIR_LENGTH // set by FindKDE4Internal.cmake
            file = file + KDE4_CMAKE_TOPLEVEL_DIR_LENGTH + 1;
#endif
        }

    inline QDebug operator()(int area = KDE_DEFAULT_DEBUG_AREA)
        { return kDebugStream(level, area, file, line, funcinfo); }
    inline QDebug operator()(bool cond, int area = KDE_DEFAULT_DEBUG_AREA)
        { if (cond) return operator()(area); return kDebugDevNull(); }

    /// @internal
    static KDECORE_EXPORT bool hasNullOutput(QtMsgType type,
                                             bool condition,
                                             int area,
                                             bool enableByDefault);

    /// @internal
    static inline bool hasNullOutputQtDebugMsg(int area = KDE_DEFAULT_DEBUG_AREA)
        { return hasNullOutput(QtDebugMsg, true, area, KDE_DEBUG_ENABLED_BY_DEFAULT); }
    /// @internal
    static inline bool hasNullOutputQtDebugMsg(bool condition, int area = KDE_DEFAULT_DEBUG_AREA)
        { return hasNullOutput(QtDebugMsg, condition, area, KDE_DEBUG_ENABLED_BY_DEFAULT); }

    /**
     * @since 4.4
     * Register a debug area dynamically.
     * @param areaName the name of the area
     * @param enabled whether debug output should be enabled by default
     * (all debug messages are disabled by default via DisableAll=true in
       kdebugrc which can be changed by users from the system settings)
     * @return the area code that was allocated for this area
     *
     * Typical usage:
     * If all uses of the debug area are restricted to a single class, add a method like this
     * (e.g. into the Private class, if there's one)
     * <code>
     *  static int debugArea() { static int s_area = KDebug::registerArea("areaName"); return s_area; }
     * </code>
     * Please do not use a file-static int, it would (indirectly) create KGlobal too early,
     * create KConfig instances too early (breaking unittests which set KDEHOME), etc.
     * By using a function as shown above, you make it all happen on-demand, rather than upfront.
     *
     * If all uses of the debug area are restricted to a single .cpp file, do the same
     * but outside any class, and then use a more specific name for the function.
     *
     * If however multiple classes and files need the debug area, then
     * declare it in one file without static, and use "extern int debugArea();"
     * in other files (with a better name for the function of course).
     */
    static KDECORE_EXPORT int registerArea(const QByteArray& areaName, bool enabled = true);
};


#if !defined(KDE_NO_DEBUG_OUTPUT)
/* __VA_ARGS__ should work with any supported GCC version and MSVC > 2005 */
# if defined(Q_CC_GNU)
#  define kDebug(...) for (bool _k_kDebugDoOutput_ = !KDebug::hasNullOutputQtDebugMsg(__VA_ARGS__); \
                           Q_UNLIKELY(_k_kDebugDoOutput_); _k_kDebugDoOutput_ = false) \
                           KDebug(QtDebugMsg, __FILE__, __LINE__, Q_FUNC_INFO)(__VA_ARGS__)
# else
#  define kDebug     KDebug(QtDebugMsg, __FILE__, __LINE__, Q_FUNC_INFO)
# endif
#else
# define kDebug      while (false) kDebug
#endif
#if !defined(KDE_NO_WARNING_OUTPUT)
# define kWarning    KDebug(QtWarningMsg, __FILE__, __LINE__, Q_FUNC_INFO)
#else
# define kWarning    while (false) kWarning
#endif

/**
 * Convenience macro, use this to remind yourself to finish the implementation of a function
 * The function name will appear in the output (unless $KDE_DEBUG_NOMETHODNAME is set)
 * @since 4.6
 */
#define KWARNING_NOTIMPLEMENTED kWarning() << "NOT-IMPLEMENTED";

/**
 * Convenience macro, use this to alert other developers to stop using a function
 * The function name will appear in the output (unless $KDE_DEBUG_NOMETHODNAME is set)
 * @since 4.6
 */
#define KWARNING_DEPRECATED kWarning() << "DEPRECATED";

/** @} */

#endif
