//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
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

#ifndef KBOOKMARKMENU_P_H
#define KBOOKMARKMENU_P_H

#include <QTreeWidget>

class KBookmarkGroup;

class KBookmarkTreeItem : public QTreeWidgetItem
{
public:
    KBookmarkTreeItem(QTreeWidget *tree);
    KBookmarkTreeItem(QTreeWidgetItem *parent, QTreeWidget *tree, const KBookmarkGroup &bk);
    ~KBookmarkTreeItem();
    QString address();
private:
    QString m_address;
};

class KBookmarkSettings
{
public:
    bool m_advancedaddbookmark;
    bool m_contextmenu;
    static KBookmarkSettings *s_self;
    static void readSettings();
    static KBookmarkSettings *self();
};

#endif // KBOOKMARKMENU_P_H
