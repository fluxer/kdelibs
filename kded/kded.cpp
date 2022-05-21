// vim: expandtab sw=4 ts=4
/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
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
 **/

#include "kded.h"
#include "kdedadaptor.h"
#include "kdedmodule.h"

#include <kcrash.h>
#include <kdeversion.h>
#include <kapplication.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>
#include <kservicetypetrader.h>
#include <kde_file.h>
#include "klauncher_iface.h"

#include <QProcess>
#include <QHostInfo>

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#define KDED_EXENAME "kded4"

#define MODULES_PATH "/modules/"

Kded *Kded::_self = 0;

static bool checkStamps = true;
static bool delayedCheck = false;
static int HostnamePollInterval = 5000;
static bool bCheckSycoca = true;
static bool bCheckUpdates = true;
static bool bCheckHostname = true;

QT_BEGIN_NAMESPACE
#ifdef Q_DBUS_EXPORT
extern Q_DBUS_EXPORT void qDBusAddSpyHook(void (*)(const QDBusMessage&));
#else
extern QDBUS_EXPORT void qDBusAddSpyHook(void (*)(const QDBusMessage&));
#endif
QT_END_NAMESPACE

static bool runBuildSycoca()
{
    const QString exe = KStandardDirs::findExe(KBUILDSYCOCA_EXENAME);
    Q_ASSERT(!exe.isEmpty());
    QStringList args;
    args.append("--incremental");
    if (checkStamps) {
        args.append("--checkstamps");
    }
    if (delayedCheck) {
        args.append("--nocheckfiles");
    } else {
        checkStamps = false; // useful only during kded startup
    }
    return (QProcess::execute(exe, args) == 0);
}

static void runDontChangeHostname(const QByteArray &oldName, const QByteArray &newName)
{
    QStringList args;
    args.append(QFile::decodeName(oldName));
    args.append(QFile::decodeName(newName));
    QProcess::execute("kdontchangethehostname", args);
}

Kded::Kded()
    : m_needDelayedCheck(false)
{
    _self = this;

    m_serviceWatcher = new QDBusServiceWatcher(this);
    m_serviceWatcher->setConnection(QDBusConnection::sessionBus());
    m_serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    QObject::connect(m_serviceWatcher, SIGNAL(serviceUnregistered(QString)),
                     this, SLOT(slotApplicationRemoved(QString)));

    new KBuildsycocaAdaptor(this);
    new KdedAdaptor(this);

    QDBusConnection session = QDBusConnection::sessionBus();
    session.registerObject("/kbuildsycoca", this);
    session.registerObject("/kded", this);
    session.registerService("org.kde.kded");

    qDBusAddSpyHook(messageFilter);

    m_pTimer = new QTimer(this);
    m_pTimer->setSingleShot(true);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(recreate()));

    m_pDirWatch = 0;

    m_recreateBusy = false;
}

Kded::~Kded()
{
    QDBusConnection session = QDBusConnection::sessionBus();
    session.unregisterObject("/kbuildsycoca");
    session.unregisterObject("/kded");
    session.unregisterService("org.kde.kded");

    _self = 0;
    m_pTimer->stop();
    delete m_pTimer;
    delete m_pDirWatch;

    QHashIterator<QString,KDEDModule*> it(m_modules);
    while (it.hasNext()) {
        it.next();
        KDEDModule* module(it.value());

        // first disconnect otherwise slotKDEDModuleRemoved() is called
        // and changes m_modules while we're iterating over it
        disconnect(module, SIGNAL(moduleDeleted(KDEDModule*)),
                   this, SLOT(slotKDEDModuleRemoved(KDEDModule*)));

        delete module;
    }
}

