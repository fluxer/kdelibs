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

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>
#include <QTimerEvent>
#include <QNetworkInterface>

// for reference:
// https://developer-old.gnome.org/NetworkManager/stable/gdbus-org.freedesktop.NetworkManager.html
// https://git.kernel.org/pub/scm/network/connman/connman.git/tree/doc/overview-api.txt
// https://git.kernel.org/pub/scm/network/connman/connman.git/tree/doc/manager-api.txt
// https://www.freedesktop.org/software/systemd/man/org.freedesktop.network1.html

typedef QMap<QString,QVariant> ConnmanPropertiesType;

class KNetworkManagerPrivate : public QObject
{
    Q_OBJECT
public:
    KNetworkManagerPrivate(KNetworkManager *parent);
    ~KNetworkManagerPrivate();

    KNetworkManager::KNetworkStatus status() const;
    
protected:
    // QObject reimplementation
    void timerEvent(QTimerEvent *event) final;

private Q_SLOTS:
    void nmStateChanged(const uint nmstate);
    void cmStateChanged(const QString &cmname, const QDBusVariant &cmvalue);
    void n1StateChanged(const QString &n1name, const QDBusVariant &n1value);

private:
    void emitSignals();

    QDBusInterface m_nm;
    QDBusInterface m_cm;
    QDBusInterface m_n1;
    int m_timerid;
    KNetworkManager::KNetworkStatus m_status;
};

KNetworkManagerPrivate::KNetworkManagerPrivate(KNetworkManager *parent)
    : QObject(parent),
    m_nm("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus()),
    m_cm("net.connman", "/", "net.connman.Manager", QDBusConnection::systemBus()),
    m_n1("org.freedesktop.network1", "/org/freedesktop/network1", "org.freedesktop.network1.Manager", QDBusConnection::systemBus()),
    m_timerid(0),
    m_status(KNetworkManager::UnknownStatus)
{
    m_status = status();
    if (m_nm.isValid()) {
        kDebug() << "Using org.freedesktop.NetworkManager";
        connect(
            &m_nm, SIGNAL(StateChanged(uint)),
            this, SLOT(nmStateChanged(uint))
        );
    } else if (m_cm.isValid()) {
        kDebug() << "Using net.connman";
        connect(
            &m_cm, SIGNAL(PropertyChanged(QString,QDBusVariant)),
            this, SLOT(cmStateChanged(QString,QDBusVariant))
        );
    } else if (m_n1.isValid()) {
        kDebug() << "Using org.freedesktop.network1";
        m_timerid = startTimer(2000);
    } else {
        kDebug() << "Using fallback";
        m_timerid = startTimer(2000);
    }
}

KNetworkManagerPrivate::~KNetworkManagerPrivate()
{
    if (m_timerid > 0) {
        killTimer(m_timerid);
    }
}

KNetworkManager::KNetworkStatus KNetworkManagerPrivate::status() const
{
    if (m_nm.isValid()) {
        QDBusReply<uint> nmreply = m_nm.call("state");
        const uint nmstate = nmreply.value();
        KNetworkManager::KNetworkStatus result = KNetworkManager::UnknownStatus;
        switch (nmstate) {
            case 0:
            case 10:
            case 20: {
                result = KNetworkManager::DisconnectedStatus;
                break;
            }
            case 50:
            case 60:
            case 70: {
                result = KNetworkManager::ConnectedStatus;
                break;
            }
            case 30:
            case 40: {
                // connecting/disconnecting
                break;
            }
            default: {
                kWarning() << "Unknown org.freedesktop.NetworkManager state" << nmstate;
                break;
            }
        }
        return result;
    }

    if (m_cm.isValid()) {
        KNetworkManager::KNetworkStatus result = KNetworkManager::UnknownStatus;
        QDBusReply<ConnmanPropertiesType> cmreply = m_cm.call("GetProperties");
        const ConnmanPropertiesType cmproperties = cmreply.value();
        const QString cmstate = cmproperties.value("State").toString();
        if (cmstate == QLatin1String("ready") || cmstate == QLatin1String("online")) {
            result = KNetworkManager::ConnectedStatus;
        } else if (cmstate == QLatin1String("association") || cmstate == QLatin1String("configuration")
            || cmstate == QLatin1String("disconnect")) {
            // connecting/disconnecting
            result = KNetworkManager::UnknownStatus;
        } else if (cmstate == QLatin1String("offline") || cmstate == QLatin1String("idle")) {
            result = KNetworkManager::DisconnectedStatus;
        } else {
            kWarning() << "Unknown net.connman state" << cmstate;
        }
        return result;
    }

    if (m_n1.isValid()) {
        KNetworkManager::KNetworkStatus result = KNetworkManager::UnknownStatus;
        const QString n1state = m_n1.property("OperationalState").toString();
        if (n1state == QLatin1String("routable")) {
            result = KNetworkManager::ConnectedStatus;
        } else if (n1state == QLatin1String("no-carrier")) {
            result = KNetworkManager::DisconnectedStatus;
        } else if (n1state == QLatin1String("carrier") || n1state == QLatin1String("degraded")) {
            // connecting/disconnecting
            result = KNetworkManager::UnknownStatus;
        } else {
            kWarning() << "Unknown org.freedesktop.network1 state" << n1state;
        }
        return result;
    }

    KNetworkManager::KNetworkStatus result = KNetworkManager::DisconnectedStatus;
    foreach (const QNetworkInterface &iface, QNetworkInterface::allInterfaces()) {
        const QNetworkInterface::InterfaceFlags iflags = iface.flags();
        if (iflags & QNetworkInterface::CanBroadcast && iflags & QNetworkInterface::IsRunning) {
            result = KNetworkManager::ConnectedStatus;
            break;
        }
    }
    return result;
}

void KNetworkManagerPrivate::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerid) {
        emitSignals();
        event->accept();
    } else {
        event->ignore();
    }
}

void KNetworkManagerPrivate::emitSignals()
{
    const int oldstatus = m_status;
    m_status = status();

    kDebug() << "Old status" << oldstatus << "new status" << m_status;

    if (oldstatus != m_status) {
        KNetworkManager* knetworkmanager = qobject_cast<KNetworkManager*>(parent());
        Q_ASSERT(knetworkmanager);
        emit knetworkmanager->statusChanged(m_status);
    }
}

void KNetworkManagerPrivate::nmStateChanged(const uint nmstate)
{
    Q_UNUSED(nmstate);
    emitSignals();
}

void KNetworkManagerPrivate::cmStateChanged(const QString &cmname, const QDBusVariant &cmvalue)
{
    Q_UNUSED(cmname);
    Q_UNUSED(cmvalue);
    emitSignals();
}

void KNetworkManagerPrivate::n1StateChanged(const QString &n1name, const QDBusVariant &n1value)
{
    Q_UNUSED(n1name);
    Q_UNUSED(n1value);
    emitSignals();
}


KNetworkManager::KNetworkManager(QObject *parent)
    : QObject(parent),
    d(new KNetworkManagerPrivate(this))
{
}

KNetworkManager::~KNetworkManager()
{
    delete d;
}

KNetworkManager::KNetworkStatus KNetworkManager::status() const
{
    return d->status();
}

bool KNetworkManager::isSupported()
{
    // NOTE: to be used for connections management capability (possibly)
    return true;
}

#include "moc_knetworkmanager.cpp"
#include "knetworkmanager.moc"
