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

#include "kdnssd.h"
#include "kurl.h"
#include "kdebug.h"

#include <QCoreApplication>
#include <QThread>

#if defined(HAVE_AVAHI)
#  include <avahi-client/client.h>
#  include <avahi-client/publish.h>
#  include <avahi-client/lookup.h>
#  include <avahi-common/simple-watch.h>
#  include <avahi-common/error.h>
#endif

class KDNSSDPrivate : public QObject
{
    Q_OBJECT
public:
    KDNSSDPrivate(QObject *parent);
    ~KDNSSDPrivate();

    bool publishService(const QByteArray &servicetype, const uint serviceport, const QString &servicename);
    bool unpublishService();

    void startBrowse(const QByteArray &servicetype);
    QList<KDNSSDService> services();

#if defined(HAVE_AVAHI)
    static void clientCallback(AvahiClient *avahiclient, AvahiClientState avahistate, void *userdata);
    static void groupCallback(AvahiEntryGroup *avahigroup, AvahiEntryGroupState avahistate, void *userdata);
    static void browseCallback(AvahiServiceBrowser *avahibrowser, AvahiIfIndex avahiinterface,
                               AvahiProtocol avahiprotocol, AvahiBrowserEvent avahievent,
                               const char *avahiname, const char *avahitype, const char *avahidomain,
                               AvahiLookupResultFlags avahiflags,
                               void* userdata);
    static void resolveCallback(AvahiServiceResolver *avahiresolver, AvahiIfIndex avahiinterface,
                                AvahiProtocol avahiprotocol, AvahiResolverEvent avahievent,
                                const char *avahiname, const char *avahitype, const char *avahidomain, const char *avahihost_name,
                                const AvahiAddress *avahiaddress, uint16_t avahiport,
                                AvahiStringList *avahitxt,
                                AvahiLookupResultFlags avahiflags,
                                void* userdata);
    static void serviceCallback(AvahiServiceTypeBrowser *avahiservice,
                                AvahiIfIndex avahiinterface,
                                AvahiProtocol avahiprotocol,
                                AvahiBrowserEvent avahievent,
                                const char *avahitype,
                                const char *avahidomain,
                                AvahiLookupResultFlags avahiflags,
                                void *userdata);

private:
    int m_poll;
    AvahiSimplePoll *m_avahipoll;
    AvahiClient *m_avahiclient;
    AvahiEntryGroup *m_avahigroup;
    QList<KDNSSDService> m_services;
    QList<QByteArray> m_servicetypes;
#endif // HAVE_AVAHI
};

KDNSSDPrivate::KDNSSDPrivate(QObject *parent)
    : QObject(parent)
#if defined(HAVE_AVAHI)
    , m_poll(0),
    m_avahipoll(nullptr),
    m_avahiclient(nullptr),
    m_avahigroup(nullptr)
#endif
{
#if defined(HAVE_AVAHI)
    m_avahipoll = avahi_simple_poll_new();
    if (!m_avahipoll) {
        kWarning() << "Could not create Avahi poll";
        return;
    }

    int avahierror = 0;
    m_avahiclient = avahi_client_new(
        avahi_simple_poll_get(m_avahipoll), AVAHI_CLIENT_NO_FAIL,
        KDNSSDPrivate::clientCallback, this,
        &avahierror
    );
    if (!m_avahiclient) {
        kWarning() << "Could not create Avahi client" << avahi_strerror(avahierror);
        return;
    }
#endif // HAVE_AVAHI
}

KDNSSDPrivate::~KDNSSDPrivate()
{
#if defined(HAVE_AVAHI)
    m_poll = 0;

    if (m_avahigroup) {
        avahi_entry_group_reset(m_avahigroup);
    }

    if (m_avahiclient) {
        avahi_client_free(m_avahiclient);
    }

    if (m_avahipoll) {
        avahi_simple_poll_free(m_avahipoll);
    }
#endif // HAVE_AVAHI
}

bool KDNSSDPrivate::publishService(const QByteArray &servicetype, const uint serviceport, const QString &servicename)
{
#if defined(HAVE_AVAHI)
    if (m_avahigroup) {
        avahi_entry_group_reset(m_avahigroup);
    }
    m_avahigroup = avahi_entry_group_new(m_avahiclient, KDNSSDPrivate::groupCallback, this);
    if (!m_avahigroup) {
        kWarning() << "Could not create Avahi group";
        return false;
    }
    const QByteArray servicenamebytes = servicename.toUtf8();
    // qDebug() << Q_FUNC_INFO << servicenamebytes << servicetype;
    int avahiresult = avahi_entry_group_add_service(
        m_avahigroup,
        AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
        AvahiPublishFlags(0),
        servicenamebytes.constData(), servicetype.constData(), NULL, NULL, serviceport,
        NULL
    );
    if (avahiresult < 0) {
        kWarning() << "Could not add Avahi service to group" << avahi_strerror(avahiresult);
        return false;
    }
    avahiresult = avahi_entry_group_commit(m_avahigroup);
    return (avahiresult >= 0);
#else
    return false;
#endif
}

