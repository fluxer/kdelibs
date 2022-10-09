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

#if defined(HAVE_AVAHI)
#  include <avahi-client/client.h>
#  include <avahi-client/publish.h>
#  include <avahi-client/lookup.h>
#  include <avahi-common/simple-watch.h>
#  include <avahi-common/error.h>
#endif

#if defined(HAVE_AVAHI)
// NOTE: resolving to and publishing IPv4 addresses since curl, ping, etc. cannot handle URLs with
// IPv6 address as host along with port (e.g. http://[fe80::fe4d:d4ff:fe4c:5575]:7287)
static const AvahiProtocol s_avahiproto = AVAHI_PROTO_INET;

static QString getAvahiError(const int avahierror)
{
    kWarning() << avahi_strerror(avahierror);
    return QString::fromAscii(avahi_strerror(avahierror));
}

static QString getAvahiClientError(AvahiClient *avahiclient)
{
    return getAvahiError(avahi_client_errno(avahiclient));
}
#endif

class KDNSSDPrivate : public QObject
{
    Q_OBJECT
public:
    KDNSSDPrivate(QObject *parent);
    ~KDNSSDPrivate();

    bool publishService(const QByteArray &servicetype, const uint serviceport, const QString &servicename);
    bool unpublishService();

    bool startBrowse(const QByteArray &servicetype);
    QList<KDNSSDService> services() const;
    QString errorString() const;

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
                                const char *avahiname, const char *avahitype, const char *avahidomain, const char *avahihostname,
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
    int m_pollcounter;
    AvahiSimplePoll *m_avahipoll;
    AvahiClient *m_avahiclient;
    AvahiEntryGroup *m_avahigroup;
    QList<KDNSSDService> m_services;
    QList<QByteArray> m_servicetypes;
#endif // HAVE_AVAHI
private:
    QString m_errorstring;
};

KDNSSDPrivate::KDNSSDPrivate(QObject *parent)
    : QObject(parent)
#if defined(HAVE_AVAHI)
    , m_pollcounter(0),
    m_avahipoll(nullptr),
    m_avahiclient(nullptr),
    m_avahigroup(nullptr)
#else
    , m_errorstring(QString::fromLatin1("Built without Avahi"))
#endif
{
#if defined(HAVE_AVAHI)
    m_avahipoll = avahi_simple_poll_new();
    if (!m_avahipoll) {
        m_errorstring = QString::fromLatin1("Could not create Avahi poll");
        return;
    }

    int avahierror = 0;
    m_avahiclient = avahi_client_new(
        avahi_simple_poll_get(m_avahipoll), AVAHI_CLIENT_NO_FAIL,
        KDNSSDPrivate::clientCallback, this,
        &avahierror
    );
    if (!m_avahiclient) {
        m_errorstring = getAvahiError(avahierror);
        return;
    }
#endif // HAVE_AVAHI
}

KDNSSDPrivate::~KDNSSDPrivate()
{
#if defined(HAVE_AVAHI)
    m_pollcounter = 0;

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
        m_errorstring = getAvahiClientError(m_avahiclient);
        return false;
    }
    const QByteArray servicenamebytes = servicename.toUtf8();
    // qDebug() << Q_FUNC_INFO << servicenamebytes << servicetype;
    int avahiresult = avahi_entry_group_add_service(
        m_avahigroup,
        AVAHI_IF_UNSPEC, s_avahiproto,
        AvahiPublishFlags(0),
        servicenamebytes.constData(), servicetype.constData(), NULL, NULL, serviceport,
        NULL
    );
    if (avahiresult < 0) {
        m_errorstring = getAvahiError(avahiresult);
        return false;
    }
    avahiresult = avahi_entry_group_commit(m_avahigroup);
    if (avahiresult < 0) {
        m_errorstring = getAvahiError(avahiresult);
        return false;
    }
    return true;
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
        m_errorstring = getAvahiError(avahiresult);
        return false;
    }
    return true;
#else
    return false;
#endif
}

