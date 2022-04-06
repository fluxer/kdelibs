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

#ifndef KPASSWDSTOREIMPL_H
#define KPASSWDSTOREIMPL_H

#include "kpasswdstore_export.h"

#include <QString>
#include <QMap>
#include <QElapsedTimer>

class KPasswdStoreImpl
{
public:
    KPasswdStoreImpl(const QString &id);
    ~KPasswdStoreImpl();

    QString storeID() const;

    bool openStore(const qlonglong windowid);
    bool closeStore();

    void setCacheOnly(const bool cacheonly);
    bool cacheOnly() const;

    QString getPasswd(const QByteArray &key, const qlonglong windowid);
    bool storePasswd(const QByteArray &key, const QString &passwd, const qlonglong windowid);

private:
    bool ensurePasswd(const qlonglong windowid, const bool showerror, bool *cancel);
    bool hasPasswd() const;
    void clearPasswd();

    QString decryptPasswd(const QString &passwd, bool *ok);
    QString encryptPasswd(const QString &passwd, bool *ok);

    bool m_cacheonly;
    QString m_storeid;
    QString m_passwdstore;
    QMap<QByteArray, QString> m_cachemap;

#if defined(HAVE_OPENSSL)
    static QByteArray genBytes(const QByteArray &data, const int length);

    QByteArray m_passwd;
    QByteArray m_passwdiv;
    QElapsedTimer m_passwdtimer;
#endif
};


#endif // KPASSWDSTOREIMPL_H
