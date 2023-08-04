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
 * This namespace contains functions to handle crashes. It allows to set a crash handler function
 * that will be called when the application crashes and also provides a default crash handler that
 * implements the following functionality:
 * @li Automatically restart the program
 * @li Notify the user about the crash with the option to report it
 * @li Log details about the crash to the system log
 *
 * @note All the above features are optional and have to be enabled explicitly. By default, the
 * defaultCrashHandler() will not do anything. However, if the program is using KApplication, it
 * will by default enable notification and logging on crashes, unless the --nocrashhandler argument
 * was passed on the command line or the environment variable KDE_DEBUG is set to any value.
 */
namespace KCrash
{
   /**
    * The default crash handler. Do not call this function directly. Instead, use setCrashHandler()
    * to set it as the application's crash handler.
    (
    * @param signal the signal number
    * @note If custom crash handler is required, you will have to call this function from your
    * implementation if you want to use the features of this namespace.
    */
   KDEUI_EXPORT void defaultCrashHandler(int signal);

   /**
    * Typedef for a pointer to a crash handler function. The function's argument is the number of
    * the signal.
    */
   typedef void (*HandlerType)(int);

    /**
     * Install a function to be called when a crash occurs. A crash occurs when one of the
     * following signals is caught:
     * @li SIGSEGV
     * @li SIGBUS
     * @li SIGFPE
     * @li SIGILL
     * @li SIGABRT
     *
     * @param handler this can be one of:
     * @li null, in which case signal catching is disabled (by setting the signal handler for the
     *     crash signals to SIG_DFL)
     * @li a user defined function in the form
     * @li if handler is omitted, the default crash handler is installed
     */
    KDEUI_EXPORT void setCrashHandler(HandlerType handler = defaultCrashHandler);

    /**
     * Returns the installed crash handler.
     * @return the crash handler
     */
    KDEUI_EXPORT HandlerType crashHandler();

    /**
     * Options to determine how the default crash handler should behave.
     */
    enum CrashFlag {
        AutoRestart = 1,  ///< autorestart the application. @since 4.1.
        Notify = 2,       ///< notify about the crash via system notification. @since 4.23.
        Log = 4           ///< log details about the crash to the system log. @since 4.23.
    };
    Q_DECLARE_FLAGS(CrashFlags, CrashFlag)

    /**
     * Set options to determine how the default crash handler should behave.
     * @param flags ORed together CrashFlags
     * @note The default crash handler is automatically installed by this functions if needed.
     * However, if custom crash handler is set this functions will not change it.
     */
    KDEUI_EXPORT void setFlags(CrashFlags flags);

    /**
     * Get currently set options that determine how the default crash handler should behave.
     * @param flags ORed together CrashFlags
     */
    KDEUI_EXPORT CrashFlags flags();
}

Q_DECLARE_OPERATORS_FOR_FLAGS(KCrash::CrashFlags)

#endif // KCRASH_H
