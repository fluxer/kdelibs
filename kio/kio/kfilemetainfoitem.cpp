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

#include "kfilemetainfoitem.h"
#include "kfilemetainfoitem_p.h"
#include "kfilemetainfo.h"

KFileMetaInfoItem::KFileMetaInfoItem()
    : d(new KFileMetaInfoItemPrivate())
{
}

KFileMetaInfoItem::KFileMetaInfoItem(const KFileMetaInfoItem &other)
    : d(other.d)
{
}

KFileMetaInfoItem::KFileMetaInfoItem(const QString& key, const QString& value)
    : d(new KFileMetaInfoItemPrivate())
{
    d->value = value;
    d->key = key;
    d->name = KFileMetaInfo::name(d->key);
}

KFileMetaInfoItem::~KFileMetaInfoItem()
{
}

const KFileMetaInfoItem& KFileMetaInfoItem::operator=(const KFileMetaInfoItem& other)
{
    d = other.d;
    return other;
}

const QString& KFileMetaInfoItem::key() const
{
    return d->key;
}

const QString& KFileMetaInfoItem::value() const
{
    return d->value;
}

const QString& KFileMetaInfoItem::name() const
{
    return d->name;
}

bool KFileMetaInfoItem::isValid() const
{
    return (!d->key.isEmpty() && !d->value.isEmpty());
}

bool KFileMetaInfoItem::operator<(const KFileMetaInfoItem &other) const
{
    return d->name < other.d->name;
}
