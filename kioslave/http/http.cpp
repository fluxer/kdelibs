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
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>

#include <sys/types.h>
#include <unistd.h>

static inline QByteArray HTTPMIMEType(const QByteArray &contenttype)
{
    const QList<QByteArray> splitcontenttype = contenttype.split(';');
    if (splitcontenttype.isEmpty()) {
        return QByteArray("application/octet-stream");
    }
    return splitcontenttype.at(0);
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
    : SlaveBase("http", pool, app)
{
}

HttpProtocol::~HttpProtocol()
{
    kDebug(7103);
}

void HttpProtocol::get(const KUrl &url)
{
    kDebug(7103) << url.prettyUrl();

    QNetworkAccessManager netmanager(this);
    QNetworkDiskCache netcache(this);
    QNetworkRequest netrequest(url);

    // metadata from scheduler
    kDebug(7103) << metaData("Languages") << metaData("Charsets") << metaData("CacheDir") << metaData("UserAgent");
    if (hasMetaData("Languages")) {
        netrequest.setRawHeader("Accept-Language", metaData("Languages").toAscii());
    }
    if (hasMetaData("Charsets")) {
        netrequest.setRawHeader("Accept-Charset", metaData("Charsets").toAscii());
    }
    if (hasMetaData("CacheDir")) {
        netcache.setCacheDirectory(metaData("CacheDir"));
    }
    if (hasMetaData("UserAgent")) {
        netrequest.setRawHeader("User-Agent", metaData("UserAgent").toAscii());
    }
    netmanager.setCache(&netcache);

    QNetworkReply* netreply = netmanager.get(netrequest);
    connect(netreply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(slotProgress(qint64,qint64)));
    while (!netreply->isFinished()) {
        QCoreApplication::processEvents();
    }

    if (netreply->error() != QNetworkReply::NoError) {
        kWarning(7103) << netreply->url() << netreply->error();
        error(KIO::ERR_COULD_NOT_CONNECT, url.prettyUrl());
        return;
    }

    const QVariant netredirect = netreply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (netredirect.isValid()) {
        const QUrl netredirecturl = netreply->url().resolved(netredirect.toUrl());
        kDebug(7103) << "Redirecting to" << netredirecturl;
        redirection(netredirecturl);
        finished();
        return;
    }

    kDebug(7103) << "Headers" << netreply->rawHeaderPairs();

    const QByteArray httpmimetype = HTTPMIMEType(netreply->rawHeader("content-type"));
    kDebug(7103) << "MIME type" << httpmimetype;
    emit mimeType(httpmimetype);

    data(netreply->readAll());

    finished();
}

void HttpProtocol::slotProgress(qint64 received, qint64 total)
{
    kDebug(7103) << "Received" << received << "from" << total;
    emit processedSize(static_cast<KIO::filesize_t>(received));
    emit totalSize(static_cast<KIO::filesize_t>(total));
}

#include "moc_http.cpp"
