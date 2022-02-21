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

#include "khttpheader.h"
#include "kdebug.h"

class KHTTPHeaderPrivate
{
public:
    int m_status;
    QByteArray m_path;
    QMap<QString,QString> m_map;
};

KHTTPHeader::KHTTPHeader()
    : d(new KHTTPHeaderPrivate())
{
    clear();
}

KHTTPHeader::~KHTTPHeader()
{
    delete d;
}

void KHTTPHeader::setUrl(const KUrl &url)
{
    set(QLatin1String("Host"), url.host());
    d->m_path = url.path().toAscii();
}

QString KHTTPHeader::get(const QString &field) const
{
    return d->m_map.value(field.toLower());
}

void KHTTPHeader::set(const QString &field, const QString &value)
{
    d->m_map.insert(field.toLower(), value);
}

void KHTTPHeader::remove(const QString &field)
{
    d->m_map.remove(field.toLower());
}

void KHTTPHeader::clear()
{
    d->m_status = HTTP_Unknown;
    d->m_map.clear();
    // mandatory
    set(QLatin1String("Accept"), QLatin1String("*/*"));
    // HTTP/1.1
#if 0
    set(QLatin1String("Connection"), QLatin1String("keep-alive"));
    set(QLatin1String("Keep-Alive"), QLatin1String("timeout=30, max=10"));
#endif
}

QByteArray KHTTPHeader::toHead(const HTTPVersion version) const
{
    QByteArray result("HEAD ");
    result.append(d->m_path);
    switch (version) {
        case HTTP_1_0: {
            result.append(" HTTP/1.0\r\n");
            break;
        }
        case HTTP_1_1: {
            result.append(" HTTP/1.1\r\n");
            break;
        }
        case HTTP_2_0: {
            result.append(" HTTP/2.0\r\n");
            break;
        }
        default: {
            kWarning() << "Invalid HTTP version" << version;
            result.append(" HTTP/1.0\r\n");
            break;
        }
    }
    QMapIterator<QString,QString> iter(d->m_map);
    while (iter.hasNext()) {
        iter.next();
        result.append(iter.key().toAscii());
        result.append(": ");
        result.append(iter.value().toAscii());
        result.append("\r\n");
    }
    result.append("\r\n");
    return result;
}

QByteArray KHTTPHeader::toGet(const HTTPVersion version) const
{
    QByteArray result("GET ");
    result.append(d->m_path);
    switch (version) {
        case HTTP_1_0: {
            result.append(" HTTP/1.0\r\n");
            break;
        }
        case HTTP_1_1: {
            result.append(" HTTP/1.1\r\n");
            break;
        }
        case HTTP_2_0: {
            result.append(" HTTP/2.0\r\n");
            break;
        }
        default: {
            kWarning() << "Invalid HTTP version" << version;
            result.append(" HTTP/1.0\r\n");
            break;
        }
    }
    QMapIterator<QString,QString> iter(d->m_map);
    while (iter.hasNext()) {
        iter.next();
        result.append(iter.key().toAscii());
        result.append(": ");
        result.append(iter.value().toAscii());
        result.append("\r\n");
    }
    result.append("\r\n");
    return result;
}

void KHTTPHeader::parseHeader(const QByteArray &header)
{
    clear();

    bool isfirstline = true;
    foreach (const QByteArray &field, header.split('\n')) {
        if (field.trimmed().isEmpty()) {
            continue;
        }
        if (isfirstline && field.startsWith("HTTP/")) {
            const QList<QByteArray> statussplit = field.split(' ');
            if (statussplit.size() < 3) {
                kWarning() << "Invalid status" << field;
                continue;
            }
            d->m_status = statussplit.at(1).toInt();
            isfirstline = false;
            continue;
        }
        isfirstline = false;
        const int valuesplitterindex = field.indexOf(':');
        if (valuesplitterindex < 0) {
            kWarning() << "Invalid field" << field;
            continue;
        }
        const QByteArray headerkey = field.mid(0, valuesplitterindex).trimmed();
        const QByteArray headervalue = field.mid(valuesplitterindex + 1, field.size() - valuesplitterindex - 1).trimmed();
        set(
            QString::fromAscii(headerkey.constData(), headerkey.size()),
            QString::fromAscii(headervalue.constData(), headervalue.size())
        );
    }
}

QString KHTTPHeader::path() const
{
    return d->m_path;
}

int KHTTPHeader::status() const
{
    return d->m_status;
}

