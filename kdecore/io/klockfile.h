/*  This file is part of the KDE libraries
    Copyright (c) 2004 Waldo Bastian <bastian@kde.org>
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

#ifndef KLOCKFILE_H
#define KLOCKFILE_H

#include <kdecore_export.h>

#include <QString>

class KLockFilePrivate;

/**
 * \class KLockFile klockfile.h <KLockFile>
 * 
 * The KLockFile class provides NFS safe lockfiles.
 *
 * @author Ivailo Monev <xakepa10@gmail.com>
 * @author Waldo Bastian <bastian@kde.org>
 */
class KDECORE_EXPORT KLockFile
{
public:
    /*!
        @brief Creates the object, does not attempt to acquire the lock
    */
    KLockFile(const QString &file);

    /*!
        @brief Destroys the object, releasing the lock if held
    */
    ~KLockFile();

    /*!
        @brief Possible return values of the lock function
    */
    enum LockResult {
        //! @brief Lock was acquired successfully
        LockOK = 0,

        //! @brief The lock could not be acquired because it is held by another process
        LockFail,

        //! @brief The lock could not be acquired due to an error
        LockError,

        //! @brief A stale lock has been detected, i.e. the process that owned the lock has crashed
        LockStale
    };

    enum LockFlag {
        //! @brief Return immediately, do not wait for the lock to become available
        NoBlockFlag = 1,

        //! @brief Automatically remove a lock when it is detected to be stale
        ForceFlag = 2
    };
    Q_DECLARE_FLAGS(LockFlags, LockFlag)

    /*!
        @brief Attempt to acquire the lock
        @param flags A set of @ref LockFlag values OR'ed together
    */
    LockResult lock(LockFlags flags = LockFlags());

    /*!
        @brief Returns whether the lock is held or not
    */
    bool isLocked() const;

    /*!
        @brief Release the lock
    */
    void unlock();

    /*!
        @brief Returns the pid and hostname of the process holding the lock.
        @returns false if the pid and hostname could not be determined
    */
    bool getLockInfo(qint64 &pid, QString &hostname);

private:
    Q_DISABLE_COPY(KLockFile);
    KLockFilePrivate *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KLockFile::LockFlags)

#endif
