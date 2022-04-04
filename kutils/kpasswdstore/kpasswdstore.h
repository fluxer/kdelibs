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

#ifndef KPASSWDSTORE_H
#define KPASSWDSTORE_H

#include "kpasswdstore_export.h"

#include <QObject>

class KPasswdStorePrivate;

/*!
    Class to store and retrieve passwords.

    The password used for encrypting and decrypting the store will be asked for
    upon the first request and again after 30sec of inactivity.

    @code
    KPasswdStore kpasswdstore;
    kpasswdstore.setStoreID("myid");
    qDebug() << kpasswdstore.storePasswd("mykey", "mypass");
    qDebug() << kpasswdstore.getPasswd("mykey");
    @endcode

    @since 4.21
    @warning the API is subject to change
*/
class KPASSWDSTORE_EXPORT KPasswdStore : public QObject
{
public:
    /*!
        @brief Contructs object with @p parent
    */
    KPasswdStore(QObject *parent = nullptr);
    ~KPasswdStore();

    /*!
        @brief Sets the store ID to @p id
        @note The ID is @p QApplication::applicationName() by default
    */
    void setStoreID(const QString &id);

    /*!
        @brief If @p cacheonly is @p true then no permanent storage is used and
        passwords store is discarded when the object is destroyed. Whenever
        called the cache is also cleared
        @note Caching only is disabled by default
    */
    void setCacheOnly(const bool cacheonly);
    /*!
        @brief Returns @p true if password are cached only, @p false otherwise
    */
    bool cacheOnly() const;

    /*!
        @brief Returns @p true if there is password for the given @p key in the
        password store, @p false otherwise
    */
    bool hasPasswd(const QByteArray &key, const qlonglong windowid = 0);
    /*!
        @brief Retrieves password for the given @p key from the password store
    */
    QString getPasswd(const QByteArray &key, const qlonglong windowid = 0) const;
    /*!
        @brief Stores @p passwd with the given @p key in the password store
    */
    bool storePasswd(const QByteArray &key, const QString &passwd, const qlonglong windowid = 0);

    /*!
        @brief Makes a unique key from @p string for use with @p getPasswd() and @p storePasswd()
    */
    static QByteArray makeKey(const QString &string);

private:
    Q_DISABLE_COPY(KPasswdStore);
    KPasswdStorePrivate *d;
};

#endif // KPASSWDSTORE_H
