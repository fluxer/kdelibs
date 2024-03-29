/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
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
#ifndef KMIMETYPE_P_H
#define KMIMETYPE_P_H

#include <QStringList>

class KMimeTypePrivate
{
public:
    KMimeTypePrivate(const QString &path)
        : m_path(path),
        m_xmlDataLoaded(false)
    {
    }

    bool inherits(const QString &mime) const;
    void ensureXmlDataLoaded() const;

    const QString m_path;
    QString m_strName;
    mutable QString m_strComment;
    mutable QStringList m_lstPatterns;
    mutable QString m_iconName; // user-specified
    mutable bool m_xmlDataLoaded;
};

#endif // KMIMETYPE_P_H
