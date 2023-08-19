/*
 *  Copyright (C) 2006 David Faure   <faure@kde.org>
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

#include "kservicetest.h"
#include <qtest_kde.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <ksycoca.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kprotocolinfo.h>
#include <kdebug.h>
#include <kservicegroup.h>
#include <kservicetypetrader.h>
#include <kservicetype.h>

#include <QtCore/qprocess.h>
#include <QtCore/qthread.h>

#include <stdlib.h>
#include <future>
#include <chrono>

QTEST_KDEMAIN_CORE( KServiceTest )

void KServiceTest::initTestCase()
{
    // A non-C locale is necessary for some tests.
    // This locale must have the following properties:
    //   - some character other than dot as decimal separator
    m_hasNonCLocale = !KStandardDirs::locate("locale", "fr/kdelibs4.tr").isEmpty();
    if (m_hasNonCLocale) {
        kDebug() << "Setting locale to fr_FR.UTF-8";
        ::setenv("LC_ALL", "fr_FR.UTF-8", 1);
    } else {
        kWarning() << "fr_FR.UTF-8 locale no available";
    }

    m_hasKde4Konsole = false;

    // Create some fake services for the tests below, and ensure they are in ksycoca.

    // fakeservice: deleted and recreated by testKSycocaUpdate, don't use in other tests
    bool mustUpdateKSycoca = !KService::serviceByDesktopPath("fakeservice.desktop");
    const QString fakeService = KStandardDirs::locateLocal("services", "fakeservice.desktop");
    if (!QFile::exists(fakeService)) {
        mustUpdateKSycoca = true;
        createFakeService();
    }

    // fakepart: a readwrite part, like katepart
    if (!KService::serviceByDesktopPath("fakepart.desktop")) {
        mustUpdateKSycoca = true;
    }
    const QString fakePart = KStandardDirs::locateLocal("services", "fakepart.desktop");
    if (!QFile::exists(fakePart)) {
        mustUpdateKSycoca = true;
	KDesktopFile file(fakePart);
	KConfigGroup group = file.desktopGroup();
	group.writeEntry("Name", "FakePart");
	group.writeEntry("Type", "Service");
	group.writeEntry("X-KDE-Library", "fakepart");
	group.writeEntry("X-KDE-Protocols", "http,ftp");
	group.writeEntry("X-KDE-ServiceTypes", "KParts/ReadOnlyPart,KParts/ReadWritePart");
	group.writeEntry("MimeType", "text/plain;text/html;");
    }

    // faketextplugin: a ktexteditor plugin
    if (!KService::serviceByDesktopPath("faketextplugin.desktop")) {
        mustUpdateKSycoca = true;
    }
    const QString fakeTextplugin = KStandardDirs::locateLocal("services", "faketextplugin.desktop");
    if (!QFile::exists(fakeTextplugin)) {
        mustUpdateKSycoca = true;
	KDesktopFile file(fakeTextplugin);
	KConfigGroup group = file.desktopGroup();
	group.writeEntry("Name", "FakeTextPlugin");
	group.writeEntry("Type", "Service");
	group.writeEntry("X-KDE-Library", "faketextplugin");
	group.writeEntry("X-KDE-ServiceTypes", "KTextEditor/Plugin");
	group.writeEntry("MimeType", "text/plain;");
    }

    if ( mustUpdateKSycoca ) {
        // Update ksycoca in ~/.kde-unit-test after creating the above
        QProcess::execute( KStandardDirs::findExe(KBUILDSYCOCA_EXENAME), QStringList() << "--noincremental" );
        kDebug() << "waiting for signal";
        QVERIFY(QTest::kWaitForSignal(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), 10000));
        kDebug() << "got signal";
    }
}

void KServiceTest::cleanupTestCase()
{
    // If I want the konqueror unit tests to work, then I better not have a non-working part
    // as the preferred part for text/plain...
    QStringList services; services << "fakeservice.desktop" << "fakepart.desktop" << "faketextplugin.desktop";
    Q_FOREACH(const QString& service, services) {
        const QString fakeService = KStandardDirs::locateLocal("services", service);
        QFile::remove(fakeService);
    }
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels); // silence kbuildsycoca output
    proc.start(KStandardDirs::findExe(KBUILDSYCOCA_EXENAME));
    proc.waitForFinished();
}

void KServiceTest::testByName()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KServiceType::Ptr s0 = KServiceType::serviceType("KParts/ReadOnlyPart");
    QVERIFY( s0 );
    QCOMPARE( s0->name(), QString::fromLatin1("KParts/ReadOnlyPart") );

    KService::Ptr kfilemodule = KService::serviceByDesktopPath("kfilemodule.desktop");
    QVERIFY(kfilemodule);
    QCOMPARE( kfilemodule->name(), QString::fromLatin1("KFileModule"));
}

void KServiceTest::testProperty()
{
    KService::Ptr kdedkaudioplayer = KService::serviceByDesktopPath("kded/kaudioplayer.desktop");
    QVERIFY(kdedkaudioplayer);
    QCOMPARE(kdedkaudioplayer->entryPath(), QString("kded/kaudioplayer.desktop"));

    QCOMPARE(kdedkaudioplayer->property("ServiceTypes").toStringList().join(","), QString("KDEDModule"));
    QCOMPARE(kdedkaudioplayer->property("X-KDE-Kded-autoload").toBool(), false);
    QCOMPARE(kdedkaudioplayer->property("X-KDE-Kded-load-on-demand").toBool(), true);
    QVERIFY(!kdedkaudioplayer->property("Name").toString().isEmpty());
    QVERIFY(!kdedkaudioplayer->property("Name[fr]", QVariant::String).isValid());

    // Test property("X-KDE-Protocols"), which triggers the KServiceReadProperty code.
    KService::Ptr fakePart = KService::serviceByDesktopPath("fakepart.desktop");
    QVERIFY(fakePart); // see initTestCase; it should be found.
    QVERIFY(fakePart->propertyNames().contains("X-KDE-Protocols"));
    QCOMPARE(fakePart->mimeTypes(), QStringList() << "text/plain" << "text/html"); // okular relies on subclasses being kept here
    const QStringList protocols = fakePart->property("X-KDE-Protocols").toStringList();
    QCOMPARE(protocols, QStringList() << "http" << "ftp");
}

void KServiceTest::testAllServiceTypes()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    const KServiceType::List allServiceTypes = KServiceType::allServiceTypes();

    // A bit of checking on the allServiceTypes list itself
    KServiceType::List::ConstIterator stit = allServiceTypes.begin();
    const KServiceType::List::ConstIterator stend = allServiceTypes.end();
    for ( ; stit != stend; ++stit ) {
        const KServiceType::Ptr servtype = (*stit);
        const QString name = servtype->name();
        QVERIFY( !name.isEmpty() );
        QVERIFY( servtype->sycocaType() == KST_KServiceType );
    }
}

void KServiceTest::testAllServices()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );
    const KService::List lst = KService::allServices();
    QVERIFY( !lst.isEmpty() );

    for ( KService::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KService::Ptr service = (*it);
        QVERIFY( service->isType( KST_KService ) );

        const QString name = service->name();
        const QString entryPath = service->entryPath();
        //kDebug() << name << "entryPath=" << entryPath << "menuId=" << service->menuId();
        QVERIFY( !name.isEmpty() );
        QVERIFY( !entryPath.isEmpty() );

        KService::Ptr lookedupService = KService::serviceByDesktopPath( entryPath );
        QVERIFY( lookedupService ); // not null
        QCOMPARE( lookedupService->entryPath(), entryPath );

        if ( service->isApplication() )
        {
            const QString menuId = service->menuId();
            if ( menuId.isEmpty() )
                qWarning( "%s has an empty menuId!", qPrintable( entryPath ) );
            else if ( menuId == "kde4-konsole.desktop" )
                m_hasKde4Konsole = true;
            QVERIFY( !menuId.isEmpty() );
            lookedupService = KService::serviceByMenuId( menuId );
            QVERIFY( lookedupService ); // not null
            QCOMPARE( lookedupService->menuId(), menuId );
        }
    }
}

// Helper method for all the trader tests
static bool offerListHasService( const KService::List& offers,
                                 const QString& entryPath )
{
    bool found = false;
    KService::List::const_iterator it = offers.begin();
    for ( ; it != offers.end() ; ++it )
    {
        if ( (*it)->entryPath() == entryPath ) {
            if( found ) { // should be there only once
                qWarning( "ERROR: %s was found twice in the list", qPrintable( entryPath ) );
                return false; // make test fail
            }
            found = true;
        }
    }
    return found;
}

void KServiceTest::testByStorageId()
{
    if ( !KSycoca::isAvailable() )
        QSKIP("ksycoca not available", SkipAll);
    if (KGlobal::dirs()->locate("xdgdata-apps", "kde4/kmailservice.desktop").isEmpty()) {
        QSKIP("kde4/kmailservice.desktop not available", SkipAll);
    }
    QVERIFY(KService::serviceByMenuId("kde4-kmailservice.desktop"));
    QVERIFY(!KService::serviceByMenuId("kde4-kmailservice")); // doesn't work, extension mandatory
    QVERIFY(KService::serviceByStorageId("kde4-kmailservice.desktop"));
    //QVERIFY(!KService::serviceByStorageId("kde4-kmailservice")); // doesn't work, extension mandatory; also shows a debug

    // This one fails here; probably because there are two such files, so this would be too
    // ambiguous... According to the testAllServices output, the entryPaths are
    // entryPath="/d/kde/inst/kde4/share/applications/kde4/kmailservice.desktop"
    // entryPath= "/usr/share/applications/kde4/kmailservice.desktop"
    //
    //QVERIFY(KService::serviceByDesktopPath("kmailservice.desktop"));

    QVERIFY(KService::serviceByDesktopName("kmailservice"));
    // This could fail if it finds the kde3 kmailservice from /usr/share. But who still has kde3 :-)
    QCOMPARE(KService::serviceByDesktopName("kmailservice")->menuId(), QString("kde4-kmailservice.desktop"));
}

void KServiceTest::testServiceTypeTraderForReadOnlyPart()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Querying trader for services associated with KParts/ReadOnlyPart
    KService::List offers = KServiceTypeTrader::self()->query("KParts/ReadOnlyPart");
    QVERIFY( offers.count() > 0 );
    //foreach( KService::Ptr service, offers )
    //    qDebug( "%s %s", qPrintable( service->name() ), qPrintable( service->entryPath() ) );

    m_firstOffer = offers[0]->entryPath();

    // Only test for parts provided by kdelibs, or better, by this unittest:
    QVERIFY( offerListHasService( offers, "fakepart.desktop" ) );

    // Check ordering according to InitialPreference
    int lastPreference = -1;
    bool lastAllowedAsDefault = true;
    Q_FOREACH(KService::Ptr service, offers) {
        const QString path = service->entryPath();
        const int preference = service->initialPreference(); // ## might be wrong if we use per-servicetype preferences...
        //qDebug( "%s has preference %d, allowAsDefault=%d", qPrintable( path ), preference, service->allowAsDefault() );
        if ( lastAllowedAsDefault && !service->allowAsDefault() ) {
            // first "not allowed as default" offer
            lastAllowedAsDefault = false;
            lastPreference = -1; // restart
        }
        if ( lastPreference != -1 )
            QVERIFY( preference <= lastPreference );
        lastPreference = preference;
    }

    // Now look for any KTextEditor/Plugin
    offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin");
    QVERIFY( offerListHasService( offers, "fakeservice.desktop" ) );
    QVERIFY( offerListHasService( offers, "faketextplugin.desktop" ) );
}

void KServiceTest::testTraderConstraints()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KService::List offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin", "Library == 'faketextplugin'");
    QCOMPARE(offers.count(), 1);
    QVERIFY( offerListHasService( offers, "faketextplugin.desktop" ) );

    if (m_hasNonCLocale) {
        // Test float parsing, must use dot as decimal separator independent of locale.
        offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin", "([X-KDE-Version] > 4.559) and ([X-KDE-Version] < 4.561)");
        QCOMPARE(offers.count(), 1);
        QVERIFY(offerListHasService( offers, "fakeservice.desktop"));
    }

    // A test with an invalid query, to test for memleaks
    offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin", "A == B OR C == D AND OR Foo == 'Parse Error'");
    QVERIFY(offers.isEmpty());
}

void KServiceTest::testHasServiceType1() // with services constructed with a full path (rare)
{
    QString fakepartPath = KStandardDirs::locate( "services", "fakepart.desktop" );
    QVERIFY( !fakepartPath.isEmpty() );
    KService fakepart( fakepartPath );
    QVERIFY( fakepart.hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( fakepart.hasServiceType( "KParts/ReadWritePart" ) );
    QCOMPARE(fakepart.mimeTypes(), QStringList() << "text/plain" << "text/html");

    QString faketextPluginPath = KStandardDirs::locate( "services", "faketextplugin.desktop" );
    QVERIFY( !faketextPluginPath.isEmpty() );
    KService faketextPlugin( faketextPluginPath );
    QVERIFY( faketextPlugin.hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !faketextPlugin.hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KServiceTest::testHasServiceType2() // with services coming from ksycoca
{
    KService::Ptr fakepart = KService::serviceByDesktopPath( "fakepart.desktop" );
    QVERIFY( !fakepart.isNull() );
    QVERIFY( fakepart->hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( fakepart->hasServiceType( "KParts/ReadWritePart" ) );
    QCOMPARE(fakepart->mimeTypes(), QStringList() << "text/plain" << "text/html");

    KService::Ptr faketextPlugin = KService::serviceByDesktopPath( "faketextplugin.desktop" );
    QVERIFY( !faketextPlugin.isNull() );
    QVERIFY( faketextPlugin->hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !faketextPlugin->hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KServiceTest::testActionsAndDataStream()
{
    const QString servicePath = KStandardDirs::locate( "services", "ServiceMenus/ark_servicemenu.desktop" );
    if (servicePath.isEmpty() )
        QSKIP("kde-extraapps not installed, ark_servicemenu.desktop not found", SkipAll);
    KService service( servicePath );
    QVERIFY(!service.property("Name[fr]", QVariant::String).isValid());
    const QList<KServiceAction> actions = service.actions();
    QCOMPARE(actions.count(), 3);
    const KServiceAction setupAction = actions[0];
    QCOMPARE(setupAction.name(), QString("arkAutoExtractHere"));
    QCOMPARE(setupAction.exec(), QString("ark --batch --autodestination --autosubfolder %F"));
    QCOMPARE(setupAction.icon(), QString("ark"));
    QCOMPARE(setupAction.noDisplay(), false);
    QVERIFY(!setupAction.isSeparator());
    const KServiceAction extractHereAction = actions[2];
    QCOMPARE(extractHereAction.name(), QString("arkExtractHere"));

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    service.save(stream);
    QVERIFY(!data.isEmpty());
    QDataStream loadingStream(data);
    // loading must first get type, see KSycocaEntryPrivate::save
    // (the path that save writes out, is read by the KSycocaEntryPrivate ctor)
    qint32 type;
    loadingStream >> type;
    KService loadedService(loadingStream, 0);
    QCOMPARE(loadedService.name(), service.name());
    QCOMPARE(loadedService.exec(), service.exec());
    QCOMPARE(loadedService.actions().count(), 3);
}

void KServiceTest::testServiceGroups()
{
    KServiceGroup::Ptr root = KServiceGroup::root();
    QVERIFY(root);
    qDebug() << root->groupEntries().count();

    KServiceGroup::Ptr group = root;
    QVERIFY(group);
    const KServiceGroup::List list = group->entries(true /* sorted */,
                                                   true /* exclude no display entries */,
                                                   false /* allow separators */,
                                                   true /* sort by generic name */);

    kDebug() << list.count();
    Q_FOREACH(KServiceGroup::SPtr s, list) {
        qDebug() << s->name() << s->entryPath();
    }

    // No unit test here yet, but at least this can be valgrinded for errors.
}

