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
        @brief Attempts to acquire the lock
    */
    bool tryLock();

    /*!
        @brief Acquires the lock
    */
    void lock();

    /*!
        @brief Returns whether the lock is held or not
    */
    bool isLocked() const;

    /*!
        @brief Release the lock
    */
    void unlock();

private:
    Q_DISABLE_COPY(KLockFile);
    KLockFilePrivate *const d;
};

#endif // KLOCKFILE_H
