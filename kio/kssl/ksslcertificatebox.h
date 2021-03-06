/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
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

#ifndef KSSLCERTIFICATEBOX_H
#define KSSLCERTIFICATEBOX_H

#include "kio_export.h"

#include <QtGui/QWidget>

#include <QSslCertificate>

class KSslCertificateBoxPrivate;

class KIO_EXPORT KSslCertificateBox : public QWidget
{
    Q_OBJECT
public:
    enum CertificateParty {
        Subject = 0,
        Issuer
    };

    explicit KSslCertificateBox(QWidget *parent = 0);
    ~KSslCertificateBox();

    void setCertificate(const QSslCertificate &cert, CertificateParty party);
    void clear();

    KSslCertificateBoxPrivate *const d;
};

#endif // KSSLCERTIFICATEBOX_H
