/* This file is part of the KDE libraries
    Copyright (C) 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>

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

#include "httpobjecttest.h"

#include <qtest_kde.h>

#include <QtCore/QByteArray>
#include <kdebug.h>

#include "moc_httpobjecttest.cpp"

QTEST_KDEMAIN(HeaderObjectTest, NoGUI)

static void runTest()
{
    TestHTTPProtocol protocol("http", QByteArray(), "tcp://");

    protocol.testParseContentDisposition(QLatin1String("inline; filename=\"foo.pdf\""));
}

void HeaderObjectTest::runAllTests()
{
    runTest();
}

TestHTTPProtocol::TestHTTPProtocol ( const QByteArray& protocol, const QByteArray& pool, const QByteArray& app )
  : HTTPProtocol(protocol, pool, app)
{
}

TestHTTPProtocol::~TestHTTPProtocol()
{
}

void TestHTTPProtocol::testParseContentDisposition ( const QString& disposition )
{
  parseContentDisposition(disposition);
}
