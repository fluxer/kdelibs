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

#include "http.h"
#include "kdebug.h"
#include "kcomponentdata.h"

#include <QApplication>
#include <QHostAddress>
#include <QHostInfo>

#include <sys/types.h>
#include <unistd.h>

static inline QByteArray curlProxyBytes(const QString &proxy)
{
    const KUrl proxyurl(proxy);
    const QString proxyhost = proxyurl.host();
    if (proxyurl.port() > 0) {
        QByteArray curlproxybytes = proxyhost.toAscii();
        curlproxybytes.append(':');
        curlproxybytes.append(QByteArray::number(proxyurl.port()));
        return curlproxybytes;
    }
    return proxyhost.toAscii();
}

static inline curl_proxytype curlProxyType(const QString &proxy)
{
    const QString proxyprotocol = KUrl(proxy).protocol();

#if CURL_AT_LEAST_VERSION(7, 52, 0)
    if (proxyprotocol.startsWith(QLatin1String("https"))) {
        return CURLPROXY_HTTPS;
    }
#endif
    if (proxyprotocol.startsWith(QLatin1String("socks4"))) {
        return CURLPROXY_SOCKS4;
    } else if (proxyprotocol.startsWith(QLatin1String("socks4a"))) {
        return CURLPROXY_SOCKS4A;
    } else if (proxyprotocol.startsWith(QLatin1String("socks5"))) {
        return CURLPROXY_SOCKS5;
    }
    return CURLPROXY_HTTP;
}

static inline QString HTTPMIMEType(const QString &contenttype)
{
    const QList<QString> splitcontenttype = contenttype.split(QLatin1Char(';'));
    if (splitcontenttype.isEmpty() || splitcontenttype.at(0).isEmpty()) {
        return QString::fromLatin1("application/octet-stream");
    }
    return splitcontenttype.at(0);
}

static inline QString HTTPCharset(const QString &contenttype)
{
    const QList<QString> splitcontenttype = contenttype.split(QLatin1Char(';'));
    if (splitcontenttype.size() < 2) {
        return QString();
    }
    return splitcontenttype.at(1);
}

static inline long HTTPCode(CURL *curl)
{
    long curlresponsecode = 0;
    const CURLcode curlresult = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curlresponsecode);
    if (curlresult != CURLE_OK) {
        kWarning(7103) << "Could not get response code info" << curl_easy_strerror(curlresult);
    }
    return curlresponsecode;
}

static inline KIO::Error HTTPToKIOError(const long httpcode)
{
    switch (httpcode) {
        case 401:
        case 403:
        case 407: {
            return KIO::ERR_COULD_NOT_AUTHENTICATE;
        }
        case 408: {
            return KIO::ERR_SERVER_TIMEOUT;
        }
        case 500: {
            return KIO::ERR_INTERNAL_SERVER;
        }
        default: {
            return KIO::ERR_NO_CONTENT;
        }
    }
    Q_UNREACHABLE();
}

