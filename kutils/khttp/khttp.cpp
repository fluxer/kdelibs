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

#include "khttp.h"
#include "kurl.h"
#include "kdebug.h"

#include <QFile>
#include <QTimer>

#if defined(HAVE_LIBMICROHTTPD)
#  include <netinet/in.h>
#  include <microhttpd.h>
#endif

static const int MHDPollInterval = 100;

class KHTTPPrivate : public QObject
{
    Q_OBJECT
public:
    KHTTPPrivate(QObject *parent);
    ~KHTTPPrivate();

    bool setCertificate(const QByteArray &keydata, const QByteArray &certdata, const QByteArray &password);
    bool setAuthenticate(const QByteArray &username, const QByteArray &password, const QString &message);
    bool start(const QHostAddress &address, quint16 port);
    bool stop();
    QString errorString() const;

#if defined(HAVE_LIBMICROHTTPD)
    static enum MHD_Result keyValueCallback(void *cls,
                                            enum MHD_ValueKind kind,
                                            const char *key,
                                            const char *value);
    static enum MHD_Result acceptCallback(void *cls, const struct sockaddr *addr, socklen_t addrlen);
    static enum MHD_Result accessCallback(void *cls,
                                          struct MHD_Connection *connection,
                                          const char *url,
                                          const char *method,
                                          const char *version,
                                          const char *upload_data,
                                          size_t *upload_data_size,
                                          void **con_cls);
    static void loggerCallback(void *cls, const char *fm, va_list ap);
    static void panicCallback(void *cls, const char *file, unsigned int line, const char *reason);

private Q_SLOTS:
    void slotMHDPoll();

private:
    QByteArray m_tlskey;
    QByteArray m_tlscert;
    QByteArray m_tlspassword;
    QByteArray m_authusername;
    QByteArray m_authpassword;
    QByteArray m_authmessage;
    struct MHD_Daemon* m_mhddaemon;
    QTimer m_polltimer;
    QUrl m_url;
#endif // HAVE_LIBMICROHTTPD
    QString m_errorstring;
};

KHTTPPrivate::KHTTPPrivate(QObject *parent)
    : QObject(parent)
#if defined(HAVE_LIBMICROHTTPD)
    , m_mhddaemon(nullptr),
    m_polltimer(this)
#endif
{
#if defined(HAVE_LIBMICROHTTPD)
    connect(
        &m_polltimer, SIGNAL(timeout()),
        this, SLOT(slotMHDPoll())
    );
#endif
}

KHTTPPrivate::~KHTTPPrivate()
{
}

bool KHTTPPrivate::setCertificate(const QByteArray &keydata, const QByteArray &certdata, const QByteArray &password)
{
#if defined(HAVE_LIBMICROHTTPD)
    const enum MHD_Result mhdresult = MHD_is_feature_supported(MHD_FEATURE_TLS);
    if (mhdresult == MHD_NO) {
        kWarning() << "TLS is not supported";
        return false;
    }
    if (keydata.isEmpty() || certdata.isEmpty()) {
        kWarning() << "TLS key or certificate data is empty";
        m_tlskey.clear();
        m_tlscert.clear();
        return false;
    }
    m_tlskey = keydata;
    m_tlscert = certdata;
    m_tlspassword = password;
    return true;
#else
    // TODO: set error?
    return false;
#endif
}

bool KHTTPPrivate::setAuthenticate(const QByteArray &username, const QByteArray &password, const QString &message)
{
#if defined(HAVE_LIBMICROHTTPD)
    const enum MHD_Result mhdresult = MHD_is_feature_supported(MHD_FEATURE_BASIC_AUTH);
    if (mhdresult == MHD_NO) {
        kWarning() << "Authentication is not supported";
        return false;
    }
    if (username.isEmpty() || password.isEmpty()) {
        kWarning() << "Empty user name or password";
        m_authusername.clear();
        m_authpassword.clear();
        return false;
    }
    m_authusername = username;
    m_authpassword = password;
    m_authmessage = message.toAscii();
    return true;
#else
    // TODO: set error?
    return false;
#endif
}

