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

#include "khttpheader.h"
#include "kdebug.h"

class KHTTPHeaderPrivate
{
public:
    int m_status;
    QMap<QString,QString> m_map;
};

KHTTPHeader::KHTTPHeader()
    : d(new KHTTPHeaderPrivate())
{
    clear();
}

KHTTPHeader::~KHTTPHeader()
{
    delete d;
}

QString KHTTPHeader::get(const QString &field) const
{
    return d->m_map.value(field.toLower());
}

void KHTTPHeader::set(const QString &field, const QString &value)
{
    d->m_map.insert(field.toLower(), value);
}

void KHTTPHeader::remove(const QString &field)
{
    d->m_map.remove(field.toLower());
}

void KHTTPHeader::clear()
{
    d->m_status = 0;
    d->m_map.clear();
}

void KHTTPHeader::parseHeader(const QByteArray &header)
{
    clear();

    bool isfirstline = true;
    foreach (const QByteArray &field, header.split('\n')) {
        const QByteArray trimmedfield = field.trimmed();
        if (trimmedfield.isEmpty()) {
            continue;
        }
        if (isfirstline) {
            const QList<QByteArray> statussplit = trimmedfield.split(' ');
            if (statussplit.size() < 2) {
                kWarning() << "Invalid status" << trimmedfield;
                continue;
            }
            d->m_status = statussplit.at(1).toInt();
            isfirstline = false;
            continue;
        }
        isfirstline = false;
        const int valuesplitterindex = trimmedfield.indexOf(':');
        if (valuesplitterindex < 0) {
            kWarning() << "Invalid field" << trimmedfield;
            continue;
        }
        const QByteArray headerkey = trimmedfield.mid(0, valuesplitterindex).trimmed();
        const QByteArray headervalue = trimmedfield.mid(valuesplitterindex + 1, trimmedfield.size() - valuesplitterindex - 1).trimmed();
        set(
            QString::fromAscii(headerkey.constData(), headerkey.size()),
            QString::fromAscii(headervalue.constData(), headervalue.size())
        );
    }
}

int KHTTPHeader::status() const
{
    return d->m_status;
}
