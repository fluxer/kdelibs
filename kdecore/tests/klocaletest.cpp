/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "klocaletest.h"
#include "qtest_kde.h"
#include "klocale.h"
#include "kdebug.h"

#include "moc_klocaletest.cpp"

void KLocaleTest::initTestCase()
{
}

void KLocaleTest::languages()
{
    QStringList installed = KLocale::installedLanguages();
    if (installed.size() == 0) {
        QSKIP("This test requires translations", SkipSingle);
        return;
    }
    QVERIFY(installed.contains("en_GB"));
}

QTEST_KDEMAIN_CORE(KLocaleTest)
