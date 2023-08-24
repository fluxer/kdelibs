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

#include "kded_knetworkmanager.h"
#include "kpluginfactory.h"
#include "knotification.h"
#include "klocale.h"
#include "kdebug.h"

K_PLUGIN_FACTORY(KNetworkManagerModuleFactory, registerPlugin<KNetworkManagerModule>();)
K_EXPORT_PLUGIN(KNetworkManagerModuleFactory("knetworkmanager"))

KNetworkManagerModule::KNetworkManagerModule(QObject *parent, const QList<QVariant> &args)
    : KDEDModule(parent),
    m_networkmanager(nullptr)
{
    Q_UNUSED(args);

    m_networkmanager = new KNetworkManager(this);
    connect(
        m_networkmanager, SIGNAL(statusChanged(KNetworkManager::KNetworkStatus)),
        this, SLOT(slotStatusChanged(KNetworkManager::KNetworkStatus))
    );
}

KNetworkManagerModule::~KNetworkManagerModule()
{
    m_networkmanager->deleteLater();
}

int KNetworkManagerModule::status() const
{
    return static_cast<int>(m_networkmanager->status());
}

bool KNetworkManagerModule::enable(const bool enable)
{
    return m_networkmanager->enable(enable);
}

void KNetworkManagerModule::slotStatusChanged(const KNetworkManager::KNetworkStatus status)
{
    switch (status) {
        case KNetworkManager::UnknownStatus: {
            KNotification::event("knetworkmanager/Unknown");
            break;
        }
        case KNetworkManager::ConnectedStatus: {
            KNotification::event("knetworkmanager/Connected");
            break;
        }
        case KNetworkManager::DisconnectedStatus: {
            KNotification::event("knetworkmanager/Disconnected");
            break;
        }
        case KNetworkManager::IntermediateStatus: {
            // no notification for intermediate status changes
            break;
        }
    }
}

#include "moc_kded_knetworkmanager.cpp"
