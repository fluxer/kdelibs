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

#include <kio/kio_export.h>

#include <QObject>
#include <QMap>
#include <QHostAddress>

/*!
    Map that holds information about HTTP headers.

    @ingroup Types

    @since 4.21
    @see KHTTP
*/
typedef QMap<QByteArray,QByteArray> KHTTPHeaders;

class KHTTPPrivate;

/*!
    Class to serve data over HTTP.

    @since 4.21
    @see KHTTPHeaders
*/
class KIO_EXPORT KHTTP : public QObject
{
    Q_OBJECT
public:
    /*!
        @brief Contructs object with @p parent
    */
    KHTTP(QObject *parent = nullptr);
    ~KHTTP();

    /*!
        @brief Sets @p username and @p password to be used for authentication.
        @note The authentication method used is basic.
    */
    bool setAuthenticate(const QByteArray &username, const QByteArray &password);

    /*!
        @brief Starts serving data for requests at @p address on @p port.
        @note If port is 0 (the default) then a random port is chosen.
    */
    bool start(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    bool stop();

    /*!
        @brief Returns human-readable description of the error that occured, if @p start() returns
        @p false for example it may be used along with @p KMessageBox to notify the user about the
        error.
    */
    QString errorString() const;

protected:
    /*!
        @brief Reimplement this method to send back data to clients when @p url is requested.
        @p outdata is the content, @p outhttpstatus is a standard HTTP status (e.g. 404) and
        @p outheaders is map of additional headers to be send (e.g. "Content-Type"). Either
        @p outdata must be non-empty or @p outfilepath must be pointing to a file, the other
        arguments (@p outhttpstatus and @p outheaders) are optional.
        @note Prefer @p outfilepath over @p outdata for serving files, Large File Support is
        transparent.
        @link https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
        @link https://en.wikipedia.org/wiki/Large-file_support
    */
    virtual void respond(
        const QByteArray &url,
        QByteArray *outdata, ushort *outhttpstatus, KHTTPHeaders *outheaders, QString *outfilepath
    ) = 0;
    
private:
    friend KHTTPPrivate;
    Q_DISABLE_COPY(KHTTP);
    KHTTPPrivate *d;
};

#endif // KHTTP_H