// on-demand module loading
// this function is called by the D-Bus message processing function before
// calls are delivered to objects
void Kded::messageFilter(const QDBusMessage &message)
{
    // This happens when kded goes down and some modules try to clean up.
    if (!self()) {
        return;
    }

    if (message.type() != QDBusMessage::MethodCallMessage) {
        return;
    }

    QString obj = message.path();
    if (!obj.startsWith(MODULES_PATH)) {
        return;
    }

    // Remove the <MODULES_PATH> part
    obj = obj.mid(strlen(MODULES_PATH));

    // Remove the part after the modules name
    int index = obj.indexOf('/');
    if (index != -1) {
        obj = obj.left(index);
    }

    if (self()->m_dontLoad.value(obj, 0)) {
        return;
    }

    KDEDModule *module = self()->loadModule(obj, true);
    if (!module) {
        kDebug(7020) << "Failed to load module for " << obj;
    }
    Q_UNUSED(module);
}

static int phaseForModule(const KService::Ptr& service)
{
    const QVariant phasev = service->property("X-KDE-Kded-phase", QVariant::Int );
    return (phasev.isValid() ? phasev.toInt() : 2);
}

void Kded::initModules()
{
    m_dontLoad.clear();
    bool kde_running = !qgetenv("KDE_FULL_SESSION").isEmpty();
    if (kde_running) {
        // not the same user like the one running the session (most likely we're run via sudo or something)
        const QByteArray sessionUID = qgetenv("KDE_SESSION_UID");
        if (!sessionUID.isEmpty() && uid_t( sessionUID.toInt()) != ::getuid()) {
            kde_running = false;
        }

        // not the same kde version as the current desktop
        const QByteArray kdeSession = qgetenv("KDE_SESSION_VERSION");
        if (kdeSession.toInt() != KDE_VERSION_MAJOR) {
            kde_running = false;
        }
    }

    // Preload kded modules.
    const KService::List kdedModules = KServiceTypeTrader::self()->query("KDEDModule");
    foreach (KService::Ptr service, kdedModules) {
        // Should the service load on startup?
        const bool autoload = isModuleAutoloaded(service);

        // see ksmserver's README for description of the phases
        bool prevent_autoload = false;
        switch( phaseForModule(service) ) {
            case 0: {
                // always autoload
                break;
            }
            case 1: {
                // autoload only in KDE
                if (!kde_running) {
                    prevent_autoload = true;
                }
                break;
            }
            case 2: // autoload delayed, only in KDE
            default: {
                // "phase 2" only in KDE
                if (!kde_running) {
                    prevent_autoload = true;
                }
                break;
            }
        }

        // Load the module if necessary and allowed
        if (autoload && !prevent_autoload) {
            if (!loadModule(service, false)) {
                continue;
            }
        }

        // Remember if the module is allowed to load on demand
        bool loadOnDemand = isModuleLoadedOnDemand(service);
        if (!loadOnDemand) {
            noDemandLoad(service->desktopEntryName());
        }

        // In case of reloading the configuration it is possible for a module
        // to run even if it is now allowed to. Stop it then.
        if (!loadOnDemand && !autoload) {
            unloadModule(service->desktopEntryName().toLatin1());
        }
    }
}

void Kded::loadSecondPhase()
{
    kDebug(7020) << "Loading second phase autoload";
    KSharedConfig::Ptr config = KGlobal::config();
    KService::List kdedModules = KServiceTypeTrader::self()->query("KDEDModule");
    foreach (const KService::Ptr service, kdedModules) {
        const bool autoload = isModuleAutoloaded(service);
        if (autoload && phaseForModule(service) == 2) {
            // kDebug(7020) << "2nd phase: loading" << service->desktopEntryName();
            loadModule(service, false);
        }
    }
}

void Kded::noDemandLoad(const QString &obj)
{
    m_dontLoad.insert(obj.toLatin1(), this);
}

void Kded::setModuleAutoloading(const QString &obj, bool autoload)
{
    KSharedConfig::Ptr config = KGlobal::config();
    // Ensure the service exists.
    KService::Ptr service = KService::serviceByDesktopPath("kded/" + obj + ".desktop");
    if (!service) {
        return;
    }
    KConfigGroup cg(config, QString("Module-%1").arg(service->desktopEntryName()));
    cg.writeEntry("autoload", autoload);
    cg.sync();
}

