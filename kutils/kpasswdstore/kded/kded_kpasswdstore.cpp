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

#include "kded_kpasswdstore.h"
#include "kpluginfactory.h"
#include "kpluginloader.h"

K_PLUGIN_FACTORY(KPasswdStoreModuleFactory, registerPlugin<KPasswdStoreModule>();)
K_EXPORT_PLUGIN(KPasswdStoreModuleFactory("kpasswdstore"))

KPasswdStoreModule::KPasswdStoreModule(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
{
}

KPasswdStoreModule::~KPasswdStoreModule()
{
    qDeleteAll(m_stores);
}

bool KPasswdStoreModule::openStore(const QByteArray &cookie, const QString &storeid, const qlonglong windowid)
{
    KPasswdStoreMap::iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        KPasswdStoreImpl *store = it.value();
        if (store->storeID() == storeid) {
            return store->openStore(windowid);
        }
        it++;
    }
    KPasswdStoreImpl *store = new KPasswdStoreImpl(storeid);
    m_stores.insert(cookie, store);
    return store->openStore(windowid);
}

bool KPasswdStoreModule::closeStore(const QByteArray &cookie, const QString &storeid, const qlonglong windowid)
{
    KPasswdStoreMap::iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        KPasswdStoreImpl *store = it.value();
        if (store->storeID() == storeid) {
            return store->closeStore();
        }
        it++;
    }
    return false;
}

void KPasswdStoreModule::setCacheOnly(const QByteArray &cookie, const QString &storeid, const bool cacheonly)
{
    KPasswdStoreMap::iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        KPasswdStoreImpl *store = it.value();
        if (store->storeID() == storeid) {
            store->setCacheOnly(cacheonly);
        }
        it++;
    }
}

bool KPasswdStoreModule::cacheOnly(const QByteArray &cookie, const QString &storeid) const
{
    KPasswdStoreMap::const_iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        const KPasswdStoreImpl *store = it.value();
        if (store->storeID() == storeid) {
            return store->cacheOnly();
        }
        it++;
    }
    return false;
}

QString KPasswdStoreModule::getPasswd(const QByteArray &cookie, const QString &storeid, const QByteArray &key, const qlonglong windowid)
{
    KPasswdStoreMap::iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        KPasswdStoreImpl *store = it.value();
        if (store->storeID() == storeid) {
            return store->getPasswd(key, windowid);
        }
        it++;
    }
    KPasswdStoreImpl *store = new KPasswdStoreImpl(storeid);
    m_stores.insert(cookie, store);
    return store->getPasswd(key, windowid);
}

bool KPasswdStoreModule::storePasswd(const QByteArray &cookie, const QString &storeid, const QByteArray &key, const QString &passwd, const qlonglong windowid)
{
    KPasswdStoreMap::iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        KPasswdStoreImpl *store = it.value();
        if (store->storeID() == storeid) {
            return store->storePasswd(key, passwd, windowid);
        }
        it++;
    }
    KPasswdStoreImpl *store = new KPasswdStoreImpl(storeid);
    m_stores.insert(cookie, store);
    return store->storePasswd(key, passwd, windowid);
}

#include "moc_kded_kpasswdstore.cpp"