static inline KIO::Error curlToKIOError(const CURLcode curlcode, CURL *curl)
{
    switch (curlcode) {
        case CURLE_HTTP_RETURNED_ERROR:
        case CURLE_ABORTED_BY_CALLBACK: {
            const long httpcode = HTTPCode(curl);
            kDebug(7103) << "HTTP error" << httpcode;
            return HTTPToKIOError(httpcode);
        }
        case CURLE_URL_MALFORMAT: {
            return KIO::ERR_MALFORMED_URL;
        }
#if CURL_AT_LEAST_VERSION(7, 73, 0)
        case CURLE_PROXY:
#endif
        case CURLE_COULDNT_RESOLVE_PROXY: {
            return KIO::ERR_UNKNOWN_PROXY_HOST;
        }
        case CURLE_AUTH_ERROR:
        case CURLE_LOGIN_DENIED:
        case CURLE_REMOTE_ACCESS_DENIED: {
            return KIO::ERR_COULD_NOT_AUTHENTICATE;
        }
        case CURLE_FILE_COULDNT_READ_FILE:
        case CURLE_READ_ERROR: {
            return KIO::ERR_COULD_NOT_READ;
        }
        case CURLE_WRITE_ERROR: {
            return KIO::ERR_COULD_NOT_WRITE;
        }
        case CURLE_OUT_OF_MEMORY: {
            return KIO::ERR_OUT_OF_MEMORY;
        }
        case CURLE_BAD_DOWNLOAD_RESUME: {
            return KIO::ERR_CANNOT_RESUME;
        }
        case CURLE_REMOTE_FILE_NOT_FOUND: {
            return KIO::ERR_DOES_NOT_EXIST;
        }
        case CURLE_GOT_NOTHING: {
            return KIO::ERR_NO_CONTENT;
        }
        case CURLE_REMOTE_DISK_FULL: {
            return KIO::ERR_DISK_FULL;
        }
        case CURLE_OPERATION_TIMEDOUT: {
            return KIO::ERR_SERVER_TIMEOUT;
        }
        case CURLE_COULDNT_CONNECT:
        default: {
            kWarning(7103) << "curl error" << curl_easy_strerror(curlcode);
            return KIO::ERR_COULD_NOT_CONNECT;
        }
    }
    Q_UNREACHABLE();
}

size_t curlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    HttpProtocol* httpprotocol = static_cast<HttpProtocol*>(userdata);
    if (!httpprotocol) {
        return 0;
    }
    // size should always be 1
    Q_ASSERT(size == 1);
    httpprotocol->slotData(ptr, nmemb);
    return nmemb;
}

size_t curlReadCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    HttpProtocol* httpprotocol = static_cast<HttpProtocol*>(userdata);
    if (!httpprotocol) {
        return 0;
    }
    httpprotocol->dataReq();
    QByteArray kioreadbuffer;
    const int kioreadresult = httpprotocol->readData(kioreadbuffer);
    if (kioreadbuffer.size() > nmemb) {
        kWarning(7103) << "Request data size larger than the buffer size";
        return 0;
    }
    ::memcpy(ptr, kioreadbuffer.constData(), kioreadbuffer.size() * sizeof(char));
    return kioreadresult;
}

int curlXFERCallback(void *userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    HttpProtocol* httpprotocol = static_cast<HttpProtocol*>(userdata);
    if (!httpprotocol) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    if (httpprotocol->aborttransfer) {
        return CURLE_HTTP_RETURNED_ERROR;
    }
    httpprotocol->slotProgress(KIO::filesize_t(dlnow), KIO::filesize_t(dltotal));
    return CURLE_OK;
}

extern "C" int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    QApplication app(argc, argv);
    KComponentData componentData("kio_http", "kdelibs4");

    kDebug(7103) << "Starting" << ::getpid();

    if (argc != 4) {
        ::fprintf(stderr, "Usage: kio_http protocol domain-socket1 domain-socket2\n");
        ::exit(-1);
    }

    HttpProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kDebug(7103) << "Done";
    return 0;
}

