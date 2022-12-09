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

#ifndef KCRASH_H
#define KCRASH_H

#include <kdeui_export.h>

#include <QString>

/**
 * This namespace contains functions to handle crashes.
 * It allows you to set a crash handler function that will be called
 * when your application crashes and also provides a default crash
 * handler that implements the following functionality:
 * @li Launches the KDE crash display application (DrKonqi) to let
 * the user report the bug and/or debug it.
 * @li Calls an emergency save function that you can set with
 * setEmergencySaveFunction() to attempt to save the application's data.
 * @li Autorestarts your application.
 *
 * @note All the above features are optional and you need to enable them
 * explicitly. By default, the defaultCrashHandler() will not do anything.
 * However, if you are using KApplication, it will by default enable launching
 * DrKonqi on crashes, unless the --nocrashhandler argument was passed on
 * the command line or the environment variable KDE_DEBUG is set to any value.
 */
namespace KCrash
{
   /**
    * The default crash handler.
    * Do not call this function directly. Instead, use
    * setCrashHandler() to set it as your application's crash handler.
    * @param signal the signal number
    * @note If you implement your own crash handler, you will have to
    * call this function from your implementation if you want to use the
    * features of this namespace.
    */
   KDEUI_EXPORT void defaultCrashHandler (int signal);

   /**
    * Typedef for a pointer to a crash handler function.
    * The function's argument is the number of the signal.
    */
   typedef void (*HandlerType)(int);

    /**
     * Install a function to be called when a crash occurs.
     * A crash occurs when one of the following signals is
     * caught: SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT.
     * @param handler this can be one of:
     * @li null, in which case signal catching is disabled
     * (by setting the signal handler for the crash signals to SIG_DFL)
     * @li a user defined function in the form:
     * static (if in a class) void myCrashHandler(int);
     * @li if handler is omitted, the default crash handler is installed
     * @note If you use setFlags(AutoRestart), you do not need to call this function
     * explicitly. The default crash handler is automatically installed by
     * those functions if needed. However, if you set a custom crash handler,
     * those functions will not change it.
     */
    KDEUI_EXPORT void setCrashHandler(HandlerType handler = defaultCrashHandler);

    /**
     * Returns the installed crash handler.
     * @return the crash handler
     */
    KDEUI_EXPORT HandlerType crashHandler();

    /**
     * Options to determine how the default crash handler should behave.
     * @note Options are prioritised in their numerical order, i.e. if
     * AutoRestart is set all other options are ignored.
     */
    enum CrashFlag {
        AutoRestart = 1,  ///< autorestart this application. Only sensible for KUniqueApplications. @since 4.1.
        DrKonqi = 2,      ///< launchers DrKonqi. @since 4.23.
        NoRestart = 4,    ///< tell DrKonqi not to restart the program. @since 4.23.
        Backtrace = 8     ///< log backtrace if the program crashes. @since 4.23.
    };
    Q_DECLARE_FLAGS(CrashFlags, CrashFlag)

    /**
     * Set options to determine how the default crash handler should behave.
     * @param flags ORed together CrashFlags
     */
    KDEUI_EXPORT void setFlags(CrashFlags flags);

    /**
     * Get currently set options that determine how the default crash handler should behave.
     * @param flags ORed together CrashFlags
     */
    KDEUI_EXPORT CrashFlags flags();
}

Q_DECLARE_OPERATORS_FOR_FLAGS(KCrash::CrashFlags)

#endif

