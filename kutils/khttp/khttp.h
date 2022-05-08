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

#ifndef KHTTP_H
#define KHTTP_H

#include "khttp_export.h"

#include <QObject>
#include <QHostAddress>

typedef QMap<QByteArray,QByteArray> KHTTPHeaders;

class KHTTPPrivate;

/*!
    Class to serve data over HTTP(S).

    @since 4.21
    @warning the API is subject to change
*/
class KHTTP_EXPORT KHTTP : public QObject
{
    Q_OBJECT
public:
    /*!
        @brief Contructs object with @p parent
    */
    KHTTP(QObject *parent = nullptr);
    ~KHTTP();

    /*!
        @brief Sets @p keydata and @p certdata to be used for TLS/SSL handshake, if the key
        requires password it must also be provided as @p password.
        @note HTTP requests to the server address will not be redirected, clients must request
        HTTPS address. For example if TLS/SSL certificate is set "http://foo.bar" will not be
        reachable (no data is send) however "https://foo.bar" will be, unless external means are
        used to redirect the request. This is the case only when non-standard ports are used, if
        HTTP server runs on port 80 and HTTPS server runs on port 443 then both are accessible but
        the client will most likely be making requests to the HTTP server on port 80.
    */
    bool setCertificate(const QByteArray &keydata, const QByteArray &certdata, const QByteArray &password = QByteArray());
    /*!
        @brief Sets @p username and @p password to be used for authentication with @p message as
        content to be send to the client when authentication fails.
        @note The authentication method used is basic
    */
    bool setAuthenticate(const QByteArray &username, const QByteArray &password, const QString &message);

    bool start(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    bool stop();
    QString errorString() const;

protected:
    /*!
        @brief Reimplement this method to send back data to clients when @p url is requested.
        @p outdata is the content, @p httpstatus is a standard HTTP status (e.g. 404) and
        @p outheaders is map of additional headers to be send (e.g. "Content-Type"). All input
        arguments (@p outdata, @p httpstatus and @p outheaders) are optional however if none is
        changed 404 response will be send to clients with no content.
    */
    virtual void respond(const QByteArray &url, QByteArray *outdata, ushort *httpstatus, KHTTPHeaders *outheaders) = 0;
    
private:
    friend KHTTPPrivate;
    Q_DISABLE_COPY(KHTTP);
    KHTTPPrivate *d;
};

#endif // KHTTP_H