HttpProtocol::HttpProtocol(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("http", pool, app),
    aborttransfer(false), m_emitmime(true),
    m_curl(nullptr), m_curlheaders(nullptr)
{
    m_curl = curl_easy_init();
    if (!m_curl) {
        kWarning(7103) << "Could not create context";
        return;
    }
}

HttpProtocol::~HttpProtocol()
{
    if (m_curlheaders) {
        curl_slist_free_all(m_curlheaders);
    }
    if (m_curl) {
        curl_easy_cleanup(m_curl);
    }
}

void HttpProtocol::stat(const KUrl &url)
{
    kDebug(7103) << "URL" << url.prettyUrl();

    if (redirectUrl(url)) {
        return;
    }

    if (!setupCurl(url)) {
        return;
    }

    // NOTE: do not set CURLOPT_NOBODY, server may not send some headers
    CURLcode curlresult = curl_easy_perform(m_curl);
    if (curlresult != CURLE_OK) {
        const KIO::Error kioerror = curlToKIOError(curlresult, m_curl);
        if (kioerror == KIO::ERR_COULD_NOT_AUTHENTICATE) {
            if (authUrl(url)) {
                return;
            }
        }
        error(kioerror, url.prettyUrl());
        return;
    }

    QString httpmimetype;
    char *curlcontenttype = nullptr;
    curlresult = curl_easy_getinfo(m_curl, CURLINFO_CONTENT_TYPE, &curlcontenttype);
    if (curlresult == CURLE_OK) {
        httpmimetype = HTTPMIMEType(QString::fromAscii(curlcontenttype));
    } else {
        kWarning(7103) << "Could not get content type info" << curl_easy_strerror(curlresult);
    }

    curl_off_t curlfiletime = 0;
    curlresult = curl_easy_getinfo(m_curl, CURLINFO_FILETIME_T, &curlfiletime);
    if (curlresult != CURLE_OK) {
        kWarning(7103) << "Could not get filetime info" << curl_easy_strerror(curlresult);
    }

    curl_off_t curlcontentlength = 0;
    curlresult = curl_easy_getinfo(m_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &curlcontentlength);
    if (curlresult != CURLE_OK) {
        kWarning(7103) << "Could not get content length info" << curl_easy_strerror(curlresult);
    }

    KIO::UDSEntry kioudsentry;
    kDebug(7103) << "HTTP last-modified" << curlfiletime;
    kDebug(7103) << "HTTP content-length" << curlcontentlength;
    kDebug(7103) << "HTTP content-type" << httpmimetype;
    kioudsentry.insert(KIO::UDSEntry::UDS_NAME, url.fileName());
    kioudsentry.insert(KIO::UDSEntry::UDS_SIZE, qlonglong(curlcontentlength));
    kioudsentry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, qlonglong(curlfiletime));
    if (!httpmimetype.isEmpty()) {
        kioudsentry.insert(KIO::UDSEntry::UDS_MIME_TYPE, httpmimetype);
    }
    statEntry(kioudsentry);

    finished();
}

void HttpProtocol::get(const KUrl &url)
{
    kDebug(7103) << "URL" << url.prettyUrl();

    if (redirectUrl(url)) {
        return;
    }

    if (!setupCurl(url)) {
        return;
    }

    CURLcode curlresult = curl_easy_perform(m_curl);
    if (curlresult != CURLE_OK) {
        const KIO::Error kioerror = curlToKIOError(curlresult, m_curl);
        if (kioerror == KIO::ERR_COULD_NOT_AUTHENTICATE) {
            if (authUrl(url)) {
                return;
            }
        }
        error(kioerror, url.prettyUrl());
        return;
    }

    finished();
}


void HttpProtocol::put(const KUrl &url, int permissions, KIO::JobFlags flags)
{
    kDebug(7103) << "URL" << url.prettyUrl();

    if (redirectUrl(url)) {
        return;
    }

    if (!setupCurl(url)) {
        return;
    }

    CURLcode curlresult = curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
    if (curlresult != CURLE_OK) {
        error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
        return;
    }

    curlresult = curl_easy_perform(m_curl);
    if (curlresult != CURLE_OK) {
        const KIO::Error kioerror = curlToKIOError(curlresult, m_curl);
        if (kioerror == KIO::ERR_COULD_NOT_AUTHENTICATE) {
            if (authUrl(url)) {
                return;
            }
        }
        error(kioerror, url.prettyUrl());
        return;
    }

    finished();
}

