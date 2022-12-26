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
#include "klocale.h"
#include "kdebug.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
#include <QCoreApplication>
#include <QThread>
#include <QFile>
#include <QFileInfo>

#include <limits.h>

#define KHTTP_TIMEOUT 250
#define KHTTP_SLEEPTIME 50
#define KHTTP_BUFFSIZE 1024 * 1000 // 1MB

// see kdebug.areas
static const int s_khttpdebugarea = 7050;

// for reference:
// https://datatracker.ietf.org/doc/html/rfc9110
// https://datatracker.ietf.org/doc/html/rfc7230
// https://datatracker.ietf.org/doc/html/rfc7235
// https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml

static QByteArray HTTPStatusToBytes(const ushort httpstatus)
{
    switch (httpstatus) {
        case 100: {
            return QByteArray("Continue");
        }
        case 101: {
            return QByteArray("Switching Protocols");
        }
        case 102: {
            return QByteArray("Processing");
        }
        case 103: {
            return QByteArray("Early Hints");
        }
        case 200: {
            return QByteArray("OK");
        }
        case 201: {
            return QByteArray("Created");
        }
        case 202: {
            return QByteArray("Accepted");
        }
        case 203: {
            return QByteArray("Non-Authoritative Information");
        }
        case 204: {
            return QByteArray("No Content");
        }
        case 205: {
            return QByteArray("Reset Content");
        }
        case 206: {
            return QByteArray("Partial Content");
        }
        case 207: {
            return QByteArray("Multi-Status");
        }
        case 208: {
            return QByteArray("Already Reported");
        }
        case 226: {
            return QByteArray("IM Used");
        }
        case 300: {
            return QByteArray("Multiple Choices");
        }
        case 301: {
            return QByteArray("Moved Permanently");
        }
        case 302: {
            return QByteArray("Found");
        }
        case 303: {
            return QByteArray("See Other");
        }
        case 304: {
            return QByteArray("Not Modified");
        }
        case 305: {
            return QByteArray("Use Proxy");
        }
        // (Unused)
        case 306: {
            return QByteArray("Switch Proxy");
        }
        case 307: {
            return QByteArray("Temporary Redirect");
        }
        case 308: {
            return QByteArray("Permanent Redirect");
        }
        case 400: {
            return QByteArray("Bad Request");
        }
        case 401: {
            return QByteArray("Unauthorized");
        }
        case 402: {
            return QByteArray("Payment Required");
        }
        case 403: {
            return QByteArray("Forbidden");
        }
        case 404: {
            return QByteArray("Not Found");
        }
        case 405: {
            return QByteArray("Method Not Allowed");
        }
        case 406: {
            return QByteArray("Not Acceptable");
        }
        case 407: {
            return QByteArray("Proxy Authentication Required");
        }
        case 408: {
            return QByteArray("Request Timeout");
        }
        case 409: {
            return QByteArray("Conflict");
        }
        case 410: {
            return QByteArray("Gone");
        }
        case 411: {
            return QByteArray("Length Required");
        }
        case 412: {
            return QByteArray("Precondition Failed");
        }
        case 413: {
            return QByteArray("Content Too Large");
        }
        case 414: {
            return QByteArray("URI Too Long");
        }
        case 415: {
            return QByteArray("Unsupported Media Type");
        }
        case 416: {
            return QByteArray("Range Not Satisfiable");
        }
        case 417: {
            return QByteArray("Expectation Failed");
        }
        // (Unused)
        case 418: {
            return QByteArray("I'm a teapot");
        }
        case 421: {
            return QByteArray("Misdirected Request");
        }
        case 422: {
            return QByteArray("Unprocessable Content");
        }
        case 423: {
            return QByteArray("Locked");
        }
        case 424: {
            return QByteArray("Failed Dependency");
        }
        case 425: {
            return QByteArray("Too Early");
        }
        case 426: {
            return QByteArray("Upgrade Required");
        }
        case 428: {
            return QByteArray("Precondition Required");
        }
        case 429: {
            return QByteArray("Too Many Requests");
        }
        case 431: {
            return QByteArray("Request Header Fields Too Large");
        }
        case 451: {
            return QByteArray("Unavailable For Legal Reasons");
        }
        case 500: {
            return QByteArray("Internal Server Error");
        }
        case 501: {
            return QByteArray("Not Implemented");
        }
        case 502: {
            return QByteArray("Bad Gateway");
        }
        case 503: {
            return QByteArray("Service Unavailable");
        }
        case 504: {
            return QByteArray("Gateway Timeout");
        }
        case 505: {
            return QByteArray("HTTP Version Not Supported");
        }
        case 506: {
            return QByteArray("Variant Also Negotiates");
        }
        case 507: {
            return QByteArray("Insufficient Storage");
        }
        case 508: {
            return QByteArray("Loop Detected");
        }
        case 510: {
            return QByteArray("Not Extended");
        }
        case 511: {
            return QByteArray("Network Authentication Required");
        }
    }
    kWarning(s_khttpdebugarea) << "unknown HTTP status code" << httpstatus;
    return QByteArray("OK");
}

