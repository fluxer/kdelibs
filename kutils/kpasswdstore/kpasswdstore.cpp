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

#include <QApplication>
#include <QDBusInterface>
#include <QDBusReply>
#include <QCryptographicHash>

#include <sys/types.h>
#include <unistd.h>

static QByteArray getCookie()
{
    // TODO: config knob for this, eavesdropping will be piece of cake
    return QByteArray::number(::getuid());
#if 0
    return QByteArray::number(::getpid());
    return qRandomUuid();
#endif
}

KPasswdStore::KPasswdStore(QObject *parent)
    : QObject(parent),
    m_interface("org.kde.kded", "/modules/kpasswdstore", "org.kde.kpasswdstore"),
    m_cookie(getCookie()),
    m_storeid(QApplication::applicationName())
{
}

KPasswdStore::~KPasswdStore()
{
}

void KPasswdStore::setStoreID(const QString &id)
{
    m_storeid = id;
}

bool KPasswdStore::openStore(const qlonglong windowid)
{
    QDBusReply<bool> result = m_interface.call("openStore", m_cookie, m_storeid, windowid);
    return result.value();
}

void KPasswdStore::setCacheOnly(const bool cacheonly)
{
    m_interface.call("setCacheOnly", m_cookie, m_storeid, cacheonly);
}

bool KPasswdStore::cacheOnly() const
{
    QDBusReply<bool> result = m_interface.call("cacheOnly", m_cookie, m_storeid);
    return result.value();
}

bool KPasswdStore::hasPasswd(const QByteArray &key, const qlonglong windowid)
{
    return !getPasswd(key, windowid).isEmpty();
}

QString KPasswdStore::getPasswd(const QByteArray &key, const qlonglong windowid)
{
    QDBusReply<QString> result = m_interface.call("getPasswd", m_cookie, m_storeid, key, windowid);
    return result.value();
}

bool KPasswdStore::storePasswd(const QByteArray &key, const QString &passwd, const qlonglong windowid)
{
    QDBusReply<bool> result = m_interface.call("storePasswd", m_cookie, m_storeid, key, passwd, windowid);
    return result.value();
}

QByteArray KPasswdStore::makeKey(const QString &string)
{
#if QT_VERSION >= 0x041200
    return QCryptographicHash::hash(string.toUtf8(), QCryptographicHash::KAT).toHex();
#else
    return QCryptographicHash::hash(string.toUtf8(), QCryptographicHash::Sha256).toHex();
#endif
}
