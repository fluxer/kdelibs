/* This file is part of KDE
    Copyright (c) 2006, 2008 David Faure <faure@kde.org>

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

#include <qtest_kde.h>

#include "fileundomanagertest.h"

#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <kio/fileundomanager.h>
#include <kio/copyjob.h>
#include <kio/job.h>
#include <kio/deletejob.h>
#include <kio/netaccess.h>
#include <kio/paste.h>
#include <kprotocolinfo.h>
#include <kde_file.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <errno.h>
#include <utime.h>
#include <time.h>
#include <sys/time.h>

#include "moc_fileundomanagertest.cpp"

QTEST_KDEMAIN( FileUndoManagerTest, GUI )

using namespace KIO;

static QString homeTmpDir() { return QFile::decodeName( getenv( "KDEHOME" ) ) + "/jobtest/"; }
static QString destDir() { return homeTmpDir() + "destdir/"; }

static QString srcFile() { return homeTmpDir() + "testfile"; }
static QString destFile() { return destDir() + "testfile"; }

static QString srcLink() { return homeTmpDir() + "symlink"; }
static QString destLink() { return destDir() + "symlink"; }

static QString srcSubDir() { return homeTmpDir() + "subdir"; }
static QString destSubDir() { return destDir() + "subdir"; }

static KUrl::List sourceList()
{
    KUrl::List lst;
    lst << KUrl( srcFile() );
    lst << KUrl( srcLink() );
    return lst;
}

static void createTestFile( const QString& path, const char* contents )
{
    QFile f( path );
    if ( !f.open( QIODevice::WriteOnly ) )
        kFatal() << "Can't create " << path ;
    f.write( QByteArray( contents ) );
    f.close();
}

static void createTestSymlink( const QString& path )
{
    // Create symlink if it doesn't exist yet
    KDE_struct_stat buf;
    if ( KDE_lstat( QFile::encodeName( path ), &buf ) != 0 ) {
        bool ok = symlink( "/IDontExist", QFile::encodeName( path ) ) == 0; // broken symlink
        if ( !ok )
            kFatal() << "couldn't create symlink: " << strerror( errno ) ;
        QVERIFY( KDE_lstat( QFile::encodeName( path ), &buf ) == 0 );
        QVERIFY( S_ISLNK( buf.st_mode ) );
    } else {
        QVERIFY( S_ISLNK( buf.st_mode ) );
    }
    qDebug( "symlink %s created", qPrintable( path ) );
    QVERIFY( QFileInfo( path ).isSymLink() );
}

static void checkTestDirectory( const QString& path )
{
    QVERIFY( QFileInfo( path ).isDir() );
    QVERIFY( QFileInfo( path + "/fileindir" ).isFile() );
    QVERIFY( QFileInfo( path + "/testlink" ).isSymLink() );
    QVERIFY( QFileInfo( path + "/dirindir" ).isDir() );
    QVERIFY( QFileInfo( path + "/dirindir/nested" ).isFile() );
}

static void createTestDirectory( const QString& path )
{
    QDir dir;
    bool ok = dir.mkdir( path );
    if ( !ok )
        kFatal() << "couldn't create " << path ;
    createTestFile( path + "/fileindir", "File in dir" );
    createTestSymlink( path + "/testlink" );
    ok = dir.mkdir( path + "/dirindir" );
    if ( !ok )
        kFatal() << "couldn't create " << path ;
    createTestFile( path + "/dirindir/nested", "Nested" );
    checkTestDirectory( path );
}

class TestUiInterface : public FileUndoManager::UiInterface
{
public:
    TestUiInterface() : FileUndoManager::UiInterface(), m_nextReplyToConfirmDeletion(true) {
        setShowProgressInfo( false );
    }
    virtual void jobError( KIO::Job* job ) {
        kFatal() << job->errorString() ;
    }
    virtual bool copiedFileWasModified( const KUrl& src, const KUrl& dest, const QDateTime& srcTime, const QDateTime& destTime ) {
        Q_UNUSED( src );
        m_dest = dest;
        Q_UNUSED( srcTime );
        Q_UNUSED( destTime );
        return true;
    }
    virtual bool confirmDeletion( const KUrl::List& files ) {
        m_files = files;
        return m_nextReplyToConfirmDeletion;
    }
    void setNextReplyToConfirmDeletion( bool b ) {
        m_nextReplyToConfirmDeletion = b;
    }
    KUrl::List files() const { return m_files; }
    KUrl dest() const { return m_dest; }
    void clear() {
        m_dest = KUrl();
        m_files.clear();
    }
private:
    bool m_nextReplyToConfirmDeletion;
    KUrl m_dest;
    KUrl::List m_files;
};

void FileUndoManagerTest::initTestCase()
{
    qDebug( "initTestCase" );

    // Start with a clean base dir
    cleanupTestCase();

    QDir dir; // TT: why not a static method?
    if ( !QFile::exists( homeTmpDir() ) ) {
        bool ok = dir.mkdir( homeTmpDir() );
        if ( !ok )
            kFatal() << "Couldn't create " << homeTmpDir() ;
    }

    createTestFile( srcFile(), "Hello world" );
    createTestSymlink( srcLink() );
    createTestDirectory( srcSubDir() );

    QDir().mkdir( destDir() );
    QVERIFY( QFileInfo( destDir() ).isDir() );

    QVERIFY( !FileUndoManager::self()->undoAvailable() );
    m_uiInterface = new TestUiInterface; // owned by FileUndoManager
    FileUndoManager::self()->setUiInterface( m_uiInterface );
}

void FileUndoManagerTest::cleanupTestCase()
{
    KIO::Job* job = KIO::del( KUrl::fromPath( homeTmpDir() ), KIO::HideProgressInfo );
    KIO::NetAccess::synchronousRun( job, 0 );
}

void FileUndoManagerTest::doUndo()
{
    QEventLoop eventLoop;
    bool ok = connect( FileUndoManager::self(), SIGNAL(undoJobFinished()),
                  &eventLoop, SLOT(quit()) );
    QVERIFY( ok );

    FileUndoManager::self()->undo();
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents); // wait for undo job to finish
}

void FileUndoManagerTest::testCopyFiles()
{
    kDebug() ;
    // Initially inspired from JobTest::copyFileToSamePartition()
    const QString destdir = destDir();
    KUrl::List lst = sourceList();
    const KUrl d( destdir );
    KIO::CopyJob* job = KIO::copy( lst, d, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    FileUndoManager::self()->recordCopyJob(job);

    QSignalSpy spyUndoAvailable( FileUndoManager::self(), SIGNAL(undoAvailable(bool)) );
    QVERIFY( spyUndoAvailable.isValid() );
    QSignalSpy spyTextChanged( FileUndoManager::self(), SIGNAL(undoTextChanged(QString)) );
    QVERIFY( spyTextChanged.isValid() );

    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );

    QVERIFY( QFile::exists( destFile() ) );
    // Don't use QFile::exists, it's a broken symlink...
    QVERIFY( QFileInfo( destLink() ).isSymLink() );

    // might have to wait for dbus signal here... but this is currently disabled.
    //QTest::qWait( 20 );
    QVERIFY( FileUndoManager::self()->undoAvailable() );
    QCOMPARE( spyUndoAvailable.count(), 1 );
    QCOMPARE( spyTextChanged.count(), 1 );
    m_uiInterface->clear();

    m_uiInterface->setNextReplyToConfirmDeletion( false ); // act like the user didn't confirm
    FileUndoManager::self()->undo();
    QCOMPARE( m_uiInterface->files().count(), 1 ); // confirmDeletion was called
    QCOMPARE( m_uiInterface->files()[0].url(), KUrl(destFile()).url() );
    QVERIFY( QFile::exists( destFile() ) ); // nothing happened yet

    // OK, now do it
    m_uiInterface->clear();
    m_uiInterface->setNextReplyToConfirmDeletion( true );
    doUndo();

    QVERIFY( !FileUndoManager::self()->undoAvailable() );
    QVERIFY( spyUndoAvailable.count() >= 2 ); // it's in fact 3, due to lock/unlock emitting it as well
    QCOMPARE( spyTextChanged.count(), 2 );
    QCOMPARE( m_uiInterface->files().count(), 1 ); // confirmDeletion was called
    QCOMPARE( m_uiInterface->files()[0].url(), KUrl(destFile()).url() );

    // Check that undo worked
    QVERIFY( !QFile::exists( destFile() ) );
    QVERIFY( !QFile::exists( destLink() ) );
    QVERIFY( !QFileInfo( destLink() ).isSymLink() );
}

void FileUndoManagerTest::testMoveFiles()
{
    kDebug() ;
    const QString destdir = destDir();
    KUrl::List lst = sourceList();
    const KUrl d( destdir );
    KIO::CopyJob* job = KIO::move( lst, d, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    FileUndoManager::self()->recordCopyJob(job);

    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );

    QVERIFY( !QFile::exists( srcFile() ) ); // the source moved
    QVERIFY( QFile::exists( destFile() ) );
    QVERIFY( !QFileInfo( srcLink() ).isSymLink() );
    // Don't use QFile::exists, it's a broken symlink...
    QVERIFY( QFileInfo( destLink() ).isSymLink() );

    doUndo();

    QVERIFY( QFile::exists( srcFile() ) ); // the source is back
    QVERIFY( !QFile::exists( destFile() ) );
    QVERIFY( QFileInfo( srcLink() ).isSymLink() );
    QVERIFY( !QFileInfo( destLink() ).isSymLink() );
}

// Testing for overwrite isn't possible, because non-interactive jobs never overwrite.
// And nothing different happens anyway, the dest is removed...
#if 0
void FileUndoManagerTest::testCopyFilesOverwrite()
{
    kDebug() ;
    // Create a different file in the destdir
    createTestFile( destFile(), "An old file already in the destdir" );

    testCopyFiles();
}
#endif

void FileUndoManagerTest::testCopyDirectory()
{
    const QString destdir = destDir();
    KUrl::List lst; lst << srcSubDir();
    const KUrl d( destdir );
    KIO::CopyJob* job = KIO::copy( lst, d, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    FileUndoManager::self()->recordCopyJob(job);

    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );

    checkTestDirectory( srcSubDir() ); // src untouched
    checkTestDirectory( destSubDir() );

    doUndo();

    checkTestDirectory( srcSubDir() );
    QVERIFY( !QFile::exists( destSubDir() ) );
}

void FileUndoManagerTest::testMoveDirectory()
{
    const QString destdir = destDir();
    KUrl::List lst; lst << srcSubDir();
    const KUrl d( destdir );
    KIO::CopyJob* job = KIO::move( lst, d, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    FileUndoManager::self()->recordCopyJob(job);

    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );

    QVERIFY( !QFile::exists( srcSubDir() ) );
    checkTestDirectory( destSubDir() );

    doUndo();

    checkTestDirectory( srcSubDir() );
    QVERIFY( !QFile::exists( destSubDir() ) );
}

void FileUndoManagerTest::testRenameFile()
{
    const KUrl oldUrl( srcFile() );
    const KUrl newUrl( srcFile() + ".new" );
    KUrl::List lst;
    lst.append(oldUrl);
    QSignalSpy spyUndoAvailable( FileUndoManager::self(), SIGNAL(undoAvailable(bool)) );
    QVERIFY( spyUndoAvailable.isValid() );
    KIO::Job* job = KIO::moveAs( oldUrl, newUrl, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    FileUndoManager::self()->recordJob( FileUndoManager::Rename, lst, newUrl, job );

    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );

    QVERIFY( !QFile::exists( srcFile() ) );
    QVERIFY( QFileInfo( newUrl.toLocalFile() ).isFile() );
    QCOMPARE(spyUndoAvailable.count(), 1);

    doUndo();

    QVERIFY( QFile::exists( srcFile() ) );
    QVERIFY( !QFileInfo( newUrl.toLocalFile() ).isFile() );
}

void FileUndoManagerTest::testRenameDir()
{
    const KUrl oldUrl( srcSubDir() );
    const KUrl newUrl( srcSubDir() + ".new" );
    KUrl::List lst;
    lst.append(oldUrl);
    KIO::Job* job = KIO::moveAs( oldUrl, newUrl, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    FileUndoManager::self()->recordJob( FileUndoManager::Rename, lst, newUrl, job );

    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );

    QVERIFY( !QDir( srcSubDir() ).exists() );
    QVERIFY( QFileInfo( newUrl.toLocalFile() ).isDir() );

    doUndo();

    QVERIFY( QDir( srcSubDir() ).exists() );
    QVERIFY( !QFileInfo( newUrl.toLocalFile() ).isDir() );
}

void FileUndoManagerTest::testCreateDir()
{
    const KUrl url( srcSubDir() + ".mkdir" );
    const QString path = url.toLocalFile();
    QVERIFY( !QDir(path).exists() );

    KIO::SimpleJob* job = KIO::mkdir(url);
    job->setUiDelegate( 0 );
    FileUndoManager::self()->recordJob( FileUndoManager::Mkdir, KUrl(), url, job );
    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );
    QVERIFY( QDir(path).exists() );
    QVERIFY( QFileInfo(path).isDir() );

    m_uiInterface->clear();
    m_uiInterface->setNextReplyToConfirmDeletion( false ); // act like the user didn't confirm
    FileUndoManager::self()->undo();
    QCOMPARE( m_uiInterface->files().count(), 1 ); // confirmDeletion was called
    QCOMPARE( m_uiInterface->files()[0].url(), url.url() );
    QVERIFY( QDir(path).exists() ); // nothing happened yet

    // OK, now do it
    m_uiInterface->clear();
    m_uiInterface->setNextReplyToConfirmDeletion( true );
    doUndo();

    QVERIFY( !QFile::exists(path) );
}

void FileUndoManagerTest::testTrashFiles()
{
    if ( !KProtocolInfo::isKnownProtocol( "trash" ) )
        QSKIP( "kio_trash not installed", SkipAll );

    // Trash it all at once: the file, the symlink, the subdir.
    KUrl::List lst = sourceList();
    lst.append( srcSubDir() );
    KIO::Job* job = KIO::trash( lst, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    FileUndoManager::self()->recordJob( FileUndoManager::Trash, lst, KUrl("trash:/"), job );

    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );

    // Check that things got removed
    QVERIFY( !QFile::exists( srcFile() ) );
    QVERIFY( !QFileInfo( srcLink() ).isSymLink() );
    QVERIFY( !QDir( srcSubDir() ).exists() );

    // check trash?
    // Let's just check that it's not empty. kio_trash has its own unit tests anyway.
    KConfig cfg( "trashrc", KConfig::SimpleConfig );
    QVERIFY( cfg.hasGroup( "Status" ) );
    QCOMPARE( cfg.group("Status").readEntry( "Empty", true ), false );

    doUndo();

    QVERIFY( QFile::exists( srcFile() ) );
    QVERIFY( QFileInfo( srcLink() ).isSymLink() );
    QVERIFY( QDir( srcSubDir() ).exists() );

    // We can't check that the trash is empty; other partitions might have their own trash
}

static void setTimeStamp( const QString& path )
{
#ifdef Q_OS_UNIX
    // Put timestamp in the past so that we can check that the
    // copy actually preserves it.
    struct timeval tp;
    gettimeofday( &tp, 0 );
    struct utimbuf utbuf;
    utbuf.actime = tp.tv_sec + 30; // 30 seconds in the future
    utbuf.modtime = tp.tv_sec + 60; // 60 second in the future
    utime( QFile::encodeName( path ), &utbuf );
    qDebug( "Time changed for %s", qPrintable( path ) );
#endif
}

void FileUndoManagerTest::testModifyFileBeforeUndo()
{
    // based on testCopyDirectory (so that we check that it works for files in subdirs too)
    const QString destdir = destDir();
    KUrl::List lst; lst << srcSubDir();
    const KUrl d( destdir );
    KIO::CopyJob* job = KIO::copy( lst, d, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    FileUndoManager::self()->recordCopyJob(job);

    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );

    checkTestDirectory( srcSubDir() ); // src untouched
    checkTestDirectory( destSubDir() );
    const QString destFile =  destSubDir() + "/fileindir";
    setTimeStamp( destFile ); // simulate a modification of the file

    doUndo();

    // Check that TestUiInterface::copiedFileWasModified got called
    QCOMPARE( m_uiInterface->dest().toLocalFile(), destFile );

    checkTestDirectory( srcSubDir() );
    QVERIFY( !QFile::exists( destSubDir() ) );
}

void FileUndoManagerTest::testPasteClipboardUndo()
{
    const KUrl::List urls (sourceList());
    QMimeData* mimeData = new QMimeData();
    urls.populateMimeData(mimeData);
    mimeData->setData(QLatin1String("application/x-kde-cutselection"), "1");
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);

    // Paste the contents of the clipboard and check its status
    KUrl destDirUrl(destDir());
    KIO::CopyJob* job = qobject_cast<KIO::CopyJob*>(KIO::pasteClipboard(destDirUrl, 0, true));
    QVERIFY(job);
    FileUndoManager::self()->recordCopyJob(job);
    QVERIFY(job->exec());

    // Check if the clipboard was updated after paste operation
    KUrl::List urls2;
    Q_FOREACH(const KUrl& url, urls) {
        KUrl dUrl = destDirUrl;
        dUrl.addPath(url.fileName());
        urls2 << dUrl;
    }
    KUrl::List clipboardUrls = KUrl::List::fromMimeData(clipboard->mimeData());
    QCOMPARE(clipboardUrls, urls2);

    // Check if the clipboard was updated after undo operation
    doUndo();
    clipboardUrls = KUrl::List::fromMimeData(clipboard->mimeData());
    QCOMPARE(clipboardUrls, urls);
}


// TODO: add test (and fix bug) for  DND of remote urls / "Link here" (creates .desktop files) // Undo (doesn't do anything)
// TODO: add test for interrupting a moving operation and then using Undo - bug:91579