void HttpProtocol::slotData(const char* curldata, const size_t curldatasize)
{
    if (aborttransfer) {
        kDebug(7103) << "Transfer still in progress";
        return;
    }

    if (m_emitmime) {
        m_emitmime = false;

        // if it's HTTP error do not send data and MIME, abort transfer
        const long httpcode = HTTPCode(m_curl);
        if (httpcode >= 400) {
            aborttransfer = true;
            return;
        }

        QString httpmimetype = QString::fromLatin1("application/octet-stream");
        char *curlcontenttype = nullptr;
        CURLcode curlresult = curl_easy_getinfo(m_curl, CURLINFO_CONTENT_TYPE, &curlcontenttype);
        if (curlresult == CURLE_OK) {
            httpmimetype = HTTPMIMEType(QString::fromAscii(curlcontenttype));
        } else {
            kWarning(7103) << "Could not get content type info" << curl_easy_strerror(curlresult);
        }
        mimeType(httpmimetype);
    }

    data(QByteArray::fromRawData(curldata, curldatasize));

    curl_off_t curlspeeddownload = 0;
    CURLcode curlresult = curl_easy_getinfo(m_curl, CURLINFO_SPEED_DOWNLOAD_T, &curlspeeddownload);
    if (curlresult == CURLE_OK) {
        kDebug(7103) << "Download speed" << curlspeeddownload;
        speed(ulong(curlspeeddownload));
    } else {
        kWarning(7103) << "Could not get download speed info" << curl_easy_strerror(curlresult);
    }
}

void HttpProtocol::slotProgress(KIO::filesize_t received, KIO::filesize_t total)
{
    kDebug(7103) << "Received" << received << "from" << total;
    emit processedSize(received);
    if (total > 0 && received != total) {
        emit totalSize(total);
    }
}

bool HttpProtocol::redirectUrl(const KUrl &url)
{
    // curl cannot verify certs if the host is address, CURLOPT_USE_SSL set to CURLUSESSL_TRY
    // does not bypass such cases so resolving it manually
    const QHostAddress urladdress(url.host());
    if (!urladdress.isNull()) {
        const QHostInfo urlinfo = QHostInfo::fromName(url.host());
        if (urlinfo.error() == QHostInfo::NoError) {
            KUrl newurl(url);
            newurl.setHost(urlinfo.hostName());
            kDebug(7103) << "Rewrote" << url << "to" << newurl;
            redirection(newurl);
            finished();
            return true;
        } else {
            kWarning() << "Could not resolve" << url.host();
            return false;
        }
    }
    return false;
}

