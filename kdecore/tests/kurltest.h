/*
    Copyright (c) 2005 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KURLTEST_H
#define KURLTEST_H

#include <QObject>

class KUrlTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testUpUrl_data();
    void testUpUrl();
    void testHash_data();
    void testHash();
    void testQueryAndFragment_data();
    void testQueryAndFragment();
    void testCleanPath_data();
    void testCleanPath();
    void testEquals_data();
    void testEquals();
    void testUriMode();
    void testToLocalFile();
    void testUrl_data();
    void testUrl();
    void testToStringList();
};

#endif // KURLTEST_H
