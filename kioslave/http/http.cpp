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

#include <QCoreApplication>

#include <sys/types.h>
#include <unistd.h>

// TODO: PropagateHttpHeader, charset, modified, accept and maybe caching

static inline QByteArray HTTPMIMEType(const QByteArray &contenttype)
{
    const QList<QByteArray> splitcontenttype = contenttype.split(';');
    if (splitcontenttype.isEmpty()) {
        return QByteArray("application/octet-stream");
    }
    return splitcontenttype.at(0);
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
    // size is always 1
    Q_UNUSED(size);
    httpprotocol->slotData(ptr, nmemb);
    return nmemb;
}

int curlProgressCallback(void *userdata, double dltotal, double dlnow, double ultotal, double ulnow)
{
    HttpProtocol* httpprotocol = static_cast<HttpProtocol*>(userdata);
    if (!httpprotocol) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    httpprotocol->slotProgress(dlnow, dltotal);
    return CURLE_OK;
}

extern "C" int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    KComponentData componentData("kio_http", "kdelibs4");
    (void)KGlobal::locale();

    kDebug(7103) << "Starting " << ::getpid();

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

    curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L); // otherwise the progress callback is not called
    curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, curlProgressCallback);
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
    const QByteArray urlbytes = url.prettyUrl().toLocal8Bit();
    curl_easy_setopt(m_curl, CURLOPT_URL, urlbytes.constData());

    kDebug(7103) << "Metadata" << allMetaData();
    struct curl_slist *curllist = NULL;
    // metadata from scheduler
    if (hasMetaData("Languages")) {
        curllist = curl_slist_append(curllist, QByteArray("Accept-Language: ") + metaData("Languages").toAscii());
    }
    if (hasMetaData("Charsets")) {
        curllist = curl_slist_append(curllist, QByteArray("Accept-Charset: ") + metaData("Charsets").toAscii());
    }
    if (hasMetaData("UserAgent")) {
        const QByteArray useragentbytes = metaData("UserAgent").toAscii();
        curl_easy_setopt(m_curl, CURLOPT_USERAGENT, useragentbytes.constData());
    }
    if (hasMetaData("UseProxy")) {
        const QByteArray proxybytes = metaData("UseProxy").toAscii();
        curl_easy_setopt(m_curl, CURLOPT_PROXY, proxybytes.constData());
    }
    CURLcode curlresult = curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, curllist);
    if (curlresult != CURLE_OK) {
        curl_slist_free_all(curllist);
        kWarning(7103) << "Error" << curl_easy_strerror(curlresult);
        error(KIO::ERR_COULD_NOT_CONNECT, curl_easy_strerror(curlresult));
        return;
    }

    curlresult = curl_easy_perform(m_curl);
    if (curlresult != CURLE_OK) {
        curl_slist_free_all(curllist);
        kWarning(7103) << "Error" << curl_easy_strerror(curlresult);
        error(KIO::ERR_COULD_NOT_CONNECT, curl_easy_strerror(curlresult));
        return;
    }

    // added in v7.76.0
#ifdef CURLINFO_REFERER
    char *curlreferrer = nullptr;
    curlresult = curl_easy_getinfo(m_curl, CURLINFO_REFERER, &curlreferrer);
    if (curlresult == CURLE_OK) {
        const QString httpreferrer = QString::fromAscii(curlreferrer);
        kDebug(7103) << "Referrer" << httpreferrer;
        setMetaData(QString::fromLatin1("referrer"), httpreferrer);
    } else {
        kWarning(7103) << "Could not get referrer info" << curl_easy_strerror(curlresult);
    }
#endif

    curl_slist_free_all(curllist);
    finished();
}

void HttpProtocol::slotMIME()
{
    char *curlcontenttype = nullptr;
    CURLcode curlresult = curl_easy_getinfo(m_curl, CURLINFO_CONTENT_TYPE, &curlcontenttype);
    if (curlresult == CURLE_OK) {
        const QByteArray httpmimetype = HTTPMIMEType(QByteArray(curlcontenttype));
        kDebug(7103) << "MIME type" << httpmimetype;
        emit mimeType(httpmimetype);
    } else {
        kWarning(7103) << "Could not get info" << curl_easy_strerror(curlresult);
    }
}

void HttpProtocol::slotData(const char* curldata, const size_t curldatasize)
{
    data(QByteArray::fromRawData(curldata, curldatasize));
}

void HttpProtocol::slotProgress(qint64 received, qint64 total)
{
    kDebug(7103) << "Received" << received << "from" << total;
    emit processedSize(static_cast<KIO::filesize_t>(received));
    emit totalSize(static_cast<KIO::filesize_t>(total));
}

#include "moc_http.cpp"
