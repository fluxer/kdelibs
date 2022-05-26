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

#include <kdeversion.h>
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

#include <QFile>
#include <QProcess>
#include <QHostInfo>
#include <QDBusReply>
#include <QDBusConnectionInterface>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#include <unistd.h>

#define KDED_EXENAME "kded4"

#define MODULES_PATH "/modules/"
#define MODULES_PATH_SIZE 9

Kded *Kded::_self = 0;

static int HostnamePollInterval = 5000;
static bool bCheckStamps = true;
static bool bCheckSycoca = true;
static bool bCheckHostname = true;

QT_BEGIN_NAMESPACE
extern Q_DBUS_EXPORT void qDBusAddSpyHook(void (*)(const QDBusMessage&));
QT_END_NAMESPACE

static bool runBuildSycoca()
{
    const QString exe = KStandardDirs::findExe(KBUILDSYCOCA_EXENAME);
    Q_ASSERT(!exe.isEmpty());
    QStringList args;
    if (bCheckStamps) {
        args.append("--checkstamps");
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

Kded::Kded(QObject *parent)
    : QObject(parent),
    m_pDirWatch(nullptr),
    m_pTimer(nullptr),
    m_serviceWatcher(nullptr)
{
    _self = this;

    m_serviceWatcher = new QDBusServiceWatcher(this);
    m_serviceWatcher->setConnection(QDBusConnection::sessionBus());
    m_serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(m_serviceWatcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(slotApplicationRemoved(QString)));

    new KBuildsycocaAdaptor(this);
    new KdedAdaptor(this);

    QDBusConnection session = QDBusConnection::sessionBus();
    session.registerObject("/kbuildsycoca", this);
    session.registerObject("/kded", this);
    session.registerService("org.kde.kded");

    updateDirWatch();
    updateResourceList();
    initModules();

    qDBusAddSpyHook(messageFilter);

    m_pTimer = new QTimer(this);
    m_pTimer->setSingleShot(true);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(recreate()));
}

Kded::~Kded()
{
    _self = 0;

    QDBusConnection session = QDBusConnection::sessionBus();
    session.unregisterObject("/kbuildsycoca");
    session.unregisterObject("/kded");
    session.unregisterService("org.kde.kded");

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
    if (!obj.startsWith(QLatin1String(MODULES_PATH))) {
        return;
    }

    // Remove the <MODULES_PATH> part
    obj = obj.mid(MODULES_PATH_SIZE);

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
    if (!kde_running) {
        kde_running = (QProcess::execute("kcheckrunning") == 0);
    }
    // qDebug() << Q_FUNC_INFO << kde_running;
    if (kde_running) {
        // not the same user like the one running the session (most likely we're run via sudo or something)
        const QByteArray sessionUID = qgetenv("KDE_SESSION_UID");
        if (!sessionUID.isEmpty() && uid_t(sessionUID.toInt()) != ::getuid()) {
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
    if (!bCheckSycoca) {
        return;
    }

    delete m_pDirWatch;
    m_pDirWatch = new KDirWatch(this);
    connect(m_pDirWatch, SIGNAL(dirty(QString)), this, SLOT(update(QString)));

    foreach(const QString &it, m_allResourceDirs) {
        readDirectory(it);
    }
}

void Kded::updateResourceList()
{
    KSycoca::clearCaches();

    foreach(const QString &it, KSycoca::self()->allResourceDirs()) {
        if (!m_allResourceDirs.contains(it)) {
            m_allResourceDirs.append(it);
            readDirectory(it);
        }
    }
}

void Kded::recreate()
{
    updateDirWatch(); // this would search all the directories
    runBuildSycoca();
    updateResourceList();

    initModules();
}

void Kded::update(const QString& path)
{
    Q_UNUSED(path);
    if (!m_pTimer->isActive()) {
        m_pTimer->start(5000);
    }
}

void Kded::readDirectory(const QString& _path)
{
    if (!bCheckSycoca) {
        return;
    }

    QString path(_path);
    if (!path.endsWith('/')) {
        path += '/';
    }

    // Already seen this one?
    if (m_pDirWatch->contains(path)) {
        return;
    }

    m_pDirWatch->addDir(path, KDirWatch::WatchFiles|KDirWatch::WatchSubDirs);
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

KHostnameD::KHostnameD(QObject *parent, int pollInterval)
    : QObject(parent)
{
    m_Timer.start(pollInterval); // repetitive timer (not single-shot)
    connect(&m_Timer, SIGNAL(timeout()), this, SLOT(checkHostname()));
    checkHostname();
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

    KCmdLineArgs::init(argc, argv, &aboutData);

    KComponentData componentData(&aboutData);
    KSharedConfig::Ptr config = componentData.config(); // Enable translations.

    // NOTE: disables session manager entirely, for reference:
    // https://www.x.org/releases/X11R7.7/doc/libSM/xsmp.html
    ::unsetenv("SESSION_MANAGER");

    KApplication app;
    app.setQuitOnLastWindowClosed(false);
    app.disableSessionManagement();
    app.quitOnSignal();

    QDBusConnection session = QDBusConnection::sessionBus();
    if (!session.isConnected()) {
        kWarning() << "No DBUS session-bus found. Check if you have started the DBUS server.";
        return 1;
    }
    QDBusReply<bool> sessionReply = session.interface()->isServiceRegistered("org.kde.kded");
    if (sessionReply.isValid() && sessionReply.value() == true) {
        kWarning() << "Another instance of kded4 is already running!";
        return 2;
    }

    KConfigGroup cg(config, "General");
    HostnamePollInterval = cg.readEntry("HostnamePollInterval", 5000);
    bCheckSycoca = cg.readEntry("CheckSycoca", true);
    bCheckHostname = cg.readEntry("CheckHostname", true);
    bCheckStamps = cg.readEntry("CheckFileStamps", true);

    Kded kded(&app);

#ifdef Q_WS_X11
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = XInternAtom(QX11Info::display(), "_KDE_SPLASH_PROGRESS", False);
    e.xclient.display = QX11Info::display();
    e.xclient.window = QX11Info::appRootWindow();
    e.xclient.format = 8;
    strcpy(e.xclient.data.b, "kded");
    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureNotifyMask, &e);
#endif

    if (bCheckHostname) {
        (void)new KHostnameD(&app, HostnamePollInterval); // Watch for hostname changes
    }

    return app.exec(); // keep running
}

#include "moc_kded.cpp"