bool Kded::isModuleAutoloaded(const QString &obj) const
{
    KService::Ptr s = KService::serviceByDesktopPath("kded/" + obj + ".desktop");
    if (!s) {
        return false;
    }
    return isModuleAutoloaded(s);
}

bool Kded::isModuleAutoloaded(const KService::Ptr &module) const
{
    KSharedConfig::Ptr config = KGlobal::config();
    bool autoload = module->property("X-KDE-Kded-autoload", QVariant::Bool).toBool();
    KConfigGroup cg(config, QString("Module-%1").arg(module->desktopEntryName()));
    autoload = cg.readEntry("autoload", autoload);
    return autoload;
}

bool Kded::isModuleLoadedOnDemand(const QString &obj) const
{
    KService::Ptr s = KService::serviceByDesktopPath("kded/" + obj + ".desktop");
    if (!s) {
        return false;
    }
    return isModuleLoadedOnDemand(s);
}

bool Kded::isModuleLoadedOnDemand(const KService::Ptr &module) const
{
    KSharedConfig::Ptr config = KGlobal::config();
    bool loadOnDemand = true;
    QVariant p = module->property("X-KDE-Kded-load-on-demand", QVariant::Bool);
    if (p.isValid() && (p.toBool() == false)) {
        loadOnDemand = false;
    }
    return loadOnDemand;
}

KDEDModule *Kded::loadModule(const QString &obj, bool onDemand)
{
    // Make sure this method is only called with valid module names.
    Q_ASSERT(obj.indexOf('/') == -1);

    KDEDModule *module = m_modules.value(obj, 0);
    if (module) {
        return module;
    }
    KService::Ptr s = KService::serviceByDesktopPath("kded/" + obj + ".desktop");
    return loadModule(s, onDemand);
}

KDEDModule *Kded::loadModule(const KService::Ptr& s, bool onDemand)
{
    if (s && !s->library().isEmpty()) {
        QString obj = s->desktopEntryName();
        KDEDModule *oldModule = m_modules.value(obj, 0);
        if (oldModule) {
            return oldModule;
        }

        if (onDemand) {
            QVariant p = s->property("X-KDE-Kded-load-on-demand", QVariant::Bool);
            if (p.isValid() && (p.toBool() == false)) {
                noDemandLoad(s->desktopEntryName());
                return 0;
            }
        }

        QString libname = "kded_" + s->library();
        KPluginLoader loader(libname);

        KPluginFactory *factory = loader.factory();
        KDEDModule *module = 0;
        if (factory) {
            module = factory->create<KDEDModule>(this);
        }
        if (module) {
            module->setModuleName(obj);
            m_modules.insert(obj, module);
            // m_libs.insert(obj, lib);
            connect(module, SIGNAL(moduleDeleted(KDEDModule*)), SLOT(slotKDEDModuleRemoved(KDEDModule*)));
            kDebug(7020) << "Successfully loaded module" << obj;
            return module;
        } else {
            kDebug(7020) << "Could not load module" << obj;
            // loader.unload();
        }
    }
    return 0;
}

bool Kded::unloadModule(const QString &obj)
{
    KDEDModule *module = m_modules.value(obj, 0);
    if (!module) {
        return false;
    }
    kDebug(7020) << "Unloading module" << obj;
    m_modules.remove(obj);
    delete module;
    return true;
}

QStringList Kded::loadedModules()
{
    return m_modules.keys();
}

void Kded::slotKDEDModuleRemoved(KDEDModule *module)
{
    m_modules.remove(module->moduleName());
    // QLibrary *lib = m_libs.take(module->moduleName());
    // if (lib) {
    //     lib->unload();
    // }
}

