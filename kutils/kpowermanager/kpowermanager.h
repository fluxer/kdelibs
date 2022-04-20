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

#ifndef KPOWERMANAGER_H
#define KPOWERMANAGER_H

#include "kpowermanager_export.h"

#include <QObject>
#include <QStringList>

class KPowerManagerPrivate;

/*!
    Class to query, manage and watch the system power state.

    @since 4.21
    @warning the API is subject to change
*/
class KPOWERMANAGER_EXPORT KPowerManager : public QObject
{
    Q_OBJECT
public:
    /*!
        @brief Contructs object with @p parent
    */
    KPowerManager(QObject *parent = nullptr);
    ~KPowerManager();

    /*!
        @brief Returns the current power manager profile
    */
    QString profile() const;
    /*!
        @brief Returns all valid power manager profiles
    */
    QStringList profiles() const;
    /*!
        @brief Changes the current power manager profile to @p profile
    */
    bool setProfile(const QString &profile);

    /*!
        @brief Returns the current GPU governor
        @note The value is from the first CPU device node
    */
    QString CPUGovernor() const;
    /*!
        @brief Returns the all valid GPU governors
    */
    QStringList CPUGovernors() const;
    /*!
        @brief Changes the current CPU governor to @p governor
    */
    bool setCPUGovernor(const QString &governor);

Q_SIGNALS:
    /*!
        @brief Signals that the current profile has changed
    */
    void profileChanged(const QString &profile);
    /*!
        @brief Signals that the current CPU governor has changed
    */
    void CPUGovernorChanged(const QString &governor);

private Q_SLOTS:
    void _configDirty(const QString &path);
    void _CPUGovernorDirty(const QString &path);

private:
    Q_DISABLE_COPY(KPowerManager);
    KPowerManagerPrivate *d;
};

#endif // KPOWERMANAGER_H
