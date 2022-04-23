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
#include "khttpheader.h"

#include <QCoreApplication>

#include <sys/types.h>
#include <unistd.h>

static inline QByteArray curlProxyString(const QString &proxy)
{
    const KUrl proxyurl(proxy);
    if (proxyurl.port() > 0) {
        return QString::fromLatin1("%1:%2").arg(proxyurl.host()).arg(proxyurl.port()).toAscii();
    }
    return proxyurl.host().toAscii();
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
    if (splitcontenttype.isEmpty()) {
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

static inline KIO::Error KIOError(const CURLcode curlcode)
{
    switch (curlcode) {
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
    // emit MIME before data
    if (httpprotocol->firstchunk) {
        httpprotocol->slotMIME();
        httpprotocol->firstchunk = false;
    }
    // size should always be 1
    Q_ASSERT(size == 1);
    httpprotocol->slotData(ptr, nmemb);
    return nmemb;
}

int curlXFERCallback(void *userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    HttpProtocol* httpprotocol = static_cast<HttpProtocol*>(userdata);
    if (!httpprotocol) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    httpprotocol->slotProgress(KIO::filesize_t(dlnow), KIO::filesize_t(dltotal));
    return CURLE_OK;
}

int curlHeaderCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    HttpProtocol* httpprotocol = static_cast<HttpProtocol*>(userdata);
    if (!httpprotocol) {
        return 0;
    }
    // size should always be 1
    Q_ASSERT(size == 1);
    httpprotocol->headerdata.append(static_cast<char*>(ptr), nmemb);
    return nmemb;
}

extern "C" int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    KComponentData componentData("kio_http", "kdelibs4");
    (void)KGlobal::locale();

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
    : SlaveBase("http", pool, app), firstchunk(true), m_curl(nullptr)
{
    m_curl = curl_easy_init();
    if (!m_curl) {
        kWarning(7103) << "Could not create context";
        return;
    }
}

HttpProtocol::~HttpProtocol()
{
    if (m_curl) {
        curl_easy_cleanup(m_curl);
    }
}

void HttpProtocol::get(const KUrl &url)
{
    kDebug(7103) << "URL" << url.prettyUrl();

    if (Q_UNLIKELY(!m_curl)) {
        error(KIO::ERR_OUT_OF_MEMORY, QString::fromLatin1("Null context"));
        return;
    }

    firstchunk = true;
    headerdata.clear();
    curl_easy_reset(m_curl);
    curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, 100L); // proxies apparently cause a lot of redirects
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, SlaveBase::connectTimeout());
    // curl_easy_setopt(m_curl, CURLOPT_IGNORE_CONTENT_LENGTH, 1L); // breaks XFER info, fixes transfer of chunked content
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L); // otherwise the XFER info callback is not called
    curl_easy_setopt(m_curl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(m_curl, CURLOPT_XFERINFOFUNCTION, curlXFERCallback);
    curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, curlHeaderCallback);

    const QByteArray urlbytes = url.prettyUrl().toLocal8Bit();
    CURLcode curlresult = curl_easy_setopt(m_curl, CURLOPT_URL, urlbytes.constData());
    if (curlresult != CURLE_OK) {
        kWarning(7103) << curl_easy_strerror(curlresult);
        error(KIO::ERR_MALFORMED_URL, curl_easy_strerror(curlresult));
        return;
    }

    curlresult = curl_easy_setopt(m_curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    if (curlresult != CURLE_OK) {
        kWarning(7103) << curl_easy_strerror(curlresult);
        error(KIO::ERR_CONNECTION_BROKEN, curl_easy_strerror(curlresult));
        return;
    }

    curlresult = curl_easy_setopt(m_curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    if (curlresult != CURLE_OK) {
        kWarning(7103) << curl_easy_strerror(curlresult);
        error(KIO::ERR_CONNECTION_BROKEN, curl_easy_strerror(curlresult));
        return;
    }

    kDebug(7103) << "Metadata" << allMetaData();

    if (hasMetaData(QLatin1String("UserAgent"))) {
        const QByteArray useragentbytes = metaData("UserAgent").toAscii();
        curlresult = curl_easy_setopt(m_curl, CURLOPT_USERAGENT, useragentbytes.constData());
        if (curlresult != CURLE_OK) {
            kWarning(7103) << curl_easy_strerror(curlresult);
        }
    }

    const bool noauth = (metaData("no-auth") == QLatin1String("yes"));
    if (hasMetaData(QLatin1String("UseProxy"))) {
        const QString proxystring = metaData("UseProxy");
        const QByteArray proxybytes = curlProxyString(proxystring);
        const curl_proxytype curlproxytype = curlProxyType(proxystring);
        kDebug(7103) << "Proxy" << proxybytes << curlproxytype;
        curlresult = curl_easy_setopt(m_curl, CURLOPT_PROXY, proxybytes.constData());
        if (curlresult != CURLE_OK) {
            kWarning(7103) << curl_easy_strerror(curlresult);
            error(KIO::ERR_UNKNOWN_PROXY_HOST, curl_easy_strerror(curlresult));
            return;
        }
        curlresult = curl_easy_setopt(m_curl, CURLOPT_PROXYTYPE, curlproxytype);
        if (curlresult != CURLE_OK) {
            kWarning(7103) << curl_easy_strerror(curlresult);
        }

        const bool noproxyauth = (noauth || metaData("no-proxy-auth") == QLatin1String("yes"));
        kDebug(7103) << "Proxy auth" << noproxyauth;
        curlresult = curl_easy_setopt(m_curl, CURLOPT_PROXYAUTH, noproxyauth ? CURLAUTH_NONE : CURLAUTH_ANY);
        if (curlresult != CURLE_OK) {
            kWarning(7103) << curl_easy_strerror(curlresult);
        }
    }

    kDebug(7103) << "Auth" << noauth;
    curlresult = curl_easy_setopt(m_curl, CURLOPT_HTTPAUTH, noauth ? CURLAUTH_NONE : CURLAUTH_ANY);
    if (curlresult != CURLE_OK) {
        kWarning(7103) << curl_easy_strerror(curlresult);
    }

    if (hasMetaData(QLatin1String("referrer"))) {
        const QByteArray referrerbytes = metaData("referrer").toAscii();
        curlresult = curl_easy_setopt(m_curl, CURLOPT_REFERER, referrerbytes.constData());
        if (curlresult != CURLE_OK) {
            kWarning(7103) << curl_easy_strerror(curlresult);
        }
    }

    if (hasMetaData(QLatin1String("resume"))) {
        const qlonglong resumeoffset = metaData(QLatin1String("resume")).toLongLong();
        curlresult = curl_easy_setopt(m_curl, CURLOPT_RESUME_FROM_LARGE, resumeoffset);
        if (curlresult != CURLE_OK) {
            kWarning(7103) << curl_easy_strerror(curlresult);
        } else {
            canResume();
        }
    }

    struct curl_slist *curllist = NULL;
    if (hasMetaData(QLatin1String("Languages"))) {
        curllist = curl_slist_append(curllist, QByteArray("Accept-Language: ") + metaData("Languages").toAscii());
    }

    if (hasMetaData(QLatin1String("Charsets"))) {
        curllist = curl_slist_append(curllist, QByteArray("Accept-Charset: ") + metaData("Charsets").toAscii());
    }

    if (hasMetaData(QLatin1String("accept"))) {
        curllist = curl_slist_append(curllist, QByteArray("Accept: ") + metaData("accept").toAscii());
    }

    curlresult = curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, curllist);
    if (curlresult != CURLE_OK) {
        curl_slist_free_all(curllist);
        kWarning(7103) << curl_easy_strerror(curlresult);
        error(KIO::ERR_CONNECTION_BROKEN, curl_easy_strerror(curlresult));
        return;
    }

    curlresult = curl_easy_perform(m_curl);
    if (curlresult != CURLE_OK) {
        curl_slist_free_all(curllist);
        kWarning(7103) << curl_easy_strerror(curlresult);
        error(KIOError(curlresult), curl_easy_strerror(curlresult));
        return;
    }

    if (hasMetaData(QLatin1String("PropagateHttpHeader"))) {
        const QString httpheaders = QString::fromAscii(headerdata.constData(), headerdata.size());
        kDebug(7103) << "HTTP headers" << httpheaders;
        setMetaData(QString::fromLatin1("HTTP-Headers"), httpheaders);
    }

    KHTTPHeader httpheader;
    httpheader.parseHeader(headerdata);
    setMetaData(QString::fromLatin1("modified"), httpheader.get(QLatin1String("Last-Modified")));

    curl_slist_free_all(curllist);

    if (httpheader.status() >= 400) {
        kDebug(7103) << "HTTP error" << httpheader.status() << httpheader.errorString();
        error(KIO::ERR_NO_CONTENT, httpheader.errorString());
        return;
    }

    finished();
}

void HttpProtocol::slotMIME()
{
    char *curlcontenttype = nullptr;
    CURLcode curlresult = curl_easy_getinfo(m_curl, CURLINFO_CONTENT_TYPE, &curlcontenttype);
    if (curlresult == CURLE_OK) {
        const QString httpmimetype = HTTPMIMEType(QString::fromAscii(curlcontenttype));
        kDebug(7103) << "MIME type" << httpmimetype;
        mimeType(httpmimetype);

        const QString httpcharset = HTTPCharset(QString::fromAscii(curlcontenttype));
        kDebug(7103) << "charset" << httpcharset;
        setMetaData(QString::fromLatin1("charset"), httpcharset);
    } else {
        kWarning(7103) << "Could not get content type info" << curl_easy_strerror(curlresult);
    }
}

void HttpProtocol::slotData(const char* curldata, const size_t curldatasize)
{
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

#include "moc_http.cpp"