bool KDNSSDPrivate::startBrowse(const QByteArray &servicetype)
{
#if defined(HAVE_AVAHI)
    // qDebug() << Q_FUNC_INFO << servicetype;

    m_pollcounter = 0;
    QList<QByteArray> servicetypes;
    if (servicetype.isEmpty()) {
        AvahiServiceTypeBrowser* avahiservice = avahi_service_type_browser_new(
            m_avahiclient,
            AVAHI_IF_UNSPEC, s_avahiproto, NULL,
            AvahiLookupFlags(0),
            KDNSSDPrivate::serviceCallback, this
        );
        if (!avahiservice) {
            m_errorstring = getAvahiClientError(m_avahiclient);
            return false;
        }

        m_pollcounter++;
        m_servicetypes.clear();
        while (m_pollcounter) {
            // qDebug() << Q_FUNC_INFO << m_pollcounter;
            avahi_simple_poll_iterate(m_avahipoll, 0);
        }

        avahi_service_type_browser_free(avahiservice);

        servicetypes = m_servicetypes;
    } else {
        servicetypes.append(servicetype);
    }

    m_pollcounter = 0;
    m_services.clear();
    foreach (const QByteArray &servicetypeit, servicetypes) {
        AvahiServiceBrowser *avahibrowser = avahi_service_browser_new(
            m_avahiclient,
            AVAHI_IF_UNSPEC, s_avahiproto, servicetypeit.constData(), NULL,
            AvahiLookupFlags(0),
            KDNSSDPrivate::browseCallback, this
        );
        if (!avahibrowser) {
            m_errorstring = getAvahiClientError(m_avahiclient);
            return false;
        }

        m_pollcounter++;
        while (m_pollcounter) {
            // qDebug() << Q_FUNC_INFO << m_pollcounter;
            avahi_simple_poll_iterate(m_avahipoll, 0);
        }

        avahi_service_browser_free(avahibrowser);
    }
    return true;
#else
    return false;
#endif // HAVE_AVAHI
}

QList<KDNSSDService> KDNSSDPrivate::services() const
{
#if defined(HAVE_AVAHI)
    return m_services;
#else
    static const QList<KDNSSDService> result;
    return result;
#endif
}

QString KDNSSDPrivate::errorString() const
{
    return m_errorstring;
}

#if defined(HAVE_AVAHI)
void KDNSSDPrivate::groupCallback(AvahiEntryGroup *avahigroup, AvahiEntryGroupState avahistate, void *userdata)
{
    // qDebug() << Q_FUNC_INFO << avahigroup << avahistate << userdata;

    if (avahistate == AVAHI_ENTRY_GROUP_FAILURE) {
        KDNSSDPrivate *kdnssdprivate = static_cast<KDNSSDPrivate*>(userdata);
        kdnssdprivate->m_errorstring = getAvahiClientError(avahi_entry_group_get_client(avahigroup));
    }
}

void KDNSSDPrivate::clientCallback(AvahiClient *avahiclient, AvahiClientState avahistate, void *userdata)
{
    // qDebug() << Q_FUNC_INFO << avahistate << userdata;

    if (avahistate == AVAHI_CLIENT_FAILURE) {
        KDNSSDPrivate *kdnssdprivate = static_cast<KDNSSDPrivate*>(userdata);
        kdnssdprivate->m_errorstring = getAvahiClientError(avahiclient);
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
            kdnssdprivate->m_pollcounter++;
            AvahiServiceResolver *avahiresolver = avahi_service_resolver_new(
                avahiclient,
                avahiinterface, avahiprotocol,
                avahiname, avahitype, avahidomain,
                s_avahiproto, AvahiLookupFlags(0),
                KDNSSDPrivate::resolveCallback,
                userdata
            );
            if (!avahiresolver) {
                kdnssdprivate->m_errorstring = getAvahiClientError(avahiclient);
            }
            break;
        }
        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED: {
            kDebug() << "Done browsing";
            kdnssdprivate->m_pollcounter--;
            KDNSSD *kdnssd= qobject_cast<KDNSSD*>(kdnssdprivate->parent());
            emit kdnssd->finished();
            break;
        }
        case AVAHI_BROWSER_FAILURE: {
            kdnssdprivate->m_errorstring = getAvahiClientError(avahiclient);
            break;
        }
        case AVAHI_BROWSER_REMOVE: {
            // shush, compiler
            break;
        }
    }
}

