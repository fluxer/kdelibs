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

#include "kpasswdstore.h"
#include "kconfig.h"
#include "kconfiggroup.h"

#include <QApplication>
#include <QDBusInterface>
#include <QDBusReply>
#include <QCryptographicHash>

#include <sys/types.h>
#include <unistd.h>

static QByteArray getCookie()
{
    KConfig kconfig("kpasswdstorerc", KConfig::SimpleConfig);
    KConfigGroup kconfiggroup = kconfig.group("KPasswdStore");
    const QByteArray cookietype = kconfiggroup.readEntry("Cookie", QByteArray()).toLower();
    if (cookietype == "pid") {
        return QByteArray::number(::getpid());
    } else if (cookietype == "random") {
        return qRandomUuid();
    }
    return QByteArray::number(::getuid());
}

class KPasswdStorePrivate
{
public:
    KPasswdStorePrivate();

    QDBusInterface interface;
    QByteArray cookie;
    QString storeid;
};

KPasswdStorePrivate::KPasswdStorePrivate()
    : interface("org.kde.kded", "/modules/kpasswdstore", "org.kde.kpasswdstore"),
    cookie(getCookie()),
    storeid(QApplication::applicationName())
{
}

KPasswdStore::KPasswdStore(QObject *parent)
    : QObject(parent),
    d(new KPasswdStorePrivate())
{
}

KPasswdStore::~KPasswdStore()
{
    delete d;
}

void KPasswdStore::setStoreID(const QString &id)
{
    d->storeid = id;
}

bool KPasswdStore::openStore(const qlonglong windowid)
{
    QDBusReply<bool> result = d->interface.call("openStore", d->cookie, d->storeid, windowid);
    return result.value();
}

void KPasswdStore::setCacheOnly(const bool cacheonly)
{
    d->interface.call("setCacheOnly", d->cookie, d->storeid, cacheonly);
}

bool KPasswdStore::cacheOnly() const
{
    QDBusReply<bool> result = d->interface.call("cacheOnly", d->cookie, d->storeid);
    return result.value();
}

bool KPasswdStore::hasPasswd(const QByteArray &key, const qlonglong windowid)
{
    return !getPasswd(key, windowid).isEmpty();
}

QString KPasswdStore::getPasswd(const QByteArray &key, const qlonglong windowid)
{
    QDBusReply<QString> result = d->interface.call("getPasswd", d->cookie, d->storeid, key, windowid);
    return result.value();
}

bool KPasswdStore::storePasswd(const QByteArray &key, const QString &passwd, const qlonglong windowid)
{
    QDBusReply<bool> result = d->interface.call("storePasswd", d->cookie, d->storeid, key, passwd, windowid);
    return result.value();
}

QByteArray KPasswdStore::makeKey(const QString &string)
{
    return QCryptographicHash::hash(string.toUtf8(), QCryptographicHash::KAT).toHex();
}

#include "moc_kpasswdstore.cpp"
