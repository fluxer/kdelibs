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

#ifndef KNETWORKMANAGER_KDED_H
#define KNETWORKMANAGER_KDED_H

#include "kdedmodule.h"
#include "knetworkmanager.h"

#include <QMap>

class KNetworkManagerModule: public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.knetworkmanager")

public:
    KNetworkManagerModule(QObject *parent, const QList<QVariant> &args);
    ~KNetworkManagerModule();

    Q_SCRIPTABLE int status() const;
    Q_SCRIPTABLE bool enable(const bool enable);

private Q_SLOTS:
    void slotStatusChanged(const KNetworkManager::KNetworkStatus status);

private:
    KNetworkManager *m_networkmanager;
};

#endif // KNETWORKMANAGER_KDED_H