bool KDNSSDPrivate::unpublishService()
{
#if defined(HAVE_AVAHI)
    if (!m_avahigroup) {
        return true;
    }
    const int avahiresult = avahi_entry_group_reset(m_avahigroup);
    if (avahiresult < 0) {
        kWarning() << "Could not reset Avahi service group" << avahi_strerror(avahiresult);
        return false;
    }
    return true;
#else
    return false;
#endif
}

void KDNSSDPrivate::startBrowse(const QByteArray &servicetype)
{
#if defined(HAVE_AVAHI)
    // qDebug() << Q_FUNC_INFO << servicetype;

    QList<QByteArray> servicetypes;
    if (servicetype.isEmpty()) {
        AvahiServiceTypeBrowser* avahiservice = avahi_service_type_browser_new(
            m_avahiclient,
            AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL,
            AvahiLookupFlags(0),
            KDNSSDPrivate::serviceCallback, this
        );
        if (!avahiservice) {
            kWarning() << "Could not create Avahi service type browser" << avahi_strerror(avahi_client_errno(m_avahiclient));
            return;
        }

        m_poll++;
        m_servicetypes.clear();
        while (m_poll) {
            // qDebug() << Q_FUNC_INFO << m_poll;
            avahi_simple_poll_iterate(m_avahipoll, 0);
        }

        avahi_service_type_browser_free(avahiservice);

        servicetypes = m_servicetypes;
    } else {
        servicetypes.append(servicetype);
    }

    m_services.clear();
    foreach (const QByteArray &servicetypeit, servicetypes) {
        AvahiServiceBrowser *avahibrowser = avahi_service_browser_new(
            m_avahiclient,
            AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, servicetypeit.constData(), NULL,
            AvahiLookupFlags(0),
            KDNSSDPrivate::browseCallback, this
        );
        if (!avahibrowser) {
            kWarning() << "Could not create Avahi browser" << avahi_strerror(avahi_client_errno(m_avahiclient));
            return;
        }

        m_poll++;
        while (m_poll) {
            // qDebug() << Q_FUNC_INFO << m_poll;
            avahi_simple_poll_iterate(m_avahipoll, 0);
        }

        avahi_service_browser_free(avahibrowser);
    }

    // TODO: filter IPv6 if IPv4 addresses are available or vice-versa?
#endif // HAVE_AVAHI
}

QList<KDNSSDService> KDNSSDPrivate::services()
{
#if defined(HAVE_AVAHI)
    return m_services;
#else
    static const QList<KDNSSDService> result;
    return result;
#endif
}

#if defined(HAVE_AVAHI)
void KDNSSDPrivate::groupCallback(AvahiEntryGroup *avahigroup, AvahiEntryGroupState avahistate, void *userdata)
{
    // qDebug() << Q_FUNC_INFO << avahigroup << avahistate << userdata;

    if (avahistate == AVAHI_ENTRY_GROUP_FAILURE) {
        kWarning() << avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(avahigroup)));
    }
}

void KDNSSDPrivate::clientCallback(AvahiClient *avahiclient, AvahiClientState avahistate, void *userdata)
{
    // qDebug() << Q_FUNC_INFO << avahistate << userdata;

    if (avahistate == AVAHI_CLIENT_FAILURE) {
        kWarning() << avahi_strerror(avahi_client_errno(avahiclient));
        // avahi_simple_poll_quit(m_avahiclient);
    }
}

void KDNSSDPrivate::browseCallback(AvahiServiceBrowser *avahibrowser, AvahiIfIndex avahiinterface,
                                   AvahiProtocol avahiprotocol, AvahiBrowserEvent avahievent,
                                   const char *avahiname, const char *avahitype, const char *avahidomain,
                                   AvahiLookupResultFlags avahiflags,
                                   void* userdata)
{
    // qDebug() << Q_FUNC_INFO << avahievent << avahiname << avahitype << avahidomain << userdata;

    KDNSSDPrivate *kdnssdprivate = static_cast<KDNSSDPrivate*>(userdata);
    AvahiClient *avahiclient = avahi_service_browser_get_client(avahibrowser);
    switch (avahievent) {
        case AVAHI_BROWSER_NEW: {
            kDebug() << "New service" << avahiname << avahitype << avahidomain;
            kdnssdprivate->m_poll++;
            AvahiServiceResolver *avahiresolver = avahi_service_resolver_new(
                avahiclient,
                avahiinterface, avahiprotocol,
                avahiname, avahitype, avahidomain,
                AVAHI_PROTO_UNSPEC, AvahiLookupFlags(0),
                KDNSSDPrivate::resolveCallback,
                userdata
            );
            if (!avahiresolver) {
                kWarning() << avahi_strerror(avahi_client_errno(avahiclient));
            }
            break;
        }
        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED: {
            kDebug() << "Done browsing";
            kdnssdprivate->m_poll--;
            KDNSSD *kdnssd= qobject_cast<KDNSSD*>(kdnssdprivate->parent());
            emit kdnssd->finished();
            break;
        }
        case AVAHI_BROWSER_FAILURE: {
            kWarning() << avahi_strerror(avahi_client_errno(avahiclient));
            kdnssdprivate->m_poll = 0;
            break;
        }
    }
}

