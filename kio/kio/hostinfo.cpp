/*  This file is part of the KDE libraries
    Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>

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

#include "hostinfo_p.h"
#include "kglobal.h"
#include "kdebug.h"

#include <QtCore/QCache>
#include <QtCore/QMetaType>
#include <QtCore/QElapsedTimer>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QTime>
#include <QtCore/QMutex>
#include <QtCore/QCoreApplication>

#include <unistd.h>

// 100 cached objects
#define HOSTINFO_CACHE_SIZE 100
// every 3sec check if object should be removed from the cache
#define HOSTINFO_EXPIRE_CHECK 3000
// any object being in the cache longer than 30sec will be removed
#define HOSTINFO_EXPIRE_AFTER 30
// if there is no QCoreApplication/QApplication sleep for 1sec instead of processing events
#define HOSTINFO_SLEEP 1000

namespace KIO
{

typedef QPair<QHostInfo, QTime> HostInfoPair;

class HostInfoAgentPrivate : public QObject
{
    Q_OBJECT
public:
    HostInfoAgentPrivate();

    QHostInfo lookupHost(const QString &hostName, const unsigned long timeout);

protected:
    // reimplementation
    void timerEvent(QTimerEvent *event) final;

private:
    int m_expiretimerid;
    QList<int> m_lookups;
    QCache<QString, HostInfoPair> m_dnsCache;
    QMutex m_mutex;

private Q_SLOTS:
    void lookupFinished(const QHostInfo &hostInfo);
};

K_GLOBAL_STATIC(HostInfoAgentPrivate, hostInfoAgentPrivate)

QHostInfo HostInfo::lookupHost(const QString &hostName, unsigned long timeout)
{
    return hostInfoAgentPrivate->lookupHost(hostName, timeout);
}

HostInfoAgentPrivate::HostInfoAgentPrivate()
    : m_dnsCache(HOSTINFO_CACHE_SIZE)
{
    qRegisterMetaType<QHostInfo>("QHostInfo");
    m_expiretimerid = startTimer(HOSTINFO_EXPIRE_CHECK);
}

QHostInfo HostInfoAgentPrivate::lookupHost(const QString &hostName, unsigned long timeout)
{
    // Do not perform a reverse lookup here...
    QHostAddress address(hostName);
    QHostInfo hostInfo;
    if (!address.isNull()) {
        QList<QHostAddress> addressList;
        addressList << address;
        hostInfo.setAddresses(addressList);
        return hostInfo;
    }

    // Look up the name in the DNS cache...
    const HostInfoPair* pair = m_dnsCache.object(hostName);
    if (pair) {
        hostInfo = pair->first;
    }
    if (!hostInfo.hostName().isEmpty() && hostInfo.error() == QHostInfo::NoError) {
        return hostInfo;
    }

    // Failing all of the above, do the lookup...
    kDebug() << "Name look up for" << hostName;
    QElapsedTimer lookupTimer;
    lookupTimer.start();
    const int lookupId = QHostInfo::lookupHost(hostName, this, SLOT(lookupFinished(QHostInfo)));
    m_lookups.append(lookupId);
    while (m_lookups.contains(lookupId)) {
        if (lookupTimer.elapsed() > timeout) {
            kDebug() << "Name look up timed out for" << hostName;
            m_lookups.removeAll(lookupId);
            QHostInfo::abortHostLookup(lookupId);
            return QHostInfo();
        }

        if (qApp) {
            qApp->processEvents();
        } else {
            ::usleep(HOSTINFO_SLEEP);
        }
    }

    kDebug() << "Name look up succeeded for" << hostName;
    return m_dnsCache.object(hostName)->first;
}

void HostInfoAgentPrivate::lookupFinished(const QHostInfo &hostInfo)
{
    m_lookups.removeAll(hostInfo.lookupId());
    if (hostInfo.hostName().isEmpty() || hostInfo.error() != QHostInfo::NoError) {
        return;
    }
    QTime expiration = QTime::currentTime();
    expiration.addSecs(HOSTINFO_EXPIRE_AFTER);
    m_dnsCache.insert(hostInfo.hostName(), new HostInfoPair(hostInfo, expiration));
}

void HostInfoAgentPrivate::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_expiretimerid) {
        kDebug() << "Checking cache for expired entries";
        QMutexLocker lock(&m_mutex);
        const QTime current = QTime::currentTime();
        foreach (const QString &hostName, m_dnsCache.keys()) {
            const HostInfoPair* pair = m_dnsCache.object(hostName);
            if (pair && current >= pair->second) {
                kDebug() << "Expiring look up for" << hostName;
                m_dnsCache.remove(hostName);
            }
        }
        event->accept();
        return;
    }
    event->ignore();
}

} // namespace KIO

#include "hostinfo.moc"
