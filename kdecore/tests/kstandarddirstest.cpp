/* This file is part of the KDE libraries
    Copyright (c) 2006, 2011 David Faure <faure@kde.org>

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

#include <config.h>
#include <config-prefix.h>
#include "kstandarddirstest.h"
#include "qtest_kde.h"
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kglobal.h>
#include <ktempdir.h>
#include <kconfiggroup.h>
#include <QtCore/QDebug>

#include <future>

// we need case-insensitive comparison of file paths on windows
#define QCOMPARE_PATHS(x,y) QCOMPARE(QString(x), QString(y))
#define PATH_SENSITIVITY Qt::CaseSensitive

QTEST_KDEMAIN_CORE( KStandarddirsTest )

void KStandarddirsTest::initTestCase()
{
    m_kdehome = QDir::home().canonicalPath() + "/.kde-unit-test";
}

void KStandarddirsTest::testLocateLocal()
{
    const QString configLocal = KStandardDirs::locateLocal( "config", "ksomethingrc" );
    // KStandardDirs resolves symlinks, so we must compare with canonicalPath()
    QCOMPARE_PATHS( configLocal, m_kdehome + "/share/config/ksomethingrc" );
}

void KStandarddirsTest::testSaveLocation()
{
    const QString saveLoc = KGlobal::dirs()->saveLocation( "appdata" );
    QCOMPARE_PATHS( saveLoc, m_kdehome + "/share/qttest/" );
}

void KStandarddirsTest::testAppData()
{
    // In addition to testSaveLocation(), we want to also check other KComponentDatas
    KComponentData cData("foo");
    const QString fooAppData = cData.dirs()->saveLocation( "appdata" );
    QCOMPARE_PATHS( fooAppData, m_kdehome + "/share/foo/" );
}

void KStandarddirsTest::testChangeSaveLocation()
{
    KComponentData cData("changeSave");
    QCOMPARE_PATHS(cData.dirs()->saveLocation("config"), m_kdehome + "/share/config/");
    // Can we change the save location?
    const QString newSaveLoc = m_kdehome + "/newconfigdir/";
    //cData.dirs()->addResourceDir("config", newSaveLoc); // can't be done, absolute paths have less priority than relative paths
    cData.dirs()->addResourceType("config", 0, "newconfigdir");
    QCOMPARE_PATHS(KGlobal::dirs()->realPath(cData.dirs()->saveLocation("config")), newSaveLoc);
}

static bool isKdelibsInstalled()
{
    // If there's only one dir, it's the local one (~/.kde-unit-test/share/),
    // meaning that kdelibs wasn't installed (or we don't find where, the environment isn't right).
    return KGlobal::dirs()->resourceDirs( "data" ).count() > 1;
}

void KStandarddirsTest::testFindResource()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

#define EXT ""
#define KIOSLAVE "/kioslave"
    const QString bin = KGlobal::dirs()->findResource( "exe", "kioslave" EXT );
    QVERIFY( !bin.isEmpty() );
    QVERIFY( bin.endsWith( KIOSLAVE ) );
    QVERIFY( !QDir::isRelativePath(bin) );

    const QString data = KGlobal::dirs()->findResource( "data", "cmake/modules/FindMtp.cmake" );
    QVERIFY( !data.isEmpty() );
    QVERIFY( data.endsWith( QLatin1String("share/cmake/modules/FindMtp.cmake") ) );
    QVERIFY( !QDir::isRelativePath(data) );
}

static bool oneEndsWith( const QStringList& lst, const QString& str)
{
    for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
        if ( (*it).endsWith( str ) )
            return true;
    }
    return false;
}

void KStandarddirsTest::testFindAllResources()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    const QStringList cmakeModulesFiles = KGlobal::dirs()->findAllResources( "data", "cmake/modules/" );
    QVERIFY( !cmakeModulesFiles.isEmpty() );
    QVERIFY( cmakeModulesFiles.count() > 20 ); // I have 35 here, installed by kdelibs.

    // Create a local config file, the file will be used as expected result
    const QString localConfigFile = m_kdehome + "/share/config/foorc";
    QFile::remove(localConfigFile);
    KConfig foorc("foorc");
    KConfigGroup dummyGroup(&foorc, "Dummy");
    dummyGroup.writeEntry("someEntry", true);
    dummyGroup.sync();
    QVERIFY2(QFile::exists(localConfigFile), qPrintable(localConfigFile));

    const QStringList configFiles = KGlobal::dirs()->findAllResources( "config" );
    QVERIFY( !configFiles.isEmpty() );
    QVERIFY( configFiles.count() > 5 ); // I have 9 here
    QVERIFY( oneEndsWith( configFiles, "share/config/kdebugrc" ) );
    QVERIFY( !oneEndsWith( configFiles, "share/config/colors/Web.colors" ) ); // recursive was false

    const QStringList configFilesRecursive = KGlobal::dirs()->findAllResources( "config", QString(),
                                                                                KStandardDirs::Recursive );
    QVERIFY( !configFilesRecursive.isEmpty() );
    QVERIFY( configFilesRecursive.count() > 5 ); // I have 15 here
    QVERIFY( oneEndsWith( configFilesRecursive, "share/config/kdebugrc" ) );
    QVERIFY( oneEndsWith( configFilesRecursive, "share/config/ui/ui_standards.rc" ) );
    QVERIFY( oneEndsWith( configFilesRecursive, "share/config/colors/Web.colors" ) ); // proves that recursive worked

    const QStringList configFilesRecursiveWithFilter = KGlobal::dirs()->findAllResources( "config", "*rc",
                                                                                          KStandardDirs::Recursive );
    QVERIFY( !configFilesRecursiveWithFilter.isEmpty() );
    QVERIFY( configFilesRecursiveWithFilter.count() >= 4 );
    QVERIFY( oneEndsWith( configFilesRecursiveWithFilter, "share/config/kdebugrc" ) );
    QVERIFY( oneEndsWith( configFilesRecursiveWithFilter, "share/config/ui/ui_standards.rc" ) );
    QVERIFY( !oneEndsWith( configFilesRecursiveWithFilter, "share/config/colors/Web.colors" ) ); // didn't match the filter

    QStringList fileNames;
    const QStringList configFilesWithFilter = KGlobal::dirs()->findAllResources("config", "*rc", KStandardDirs::NoDuplicates, fileNames);
    QVERIFY( !configFilesWithFilter.isEmpty() );
    QVERIFY( oneEndsWith( configFilesWithFilter, "share/config/kdebugrc" ) );
    QVERIFY( oneEndsWith( configFilesWithFilter, "kde-unit-test/share/config/foorc" ) );
    QVERIFY2( configFilesWithFilter.count() >= 2, qPrintable(configFilesWithFilter.join(",")) );
    QVERIFY( !oneEndsWith( configFilesWithFilter, "share/config/ui/ui_standards.rc" ) ); // not recursive
    QVERIFY( !oneEndsWith( configFilesWithFilter, "share/config/accept-languages.codes" ) ); // didn't match the filter
    QCOMPARE(fileNames.count(), configFilesWithFilter.count());
    QVERIFY(fileNames.contains("kdebugrc"));
}

void KStandarddirsTest::testFindAllResourcesNewDir()
{
    const QStringList origFiles = KGlobal::dirs()->findAllResources("data", "cmake/modules/");
    const int origCount = origFiles.count();

    const QString dir = m_kdehome + "/share/cmake/modules";
    QDir().mkpath(dir);
    QFile file(dir+"/unittest.testfile");
    QVERIFY(file.open(QIODevice::WriteOnly|QIODevice::Text));
    file.write("foo");
    file.close();

    const int newCount = KGlobal::dirs()->findAllResources("data", "cmake/modules/").count();
    QCOMPARE(newCount, origCount+1);
    file.remove();
    QDir().rmpath(dir);
}

void KStandarddirsTest::testFindDirs()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    const QString t = KStandardDirs::locateLocal("data", "LICENSES/" );
    const QStringList dirs = KGlobal::dirs()->findDirs( "data", "LICENSES" );
    QVERIFY( !dirs.isEmpty() );
    QVERIFY( dirs.count() >= 2 ); // at least local and global
    //qDebug() << dirs;
}

void KStandarddirsTest::testFindResourceDir()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    const QString configDir = KGlobal::dirs()->findResourceDir( "config", "kdebugrc" );
    QVERIFY( !configDir.isEmpty() );
    QVERIFY( configDir.endsWith( QLatin1String( "/config/" ) ) );
}

void KStandarddirsTest::testFindExe()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    // findExe with a result in bin
    const QString klauncher = KGlobal::dirs()->findExe( "klauncher4" );
    QVERIFY( !klauncher.isEmpty() );
    QVERIFY2(klauncher.endsWith("bin/klauncher4" EXT, PATH_SENSITIVITY), qPrintable(klauncher));

    // Check the "exe" resource too
    QString klauncherexe = KGlobal::dirs()->locate( "exe", "klauncher4" );
    QVERIFY2(klauncherexe.endsWith("bin/klauncher4" EXT, PATH_SENSITIVITY), qPrintable(klauncherexe));

#ifdef Q_OS_UNIX
    // findExe with a result in libexec
    const QString kioslave = KGlobal::dirs()->findExe( "kioslave" );
    QVERIFY( !kioslave.isEmpty() );
    QVERIFY( kioslave.endsWith("/kioslave" EXT, PATH_SENSITIVITY ) );

    // locate("exe") with a result in libexec
    const QString locateExeResult = KGlobal::dirs()->locate("exe", "kioslave");
    QVERIFY(locateExeResult.endsWith("/kioslave" EXT, PATH_SENSITIVITY));

    // findExe with relative path
    const QString pwd = QDir::currentPath();
    QDir::setCurrent("/bin");
    QStringList possibleResults;
    possibleResults << QString::fromLatin1("/bin/sh") << QString::fromLatin1("/usr/bin/sh");
    const QString sh = KGlobal::dirs()->findExe("./sh");
    if (!possibleResults.contains(sh)) {
        kDebug() << sh;
    }
    QVERIFY(possibleResults.contains(sh));
    QDir::setCurrent(pwd);
#endif

#ifdef Q_OS_UNIX
    QFile home( m_kdehome );
    const QString target = m_kdehome + "/linktodir";
    home.link( target );
    QVERIFY( KGlobal::dirs()->findExe( target ).isEmpty() );
#endif

#ifdef Q_OS_UNIX
    // findExe for a binary not part of KDE
    const QString ls = KGlobal::dirs()->findExe( "ls" );
    QVERIFY( !ls.isEmpty() );
    QVERIFY( ls.endsWith( QLatin1String( "bin/ls" ) ) );
#endif

    // findExe with no result
    const QString idontexist = KGlobal::dirs()->findExe( "idontexist" );
    QVERIFY( idontexist.isEmpty() );

    // findExe with empty string
    const QString empty = KGlobal::dirs()->findExe( "" );
    QVERIFY( empty.isEmpty() );
}

void KStandarddirsTest::testLocate()
{
    QString textPlain = "text/x-patch.xml";
    Q_FOREACH( const QString &path, KGlobal::dirs()->resourceDirs("xdgdata-mime") ) {
        if (QFile::exists(path + textPlain)) {
            textPlain = path + textPlain;
            break;
        }
    }
    if( textPlain == "text/x-patch.xml" )
        QSKIP("xdg-share-mime not installed", SkipAll);

    const QString res = KGlobal::dirs()->locate("xdgdata-mime", "text/x-patch.xml");
    QCOMPARE_PATHS(res, textPlain);
}

void KStandarddirsTest::testRelativeLocation()
{
    const QString file = "kdebugrc";
    QString located = KGlobal::dirs()->locate( "config", file );
    QCOMPARE_PATHS( KGlobal::dirs()->relativeLocation( "config", located ), file );
}

void KStandarddirsTest::testAddResourceType()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    QString ret = KStandardDirs::locate( "dtd", "customization/catalog.xml" );
    QCOMPARE(ret, QString()); // normal, there's no "dtd" resource in kstandarddirs by default

    KGlobal::dirs()->addResourceType("dtd", "data", "kdewidgets/");
    ret = KStandardDirs::locate( "dtd", "pics/kpushbutton.png" );
    QVERIFY(!ret.isEmpty());

    ret = KStandardDirs::locate("dtd", "pics/kruler.png");
    QVERIFY(!ret.isEmpty());

    const QStringList files = KGlobal::dirs()->findAllResources("dtd", "pics/*", KStandardDirs::NoDuplicates);
    QVERIFY(files.count() >= 5);
}

void KStandarddirsTest::testAddResourceDir()
{
    const QString dir = QString::fromLatin1(KDESRCDIR);
    const QString file = "kstandarddirstest.cpp";
    QString ret = KStandardDirs::locate( "here", file );
    QCOMPARE(ret, QString()); // not set up yet

    KGlobal::dirs()->addResourceDir("here", dir);
    ret = KStandardDirs::locate( "here", file );
    QCOMPARE_PATHS(ret, KGlobal::dirs()->realPath(dir) + "kstandarddirstest.cpp");
}

void KStandarddirsTest::testSetXdgDataDirs()
{
    // By default we should have KDEDIR/share/applications in `kde4-config --path xdgdata-apps`
    const QStringList dirs = KGlobal::dirs()->resourceDirs("xdgdata-apps");
    const QString kdeDataApps = KGlobal::dirs()->realPath(KDEDIR "/share/applications/");
    if (!dirs.contains(kdeDataApps)) {
        kDebug() << "ERROR:" << kdeDataApps << "not in" << dirs;
        kDebug() << "XDG_DATA_DIRS=" << qgetenv("XDG_DATA_DIRS");
        kDebug() << "installprefix=" << KStandardDirs::installPath("kdedir");
        kDebug() << "installdir=" << KStandardDirs::installPath("xdgdata-apps");
        kDebug() << "KStandardDirs::kfsstnd_xdg_data_prefixes=" << KGlobal::dirs()->kfsstnd_xdg_data_prefixes();
    }
    QVERIFY(dirs.contains(kdeDataApps, PATH_SENSITIVITY));

    // When setting XDG_DATA_DIR this should still be true
    const QString localApps = m_kdehome + "/share/applications/";
    QVERIFY(KStandardDirs::makeDir(localApps));
    ::setenv("XDG_DATA_DIRS", QFile::encodeName(m_kdehome + "/share"), 1 );
    KStandardDirs newStdDirs;
    const QStringList newDirs = newStdDirs.resourceDirs("xdgdata-apps");
    QVERIFY(newDirs.contains(kdeDataApps, PATH_SENSITIVITY));
    QVERIFY(newDirs.contains(localApps, PATH_SENSITIVITY));
}

void KStandarddirsTest::testSymlinkResolution()
{
    // This makes the save location for the david resource, "$HOME/.kde-unit-test/symlink/test/"
    // where symlink points to "real", and the subdir test will be created later
    // This used to confuse KStandardDirs and make it return unresolved paths,
    // and thus making comparisons fail later on in KConfig.
    const QString symlink = m_kdehome + "/symlink";
    const QString expected = m_kdehome + "/real/test/";
    QVERIFY(KTempDir::removeDir(m_kdehome + "/real"));
    QVERIFY(QDir(m_kdehome).mkdir("real"));
    QFile::remove(symlink);
    QVERIFY(!QFile::exists(symlink));
    QVERIFY(QFile::link("real", symlink));
    QVERIFY(QFileInfo(symlink).isSymLink());
    QVERIFY(!QFile::exists(expected));
    KGlobal::dirs()->addResourceType("david", 0, "symlink/test");
    QVERIFY(!QFile::exists(expected));
    const QString saveLoc = KGlobal::dirs()->resourceDirs("david").first();
    QVERIFY(!QFile::exists(expected));
    // The issue at this point is that saveLoc does not actually exist yet.
    QVERIFY(QDir(saveLoc).canonicalPath().isEmpty()); // this is why we can't use canonicalPath
    QVERIFY(!QFile::exists(saveLoc));
    QCOMPARE(saveLoc, KGlobal::dirs()->realPath(saveLoc)); // must be resolved
    QCOMPARE(saveLoc, expected);
    QVERIFY(QDir(m_kdehome).mkpath("real/test")); // KConfig calls mkdir on its own, we simulate that here
    const QString sameSaveLoc = KGlobal::dirs()->resourceDirs("david").first();
    QCOMPARE(sameSaveLoc, saveLoc);
    QCOMPARE(sameSaveLoc, KGlobal::dirs()->saveLocation("david"));

    // While we're here...
    QCOMPARE(KGlobal::dirs()->realPath(QString()), QString());
    QCOMPARE(KGlobal::dirs()->realPath(QString("/")), QString("/"));

    QCOMPARE(KGlobal::dirs()->realPath(QString("/does_not_exist/")), QString("/does_not_exist/"));
}

// To find multithreading bugs: valgrind --tool=helgrind ./kstandarddirstest testThreads
void KStandarddirsTest::testThreads()
{
    std::future<void> future1 = std::async(std::launch::async, &KStandarddirsTest::testLocateLocal, this);
    std::future<void> future2 = std::async(std::launch::async, &KStandarddirsTest::testSaveLocation, this);
    std::future<void> future3 = std::async(std::launch::async, &KStandarddirsTest::testAppData, this);
    std::future<void> future4 = std::async(std::launch::async, &KStandarddirsTest::testFindResource, this);
    std::future<void> future5 = std::async(std::launch::async, &KStandarddirsTest::testFindAllResources, this);
    std::future<void> future6 = std::async(std::launch::async, &KStandarddirsTest::testLocate, this);
    std::future<void> future7 = std::async(std::launch::async, &KStandarddirsTest::testRelativeLocation, this);
    kDebug() << "Joining all threads";
    future1.wait();
    future2.wait();
    future3.wait();
    future4.wait();
    future5.wait();
    future6.wait();
    future7.wait();
}

#include "moc_kstandarddirstest.cpp"