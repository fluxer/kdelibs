/* This file is part of the KDE libraries

   Copyright (c) 2007 Jos van den Oever <jos@vandenoever.info>
                 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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
#include "knfotranslator_p.h"

KFileMetaInfoItem::KFileMetaInfoItem()
    : d(new KFileMetaInfoItemPrivate())
{
}

KFileMetaInfoItem::KFileMetaInfoItem(const KFileMetaInfoItem& item)
    : d(item.d)
{
}

KFileMetaInfoItem::KFileMetaInfoItem(const QString& key, const QVariant& value)
    : d(new KFileMetaInfoItemPrivate())
{
    d->value = value;
    d->key = key;
    d->name = KNfoTranslator::translation(KUrl(d->key));
}

KFileMetaInfoItem::~KFileMetaInfoItem()
{
}

const KFileMetaInfoItem& KFileMetaInfoItem::operator=(const KFileMetaInfoItem& item)
{
    d = item.d;
    return item;
}

const QString& KFileMetaInfoItem::key() const
{
    return d->key;
}

const QString& KFileMetaInfoItem::name() const
{
    return d->name;
}

const QVariant& KFileMetaInfoItem::value() const 
{
    return d->value;
}

bool KFileMetaInfoItem::isValid() const
{
    return true;
}

QString KFileMetaInfoItem::toString() const
{
#warning TODO: implement properly
    return d->value.toString();
}
