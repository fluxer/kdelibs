/* This file is part of the KDE libraries
    Copyright (c) 2006 David Faure <faure@kde.org>

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

#include "qtest_kde.h"
#include "kglobalsettingstest.h"
#include "moc_kglobalsettingstest.cpp"

QTEST_KDEMAIN( KGlobalSettingsTest, GUI )

#include <kglobalsettings.h>
#include <kdebug.h>
#include <QtCore/QProcess>
#include <QtCore/QEventLoop>
#include <QtDBus/QtDBus>

/**
 * The strategy of this test is:
 * We install QSignalSpy instances on many signals from KGlobalSettings::self(),
 * and then we get another process (kglobalsettingsclient) to call emitChange(),
 * and we check that the corresponding signals are emitted, i.e. that our process
 * got the dbus signal.
 *
 */

void KGlobalSettingsTest::initTestCase()
{
    QDBusConnectionInterface *bus = 0;
    if (!QDBusConnection::sessionBus().isConnected() || !(bus = QDBusConnection::sessionBus().interface())) {
        QFAIL("Session bus not found");
    }
}

#define CREATE_ALL_SPYS \
    KGlobalSettings* settings = KGlobalSettings::self(); \
    settings->activate(KGlobalSettings::ApplySettings | KGlobalSettings::ListenForChanges);                                                 \
    QSignalSpy palette_spy( settings, SIGNAL(kdisplayPaletteChanged()) ); \
    QSignalSpy font_spy( settings, SIGNAL(kdisplayFontChanged()) ); \
    QSignalSpy style_spy( settings, SIGNAL(kdisplayStyleChanged()) ); \
    QSignalSpy appearance_spy( settings, SIGNAL(appearanceChanged()) )

static void callClient( const QString& opt, const char* signalToWaitFor ) {
    QVERIFY(QFile::exists(KDEBINDIR "/kdeui-kglobalsettingsclient"));
    QVERIFY(QProcess::execute(KDEBINDIR "/kdeui-kglobalsettingsclient", QStringList(opt)) == 0);

    QVERIFY(QTest::kWaitForSignal(KGlobalSettings::self(), signalToWaitFor, 5000));
}

void KGlobalSettingsTest::testPaletteChange()
{
    CREATE_ALL_SPYS;
    callClient("-p", SIGNAL(kdisplayPaletteChanged()));
    QCOMPARE(palette_spy.size(), 1);
    QCOMPARE(font_spy.size(), 0);
    QCOMPARE(style_spy.size(), 0);
    QCOMPARE(appearance_spy.size(), 1);
}

void KGlobalSettingsTest::testFontChange()
{
    CREATE_ALL_SPYS;
    callClient("-f", SIGNAL(kdisplayFontChanged()));
    QCOMPARE(palette_spy.size(), 0);
    QCOMPARE(font_spy.size(), 1);
    QCOMPARE(style_spy.size(), 0);
    QCOMPARE(appearance_spy.size(), 1);
}