static bool shouldWriteData(const ushort httpstatus)
{
    // 1xx and 204 are exceptions
    return (httpstatus >= 200 && httpstatus != 204);
}

static const QByteArray HTTPDate(const QDateTime &datetime)
{
    Q_ASSERT(datetime.timeSpec() == Qt::UTC);
    QByteArray httpdate = datetime.toString("ddd, dd MMM yyyy hh:mm:ss").toAscii();
    httpdate.append(" GMT");
    return httpdate;
}

static QByteArray HTTPStatusToContent(const ushort httpstatus)
{
    if (!shouldWriteData(httpstatus)) {
        return QByteArray();
    }
    QByteArray httpdata("<html>\n");
    httpdata.append(QByteArray::number(httpstatus));
    httpdata.append(" ");
    httpdata.append(HTTPStatusToBytes(httpstatus));
    httpdata.append("\n</html>");
    return httpdata;
}

static KHTTPHeaders HTTPHeaders(const QString &serverid, const bool authenticate)
{
    const QByteArray httpserver = serverid.toAscii();
    KHTTPHeaders khttpheaders;
    khttpheaders.insert("Server", httpserver);
    const QByteArray httpdate = HTTPDate(QDateTime::currentDateTimeUtc());
    khttpheaders.insert("Date", httpdate);
    // optional for anything but 405, see:
    // https://www.rfc-editor.org/rfc/rfc9110.html#section-10.2.1
    khttpheaders.insert("Allow", "GET");
    // optional, see:
    // https://www.rfc-editor.org/rfc/rfc9110.html#section-14.3
    khttpheaders.insert("Accept-Ranges", "none");
    if (authenticate) {
        const QByteArray httpauthenticate = QByteArray("Basic realm=\"") + httpserver + "\"";
        khttpheaders.insert("WWW-Authenticate", httpauthenticate);
    }
    return khttpheaders;
}

static QByteArray HTTPData(const ushort httpstatus, const KHTTPHeaders &httpheaders, const qint64 datasize)
{
    QByteArray httpdata("HTTP/1.1 ");
    httpdata.append(QByteArray::number(httpstatus));
    httpdata.append(" ");
    httpdata.append(HTTPStatusToBytes(httpstatus));
    httpdata.append("\r\n");

    bool hascontenttype = false;
    foreach (const QByteArray &httpkey, httpheaders.keys()) {
        if (qstricmp(httpkey.constData(), "Content-Type") == 0) {
            hascontenttype = true;
        }
        httpdata.append(httpkey);
        httpdata.append(": ");
        httpdata.append(httpheaders.value(httpkey));
        httpdata.append("\r\n");
    }

    if (!hascontenttype) {
        kDebug(s_khttpdebugarea) << "adding Content-Type";
        httpdata.append("Content-Type: text/html\r\n");
    }

    httpdata.append("Content-Length: ");
    httpdata.append(QByteArray::number(datasize));
    httpdata.append("\r\n\r\n");

    // qDebug() << Q_FUNC_INFO << "HTTP data" << httpdata;

    return httpdata;
}

class KHTTPHeadersParser
{
public:
    void parseHeaders(const QByteArray &header, const bool authenticate);

    QByteArray method() const { return m_method; }
    QByteArray path() const { return m_path; }
    QByteArray version() const { return m_version; }
    QByteArray authUser() const { return m_authuser; }
    QByteArray authPass() const { return m_authpass; }

private:
    QByteArray m_method;
    QByteArray m_path;
    QByteArray m_version;
    QByteArray m_authuser;
    QByteArray m_authpass;
};

