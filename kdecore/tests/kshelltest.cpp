/* This file is part of the KDE libraries
    Copyright (c) 2003,2007-2008 Oswald Buddenhagen <ossi@kde.org>
    Copyright (c) 2005 Thomas Braxton <brax108@cox.net>

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

#include <kshell.h>
#include <kuser.h>

#include <qtest_kde.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDir>

class KShellTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void tildeExpand();
    void quoteArg();
    void joinArgs();
    void splitJoin();
    void quoteSplit();
    void quoteSplit_data();
    void abortOnMeta();
};

void
KShellTest::tildeExpand()
{
    QString me(KUser().loginName());
    QCOMPARE(KShell::tildeExpand("~"), QDir::homePath());
    QCOMPARE(KShell::tildeExpand("~/dir"), QString(QDir::homePath()+"/dir"));
    QCOMPARE(KShell::tildeExpand('~' + me), QDir::homePath());
    QCOMPARE(KShell::tildeExpand('~' + me + "/dir"), QString(QDir::homePath()+"/dir"));
    QCOMPARE(KShell::tildeExpand("\\~" + me), QString('~' + me));
}

void
KShellTest::quoteArg()
{
    QCOMPARE(KShell::quoteArg("a space"), QString("'a space'"));
}

void
KShellTest::joinArgs()
{
    QStringList list;
    list << "this" << "is" << "a" << "test";
    QCOMPARE(KShell::joinArgs(list), QString("this is a test"));
}

static QString sj(const QString& str, KShell::Options flags, KShell::Errors* ret)
{
    return KShell::joinArgs(KShell::splitArgs(str, flags, ret));
}

void
KShellTest::splitJoin()
{
    KShell::Errors err = KShell::NoError;

    QCOMPARE(sj("\"~qU4rK\" 'text' 'jo'\"jo\" $'crap' $'\\\\\\'\\e\\x21' ha\\ lo \\a", KShell::NoOptions, &err),
             QString("'~qU4rK' text jojo crap '\\'\\''\x1b!' 'ha lo' a"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("\"~qU4rK\" 'text'", KShell::TildeExpand, &err),
             QString("'~qU4rK' text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~\"qU4rK\" 'text'", KShell::TildeExpand, &err),
             QString("'~qU4rK' text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~/\"dir\" 'text'", KShell::TildeExpand, &err),
             QString(QDir::homePath() + "/dir text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~ 'text' ~", KShell::TildeExpand, &err),
             QString(QDir::homePath() + " text " + QDir::homePath()));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("\\~ blah", KShell::TildeExpand, &err),
             QString("'~' blah"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj("~qU4rK ~" + KUser().loginName(), KShell::TildeExpand, &err),
             QString("'~qU4rK' " + QDir::homePath()));
    QVERIFY(err == KShell::NoError);
}

void
KShellTest::quoteSplit_data()
{
    QTest::addColumn<QString>("string");

    QTest::newRow("no space") << QString("hiho");
    QTest::newRow("regular space") << QString("hi there");
    QTest::newRow("special space") << QString::fromUtf8("如何定期清潔典型的電風扇　講義.pdf");
}

void
KShellTest::quoteSplit()
{
    QFETCH(QString, string);

    // Splitting a quote arg should always just return one argument
    const QStringList args = KShell::splitArgs(KShell::quoteArg(string));
    QCOMPARE(args.count(), 1);
}

void
KShellTest::abortOnMeta()
{
    KShell::Errors err1 = KShell::NoError, err2 = KShell::NoError;

    QCOMPARE(sj("text", KShell::AbortOnMeta, &err1),
             QString("text"));
    QVERIFY(err1 == KShell::NoError);

    QCOMPARE(sj("say \" error", KShell::NoOptions, &err1),
             QString());
    QVERIFY(err1 != KShell::NoError);

    QCOMPARE(sj("say \" still error", KShell::AbortOnMeta, &err1),
             QString());
    QVERIFY(err1 != KShell::NoError);

    QVERIFY(sj("say `echo no error`", KShell::NoOptions, &err1) !=
            sj("say `echo no error`", KShell::AbortOnMeta, &err2));
    QVERIFY(err1 != err2);

    QVERIFY(sj("BLA=say echo meta", KShell::NoOptions, &err1) !=
            sj("BLA=say echo meta", KShell::AbortOnMeta, &err2));
    QVERIFY(err1 != err2);

    QVERIFY(sj("B\"L\"A=say FOO=bar echo meta", KShell::NoOptions, &err1) ==
            sj("B\"L\"A=say FOO=bar echo meta", KShell::AbortOnMeta, &err2));
    QVERIFY(err1 == err2);
}

QTEST_KDEMAIN_CORE(KShellTest)

#include "kshelltest.moc"
