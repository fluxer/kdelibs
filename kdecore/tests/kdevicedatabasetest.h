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

#ifndef KDEVICEDATABASETEST_H
#define KDEVICEDATABASETEST_H

#include "kdevicedatabase.h"

#include <QtCore/QObject>

class KDeviceDatabaseTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testPCI();
    void testUSB();

    void testKDE4PCI();
    void testKDE4USB();

private:
    bool m_iskdelibsinstalled;
    KDeviceDatabase m_devicedb;
};

#endif // KDEVICEDATABASETEST_H
