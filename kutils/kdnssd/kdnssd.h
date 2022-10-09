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

#ifndef KDNSSD_H
#define KDNSSD_H

#include "kdnssd_export.h"

#include <QObject>
#include <QList>

/*!
    Structure that holds information about discovarable service.

    @note The @p type member is in the form _\<service\>._\<socket\> where service is one as
    described at @link http://www.dns-sd.org/ServiceTypes.html and socket is usually either
    "tcp" or "udp".

    @note The @p url member is synthetized URL in the form of \<service\>://\<hostname\>:\<port\>
    and it may not be accessible via KIO. For example there is no KIO slave for "sftp-ssh" service
    however there is KIO slave that can handle SFTP (SSH) protocol and @p url may have to adjusted
    before attempting to open it with external program. In addition to that,
    @p KProtocolInfo::isKnownProtocol() may be used to check if there is KIO slave that can handle
    @p url since some KIO slaves are optional (such as the SFTP (SSH) slave).

    @since 4.21
    @see KDNSSD
    @link http://www.dns-sd.org/ServiceTypes.html
*/
struct KDNSSDService
{
    QString name;
    QByteArray type;
    QString domain;
    QString hostname;
    QString url;
    uint port;
};

class KDNSSDPrivate;

/*!
    Class to publish and query for discovarable services.

    @since 4.21
    @see KDNSSDService
    @link http://www.dns-sd.org/ServiceTypes.html
*/
class KDNSSD_EXPORT KDNSSD : public QObject
{
    Q_OBJECT
public:
    /*!
        @brief Contructs object with @p parent
    */
    KDNSSD(QObject *parent = nullptr);
    ~KDNSSD();

    static bool isSupported();

    /*!
        @brief Publishes service of type @p servicetype on port @p serviceport with name
        @p servicename on the default domain.
        @note Only one service may be published per KDNSSD object.
    */
    bool publishService(const QByteArray &servicetype, const uint serviceport, const QString &servicename);
    /*!
        @brief Unpublishes service previously published via @p publishService().
        @note Service published via @p publishService() is automatically unpublished once the
        KDNSSD object is destroyed.
    */
    bool unpublishService();

    /*!
        @brief Starts browse request for @p servicetype, if @p servicetype is empty the result will
        include services for all service types. This will block until all services are resolved.
        When the query is done @p finished() signal is emited and the result can be retrieved via
        the @p services() method.
    */
    bool startBrowse(const QByteArray &servicetype = QByteArray());
    QList<KDNSSDService> services() const;

    /*!
        @brief Returns human-readable description of the error that occured, if
        @p publishService() returns @p false for example it may be used along with @p KMessageBox
        to notify the user about the error.
    */
    QString errorString() const;

Q_SIGNALS:
    void finished();

private:
    friend KDNSSDPrivate;
    Q_DISABLE_COPY(KDNSSD);
    KDNSSDPrivate *d;
};

#endif // KDNSSD_H