QString KHTTPHeader::errorString() const
{
    switch (status()) {
        case HTTP_Continue: {
            return QString::fromLatin1("The server has received the request headers and the client should proceed to send the request body");
        }
        case HTTP_SwitchingProtocol: {
            return QString::fromLatin1("The requester has asked the server to switch protocols and the server has agreed to do so");
        }
        case HTTP_Processing: {
            return QString::fromLatin1("A WebDAV request may contain many sub-requests involving file operations, requiring a long time to complete the request");
        }
        case HTTP_EarlyHints: {
            return QString::fromLatin1("Used to return some response headers before final HTTP message");
        }
        case HTTP_Ok: {
            return QString::fromLatin1("Standard response for successful HTTP requests");
        }
        case HTTP_Created: {
            return QString::fromLatin1("The request has been fulfilled, resulting in the creation of a new resource");
        }
        case HTTP_Accepted: {
            return QString::fromLatin1("The request has been accepted for processing, but the processing has not been completed");
        }
        case HTTP_NonAuthoritativeInformation: {
            return QString::fromLatin1("The server is a transforming proxy");
        }
        case HTTP_NoContent: {
            return QString::fromLatin1("The server successfully processed the request, and is not returning any content");
        }
        case HTTP_ResetContent: {
            return QString::fromLatin1("The server successfully processed the request, asks that the requester reset its document view, and is not returning any content");
        }
        case HTTP_PartialContent: {
            return QString::fromLatin1("The server is delivering only part of the resource (byte serving) due to a range header sent by the client");
        }
        case HTTP_MultiStatus: {
            return QString::fromLatin1("The message body that follows is by default an XML message and can contain a number of separate response codes, depending on how many sub-requests were made");
        }
        case HTTP_AlreadyReported: {
            return QString::fromLatin1("The members of a DAV binding have already been enumerated in a preceding part of the (multistatus) response, and are not being included again");
        }
        case HTTP_IMUsed: {
            return QString::fromLatin1("The server has fulfilled a request for the resource, and the response is a representation of the result of one or more instance-manipulations applied to the current instance");
        }
        case HTTP_MultipleChoices: {
            return QString::fromLatin1("Indicates multiple options for the resource from which the client may choose (via agent-driven content negotiation)");
        }
        case HTTP_MovedPermanently: {
            return QString::fromLatin1("This and all future requests should be directed to the given URI");
        }
        case HTTP_Found: {
            return QString::fromLatin1("Tells the client to look at (browse to) another URL");
        }
        case HTTP_SeeOther: {
            return QString::fromLatin1("The response to the request can be found under another URI using the GET method");
        }
        case HTTP_NotModified: {
            return QString::fromLatin1("Indicates that the resource has not been modified since the version specified by the request headers If-Modified-Since or If-None-Match");
        }
        case HTTP_UseProxy: {
            return QString::fromLatin1("The requested resource is available only through a proxy, the address for which is provided in the response");
        }
        case HTTP_SwitchProxy: {
            return QString::fromLatin1("Subsequent requests should use the specified proxy");
        }
        case HTTP_TemporaryRedirect: {
            return QString::fromLatin1("In this case, the request should be repeated with another URI; however, future requests should still use the original URI");
        }
        case HTTP_PermanentRedirect: {
            return QString::fromLatin1("This and all future requests should be directed to the given URI");
        }
        case HTTP_BadRequest: {
            return QString::fromLatin1("The server cannot or will not process the request due to an apparent client error");
        }
        case HTTP_Unauthorized: {
            return QString::fromLatin1("Similar to 403 Forbidden, but specifically for use when authentication is required and has failed or has not yet been provided");
        }
        case HTTP_PaymentRequired: {
            return QString::fromLatin1("Reserved for future use");
        }
        case HTTP_Forbidden: {
            return QString::fromLatin1("The request contained valid data and was understood by the server, but the server is refusing action");
        }
        case HTTP_NotFound: {
            return QString::fromLatin1("The requested resource could not be found but may be available in the future");
        }
        case HTTP_MethodNotAllowed: {
            return QString::fromLatin1("A request method is not supported for the requested resource");
        }
        case HTTP_NotAcceptable: {
            return QString::fromLatin1("The requested resource is capable of generating only content not acceptable according to the Accept headers sent in the request");
        }
        case HTTP_ProxyAuthenticationRequired: {
            return QString::fromLatin1("The client must first authenticate itself with the proxy");
        }
        case HTTP_RequestTimeout: {
            return QString::fromLatin1("The server timed out waiting for the request");
        }
        case HTTP_Conflict: {
            return QString::fromLatin1("Indicates that the request could not be processed because of conflict in the current state of the resource");
        }
        case HTTP_Gone: {
            return QString::fromLatin1("Indicates that the resource requested is no longer available and will not be available again");
        }
        case HTTP_LengthRequired: {
            return QString::fromLatin1("The request did not specify the length of its content, which is required by the requested resource");
        }
        case HTTP_PreconditionFailed: {
            return QString::fromLatin1("The server does not meet one of the preconditions that the requester put on the request header fields");
        }
        case HTTP_PayloadTooLarge: {
            return QString::fromLatin1("The request is larger than the server is willing or able to process");
        }
        case HTTP_URITooLong: {
            return QString::fromLatin1("The URI provided was too long for the server to process");
        }
        case HTTP_UnsupportedMediaType: {
            return QString::fromLatin1("The request entity has a media type which the server or resource does not support");
        }
        case HTTP_RangeNotSatisfiable: {
            return QString::fromLatin1("The client has asked for a portion of the file (byte serving), but the server cannot supply that portion");
        }
        case HTTP_ExpectationFailed: {
            return QString::fromLatin1("The server cannot meet the requirements of the Expect request-header field");
        }
        case HTTP_Imateapot: {
            return QString::fromLatin1("This code was defined in 1998 as one of the traditional IETF April Fools' jokes");
        }
        case HTTP_MisdirectedRequest: {
            return QString::fromLatin1("The request was directed at a server that is not able to produce a response");
        }
        case HTTP_UnprocessableEntity: {
            return QString::fromLatin1("The request was well-formed but was unable to be followed due to semantic errors");
        }
        case HTTP_Locked: {
            return QString::fromLatin1("The resource that is being accessed is locked");
        }
        case HTTP_FailedDependency: {
            return QString::fromLatin1("The request failed because it depended on another request and that request failed");
        }
        case HTTP_TooEarly: {
            return QString::fromLatin1("Indicates that the server is unwilling to risk processing a request that might be replayed");
        }
        case HTTP_UpgradeRequired: {
            return QString::fromLatin1("The client should switch to a different protocol such as TLS/1.3, given in the Upgrade header field");
        }
        case HTTP_PreconditionRequired: {
            return QString::fromLatin1("The origin server requires the request to be conditional");
        }
        case HTTP_TooManyRequests: {
            return QString::fromLatin1("The user has sent too many requests in a given amount of time");
        }
        case HTTP_RequestHeaderFieldsTooLarge: {
            return QString::fromLatin1("The server is unwilling to process the request because either an individual header field, or all the header fields collectively, are too large");
        }
        case HTTP_UnavailableForLegalReasons: {
            return QString::fromLatin1("A server operator has received a legal demand to deny access to a resource or to a set of resources that includes the requested resource");
        }
        case HTTP_InternalServerError: {
            return QString::fromLatin1("A generic error message, given when an unexpected condition was encountered and no more specific message is suitable");
        }
        case HTTP_NotImplemented: {
            return QString::fromLatin1("The server either does not recognize the request method, or it lacks the ability to fulfil the request");
        }
        case HTTP_BadGateway: {
            return QString::fromLatin1("The server was acting as a gateway or proxy and received an invalid response from the upstream server");
        }
        case HTTP_ServiceUnavailable: {
            return QString::fromLatin1("The server cannot handle the request (because it is overloaded or down for maintenance)");
        }
        case HTTP_GatewayTimeout: {
            return QString::fromLatin1("The server was acting as a gateway or proxy and did not receive a timely response from the upstream server");
        }
        case HTTP_HTTPVersionNotSupported: {
            return QString::fromLatin1("The server does not support the HTTP protocol version used in the request");
        }
        case HTTP_VariantAlsoNegotiates: {
            return QString::fromLatin1("Transparent content negotiation for the request results in a circular reference");
        }
        case HTTP_InsufficientStorage: {
            return QString::fromLatin1("The server is unable to store the representation needed to complete the request");
        }
        case HTTP_LoopDetected: {
            return QString::fromLatin1("The server detected an infinite loop while processing the request (sent instead of 208 Already Reported).");
        }
        case HTTP_NotExtended: {
            return QString::fromLatin1("Further extensions to the request are required for the server to fulfil it");
        }
        case HTTP_NetworkAuthenticationRequired: {
            return QString::fromLatin1("The client needs to authenticate to gain network access");
        }
    }
    return QString::fromLatin1("Unknown error");
} 
