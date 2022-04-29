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

#include "kdevicedatabasetest.h"
#include "qtest_kde.h"

QTEST_KDEMAIN_CORE(KDeviceDatabaseTest)

void KDeviceDatabaseTest::initTestCase()
{
    const QString pcivendor = m_devicedb.lookupPCIVendor("0001");
    const QString usbvendor = m_devicedb.lookupUSBVendor("0001");
    m_iskdelibsinstalled = (!pcivendor.isEmpty() && !usbvendor.isEmpty());
}

void KDeviceDatabaseTest::testPCI()
{
    if (!m_iskdelibsinstalled) {
        QSKIP("kdelibs not installed", SkipAll);
    }

    QCOMPARE(m_devicedb.lookupPCIVendor("0001"), QLatin1String("SafeNet (wrong ID)"));
    QCOMPARE(m_devicedb.lookupPCIDevice("0010", "8139"), QLatin1String("AT-2500TX V3 Ethernet"));
    QCOMPARE(m_devicedb.lookupPCIClass("00"), QLatin1String("Unclassified device"));
    QCOMPARE(m_devicedb.lookupPCISubClass("00", "00"), QLatin1String("Non-VGA unclassified device"));
    QCOMPARE(m_devicedb.lookupPCIProtocol("01", "01", "00"), QLatin1String("ISA Compatibility mode-only controller"));
}

void KDeviceDatabaseTest::testUSB()
{
    if (!m_iskdelibsinstalled) {
        QSKIP("kdelibs not installed", SkipAll);
    }

    QCOMPARE(m_devicedb.lookupUSBVendor("0001"), QLatin1String("Fry's Electronics"));
    QCOMPARE(m_devicedb.lookupUSBDevice("0001", "7778"), QLatin1String("Counterfeit flash drive [Kingston]"));
    QCOMPARE(m_devicedb.lookupUSBClass("00"), QLatin1String("(Defined at Interface level)"));
    QCOMPARE(m_devicedb.lookupUSBSubClass("01", "01"), QLatin1String("Control Device"));
    QCOMPARE(m_devicedb.lookupUSBProtocol("02", "02", "00"), QLatin1String("None"));
}

#include "moc_kdevicedatabasetest.cpp"