void KHTTPHeadersParser::parseHeaders(const QByteArray &header, const bool authenticate)
{
    bool firstline = true;
    foreach (const QByteArray &line, header.split('\n')) {
        if (line.isEmpty()) {
            firstline = false;
            continue;
        }
        if (firstline) {
            const QList<QByteArray> splitline = line.split(' ');
            if (splitline.size() == 3) {
                // qDebug() << Q_FUNC_INFO << "method, path and version" << splitline;
                m_method = splitline.at(0).trimmed().toUpper();
                m_path = splitline.at(1).trimmed();
                m_version = splitline.at(2).trimmed().toUpper();
            }
        } else if (authenticate && qstrnicmp(line.constData(), "Authorization", 13) == 0) {
            const QList<QByteArray> splitline = line.split(':');
            // qDebug() << Q_FUNC_INFO << "auth" << splitline;
            if (splitline.size() == 2) {
                const QByteArray authdata = splitline.at(1).trimmed();
                const QList<QByteArray> splitauth = authdata.split(' ');
                if (splitauth.size() == 2) {
                    const QByteArray authbase64 = QByteArray::fromBase64(splitauth.at(1).trimmed());
                    const QList<QByteArray> splitbase64 = authbase64.split(':');
                    if (splitbase64.size() == 2) {
                        m_authuser = splitbase64.at(0);
                        m_authpass = splitbase64.at(1);
                    }
                }
            }
        }
        firstline = false;
    }
    // qDebug() << Q_FUNC_INFO << m_method << m_path << m_version << m_authuser << m_authpass;
}

class KHTTPPrivate : public QObject
{
    Q_OBJECT
public:
    KHTTPPrivate(QObject *parent);

private Q_SLOTS:
    void slotNewConnection();

public:
    bool start(const QHostAddress &address, const quint16 port);
    void stop();

    QString serverid;
    QByteArray authusername;
    QByteArray authpassword;
    QString errorstring;
    QTcpServer* tcpserver;

private:
    void writeResponse(const ushort httpstatus, const bool authenticate, QTcpSocket *client);

    QAtomicInt m_ref;
};

