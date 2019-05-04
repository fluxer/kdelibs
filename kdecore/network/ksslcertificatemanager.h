/* This file is part of the KDE project
 *
 * Copyright (C) 2007, 2008, 2010 Andreas Hartmetz <ahartmetz@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDE_KSSLCERTIFICATEMANAGER_H
#define INCLUDE_KSSLCERTIFICATEMANAGER_H

#include "kdecore_export.h"

#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslError>
#include <QtCore/qdatetime.h>
#include <QSslCertificate>

class KSslCertificateRulePrivate;
class KSslCertificateManagerPrivate;

//### document this... :/
class KDECORE_EXPORT KSslCertificateRule
{
public:
    KSslCertificateRule(const QSslCertificate &cert = QSslCertificate(),
                        const QString &hostName = QString());
    KSslCertificateRule(const KSslCertificateRule &other);
    ~KSslCertificateRule();
    KSslCertificateRule &operator=(const KSslCertificateRule &other);

    QSslCertificate certificate() const;
    QString hostName() const;
    void setExpiryDateTime(const QDateTime &dateTime);
    QDateTime expiryDateTime() const;
    void setRejected(bool rejected);
    bool isRejected() const;
    bool isErrorIgnored(QSslError::SslError error) const;
    void setIgnoredErrors(const QList<QSslError::SslError> &errors);
    void setIgnoredErrors(const QList<QSslError> &errors);
    QList<QSslError::SslError> ignoredErrors() const;
    QList<QSslError::SslError> filterErrors(const QList<QSslError::SslError> &errors) const;
    QList<QSslError> filterErrors(const QList<QSslError> &errors) const;
private:
    KSslCertificateRulePrivate *const d;
};


//### document this too... :/
class KDECORE_EXPORT KSslCertificateManager
{
public:
    static KSslCertificateManager *self();
    void setRule(const KSslCertificateRule &rule);
    void clearRule(const KSslCertificateRule &rule);
    void clearRule(const QSslCertificate &cert, const QString &hostName);
    KSslCertificateRule rule(const QSslCertificate &cert, const QString &hostName) const;

    QList<QSslCertificate> caCertificates() const;

    static QList<QSslError> nonIgnorableErrors(const QList<QSslError> &);
    static QList<QSslError::SslError> nonIgnorableErrors(const QList<QSslError::SslError> &);

private:
    friend class KSslCertificateManagerContainer;
    friend class KSslCertificateManagerPrivate;
    KSslCertificateManager();
    ~KSslCertificateManager();

    KSslCertificateManagerPrivate *const d;
};


#endif
