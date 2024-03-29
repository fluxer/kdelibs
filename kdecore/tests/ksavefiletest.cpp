/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/*
  This file is part of the KDE libraries
  Copyright 2006 Allen Winter <winter@kde.org>
  Copyright 2006 Jaison Lee <lee.jaison@gmail.com>

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

#include "ksavefiletest.h"

#include <QtCore/QDebug>
#include <QtCore/QTextStream>

#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <ktempdir.h>
#include <ksavefile.h>

#include <qtest_kde.h>

QTEST_KDEMAIN_CORE( KSaveFileTest )

QString tmp(QDir::tempPath() + '/');

void KSaveFileTest::test_ksavefile()
{
    QString targetFile;

    {
        //This will be the file we eventually write to. Yes, I know you
        //should never remove the temporaryfile and then expect the filename
        //to continue to be unique, but this is a test for crying out loud. :)
        KTemporaryFile file;
        file.setPrefix("ksavefiletest");
        QVERIFY( file.open() );
        targetFile = file.fileName();
    }

    {
        //Test basic functionality
        KSaveFile saveFile;
        saveFile.setFileName(targetFile);
        QVERIFY( saveFile.open() );
        QVERIFY( !QFile::exists(targetFile) );

        QTextStream ts ( &saveFile );
        ts << "This is test data one.\n";
        ts.flush();
        QCOMPARE( saveFile.error(), QFile::NoError );
        QVERIFY( !QFile::exists(targetFile) );

        QVERIFY( saveFile.finalize() );
        QVERIFY( QFile::exists(targetFile) );

        QFile::remove(targetFile);
        QVERIFY( !QFile::exists(targetFile) );
    }

    {
        //Make sure destructor does what it is supposed to do.
        {
            KSaveFile saveFile;
            saveFile.setFileName(targetFile);
            QVERIFY( saveFile.open() );
            QVERIFY( !QFile::exists(targetFile) );
        }

        QVERIFY( QFile::exists(targetFile) );
        QFile::remove(targetFile);
        QVERIFY( !QFile::exists(targetFile) );
    }

    {
        //Test some error conditions
        KSaveFile saveFile;
        QVERIFY( !saveFile.open() ); //no filename
        saveFile.setFileName(targetFile);
        QVERIFY( saveFile.open() );
        QVERIFY( !QFile::exists(targetFile) );
        QVERIFY( !saveFile.open() ); //already open

        QVERIFY( saveFile.finalize() );
        QVERIFY( QFile::exists(targetFile) );
        QVERIFY( !saveFile.finalize() ); //already finalized

        QFile::remove(targetFile);
        QVERIFY( !QFile::exists(targetFile) );
    }

    {
        //Do it again, aborting this time
        KSaveFile saveFile ( targetFile );
        QVERIFY( saveFile.open() );
        QVERIFY( !QFile::exists(targetFile) );

        QTextStream ts ( &saveFile );
        ts << "This is test data two.\n";
        ts.flush();
        QCOMPARE( saveFile.error(), QFile::NoError );
        QVERIFY( !QFile::exists(targetFile) );

        saveFile.abort();
        QVERIFY( !QFile::exists(targetFile) );
    }

    QFile file ( targetFile );
    QVERIFY( file.open(QIODevice::WriteOnly | QIODevice::Text) );
    QVERIFY( file.setPermissions( file.permissions() | QFile::ExeUser ) );
    file.close();

    {
        //Test how it works when the file already exists
        //Also check for special permissions
        KSaveFile saveFile ( targetFile );
        QVERIFY( saveFile.open() );

        QVERIFY( QFile::exists(targetFile) );
        QFileInfo fi ( targetFile );

        // Windows: qt_ntfs_permission_lookup is not set by default in
        // qfsfileengine_win.cpp, could change in future Qt versions.
        QVERIFY( fi.permission( QFile::ExeUser ) );
        QVERIFY( fi.size() == 0 );

        QTextStream ts ( &saveFile );
        ts << "This is test data three.\n";
        ts.flush();

        fi.refresh();
        QVERIFY( fi.size() == 0 );
        QVERIFY( saveFile.finalize() );

        fi.refresh();
        QVERIFY( fi.size() != 0 );
        QVERIFY( fi.permission( QFile::ExeUser ) );

        QFile::remove(targetFile);
    }

    {
        QFileInfo fi ( targetFile );
        targetFile = fi.fileName();
        QDir::setCurrent(fi.path());

        //one more time, this time with relative filenames
        KSaveFile saveFile ( targetFile );
        QVERIFY( saveFile.open() );
        QVERIFY( !QFile::exists(targetFile) );

        QTextStream ts ( &saveFile );
        ts << "This is test data four.\n";
        ts.flush();
        QCOMPARE( saveFile.error(), QFile::NoError );
        QVERIFY( !QFile::exists(targetFile) );

        QVERIFY( saveFile.finalize() );
        QVERIFY( QFile::exists(targetFile) );
        QFile::remove(targetFile);
    }
}

void KSaveFileTest::transactionalWriteNoPermissionsOnDir_data()
{
    QTest::addColumn<bool>("directWriteFallback");

    QTest::newRow("default") << false;
    QTest::newRow("directWriteFallback") << true;
}

void KSaveFileTest::transactionalWriteNoPermissionsOnDir()
{
#ifdef Q_OS_UNIX
    QFETCH(bool, directWriteFallback);
    // Restore permissions so that the QTemporaryDir cleanup can happen
    class PermissionRestorer
    {
        QString m_path;
    public:
        PermissionRestorer(const QString& path)
            : m_path(path)
        {}

        ~PermissionRestorer()
        {
            restore();
        }
        void restore()
        {
            QFile file(m_path);
            file.setPermissions(QFile::Permissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        }
    };


    KTempDir dir;
    QVERIFY(QFile(dir.name()).setPermissions(QFile::ReadOwner | QFile::ExeOwner));
    PermissionRestorer permissionRestorer(dir.name());

    const QString targetFile = dir.name() + QString::fromLatin1("/outfile");
    KSaveFile firstTry(targetFile);
    QVERIFY(!firstTry.open(QIODevice::WriteOnly));
    QCOMPARE((int)firstTry.error(), (int)QFile::PermissionsError); // actually better than QSaveFile (limited because of QTemporaryFileEngine)
    QVERIFY(!firstTry.finalize());

    // Now make an existing writable file
    permissionRestorer.restore();
    QFile f(targetFile);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QCOMPARE(f.write("Hello"), Q_INT64_C(5));
    f.close();

    // Make the directory non-writable again
    QVERIFY(QFile(dir.name()).setPermissions(QFile::ReadOwner | QFile::ExeOwner));

    // And write to it again using KSaveFile; only works if directWriteFallback is enabled
    KSaveFile file(targetFile);
    file.setDirectWriteFallback(directWriteFallback);
    QCOMPARE(file.directWriteFallback(), directWriteFallback);
    if (directWriteFallback) {
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE((int)file.error(), (int)QFile::NoError);
        QCOMPARE(file.write("World"), Q_INT64_C(5));
        QVERIFY(file.finalize());

        QFile reader(targetFile);
        QVERIFY(reader.open(QIODevice::ReadOnly));
        QCOMPARE(QString::fromLatin1(reader.readAll()), QString::fromLatin1("World"));
        reader.close();

        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE((int)file.error(), (int)QFile::NoError);
        QCOMPARE(file.write("W"), Q_INT64_C(1));
        QVERIFY(file.finalize());

        QVERIFY(reader.open(QIODevice::ReadOnly));
        QCOMPARE(QString::fromLatin1(reader.readAll()), QString::fromLatin1("W"));
    } else {
        QVERIFY(!file.open(QIODevice::WriteOnly));
        QCOMPARE((int)file.error(), (int)QFile::PermissionsError);
    }
#endif
}



void KSaveFileTest::test_backupFile()
{
    KTemporaryFile file;
    QVERIFY( file.open() );

    QVERIFY( KSaveFile::backupFile(file.fileName()));
    QVERIFY( QFile::exists(file.fileName() + '~'));
    QFile::remove(file.fileName() + '~');

    QVERIFY( KSaveFile::backupFile(file.fileName(), tmp) );
    QFileInfo fi ( file.fileName() );
    QVERIFY( QFile::exists(tmp + fi.fileName() + '~') );
    QFile::remove(tmp + fi.fileName() + '~');
}

void KSaveFileTest::cleanupTestCase()
{
    foreach ( const QString &fileToRemove, filesToRemove ) {
        QFile::remove(fileToRemove);
    }
}

#include "moc_ksavefiletest.cpp"