void KServiceTest::testKSycocaUpdate()
{
    kWarning();
    connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), this, SLOT(slotDBUpdate(QStringList)));
    KService::Ptr fakeService = KService::serviceByDesktopPath("fakeservice.desktop");
    QVERIFY(fakeService); // see initTestCase; it should be found.

    // Test deleting a service
    const QString servPath = KStandardDirs::locateLocal("services", "fakeservice.desktop");
    QVERIFY(QFile::exists(servPath));
    m_resourcesUpdated.clear();
    QFile::remove(servPath);
    kDebug() << QThread::currentThread() << "executing kbuildsycoca";
    QProcess::execute( KStandardDirs::findExe(KBUILDSYCOCA_EXENAME) );
    kDebug() << QThread::currentThread() << "done";
    QVERIFY(QTest::kWaitForSignal(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), 10000));
    QVERIFY(!KService::serviceByDesktopPath("fakeservice.desktop")); // not in ksycoca anymore
    QVERIFY(m_resourcesUpdated.contains("services"));
    kDebug() << QThread::currentThread() << "got signal ok";

    QVERIFY(fakeService); // the whole point of refcounting is that this KService instance is still valid.
    QVERIFY(!QFile::exists(servPath));

    // Recreate it, for future tests
    m_resourcesUpdated.clear();
    createFakeService();
    QVERIFY(QFile::exists(servPath));
    kDebug() << QThread::currentThread() << "executing kbuildsycoca (2)";
    QProcess::execute( KStandardDirs::findExe(KBUILDSYCOCA_EXENAME) );
    kDebug() << QThread::currentThread() << "done (2)";
    QVERIFY(QTest::kWaitForSignal(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), 10000));
    kDebug() << QThread::currentThread() << "got signal ok (2)";
    QVERIFY(m_resourcesUpdated.contains("services"));
}