void Kded::slotApplicationRemoved(const QString &name)
{
#if 0 // see kdedmodule.cpp (KDED_OBJECTS)
    foreach (KDEDModule* module, m_modules) {
        module->removeAll(appId);
    }
#endif
    m_serviceWatcher->removeWatchedService(name);
    const QList<qlonglong> windowIds = m_windowIdList.value(name);
    foreach(const qlonglong windowId, windowIds) {
        m_globalWindowIdList.remove(windowId);
        foreach(KDEDModule* module, m_modules) {
            emit module->windowUnregistered(windowId);
        }
    }
    m_windowIdList.remove(name);
}

void Kded::updateDirWatch()
{
    if (!bCheckUpdates) {
        return;
    }

    delete m_pDirWatch;
    m_pDirWatch = new KDirWatch;

    QObject::connect(m_pDirWatch, SIGNAL(dirty(QString)), this, SLOT(update(QString)));

    foreach(const QString it, m_allResourceDirs) {
        readDirectory(it);
    }
}

void Kded::updateResourceList()
{
    KSycoca::clearCaches();

    if (!bCheckUpdates || delayedCheck) {
        return;
    }

    foreach(const QString &it, KSycoca::self()->allResourceDirs()) {
        if (!m_allResourceDirs.contains(it)) {
            m_allResourceDirs.append(it);
            readDirectory(it);
        }
    }
}

void Kded::recreate()
{
    recreate(false);
}

void Kded::runDelayedCheck()
{
    if (m_needDelayedCheck) {
        recreate(false);
    }
    m_needDelayedCheck = false;
}

void Kded::recreate(bool initial)
{
    m_recreateBusy = true;
    // Using KLauncher here is difficult since we might not have a
    // database

    if (!initial) {
        updateDirWatch(); // Update tree first, to be sure to miss nothing.
        if (runBuildSycoca()) {
            recreateDone();
        } else {
            recreateFailed();
        }
    } else {
        if(!delayedCheck) {
            updateDirWatch(); // this would search all the directories
        }
        if (bCheckSycoca) {
            runBuildSycoca();
        }
        recreateDone();
        if (delayedCheck) {
            // do a proper ksycoca check after a delay
            QTimer::singleShot(60000, this, SLOT(runDelayedCheck()));
            m_needDelayedCheck = true;
            delayedCheck = false;
        } else {
            m_needDelayedCheck = false;
        }
    }
}

void Kded::recreateFailed()
{
    afterRecreateFinished();
}

void Kded::recreateDone()
{
    updateResourceList();
    afterRecreateFinished();
}

void Kded::afterRecreateFinished()
{
    m_recreateBusy = false;

    initModules();
}

void Kded::update(const QString& )
{
    if (!m_recreateBusy) {
        m_pTimer->start(10000);
    }
}

void Kded::readDirectory(const QString& _path)
{
    QString path(_path);
    if (!path.endsWith('/')) {
        path += '/';
    }

    // Already seen this one?
    if (m_pDirWatch->contains(path)) {
        return;
    }

    m_pDirWatch->addDir(path,KDirWatch::WatchFiles|KDirWatch::WatchSubDirs);
}

#if 0
bool Kded::isWindowRegistered(long windowId) const
{
    return m_globalWindowIdList.contains(windowId);
}
#endif

void Kded::registerWindowId(qlonglong windowId, const QString &sender)
{
    if (!m_windowIdList.contains(sender)) {
        m_serviceWatcher->addWatchedService(sender);
    }

    m_globalWindowIdList.insert(windowId);
    QList<qlonglong> windowIds = m_windowIdList.value(sender);
    windowIds.append(windowId);
    m_windowIdList.insert(sender, windowIds);

    foreach (KDEDModule* module, m_modules) {
        // kDebug() << module->moduleName();
        emit module->windowRegistered(windowId);
    }
}

