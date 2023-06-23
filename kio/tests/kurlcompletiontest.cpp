/*
 *  Copyright (C) 2004 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kurlcompletiontest.h"

#include <QDir>
#include <QFile>
#include <kdebug.h>
#include <qtest_kde.h>

#include <unistd.h>

QTEST_KDEMAIN( KUrlCompletionTest, GUI )

static const bool setDirAsURL = false;

void KUrlCompletionTest::initTestCase()
{
    m_completion = new KUrlCompletion();
    m_tempDir = new KTempDir();
    m_dir = m_tempDir->name();
    Q_ASSERT( m_dir.endsWith( "/" ) );
    m_dir += "Dir With#Spaces/";
    QDir().mkdir(m_dir);
    kDebug() << "m_dir=" << m_dir;
    if ( setDirAsURL ) {
        m_completion->setDir( KUrl(m_dir).url() );
    } else {
        m_completion->setDir( m_dir );
    }
    m_dirURL.setPath( m_dir );

    QFile f1( m_dir + "/file1" );
    bool ok = f1.open( QIODevice::WriteOnly );
    QVERIFY( ok );
    Q_UNUSED( ok );
    f1.close();

    QFile f2( m_dir + "/file#a" );
    ok = f2.open( QIODevice::WriteOnly );
    QVERIFY( ok );
    Q_UNUSED( ok );
    f2.close();

    QDir().mkdir( m_dir + "/file_subdir" );

    m_completionEmptyCwd = new KUrlCompletion;
    m_completionEmptyCwd->setDir( "" );
}

void KUrlCompletionTest::cleanupTestCase()
{
    delete m_completion;
    m_completion = 0;
    delete m_tempDir;
    m_tempDir = 0;
    delete m_completionEmptyCwd;
    m_completionEmptyCwd = 0;
}
void KUrlCompletionTest::waitForCompletion()
{
    while ( m_completion->isRunning() ) {
        kDebug() << "waiting for thread...";
        usleep( 10 );
    }
}

void KUrlCompletionTest::testLocalRelativePath()
{
    kDebug() ;
    // Completion from relative path, with two matches
    m_completion->makeCompletion( "f" );
    waitForCompletion();
    QStringList comp1all = m_completion->allMatches();
    kDebug() << comp1all;
    QCOMPARE( comp1all.count(), 3 );
    QVERIFY( comp1all.contains( "file1" ) );
    QVERIFY( comp1all.contains( "file#a" ) );
    QVERIFY( comp1all.contains( "file_subdir/" ) );
    QString comp1 = m_completion->replacedPath( "file1" ); // like KUrlRequester does
    QCOMPARE( comp1, QString::fromLatin1("file1") );

    // Completion from relative path
    kDebug() << "now completing on 'file#'";
    m_completion->makeCompletion( "file#" );
    waitForCompletion();
    QStringList compall = m_completion->allMatches();
    kDebug() << compall;
    QCOMPARE( compall.count(), 1 );
    QCOMPARE( compall.first(), QString::fromLatin1("file#a") );
    QString comp2 = m_completion->replacedPath( compall.first() ); // like KUrlRequester does
    QCOMPARE( comp2, QString::fromLatin1("file#a") );

    // Completion with empty string
    kDebug () << "now completing on ''";
    m_completion->makeCompletion( "" );
    waitForCompletion();
    QStringList compEmpty = m_completion->allMatches();
    QCOMPARE( compEmpty.count(), 0 );
}

void KUrlCompletionTest::testLocalAbsolutePath()
{
    // Completion from absolute path
    kDebug() << m_dir+"file#";
    m_completion->makeCompletion( m_dir + "file#" );
    waitForCompletion();
    QStringList compall = m_completion->allMatches();
    kDebug() << compall;
    QCOMPARE( compall.count(), 1 );
    QString comp = compall.first();
    QCOMPARE( comp, m_dir + "file#a" );
    comp = m_completion->replacedPath( comp ); // like KUrlRequester does
    QCOMPARE( comp, m_dir + "file#a" );
}

void KUrlCompletionTest::testLocalURL()
{
    // Completion from URL
    KUrl url( m_dirURL.toLocalFile() + "file" );
    m_completion->makeCompletion( url.prettyUrl() );
    waitForCompletion();
    QStringList comp1all = m_completion->allMatches();
    kDebug() << comp1all;
    QCOMPARE( comp1all.count(), 3 );
    kDebug() << "Looking for" << m_dirURL.prettyUrl() + "file1";
    QVERIFY( comp1all.contains( m_dirURL.prettyUrl() + "file1" ) );
    QVERIFY( comp1all.contains( m_dirURL.prettyUrl() + "file_subdir/" ) );
    QString filehash = m_dirURL.prettyUrl() + "file%23a";
    QVERIFY( comp1all.contains( filehash ) );
    QString filehashPath = m_completion->replacedPath( filehash ); // note that it returns a path!!
    kDebug() << filehashPath;
    QCOMPARE( filehashPath, m_dirURL.toLocalFile() + "file#a" );

    // Completion from URL with no match
    url = KUrl( m_dirURL.toLocalFile() + "foobar" );
    kDebug() << "makeCompletion(" << url << ")";
    QString comp2 = m_completion->makeCompletion( url.prettyUrl() );
    QVERIFY( comp2.isEmpty() );
    waitForCompletion();
    QVERIFY( m_completion->allMatches().isEmpty() );

    // Completion from URL with a ref -> no match
    url = KUrl( m_dirURL.toLocalFile() + 'f' );
    url.setRef( "ref" );
    kDebug() << "makeCompletion(" << url << ")";
    m_completion->makeCompletion( url.prettyUrl() );
    waitForCompletion();
    QVERIFY( m_completion->allMatches().isEmpty() );
}

void KUrlCompletionTest::testEmptyCwd()
{
    kDebug() ;
    // Completion with empty string (with a KUrlCompletion whose cwd is "")
    kDebug () << "now completing on '' with empty cwd";
    m_completionEmptyCwd->makeCompletion( "" );
    waitForCompletion();
    QStringList compEmpty = m_completionEmptyCwd->allMatches();
    QCOMPARE( compEmpty.count(), 0 );
}

#include "moc_kurlcompletiontest.cpp"
