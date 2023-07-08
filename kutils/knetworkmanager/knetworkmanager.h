/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KNETWORKMANAGER_H
#define KNETWORKMANAGER_H

#include "knetworkmanager_export.h"

#include <QObject>

class KNetworkManagerPrivate;

/*!
    Class to query, manage and watch the system network state.

    @since 4.23
    @warning the API is subject to change
*/
class KNETWORKMANAGER_EXPORT KNetworkManager : public QObject
{
    Q_OBJECT
public:
    enum KNetworkStatus {
        UnknownStatus = 0,
        ConnectedStatus = 1,
        DisconnectedStatus = 2
    };

    /*!
        @brief Contructs object with @p parent
    */
    KNetworkManager(QObject *parent = nullptr);
    ~KNetworkManager();

    /*!
        @brief Returns the current network manager status
    */
    KNetworkStatus status() const;

    /*!
        @brief Returns @p true if network management is supported on this host,
        @p false otherwise
    */
    static bool isSupported();

Q_SIGNALS:
    /*!
        @brief Signals that the current status has changed to @p status
    */
    void statusChanged(const KNetworkStatus status);

private Q_SLOTS:
    void _checkStatus();

private:
    Q_DISABLE_COPY(KNetworkManager);
    KNetworkManagerPrivate *d;
};

#endif // KNETWORKMANAGER_H
