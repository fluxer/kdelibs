/*
    Copyright 2013 Albert Astals Cid <aacid@kde.org>

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

#include <QtTestGui>
#include <QComboBox>
#include <QDialogButtonBox>

#include "kcolorbuttontest.h"
#include "qtest_kde.h"
#include "kcolorbutton.h"
#include "kcolordialog.h"
#include "kstandarddirs.h"

QTEST_KDEMAIN(KColorButtonTest, GUI)

#include "moc_kcolorbuttontest.cpp"

void KColorButtonTest::initTestCase()
{
    if (KStandardDirs::locate("config", "colors/40.colors").isEmpty()) {
        // e.g. kdelibs not installed
        QSKIP("40.colors not found", SkipAll);
    }

    black40Colors.setHsv(-1, 0, 0);
}

void KColorButtonTest::testChangeAndCancel()
{
    KColorButton colorButton(Qt::red);
    colorButton.show();
    QVERIFY(QTest::qWaitForWindowShown(&colorButton));
    QTest::mouseClick(&colorButton, Qt::LeftButton);
    KColorDialog *dialog = colorButton.findChild<KColorDialog*>();
    QVERIFY(dialog != NULL);
    QVERIFY(QTest::qWaitForWindowShown(dialog));
    KColorCells *cells = dialog->findChild<KColorCells*>();
    QVERIFY(cells != NULL);
    QTest::mouseClick(cells->viewport(), Qt::LeftButton, 0, QPoint(1, 1));
    QCOMPARE(dialog->color(), black40Colors);
    dialog->reject();
    QCOMPARE(colorButton.color(), QColor(Qt::red));
}

void KColorButtonTest::testDoubleClickChange()
{
    KColorButton colorButton(Qt::red);
    colorButton.show();
    QVERIFY(QTest::qWaitForWindowShown(&colorButton));
    QTest::mouseClick(&colorButton, Qt::LeftButton);
    KColorDialog *dialog = colorButton.findChild<KColorDialog*>();
    QVERIFY(dialog != NULL);
    QVERIFY(QTest::qWaitForWindowShown(dialog));
    KColorCells *cells = dialog->findChild<KColorCells*>();
    QVERIFY(cells != NULL);
    QTest::mouseDClick(cells->viewport(), Qt::LeftButton, 0, QPoint(1, 1));
    QCOMPARE(colorButton.color(), black40Colors);
}

void KColorButtonTest::testOkChange()
{
    KColorButton colorButton(Qt::red);
    colorButton.show();
    QVERIFY(QTest::qWaitForWindowShown(&colorButton));
    QTest::mouseClick(&colorButton, Qt::LeftButton);
    KColorDialog *dialog = colorButton.findChild<KColorDialog*>();
    QVERIFY(dialog != NULL);
    QVERIFY(QTest::qWaitForWindowShown(dialog));
    KColorCells *cells = dialog->findChild<KColorCells*>();
    QVERIFY(cells != NULL);
    QTest::mouseClick(cells->viewport(), Qt::LeftButton, 0, QPoint(1, 1));
    QCOMPARE(dialog->color(), black40Colors);

    QSignalSpy okClickedSpy(dialog, SIGNAL(okClicked()));
    const QDialogButtonBox *buttonBox = dialog->findChild<QDialogButtonBox*>();
    const QList<QAbstractButton *> buttons = buttonBox->buttons();
    foreach(QAbstractButton *button, buttons) {
        if (buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole) {
            QTest::mouseClick(button, Qt::LeftButton);
            break;
        }
    }
    QCOMPARE(okClickedSpy.count(), 1);

    QCOMPARE(colorButton.color(), black40Colors);
}

void KColorButtonTest::testRecentColorsPick()
{
    KColorButton colorButton(Qt::red);
    colorButton.show();
    QVERIFY(QTest::qWaitForWindowShown(&colorButton));
    QTest::mouseClick(&colorButton, Qt::LeftButton);
    KColorDialog *dialog = colorButton.findChild<KColorDialog*>();
    QVERIFY(dialog != NULL);
    QVERIFY(QTest::qWaitForWindowShown(dialog));

    QComboBox *combo = dialog->findChild<QComboBox*>();
    combo->setFocus();
    QTest::keyPress(combo, Qt::Key_Up);
    QTest::keyPress(combo, Qt::Key_Up);

    KColorCells *cells = dialog->findChild<KColorCells*>();
    QVERIFY(cells != NULL);
    QTest::mouseMove(cells->viewport(), QPoint(1, 1));
    QTest::mouseClick(cells->viewport(), Qt::LeftButton, 0, QPoint(30, 1));
    const QColor color = dialog->color();
    dialog->accept();
    QCOMPARE(colorButton.color(), color);
}