bool KHTTPPrivate::start(const QHostAddress &address, quint16 port)
{
#if defined(HAVE_LIBMICROHTTPD)
    if (m_mhddaemon) {
        kWarning() << "Already started";
        return false;
    }
    // qDebug() << Q_FUNC_INFO << address.protocol();
    int mhdflags = MHD_NO_FLAG;
    if (!m_tlskey.isEmpty() && !m_tlscert.isEmpty()) {
        // qDebug() << Q_FUNC_INFO << "Enabling TLS";
        mhdflags |= MHD_USE_TLS;
    }
    const enum MHD_Result mhdresult = MHD_is_feature_supported(MHD_FEATURE_MESSAGES);
    if (mhdresult == MHD_NO) {
        kWarning() << "Messages are not supported";
    } else {
        kDebug() << "Enabling logger";
        mhdflags |= MHD_USE_ERROR_LOG;
    }
    switch (address.protocol()) {
        case QAbstractSocket::IPv4Protocol: {
            struct sockaddr_in socketaddress;
            ::memset(&socketaddress, 0, sizeof(struct sockaddr_in));
            socketaddress.sin_family = AF_INET;
            socketaddress.sin_port = htons(port);
            socketaddress.sin_addr.s_addr = htonl(address.toIPv4Address());
            m_mhddaemon = MHD_start_daemon(
                mhdflags,
                0,
                KHTTPPrivate::acceptCallback, this,
                KHTTPPrivate::accessCallback, this,
                MHD_OPTION_EXTERNAL_LOGGER, KHTTPPrivate::loggerCallback, this,
                MHD_OPTION_HTTPS_MEM_KEY, m_tlskey.constData(),
                MHD_OPTION_HTTPS_MEM_CERT, m_tlscert.constData(),
                MHD_OPTION_HTTPS_KEY_PASSWORD, m_tlspassword.constData(),
                MHD_OPTION_SOCK_ADDR, &socketaddress,
                MHD_OPTION_END
            );
            break;
        }
        case QAbstractSocket::IPv6Protocol: {
            if (MHD_is_feature_supported(MHD_FEATURE_IPv6) != MHD_YES) {
                kWarning() << "IPv6 is not supported";
                return false;
            }

            struct sockaddr_in6 socketaddress;
            ::memset(&socketaddress, 0, sizeof(struct sockaddr_in6));
            socketaddress.sin6_family = AF_INET6;
            socketaddress.sin6_port = htons(port);
            Q_IPV6ADDR ipv6address = address.toIPv6Address();
            ::memcpy(&socketaddress.sin6_addr.s6_addr, &ipv6address, sizeof(ipv6address));
            m_mhddaemon = MHD_start_daemon(
                mhdflags | MHD_USE_IPv6,
                0,
                KHTTPPrivate::acceptCallback, this,
                KHTTPPrivate::accessCallback, this,
                MHD_OPTION_EXTERNAL_LOGGER, KHTTPPrivate::loggerCallback, this,
                MHD_OPTION_HTTPS_MEM_KEY, m_tlskey.constData(),
                MHD_OPTION_HTTPS_MEM_CERT, m_tlscert.constData(),
                MHD_OPTION_HTTPS_KEY_PASSWORD, m_tlspassword.constData(),
                MHD_OPTION_SOCK_ADDR, &socketaddress,
                MHD_OPTION_END
            );
            break;
        }
        default: {
            kWarning() << "Invalid address protocol" << address.protocol();
            return false;
        }
    }
    if (!m_mhddaemon) {
        kWarning() << "Could not start MHD";
        return false;
    }
    MHD_set_panic_func(KHTTPPrivate::panicCallback, this);
    m_polltimer.start(MHDPollInterval);
    return true;
#else
    // TODO: set error?
    return false;
#endif
}

bool KHTTPPrivate::stop()
{
#if defined(HAVE_LIBMICROHTTPD)
    if (!m_mhddaemon) {
        kDebug() << "MHD not started";
        return true;
    }
    MHD_stop_daemon(m_mhddaemon);
    m_mhddaemon = nullptr;
    return true;
#else
    return true;
#endif
}

QString KHTTPPrivate::errorString() const
{
    return m_errorstring;
}

#if defined(HAVE_LIBMICROHTTPD)
void KHTTPPrivate::slotMHDPoll()
{
    if (m_mhddaemon) {
        MHD_run(m_mhddaemon);
    }
}

enum MHD_Result KHTTPPrivate::keyValueCallback(void *cls,
                                               enum MHD_ValueKind kind,
                                               const char *key,
                                               const char *value)
{
    // qDebug() << Q_FUNC_INFO << key << value;
    Q_UNUSED(kind);
    KHTTPPrivate* khttpprivate = static_cast<KHTTPPrivate*>(cls);
    khttpprivate->m_url.addQueryItem(QString::fromUtf8(key), QString::fromUtf8(value));
    return MHD_YES;
}

enum MHD_Result KHTTPPrivate::acceptCallback(void *cls, const struct sockaddr *addr, socklen_t addrlen)
{
    // qDebug() << Q_FUNC_INFO;
    return MHD_YES;
}

