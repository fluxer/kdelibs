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
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDBusMetaType>
#include <QTimerEvent>
#include <QNetworkInterface>

// for reference:
// https://developer-old.gnome.org/NetworkManager/stable/gdbus-org.freedesktop.NetworkManager.html
// https://git.kernel.org/pub/scm/network/connman/connman.git/tree/doc/overview-api.txt
// https://git.kernel.org/pub/scm/network/connman/connman.git/tree/doc/manager-api.txt
// https://www.freedesktop.org/software/systemd/man/org.freedesktop.network1.html

typedef QMap<QString,QVariant> ConnmanPropertiesType;

struct ConnmanServiceType
{
    QDBusObjectPath service_object;
    QVariantMap service_dict;
};
Q_DECLARE_METATYPE(ConnmanServiceType);
Q_DECLARE_METATYPE(QList<ConnmanServiceType>);

QDBusArgument& operator<<(QDBusArgument &argument, const ConnmanServiceType &connmanservice)
{
    argument.beginStructure();
    argument << connmanservice.service_object;
    argument << connmanservice.service_dict;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument &argument, ConnmanServiceType &connmanservice)
{
    argument.beginStructure();
    argument >> connmanservice.service_object;
    argument >> connmanservice.service_dict;
    argument.endStructure();
    return argument;
}


class KNetworkManagerPrivate : public QObject
{
    Q_OBJECT
public:
    KNetworkManagerPrivate(KNetworkManager *parent);
    ~KNetworkManagerPrivate();

    KNetworkManager::KNetworkStatus status() const;
    bool enable(const bool enable);

protected:
    // QObject reimplementation
    void timerEvent(QTimerEvent *event) final;

private Q_SLOTS:
    void nmStateChanged(const uint nmstate);
    void cmStateChanged(const QString &cmname, const QDBusVariant &cmvalue);

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
        qDBusRegisterMetaType<ConnmanServiceType>();
        qDBusRegisterMetaType<QList<ConnmanServiceType>>();
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
                result = KNetworkManager::IntermediateStatus;
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
            result = KNetworkManager::IntermediateStatus;
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
            result = KNetworkManager::IntermediateStatus;
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

bool KNetworkManagerPrivate::enable(const bool enable)
{
    if (m_nm.isValid()) {
        bool result = false;
        QDBusReply<void> nmenablereply = m_nm.call("Enable", enable);
        result = nmenablereply.isValid();
        if (!result) {
            kWarning() << "Invalid org.freedesktop.NetworkManager reply" << nmenablereply.error();
            return result;
        }
        if (enable) {
            kDebug() << "Done enabling org.freedesktop.NetworkManager connections" << result;
        } else {
            kDebug() << "Done disabling org.freedesktop.NetworkManager connections" << result;
        }
        return result;
    }

    if (m_cm.isValid()) {
        bool result = false;
        QDBusReply<QList<ConnmanServiceType>> cmservicesreply = m_cm.call("GetServices");
        if (!cmservicesreply.isValid()) {
            kWarning() << "Invalid net.connman reply" << cmservicesreply.error();
            return result;
        }
        const QList<ConnmanServiceType> cmservicesvalue = cmservicesreply.value();
        foreach (const ConnmanServiceType &cmservice, cmservicesvalue) {
            const QString cmservicename = cmservice.service_dict.value(QLatin1String("Name")).toString();
            QDBusInterface cmserviceinterface(
                "net.connman", cmservice.service_object.path(), "net.connman.Service",
                QDBusConnection::systemBus()
            );
            QDBusReply<void> cmservicereply;
            if (enable) {
                kDebug() << "Enabling net.connman service" << cmservicename;
                cmservicereply = cmserviceinterface.call("Connect");
            } else {
                kDebug() << "Disabling net.connman service" << cmservicename;
                cmservicereply = cmserviceinterface.call("Disconnect");
            }
            result |= cmservicereply.isValid();
            if (!result) {
                kWarning() << "Invalid net.connman.Service reply" << cmservicereply.error();
            }
        }
        if (enable) {
            kDebug() << "Done enabling net.connman services" << result;
        } else {
            kDebug() << "Done disabling net.connman services" << result;
        }
        return result;
    }

    kDebug() << "Connection management not supported";
    return false;
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


KNetworkManager::KNetworkManager(QObject *parent)
    : QObject(parent),
    d(new KNetworkManagerPrivate(this))
{
#if 0
    qDebug() << Q_FUNC_INFO << isSupported() << enable(false) << enable(true);
#endif
}

KNetworkManager::~KNetworkManager()
{
    delete d;
}

KNetworkManager::KNetworkStatus KNetworkManager::status() const
{
    return d->status();
}

bool KNetworkManager::enable(const bool enable)
{
    return d->enable(enable);
}

bool KNetworkManager::isSupported()
{
    QDBusConnection dbusconnection = QDBusConnection::systemBus();
    QDBusConnectionInterface* dbusinterface = dbusconnection.interface();
    if (!dbusinterface) {
        kDebug() << "Null system D-Bus connection interface";
        return false;
    }
    QDBusReply<bool> dbusreply = dbusinterface->isServiceRegistered("org.freedesktop.NetworkManager");
    bool result = dbusreply.value();
    if (!result) {
        dbusreply = dbusinterface->isServiceRegistered("net.connman");
        result = dbusreply.value();
    }
    return result;
}

#include "moc_knetworkmanager.cpp"
#include "knetworkmanager.moc"
