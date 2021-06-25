/*
Copyright 2008 Roland Harnau <tau@gmx.eu>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hostinfo_p.h"

#include <kglobal.h>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QCache>
#include <QtCore/QMetaType>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QThread>
#include <QtCore/QSemaphore>
#include <QtCore/QSharedPointer>
#include <QtNetwork/QHostInfo>
#include "kdebug.h"

#define HOSTINFO_TTL 300
#define HOSTINFO_CACHE_SIZE 100

namespace KIO
{
    class HostInfoAgentPrivate : public QObject
    {
        Q_OBJECT
    public:
        HostInfoAgentPrivate();
        virtual ~HostInfoAgentPrivate() {};
        QHostInfo lookupCachedHostInfoFor(const QString& hostName);
        void cacheLookup(const QHostInfo&);
    private:
        class Result;
        class Query;

        QHash<QString, Query*> openQueries;
        QCache<QString, QPair<QHostInfo, QTime> > dnsCache;
    };

    class HostInfoAgentPrivate::Result : public QObject
    {
        Q_OBJECT
    signals:
        void result(QHostInfo);
    private:
        friend class HostInfoAgentPrivate;
    };

    class HostInfoAgentPrivate::Query : public QObject
    {
        Q_OBJECT
    public:
        Query(): m_hostName()
        {
        }
        void start(const QString& hostName)
        {
            m_hostName = hostName;
            QHostInfo::lookupHost(hostName, this, SLOT(relayFinished(QHostInfo)));
        }
        QString hostName() const
        {
            return m_hostName;
        }
    signals:
        void result(QHostInfo);
    private slots:
        void relayFinished(const QHostInfo &hostinfo)
        {
            emit result(hostinfo);
        }
    private:
        QString m_hostName;
    };

    class NameLookupThreadRequest
    {
    public:
        NameLookupThreadRequest(const QString& hostName) : m_hostName(hostName)
        {
        }

        QSemaphore *semaphore()
        {
            return &m_semaphore;
        }

        QHostInfo result() const
        {
            return m_hostInfo;
        }

        void setResult(const QHostInfo& hostInfo)
        {
            m_hostInfo = hostInfo;
        }

        QString hostName() const
        {
            return m_hostName;
        }

        int lookupId() const
        {
            return m_lookupId;
        }

        void setLookupId(int id)
        {
            m_lookupId = id;
        }

    private:
        Q_DISABLE_COPY(NameLookupThreadRequest)
        QString m_hostName;
        QSemaphore m_semaphore;
        QHostInfo m_hostInfo;
        int m_lookupId;
    };

    class NameLookUpThreadWorker : public QObject
    {
        Q_OBJECT
    public slots:
        void lookupHost(const QSharedPointer<NameLookupThreadRequest>& request)
        {
            const QString hostName = request->hostName();
            const int lookupId = QHostInfo::lookupHost(hostName, this, SLOT(lookupFinished(QHostInfo)));
            request->setLookupId(lookupId);
            m_lookups.insert(lookupId, request);
        }

        void abortLookup(const QSharedPointer<NameLookupThreadRequest>& request)
        {
            QHostInfo::abortHostLookup(request->lookupId());
            m_lookups.remove(request->lookupId());
        }

        void lookupFinished(const QHostInfo &hostInfo)
        {
            QMap<int, QSharedPointer<NameLookupThreadRequest> >::iterator it = m_lookups.find(hostInfo.lookupId());
            if (it != m_lookups.end()) {
                (*it)->setResult(hostInfo);
                (*it)->semaphore()->release();
                m_lookups.erase(it);
            }
        }

    private:
        QMap<int, QSharedPointer<NameLookupThreadRequest> > m_lookups;
    };

    class NameLookUpThread : public QThread
    {
        Q_OBJECT
    public:
        NameLookUpThread () : m_worker(0)
        {
            qRegisterMetaType< QSharedPointer<NameLookupThreadRequest> > ("QSharedPointer<NameLookupThreadRequest>");
            start();
        }

        ~NameLookUpThread ()
        {
            quit();
            wait();
        }

        NameLookUpThreadWorker *worker()
        {
            return m_worker;
        }

        QSemaphore *semaphore()
        {
            return &m_semaphore;
        }

        void run()
        {
            NameLookUpThreadWorker worker;
            m_worker = &worker;
            m_semaphore.release();
            exec();
        }

    private:
        NameLookUpThreadWorker *m_worker;
        QSemaphore m_semaphore;
    };
}

using namespace KIO;

K_GLOBAL_STATIC(HostInfoAgentPrivate, hostInfoAgentPrivate)
K_GLOBAL_STATIC(NameLookUpThread, nameLookUpThread)

QHostInfo HostInfo::lookupHost(const QString& hostName, unsigned long timeout)
{
    // Do not perform a reverse lookup here...
    QHostAddress address (hostName);
    QHostInfo hostInfo;
    if (!address.isNull()) {
        QList<QHostAddress> addressList;
        addressList << address;
        hostInfo.setAddresses(addressList);
        return hostInfo;
    }

    // Look up the name in the KIO/KHTML DNS cache...
    hostInfo = HostInfo::lookupCachedHostInfoFor(hostName);
    if (!hostInfo.hostName().isEmpty() && hostInfo.error() == QHostInfo::NoError) {
        return hostInfo;
    }

    // Failing all of the above, do the lookup...
    QSharedPointer<NameLookupThreadRequest> request = QSharedPointer<NameLookupThreadRequest>(new NameLookupThreadRequest(hostName));
    nameLookUpThread->semaphore()->acquire();
    nameLookUpThread->semaphore()->release();
    QMetaObject::invokeMethod(nameLookUpThread->worker(), "lookupHost", Qt::QueuedConnection, Q_ARG(QSharedPointer<NameLookupThreadRequest>, request));
    if (request->semaphore()->tryAcquire(1, timeout)) {
        hostInfo = request->result();
        if (!hostInfo.hostName().isEmpty() && hostInfo.error() == QHostInfo::NoError) {
            HostInfo::cacheLookup(hostInfo); // cache the look up...
        }
    } else {
        QMetaObject::invokeMethod(nameLookUpThread->worker(), "abortLookup", Qt::QueuedConnection, Q_ARG(QSharedPointer<NameLookupThreadRequest>, request));
    }

    //kDebug(7022) << "Name look up succeeded for" << hostName;
    return hostInfo;
}

QHostInfo HostInfo::lookupCachedHostInfoFor(const QString& hostName)
{
    return hostInfoAgentPrivate->lookupCachedHostInfoFor(hostName);
}

void HostInfo::cacheLookup(const QHostInfo& info)
{
    hostInfoAgentPrivate->cacheLookup(info);
}

HostInfoAgentPrivate::HostInfoAgentPrivate()
    : dnsCache(HOSTINFO_CACHE_SIZE)
{
    qRegisterMetaType<QHostInfo>("QHostInfo");
}

QHostInfo HostInfoAgentPrivate::lookupCachedHostInfoFor(const QString& hostName)
{
    QPair<QHostInfo, QTime>* info = dnsCache.object(hostName);
    if (info && info->second.addSecs(HOSTINFO_TTL) >= QTime::currentTime())
        return info->first;

    return QHostInfo();
}

void HostInfoAgentPrivate::cacheLookup(const QHostInfo& info)
{
    if (info.hostName().isEmpty())
        return;

    if (info.error() != QHostInfo::NoError)
        return;

    dnsCache.insert(info.hostName(), new QPair<QHostInfo, QTime>(info, QTime::currentTime()));
}

#include "hostinfo.moc"