enum MHD_Result KHTTPPrivate::accessCallback(void *cls,
                                             struct MHD_Connection *connection,
                                             const char *url,
                                             const char *method,
                                             const char *version,
                                             const char *upload_data,
                                             size_t *upload_data_size,
                                             void **con_cls)
{
    // qDebug() << Q_FUNC_INFO << url << method << version;
    Q_UNUSED(method);
    Q_UNUSED(version);
    Q_UNUSED(upload_data);
    Q_UNUSED(upload_data_size);
    Q_UNUSED(con_cls);

    KHTTPPrivate* khttpprivate = static_cast<KHTTPPrivate*>(cls);
    KHTTP* khttp = qobject_cast<KHTTP*>(khttpprivate->parent());

    if (!khttpprivate->m_authusername.isEmpty() && !khttpprivate->m_authpassword.isEmpty()) {
        char* mhdpassword = NULL;
        char* mhdusername = MHD_basic_auth_get_username_password(connection, &mhdpassword);
        if (!mhdpassword || !mhdusername
            || khttpprivate->m_authusername != mhdusername
            || khttpprivate->m_authpassword != mhdpassword) {
            if (mhdusername) {
                MHD_free(mhdusername);
            }
            if (mhdpassword) {
                MHD_free(mhdpassword);
            }
            struct MHD_Response *mhdresponse = MHD_create_response_from_buffer(
                khttpprivate->m_authmessage.size(), khttpprivate->m_authmessage.data(),
                MHD_RESPMEM_MUST_COPY
            );
            if (!mhdresponse) {
                kWarning() << "Could not create MHD auth response";
                return MHD_NO;
            }
            enum MHD_Result mhdresult = MHD_queue_basic_auth_fail_response(connection, "KHTTPD Ream", mhdresponse);
            MHD_destroy_response(mhdresponse);
            return mhdresult;
        }
        if (mhdusername) {
            MHD_free(mhdusername);
        }
        if (mhdpassword) {
            MHD_free(mhdpassword);
        }
    }

    khttpprivate->m_url = QUrl(QString::fromUtf8(url));
    MHD_get_connection_values(connection,
        MHD_GET_ARGUMENT_KIND,
        KHTTPPrivate::keyValueCallback,
        cls
    );

    QByteArray khttpurl = khttpprivate->m_url.toEncoded();
    KHTTPHeaders khttpheaders;
    QByteArray mhdoutdata;
    ushort mhdouthttpstatus = MHD_HTTP_NOT_FOUND;
    khttp->respond(khttpurl, &mhdoutdata, &mhdouthttpstatus, &khttpheaders);

    struct MHD_Response *mhdresponse = MHD_create_response_from_buffer(
        mhdoutdata.size(), mhdoutdata.data(),
        MHD_RESPMEM_MUST_COPY
    );
    if (!mhdresponse) {
        kWarning() << "Could not create MHD response";
        return MHD_NO;
    }

    enum MHD_Result mhdresult = MHD_NO;
    foreach (const QByteArray &httpheaderkey, khttpheaders.keys()) {
        if (qstricmp(httpheaderkey.constData(), "Content-Length") == 0) {
            // MHD refuses to add it
            kDebug() << "Ignoring content-length";
            continue;
        }
        const QByteArray httpheadervalue = khttpheaders.value(httpheaderkey);
        mhdresult = MHD_add_response_header(mhdresponse, httpheaderkey.constData(), httpheadervalue.constData());
        if (mhdresult == MHD_NO) {
            kWarning() << "Could not add response header" << httpheaderkey << httpheadervalue;
        }
    }

    mhdresult = MHD_queue_response(connection, mhdouthttpstatus, mhdresponse);
    MHD_destroy_response(mhdresponse);
    return mhdresult;
}

void KHTTPPrivate::loggerCallback(void *cls, const char *fm, va_list ap)
{
    char mhdloggerbuff[1024];
    ::memset(mhdloggerbuff, 0, sizeof(mhdloggerbuff) * sizeof(char));
    ::vsnprintf(mhdloggerbuff, sizeof(mhdloggerbuff), fm, ap);
    // qDebug() << Q_FUNC_INFO << mhdloggerbuff;
    KHTTPPrivate* khttpprivate = static_cast<KHTTPPrivate*>(cls);
    khttpprivate->m_errorstring = QString::fromAscii(mhdloggerbuff);
}

void KHTTPPrivate::panicCallback(void *cls, const char *file, unsigned int line, const char *reason)
{
    KHTTPPrivate* khttpprivate = static_cast<KHTTPPrivate*>(cls);
    khttpprivate->m_mhddaemon = nullptr;
    kFatal() << QString::fromAscii(file) << QString::number(line) << QString::fromAscii(reason);
}
#endif // HAVE_LIBMICROHTTPD

KHTTP::KHTTP(QObject *parent)
    : QObject(parent),
    d(new KHTTPPrivate(this))
{
    stop();
}

KHTTP::~KHTTP()
{
    delete d;
}

bool KHTTP::setCertificate(const QByteArray &keydata, const QByteArray &certdata, const QByteArray &password)
{
    return d->setCertificate(keydata, certdata, password);
}

bool KHTTP::setAuthenticate(const QByteArray &username, const QByteArray &password, const QString &message)
{
    return d->setAuthenticate(username, password, message);
}

bool KHTTP::start(const QHostAddress &address, quint16 port)
{
    return d->start(address, port);
}

bool KHTTP::stop()
{
    return d->stop();
}

QString KHTTP::errorString() const
{
    return d->errorString();
}

#include "khttp.moc"