void KDNSSDPrivate::resolveCallback(AvahiServiceResolver *avahiresolver, AvahiIfIndex avahiinterface,
                                    AvahiProtocol avahiprotocol, AvahiResolverEvent avahievent,
                                    const char *avahiname, const char *avahitype, const char *avahidomain, const char *avahihost_name,
                                    const AvahiAddress *avahiaddress, uint16_t avahiport,
                                    AvahiStringList *avahitxt,
                                    AvahiLookupResultFlags avahiflags,
                                    void* userdata)
{
    // qDebug() << Q_FUNC_INFO << avahievent << avahiname << avahitype << avahidomain << userdata;

    KDNSSDPrivate *kdnssdprivate = static_cast<KDNSSDPrivate*>(userdata);
    switch (avahievent) {
        case AVAHI_RESOLVER_FOUND: {
            kDebug() << "Resolved service" << avahiname << avahitype << avahidomain;
            char avahiaddressbuff[AVAHI_ADDRESS_STR_MAX];
            ::memset(avahiaddressbuff, 0, sizeof(avahiaddressbuff) * sizeof(char));
            avahi_address_snprint(avahiaddressbuff, sizeof(avahiaddressbuff), avahiaddress);

            QString kdnssdserviceprotocol = QString::fromLatin1(avahitype);
            kdnssdserviceprotocol = kdnssdserviceprotocol.mid(1);
            const int dotindex = kdnssdserviceprotocol.indexOf(QLatin1Char('.'));
            kdnssdserviceprotocol = kdnssdserviceprotocol.left(dotindex);
            KUrl kdnssdserviceurl;
            kdnssdserviceurl.setProtocol(kdnssdserviceprotocol);
            kdnssdserviceurl.setHost(QString::fromLatin1(avahiaddressbuff));
            kdnssdserviceurl.setPort(avahiport);

            KDNSSDService kdnssdservice;
            kdnssdservice.name = QString::fromUtf8(avahiname);
            kdnssdservice.type = QByteArray(avahitype);
            kdnssdservice.domain = QString::fromUtf8(avahidomain);
            kdnssdservice.hostname = QString::fromUtf8(avahihost_name);
            kdnssdservice.url = kdnssdserviceurl.prettyUrl();
            kdnssdservice.port = avahiport;
            kdnssdprivate->m_services.append(kdnssdservice);
            break;
        }
        case AVAHI_RESOLVER_FAILURE: {
            kWarning() << avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(avahiresolver)));
            break;
        }
    }

    kdnssdprivate->m_poll--;
    avahi_service_resolver_free(avahiresolver);
}

void KDNSSDPrivate::serviceCallback(AvahiServiceTypeBrowser *avahiservice,
                                    AvahiIfIndex avahiinterface,
                                    AvahiProtocol avahiprotocol,
                                    AvahiBrowserEvent avahievent,
                                    const char *avahitype,
                                    const char *avahidomain,
                                    AvahiLookupResultFlags avahiflags,
                                    void *userdata)
{
    // qDebug() << Q_FUNC_INFO << avahievent << avahitype << avahidomain << userdata;

    KDNSSDPrivate *kdnssdprivate = static_cast<KDNSSDPrivate*>(userdata);
    AvahiClient *avahiclient = avahi_service_type_browser_get_client(avahiservice);
    switch (avahievent) {
        case AVAHI_BROWSER_NEW: {
            kDebug() << "New service type" << avahitype << avahidomain;
            const QByteArray kdnsservicetype(avahitype);
            if (!kdnssdprivate->m_servicetypes.contains(kdnsservicetype)) {
                kdnssdprivate->m_servicetypes.append(kdnsservicetype);
            }
            break;
        }
        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED: {
            kDebug() << "Done browsing service types";
            kdnssdprivate->m_poll--;
            break;
        }
        case AVAHI_BROWSER_FAILURE: {
            kWarning() << avahi_strerror(avahi_client_errno(avahiclient));
            kdnssdprivate->m_poll = 0;
            break;
        }
    }
}
#endif // HAVE_AVAHI

KDNSSD::KDNSSD(QObject *parent)
    : QObject(parent),
    d(new KDNSSDPrivate(this))
{
}

KDNSSD::~KDNSSD()
{
    delete d;
}

bool KDNSSD::publishService(const QByteArray &servicetype, const uint serviceport, const QString &servicename)
{
#if defined(HAVE_AVAHI)
    return d->publishService(servicetype, serviceport, servicename);
#else
    return false;
#endif
}

bool KDNSSD::unpublishService()
{
    return d->unpublishService();
}

QList<KDNSSDService> KDNSSD::services()
{
    return d->services();
}

void KDNSSD::startBrowse(const QByteArray &servicetype)
{
    d->startBrowse(servicetype);
}

#include "kdnssd.moc"
