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

#include "qtest_kde.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kdebug.h"

static void setupArea(const char* area, const int output, const QString &filename)
{
    KConfig kconfig(QString::fromLatin1("kdebugrc"), KConfig::NoGlobals);
    KConfigGroup kconfiggroup = kconfig.group(area);
    kconfiggroup.writeEntry("InfoOutput", output);
    kconfiggroup.writePathEntry("InfoFilename", filename);
    kconfiggroup.writeEntry("WarnOutput", output);
    kconfiggroup.writePathEntry("WarnFilename", filename);
    kconfiggroup.writeEntry("ErrorOutput", output);
    kconfiggroup.writePathEntry("ErrorFilename", filename);
    kconfiggroup.writeEntry("FatalOutput", output);
    kconfiggroup.writePathEntry("FatalFilename", filename);
    kconfiggroup.writeEntry("AbortFatal", false);
}

class KDebugTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void output_data();
    void output();
};

QTEST_KDEMAIN_CORE(KDebugTest)

void KDebugTest::initTestCase()
{
}

void KDebugTest::cleanupTestCase()
{
    ::unsetenv("KDE_DEBUG_METHODNAME");
    ::unsetenv("KDE_DEBUG_TIMESTAMP");
    ::unsetenv("KDE_DEBUG_COLOR");
}

void KDebugTest::output_data()
{
    QTest::addColumn<int>("areaoutput");
    QTest::addColumn<bool>("areafancy");
    QTest::newRow("file") << 0 << false;
    QTest::newRow("file (fancy)") << 0 << true;
    QTest::newRow("messagebox") << 1 << false;
    QTest::newRow("messagebox") << 1 << true;
    QTest::newRow("shell") << 2 << false;
    QTest::newRow("shell (fancy)") << 2 << true;
    QTest::newRow("syslog") << 3 << false;
    QTest::newRow("syslog (fancy)") << 3 << true;
    QTest::newRow("off") << 4 << false;
    QTest::newRow("off (fancy)") << 4 << true;
}

void KDebugTest::output()
{
    QFETCH(int, areaoutput);
    QFETCH(bool, areafancy);

    if (areafancy) {
        ::setenv("KDE_DEBUG_METHODNAME", "1", 1);
        ::setenv("KDE_DEBUG_TIMESTAMP", "1", 1);
        ::setenv("KDE_DEBUG_COLOR", "1", 1);
    } else {
        ::unsetenv("KDE_DEBUG_METHODNAME");
        ::unsetenv("KDE_DEBUG_TIMESTAMP");
        ::unsetenv("KDE_DEBUG_COLOR");
    }

    const QString areafilename = QFile::encodeName(KDEBINDIR "/123.log");
    QFile::remove(areafilename);
    setupArea("123", areaoutput, areafilename);

    kDebug(123) << "foo" << "info";
    kWarning(123) << "bar" << "warning";
    kError(123) << "foo" << "error";
    kFatal(123) << "bar" << "fatal";
}


#include "kdebugtest.moc"