bool HttpProtocol::setupCurl(const KUrl &url)
{
    if (Q_UNLIKELY(!m_curl)) {
        error(KIO::ERR_OUT_OF_MEMORY, QString::fromLatin1("Null context"));
        return false;
    }

    aborttransfer = false;
    m_emitmime = true;
    curl_easy_reset(m_curl);
    curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_curl, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, 100L); // proxies apparently cause a lot of redirects
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, SlaveBase::connectTimeout());
    // curl_easy_setopt(m_curl, CURLOPT_IGNORE_CONTENT_LENGTH, 1L); // breaks XFER info, fixes transfer of chunked content
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(m_curl, CURLOPT_READDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, curlReadCallback);
    curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L); // otherwise the XFER info callback is not called
    curl_easy_setopt(m_curl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(m_curl, CURLOPT_XFERINFOFUNCTION, curlXFERCallback);

    const QByteArray urlbytes = url.prettyUrl().toLocal8Bit();
    CURLcode curlresult = curl_easy_setopt(m_curl, CURLOPT_URL, urlbytes.constData());
    if (curlresult != CURLE_OK) {
        error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
        return false;
    }

    curlresult = curl_easy_setopt(m_curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    if (curlresult != CURLE_OK) {
        error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
        return false;
    }

    curlresult = curl_easy_setopt(m_curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    if (curlresult != CURLE_OK) {
        error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
        return false;
    }

    kDebug(7103) << "Metadata" << allMetaData();

    if (hasMetaData(QLatin1String("UserAgent"))) {
        const QByteArray useragentbytes = metaData("UserAgent").toAscii();
        curlresult = curl_easy_setopt(m_curl, CURLOPT_USERAGENT, useragentbytes.constData());
        if (curlresult != CURLE_OK) {
            error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
            return false;
        }
    }

    const bool noauth = (metaData("no-auth") == QLatin1String("yes"));
    if (hasMetaData(QLatin1String("UseProxy"))) {
        const QString proxystring = metaData("UseProxy");
        const QByteArray proxybytes = curlProxyBytes(proxystring);
        const curl_proxytype curlproxytype = curlProxyType(proxystring);
        kDebug(7103) << "Proxy" << proxybytes << curlproxytype;
        curlresult = curl_easy_setopt(m_curl, CURLOPT_PROXY, proxybytes.constData());
        if (curlresult != CURLE_OK) {
            error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
            return false;
        }
        curlresult = curl_easy_setopt(m_curl, CURLOPT_PROXYTYPE, curlproxytype);
        if (curlresult != CURLE_OK) {
            error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
            return false;
        }

        const bool noproxyauth = (noauth || metaData("no-proxy-auth") == QLatin1String("yes"));
        kDebug(7103) << "No proxy auth" << noproxyauth;
        curlresult = curl_easy_setopt(m_curl, CURLOPT_PROXYAUTH, noproxyauth ? CURLAUTH_NONE : CURLAUTH_ANY);
        if (curlresult != CURLE_OK) {
            error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
            return false;
        }
    }

    const bool nowwwauth = (noauth || metaData("no-www-auth") == QLatin1String("true"));
    kDebug(7103) << "No WWW auth" << nowwwauth;
    curlresult = curl_easy_setopt(m_curl, CURLOPT_HTTPAUTH, nowwwauth ? CURLAUTH_NONE : CURLAUTH_ANY);
    if (curlresult != CURLE_OK) {
        error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
        return false;
    }

    if (!nowwwauth) {
        const QByteArray urlusername = url.userName().toAscii();
        const QByteArray urlpassword = url.password().toAscii();
        if (!urlusername.isEmpty() && !urlpassword.isEmpty()) {
            curlresult = curl_easy_setopt(m_curl, CURLOPT_USERNAME, urlusername.constData());
            if (curlresult != CURLE_OK) {
                error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
                return false;
            }
            curlresult = curl_easy_setopt(m_curl, CURLOPT_PASSWORD, urlpassword.constData());
            if (curlresult != CURLE_OK) {
                error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
                return false;
            }
        }
    }

    if (hasMetaData(QLatin1String("resume"))) {
        const qlonglong resumeoffset = metaData(QLatin1String("resume")).toLongLong();
        curlresult = curl_easy_setopt(m_curl, CURLOPT_RESUME_FROM_LARGE, resumeoffset);
        if (curlresult != CURLE_OK) {
            error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
            return false;
        } else {
            canResume();
        }
    }

    if (m_curlheaders) {
        curl_slist_free_all(m_curlheaders);
        m_curlheaders = nullptr;
    }

    if (hasMetaData(QLatin1String("Languages"))) {
        m_curlheaders = curl_slist_append(m_curlheaders, QByteArray("Accept-Language: ") + metaData("Languages").toAscii());
    }

    if (hasMetaData(QLatin1String("Charsets"))) {
        m_curlheaders = curl_slist_append(m_curlheaders, QByteArray("Accept-Charset: ") + metaData("Charsets").toAscii());
    }

    if (hasMetaData(QLatin1String("accept"))) {
        m_curlheaders = curl_slist_append(m_curlheaders, QByteArray("Accept: ") + metaData("accept").toAscii());
    }

    curlresult = curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_curlheaders);
    if (curlresult != CURLE_OK) {
        curl_slist_free_all(m_curlheaders);
        m_curlheaders = nullptr;
        error(KIO::ERR_SLAVE_DEFINED, curl_easy_strerror(curlresult));
        return false;
    }

    return true;
}

bool HttpProtocol::authUrl(const KUrl &url)
{
    KIO::AuthInfo kioauthinfo;
    kioauthinfo.url = url;
    if (!checkCachedAuthentication(kioauthinfo)) {
        kioauthinfo.prompt = i18n("You need to supply a username and a password to access this URL.");
        kioauthinfo.commentLabel = i18n("URL:");
        kioauthinfo.comment = i18n("<b>%1</b>", url.prettyUrl());
        if (openPasswordDialog(kioauthinfo)) {
            KUrl newurl(url);
            newurl.setUser(kioauthinfo.username);
            newurl.setPassword(kioauthinfo.password);
            redirection(newurl);
            finished();
            return true;
        }
    }
    return false;
}
