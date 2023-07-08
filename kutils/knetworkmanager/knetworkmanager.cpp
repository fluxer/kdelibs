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

#include "knetworkmanager.h"
#include "kdebug.h"

#include <QTimer>
#include <QNetworkInterface>

class KNetworkManagerPrivate
 {
public:
    KNetworkManagerPrivate();

    KNetworkManager::KNetworkStatus status;
    QTimer* statustimer;
};

KNetworkManagerPrivate::KNetworkManagerPrivate()
    : status(KNetworkManager::UnknownStatus),
    statustimer(nullptr)
{
}

KNetworkManager::KNetworkManager(QObject *parent)
    : QObject(parent),
    d(new KNetworkManagerPrivate())
{
    d->statustimer = new QTimer(this);
    connect(d->statustimer, SIGNAL(timeout()), this, SLOT(_checkStatus()));
    d->statustimer->start(2000);
}

KNetworkManager::~KNetworkManager()
{
    delete d;
}

KNetworkManager::KNetworkStatus KNetworkManager::status() const
{
    return d->status;
}

bool KNetworkManager::isSupported()
{
    return true;
}

void KNetworkManager::_checkStatus()
{
    KNetworkManager::KNetworkStatus newstatus = KNetworkManager::DisconnectedStatus;
    foreach (const QNetworkInterface &iface, QNetworkInterface::allInterfaces()) {
        const QNetworkInterface::InterfaceFlags iflags = iface.flags();
        if (iflags & QNetworkInterface::CanBroadcast && iflags & QNetworkInterface::IsRunning) {
            newstatus = KNetworkManager::ConnectedStatus;
            break;
        }
    }

    if (d->status != newstatus) {
        d->status = newstatus;
        kDebug() << "Status changed to" << newstatus;
        emit statusChanged(newstatus);
    }
}

#include "moc_knetworkmanager.cpp"
