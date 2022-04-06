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

#ifndef KPASSWDSTORE_KDED_H
#define KPASSWDSTORE_KDED_H

#include "kdedmodule.h"

#include <QMap>

class KPasswdStorePrivate;

class KPasswdStoreModule: public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kpasswdstore")

public:
    KPasswdStoreModule(QObject *parent, const QList<QVariant>&);
    ~KPasswdStoreModule();

public Q_SLOTS:
    Q_SCRIPTABLE bool openStore(const QByteArray &cookie, const QString &storeid, const qlonglong windowid = 0);
    Q_SCRIPTABLE bool closeStore(const QByteArray &cookie, const QString &storeid, const qlonglong windowid = 0);

    Q_SCRIPTABLE void setCacheOnly(const QByteArray &cookie, const QString &storeid, const bool cacheonly);
    Q_SCRIPTABLE bool cacheOnly(const QByteArray &cookie, const QString &storeid) const;

    Q_SCRIPTABLE QString getPasswd(const QByteArray &cookie, const QString &storeid, const QByteArray &key, const qlonglong windowid = 0);
    Q_SCRIPTABLE bool storePasswd(const QByteArray &cookie, const QString &storeid, const QByteArray &key, const QString &passwd, const qlonglong windowid = 0);

private:
    typedef QMap<QByteArray, KPasswdStorePrivate*> KPasswdStoreMap;
    KPasswdStoreMap m_stores;
};

#endif // KPASSWDSTORE_KDED_H