KHTTPPrivate::KHTTPPrivate(QObject *parent)
    : QObject(parent),
    tcpserver(nullptr),
    m_ref(0)
{
    serverid = QCoreApplication::applicationName();

    // NOTE: the default maximum for pending connections is 30
    tcpserver = new QTcpServer(this);
    connect(tcpserver, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
}

void KHTTPPrivate::slotNewConnection()
{
    if (m_ref.load() != 0) {
        kDebug(s_khttpdebugarea) << "not accepting client connections";
        return;
    }

    QTcpSocket *client = tcpserver->nextPendingConnection();
    kDebug(s_khttpdebugarea) << "new client" << client->peerAddress() << client->peerPort();

    if (!client->waitForReadyRead()) {
        client->disconnectFromHost();
        client->deleteLater();
        kWarning(s_khttpdebugarea) << "client timed out";
        return;
    }

    QByteArray httpbuffer(KHTTP_BUFFSIZE, '\0');
    const qint64 httpclientresult = client->read(httpbuffer.data(), httpbuffer.size());
    if (client->bytesAvailable() > 0) {
        kWarning(s_khttpdebugarea) << "client payload too large" << client->peerAddress() << client->peerPort();
        writeResponse(413, false, client);
        return;
    }
    const QByteArray clientdata = httpbuffer.mid(0, httpclientresult);
    // qDebug() << Q_FUNC_INFO << "request" << clientdata;

    const bool requiresauthorization = (!authusername.isEmpty() && !authpassword.isEmpty());

    KHTTPHeadersParser khttpheadersparser;
    khttpheadersparser.parseHeaders(clientdata, requiresauthorization);
    kDebug(s_khttpdebugarea) << "client request" << khttpheadersparser.method() << khttpheadersparser.path() << khttpheadersparser.version();

    if (khttpheadersparser.method() != "GET") {
        writeResponse(405, false, client);
        return;
    }

    if (khttpheadersparser.version() != "HTTP/1.1") {
        writeResponse(505, false, client);
        return;
    }

    if (requiresauthorization &&
        (khttpheadersparser.authUser() != authusername || khttpheadersparser.authPass() != authpassword)) {
        writeResponse(401, true, client);
        return;
    }

    KHTTP* khttp = qobject_cast<KHTTP*>(parent());
    Q_ASSERT(khttp);
    const QByteArray responseurl = khttpheadersparser.path();
    QByteArray responsedata;
    ushort responsestatus = 404;
    KHTTPHeaders khttpheaders = HTTPHeaders(serverid, requiresauthorization);
    QString responsefilepath;
    khttp->respond(responseurl, &responsedata, &responsestatus, &khttpheaders, &responsefilepath);

    if (!responsefilepath.isEmpty()) {
        QFile httpfile(responsefilepath);
        if (!httpfile.open(QFile::ReadOnly)) {
            kWarning(s_khttpdebugarea) << "could not open" << responsefilepath;
            writeResponse(500, false, client);
            return;
        }

        bool haslastmodified = false;
        foreach (const QByteArray &httpkey, khttpheaders.keys()) {
            if (qstricmp(httpkey.constData(), "Last-Modified") == 0) {
                haslastmodified = true;
                break;
            }
        }
        if (!haslastmodified) {
            kDebug(s_khttpdebugarea) << "adding Last-Modified";
            const QDateTime responsefilelastmodified = QFileInfo(responsefilepath).lastModified();
            const QByteArray httpfilelastmodified = HTTPDate(responsefilelastmodified.toUTC());
            khttpheaders.insert("Last-Modified", httpfilelastmodified);
        }

        kDebug(s_khttpdebugarea) << "sending file to client" << responsefilepath << khttpheaders;
        const QByteArray httpdata = HTTPData(responsestatus, khttpheaders, httpfile.size());
        client->write(httpdata);
        client->flush();

        qint64 httpfileresult = httpfile.read(httpbuffer.data(), httpbuffer.size());
        while (httpfileresult > 0) {
            if (m_ref.load() != 0) {
                // NOTE: at that point it is not safe to access the client pointer
                kDebug(s_khttpdebugarea) << "aborting client request";
                return;
            }

            client->write(httpbuffer.constData(), httpfileresult);
            client->flush();

            // TODO: this check should be done before every write
            if (client->state() != QTcpSocket::ConnectedState) {
                kDebug(s_khttpdebugarea) << "client disconnected while writing file" << client->peerAddress() << client->peerPort();
                break;
            }

            QCoreApplication::processEvents(QEventLoop::AllEvents, KHTTP_TIMEOUT);
            QThread::msleep(KHTTP_SLEEPTIME);

            httpfileresult = httpfile.read(httpbuffer.data(), httpbuffer.size());
        }

        client->flush();
        kDebug(s_khttpdebugarea) << "done with client" << client->peerAddress() << client->peerPort();
        client->disconnectFromHost();
        client->deleteLater();
        return;
    }

    kDebug(s_khttpdebugarea) << "sending data to client";
    if (responsedata.isEmpty()) {
        responsedata = HTTPStatusToContent(responsestatus);
    }
    const QByteArray httpdata = HTTPData(responsestatus, khttpheaders, responsedata.size());
    client->write(httpdata);
    client->flush();
    if (shouldWriteData(responsestatus)) {
        client->write(responsedata);
        client->flush();
    }
    kDebug(s_khttpdebugarea) << "done with client" << client->peerAddress() << client->peerPort();
    client->disconnectFromHost();
    client->deleteLater();
}

bool KHTTPPrivate::start(const QHostAddress &address, const quint16 port)
{
    m_ref.store(0);
    return tcpserver->listen(address, port);
}

void KHTTPPrivate::stop()
{
    m_ref.store(1);
    tcpserver->close();
}

void KHTTPPrivate::writeResponse(const ushort httpstatus, const bool authenticate, QTcpSocket *client)
{
    kDebug(s_khttpdebugarea) << "sending status to client" << httpstatus << client->peerAddress() << client->peerPort();
    KHTTPHeaders khttpheaders = HTTPHeaders(serverid, authenticate);
    const QByteArray contentdata = HTTPStatusToContent(httpstatus);
    const QByteArray httpdata = HTTPData(httpstatus, khttpheaders, contentdata.size());
    client->write(httpdata);
    client->flush();
    if (shouldWriteData(httpstatus)) {
        client->write(contentdata);
        client->flush();
    }
    kDebug(s_khttpdebugarea) << "done with client" << client->peerAddress() << client->peerPort();
    client->disconnectFromHost();
    client->deleteLater();
}


KHTTP::KHTTP(QObject *parent)
    : QObject(parent),
    d(new KHTTPPrivate(this))
{
}

KHTTP::~KHTTP()
{
    stop();
    delete d;
}

void KHTTP::setServerID(const QString &id)
{
    d->serverid = id;
}

bool KHTTP::setAuthenticate(const QByteArray &username, const QByteArray &password)
{
    d->errorstring.clear();
    if (username.isEmpty() || password.isEmpty()) {
        d->errorstring = i18n("User name or password is empty");
        d->authusername.clear();
        d->authpassword.clear();
        return false;
    }
    d->authusername = username;
    d->authpassword = password;
    return true;
}

bool KHTTP::start(const QHostAddress &address, const quint16 port)
{
    return d->start(address, port);
}

bool KHTTP::stop()
{
    d->stop();
    return true;
}

QString KHTTP::errorString() const
{
    if (!d->errorstring.isEmpty()) {
        return d->errorstring;
    }
    return d->tcpserver->errorString();
}

#include "khttp.moc"
