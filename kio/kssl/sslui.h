/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Andreas Hartmetz <ahartmetz@gmail.com>
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

#ifndef KSSLUI_H
#define KSSLUI_H

#include <kio/kio_export.h>
#include <QSslSocket>

namespace KIO {
namespace SslUi {

/**
 * This class can hold all the necessary data from a KTcpSocket to ask the user
 * to continue connecting in the face of SSL errors.
 * It can be used to carry the data for the UI over time or over thread boundaries.
 *
 * @see: KSslCertificateManager::askIgnoreSslErrors()
 */
class KIO_EXPORT KSslErrorUiData
{
public:
    /**
     * Default construct an instance with no useful data.
     */
    KSslErrorUiData();
    /**
     * Create an instance and initialize it with SSL error data from @p socket.
     */
    KSslErrorUiData(const QSslSocket *socket);
    KSslErrorUiData(const KSslErrorUiData &other);
    KSslErrorUiData &operator=(const KSslErrorUiData &);
    /**
     * Destructor
     * @since 4.7
     */
    ~KSslErrorUiData();
    class Private;
private:
    friend class Private;
    Private *const d;
};

enum RulesStorage {
    RecallRules = 1, ///< apply stored certificate rules (typically ignored errors)
    StoreRules = 2, ///< make new ignore rules from the user's choice and store them
    RecallAndStoreRules = 3 ///< apply stored rules and store new rules
};

bool KIO_EXPORT askIgnoreSslErrors(const QSslSocket *socket,
                                   RulesStorage storedRules = RecallAndStoreRules);
bool KIO_EXPORT askIgnoreSslErrors(const KSslErrorUiData &uiData,
                                   RulesStorage storedRules = RecallAndStoreRules);
}
}

#endif
