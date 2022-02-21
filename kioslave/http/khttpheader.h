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

#ifndef KHTTPHEADER_H
#define KHTTPHEADER_H

#include <kurl.h>

class KHTTPHeaderPrivate;

class KHTTPHeader
{
public:
    // for reference:
    // https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
    enum HTTPStatus {
        HTTP_Unknown = 0,
        // 1xx informational response
        HTTP_Continue = 100,
        HTTP_SwitchingProtocol = 101,
        HTTP_Processing = 102,
        HTTP_EarlyHints = 103,
        // 2xx successful
        HTTP_Ok = 200,
        HTTP_Created = 201,
        HTTP_Accepted = 202,
        HTTP_NonAuthoritativeInformation = 203,
        HTTP_NoContent = 204,
        HTTP_ResetContent = 205,
        HTTP_PartialContent = 206,
        HTTP_MultiStatus = 207,
        HTTP_AlreadyReported = 208,
        HTTP_IMUsed = 226,
        // 3xx redirection
        HTTP_MultipleChoices = 300,
        HTTP_MovedPermanently = 301,
        HTTP_Found = 302,
        HTTP_SeeOther = 303,
        HTTP_NotModified = 304,
        HTTP_UseProxy = 305,
        HTTP_SwitchProxy = 306,
        HTTP_TemporaryRedirect = 307,
        HTTP_PermanentRedirect = 308,
        // 4xx client error
        HTTP_BadRequest = 400,
        HTTP_Unauthorized = 401,
        HTTP_PaymentRequired = 402,
        HTTP_Forbidden = 403,
        HTTP_NotFound = 404,
        HTTP_MethodNotAllowed = 405,
        HTTP_NotAcceptable = 406,
        HTTP_ProxyAuthenticationRequired = 407,
        HTTP_RequestTimeout = 408,
        HTTP_Conflict = 409,
        HTTP_Gone = 410,
        HTTP_LengthRequired = 411,
        HTTP_PreconditionFailed = 412,
        HTTP_PayloadTooLarge = 413,
        HTTP_URITooLong = 414,
        HTTP_UnsupportedMediaType = 415,
        HTTP_RangeNotSatisfiable = 416,
        HTTP_ExpectationFailed = 417,
        HTTP_Imateapot = 418,
        HTTP_MisdirectedRequest = 421,
        HTTP_UnprocessableEntity = 422,
        HTTP_Locked = 423,
        HTTP_FailedDependency = 424,
        HTTP_TooEarly = 425,
        HTTP_UpgradeRequired = 426,
        HTTP_PreconditionRequired = 428,
        HTTP_TooManyRequests = 429,
        HTTP_RequestHeaderFieldsTooLarge = 431,
        HTTP_UnavailableForLegalReasons = 451,
        // 5xx server error
        HTTP_InternalServerError = 500,
        HTTP_NotImplemented = 501,
        HTTP_BadGateway = 502,
        HTTP_ServiceUnavailable = 503,
        HTTP_GatewayTimeout = 504,
        HTTP_HTTPVersionNotSupported = 505,
        HTTP_VariantAlsoNegotiates = 506,
        HTTP_InsufficientStorage = 507,
        HTTP_LoopDetected = 508,
        HTTP_NotExtended = 510,
        HTTP_NetworkAuthenticationRequired = 511
    };

    enum HTTPVersion {
        HTTP_1_0 = 1,
        HTTP_1_1 = 11,
        HTTP_2_0 = 20
    };

    KHTTPHeader();
    ~KHTTPHeader();

    void setUrl(const KUrl &url);

    QString get(const QString &field) const;
    void set(const QString &field, const QString &value);
    void remove(const QString &field);
    void clear();

    void parseHeader(const QByteArray &header);
    QString path() const;
    int status() const;
    QString errorString() const;

    QByteArray toHead(const HTTPVersion version = HTTP_1_0) const;
    QByteArray toGet(const HTTPVersion version = HTTP_1_0) const;

private:
    Q_DISABLE_COPY(KHTTPHeader);
    KHTTPHeaderPrivate *d;
};

#endif // KHTTPHEADER_H
