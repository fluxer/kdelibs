/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
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
#ifndef KSYCOCARESOURCELIST_H
#define KSYCOCARESOURCELIST_H

#include <QtCore/QList>
#include <QtCore/QString>

struct KSycocaResource
{
   QString resource;
   QString extension;
};

class KSycocaResourceList : public QList<KSycocaResource>
{
public:
   KSycocaResourceList() { }

   void add(const QString &resource, const QString &extension)
   {
     KSycocaResource res;
     res.resource = resource;
     res.extension = extension;
     append(res);
   }
};

#endif
