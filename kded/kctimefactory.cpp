/*  This file is part of the KDE libraries
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kctimefactory.h"
#include <ksycoca.h>
#include <ksycocatype.h>
#include <kdebug.h>

#include <assert.h>

static inline QString key(const QString &path, const QByteArray& resource)
{
    return QString::fromLatin1(resource) + QLatin1Char('|') + path;
}

void KCTimeDict::addCTime(const QString &path, const QByteArray& resource, quint32 ctime)
{
    assert(!path.isEmpty());
    m_hash.insert(key(path, resource), ctime );
}

quint32 KCTimeDict::ctime(const QString &path, const QByteArray& resource) const
{
    return m_hash.value(key(path, resource), 0);
}

void KCTimeDict::remove(const QString &path, const QByteArray &resource)
{
    m_hash.remove(key(path, resource));
}

void KCTimeDict::dump() const
{
    kDebug() << m_hash.keys();
}

QStringList KCTimeDict::resourceList() const
{
    QStringList resources;
    QHashIterator<QString,quint32> it(m_hash);
    while (it.hasNext()) {
        it.next();
        const QString key = it.key();
        resources << key.left(key.indexOf('|'));
    }
    return resources;
}

void KCTimeDict::load(QDataStream &str)
{
    QString key;
    quint32 ctime;
    while (true) {
        KSycocaEntry::read(str, key);
        str >> ctime;
        if (key.isEmpty()) break;
        m_hash.insert(key, ctime);
    }
}

void KCTimeDict::save(QDataStream &str) const
{
    QHashIterator<QString,quint32> it(m_hash);
    while (it.hasNext()) {
        it.next();
        str << it.key() << it.value();
    }
    str << QString() << (quint32) 0;
}

///////////

KCTimeInfo::KCTimeInfo()
    : KSycocaFactory( KST_CTimeInfo ), m_ctimeDict()
{
    if (!KSycoca::self()->isBuilding()) {
        QDataStream* str = stream();
        (*str) >> m_dictOffset;
    } else {
        m_dictOffset = 0;
    }
}

KCTimeInfo::~KCTimeInfo()
{
}

void
KCTimeInfo::saveHeader(QDataStream &str)
{
  KSycocaFactory::saveHeader(str);

  str << m_dictOffset;
}

void KCTimeInfo::save(QDataStream &str)
{
    KSycocaFactory::save(str);

    m_dictOffset = str.device()->pos();
    m_ctimeDict.save(str);
    const int endOfFactoryData = str.device()->pos();
    saveHeader(str);
    str.device()->seek(endOfFactoryData);
}

KCTimeDict KCTimeInfo::loadDict() const
{
    KCTimeDict dict;
    QDataStream* str = stream();
    assert(str);
    str->device()->seek(m_dictOffset);
    dict.load(*str);
    return dict;
}