void KDNSSDPrivate::resolveCallback(AvahiServiceResolver *avahiresolver, AvahiIfIndex avahiinterface,
                                    AvahiProtocol avahiprotocol, AvahiResolverEvent avahievent,
                                    const char *avahiname, const char *avahitype, const char *avahidomain, const char *avahihostname,
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

            const QByteArray kdnssdservicetype = QByteArray(avahitype);
            const QString kdnssdservicedomain = QString::fromUtf8(avahidomain);
            const QString kdnssdservicehostname = QString::fromUtf8(avahihostname);
            const uint kdnssdserviceport = avahiport;

            // first comes, first serves
            bool isduplicate = false;
            foreach (const KDNSSDService &kdnssdserviceit, kdnssdprivate->m_services) {
                if (kdnssdserviceit.type == kdnssdservicetype
                    && kdnssdserviceit.domain == kdnssdservicedomain
                    && kdnssdserviceit.hostname == kdnssdservicehostname
                    && kdnssdserviceit.port == kdnssdserviceport) {
                    isduplicate = true;
                    break;
                }
            }
            if (!isduplicate) {
                char avahiaddressbuff[AVAHI_ADDRESS_STR_MAX];
                ::memset(avahiaddressbuff, 0, sizeof(avahiaddressbuff) * sizeof(char));
                avahi_address_snprint(avahiaddressbuff, sizeof(avahiaddressbuff), avahiaddress);

                QString kdnssdserviceprotocol = QString::fromLatin1(avahitype);
                kdnssdserviceprotocol = kdnssdserviceprotocol.mid(1);
                const int dotindex = kdnssdserviceprotocol.indexOf(QLatin1Char('.'));
                kdnssdserviceprotocol = kdnssdserviceprotocol.left(dotindex);
                KUrl kdnssdserviceurl;
                kdnssdserviceurl.setScheme(kdnssdserviceprotocol);
                kdnssdserviceurl.setHost(QString::fromLatin1(avahiaddressbuff));
                kdnssdserviceurl.setPort(kdnssdserviceport);

                KDNSSDService kdnssdservice;
                kdnssdservice.name = QString::fromUtf8(avahiname);
                kdnssdservice.type = kdnssdservicetype;
                kdnssdservice.domain = kdnssdservicedomain;
                kdnssdservice.hostname = kdnssdservicehostname;
                kdnssdservice.url = kdnssdserviceurl.prettyUrl();
                kdnssdservice.port = kdnssdserviceport;

                kdnssdprivate->m_services.append(kdnssdservice);
            }
            break;
        }
        case AVAHI_RESOLVER_FAILURE: {
            kdnssdprivate->m_errorstring = getAvahiClientError(avahi_service_resolver_get_client(avahiresolver));
            break;
        }
    }

    kdnssdprivate->m_pollcounter--;
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
            kdnssdprivate->m_pollcounter--;
            break;
        }
        case AVAHI_BROWSER_FAILURE: {
            kdnssdprivate->m_errorstring = getAvahiClientError(avahiclient);
            break;
        }
        case AVAHI_BROWSER_REMOVE: {
            // shush, compiler
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
    return d->publishService(servicetype, serviceport, servicename);
}

bool KDNSSD::unpublishService()
{
    return d->unpublishService();
}

QList<KDNSSDService> KDNSSD::services() const
{
    return d->services();
}

bool KDNSSD::startBrowse(const QByteArray &servicetype)
{
    return d->startBrowse(servicetype);
}

QString KDNSSD::errorString() const
{
    return d->errorString();
}

#include "kdnssd.moc"