void KServiceTest::createFakeService()
{
    const QString fakeService = KStandardDirs::locateLocal("services", "fakeservice.desktop");
    KDesktopFile file(fakeService);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "FakePlugin");
    group.writeEntry("Type", "Service");
    group.writeEntry("X-KDE-Library", "fakeservice");
    group.writeEntry("X-KDE-Version", "4.56");
    group.writeEntry("ServiceTypes", "KTextEditor/Plugin");
    group.writeEntry("MimeType", "text/plain;");
}

// Testing for concurrent access to ksycoca from multiple threads
// It's especially interesting to run this test as ./kservicetest testThreads
// so that even the ksycoca initialization is happening from N threads at the same time.
// Use valgrind --tool=helgrind to see the race conditions.

void KServiceTest::testReaderThreads()
{
    std::future<void> future1 = std::async(std::launch::async, &KServiceTest::testAllServices, this);
    std::future<void> future2 = std::async(std::launch::async, &KServiceTest::testAllServices, this);
    std::future<void> future3 = std::async(std::launch::async, &KServiceTest::testAllServices, this);
    std::future<void> future4 = std::async(std::launch::async, &KServiceTest::testHasServiceType1, this);
    std::future<void> future5 = std::async(std::launch::async, &KServiceTest::testAllServices, this);
    std::future<void> future6 = std::async(std::launch::async, &KServiceTest::testAllServices, this);
    kDebug() << "Joining all threads";
    future1.wait();
    future2.wait();
    future3.wait();
    future4.wait();
    future5.wait();
    future6.wait();
}

void KServiceTest::testThreads()
{
    std::future<void> future1 = std::async(std::launch::async, &KServiceTest::testAllServices, this);
    std::future<void> future2 = std::async(std::launch::async, &KServiceTest::testHasServiceType1, this);
    std::future<void> future3 = std::async(std::launch::async, &KServiceTest::testKSycocaUpdate, this);
    std::future<void> future4 = std::async(std::launch::async, &KServiceTest::testTraderConstraints, this);
    while (future3.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout) {
        QTest::qWait(100); // process D-Bus events!
        kDebug() << "Waiting";
    }
    kDebug() << "Joining all threads";
    future1.wait();
    future2.wait();
    future3.wait();
    future4.wait();
}

void KServiceTest::slotDBUpdate(const QStringList &resources)
{
    // qDebug() << Q_FUNC_INFO << resources;
    m_resourcesUpdated = resources;
}

#include "moc_kservicetest.cpp"
