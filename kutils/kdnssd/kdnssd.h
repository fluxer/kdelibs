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
    Class to register and query for discovarable services.

    @since 4.21
    @warning the API is subject to change
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

    bool publishService(const QByteArray &servicetype, const uint serviceport, const QString &servicename);
    bool unpublishService();

    /*!
        @brief Starts browse request for @p servicetype, if @p servicetype is empty the result will
        include services for all service types. This will block until all services are resolved
    */
    void startBrowse(const QByteArray &servicetype = QByteArray());
    QList<KDNSSDService> services() const;

Q_SIGNALS:
    void finished();

private:
    friend KDNSSDPrivate;
    Q_DISABLE_COPY(KDNSSD);
    KDNSSDPrivate *d;
};

#endif // KDNSSD_H