void Kded::unregisterWindowId(qlonglong windowId, const QString &sender)
{
    m_globalWindowIdList.remove(windowId);
    QList<qlonglong> windowIds = m_windowIdList.value(sender);
    if (!windowIds.isEmpty()) {
        windowIds.removeAll(windowId);
        if (windowIds.isEmpty()) {
            m_serviceWatcher->removeWatchedService(sender);
            m_windowIdList.remove(sender);
        } else {
            m_windowIdList.insert(sender, windowIds);
        }
    }

    foreach (KDEDModule* module, m_modules) {
        // kDebug() << module->moduleName();
        emit module->windowUnregistered(windowId);
    }
}


static void sighandler(int /*sig*/)
{
    if (qApp) {
        qApp->quit();
    }
}

KHostnameD::KHostnameD(int pollInterval)
{
    m_Timer.start(pollInterval); // repetitive timer (not single-shot)
    connect(&m_Timer, SIGNAL(timeout()), this, SLOT(checkHostname()));
    checkHostname();
}

KHostnameD::~KHostnameD()
{
}

void KHostnameD::checkHostname()
{
    const QByteArray newHostname = QHostInfo::localHostName().toUtf8();
    if (newHostname.isEmpty()) {
        return;
    }

    if (m_hostname.isEmpty()) {
       m_hostname = newHostname;
       return;
    }

    if (m_hostname == newHostname) {
       return;
    }

    runDontChangeHostname(m_hostname, newHostname);
    m_hostname = newHostname;
}


KBuildsycocaAdaptor::KBuildsycocaAdaptor(QObject *parent)
   : QDBusAbstractAdaptor(parent)
{
}

void KBuildsycocaAdaptor::recreate()
{
    Kded::self()->recreate();
}

int main(int argc, char *argv[])
{
    KAboutData aboutData("kded" /*don't change this one to kded4! dbus registration should be org.kde.kded etc.*/,
        "kdelibs4", ki18n("KDE Daemon"),
        KDE_VERSION_STRING,
        ki18n("KDE Daemon - triggers Sycoca database updates when needed"));

    KCmdLineOptions options;
    options.add("check", ki18n("Check Sycoca database only once"));

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineArgs::addCmdLineOptions(options);

    // WABA: Make sure not to enable session management.
    putenv(qstrdup("SESSION_MANAGER="));

    // Parse command line before checking DCOP
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KComponentData componentData(&aboutData);
    KSharedConfig::Ptr config = componentData.config(); // Enable translations.

    KApplication app;

    KConfigGroup cg(config, "General");
    if (args->isSet("check")) {
        checkStamps = cg.readEntry("CheckFileStamps", true);
        runBuildSycoca();
        return 0;
    }

    // Thiago: reenable if such a thing exists in QtDBus in the future
    //KUniqueApplication::dcopClient()->setQtBridgeEnabled(false);

    HostnamePollInterval = cg.readEntry("HostnamePollInterval", 5000);
    bCheckSycoca = cg.readEntry("CheckSycoca", true);
    bCheckUpdates = cg.readEntry("CheckUpdates", true);
    bCheckHostname = cg.readEntry("CheckHostname", true);
    checkStamps = cg.readEntry("CheckFileStamps", true);
    delayedCheck = cg.readEntry("DelayedCheck", false);

    Kded *kded = new Kded(); // Build data base

    KDE_signal(SIGTERM, sighandler);
    KDE_signal(SIGHUP, sighandler);
    app.setQuitOnLastWindowClosed(false);

    kded->recreate(true); // initial

#ifdef Q_WS_X11
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = XInternAtom( QX11Info::display(), "_KDE_SPLASH_PROGRESS", False);
    e.xclient.display = QX11Info::display();
    e.xclient.window = QX11Info::appRootWindow();
    e.xclient.format = 8;
    strcpy(e.xclient.data.b, "kded");
    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureNotifyMask, &e);
#endif

    if (bCheckHostname) {
        (void) new KHostnameD(HostnamePollInterval); // Watch for hostname changes
    }

    int result = app.exec(); // keep running

    delete kded;

    return result;
}

#include "moc_kded.cpp"
