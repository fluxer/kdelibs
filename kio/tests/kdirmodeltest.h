/* This file is part of the KDE project
   Copyright (C) 2006 David Faure <faure@kde.org>

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
#ifndef KDIRMODELTEST_H
#define KDIRMODELTEST_H

#include <QtCore/QObject>
#include <ktempdir.h>
#include <QtCore/qdatetime.h>
#include <kdirmodel.h>
#include <QtTest/QtTest>

// If you disable this, you need to change all exitLoop into quit in connect() statements...
#define USE_QTESTEVENTLOOP

class KDirModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testItemForIndex();
    void testIndexForItem();

private:
    KDirLister* m_dirLister;
    KDirModel* m_dirModel;
    KTempDir* m_tempDir;
};

#endif
