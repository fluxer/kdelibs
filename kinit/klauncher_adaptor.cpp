/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "klauncher_adaptor.h"
#include "krun.h"
#include "kstandarddirs.h"
#include "kservice.h"
#include "kautostart.h"
#include "kshell.h"
#include "kconfiggroup.h"
#include "kdebug.h"

#include <QDir>
#include <QApplication>
#include <QThread>
#include <QElapsedTimer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

static const int s_eventstime = 250;
static const int s_sleeptime = 50;
// NOTE: keep in sync with:
// kde-workspace/kwin/effects/startupfeedback/startupfeedback.cpp
// kde-workspace/kcontrol/launch/kcmlaunch.cpp
static const qint64 s_startuptimeout = 10; // 10sec
// klauncher is the last process to quit in a session (see kde-workspace/startkde.cmake) so 5sec
// for each child process is more than enough
static const qint64 s_processtimeout = 5000; // 5sec

static inline bool isPIDAlive(const pid_t pid)
{
    return (::kill(pid, 0) >= 0);
}

static inline int getExitStatus(const pid_t pid)
{
    int pidstate = 0;
    ::waitpid(pid, &pidstate, WNOHANG);
    return WEXITSTATUS(pidstate);
}

KLauncherAdaptor::KLauncherAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent),
    m_dbusconnectioninterface(nullptr),
    m_startuptimeout(s_startuptimeout * 1000)
{
    m_environment = QProcessEnvironment::systemEnvironment();
    m_dbusconnectioninterface = QDBusConnection::sessionBus().interface();

    // TODO: config watch
    KConfig klauncherconfig("klaunchrc", KConfig::NoGlobals);
    KConfigGroup kconfiggroup = klauncherconfig.group("BusyCursorSettings");
    const int busytimeout = kconfiggroup.readEntry("Timeout", s_startuptimeout);
    kconfiggroup = klauncherconfig.group("TaskbarButtonSettings");
    const int tasktimeout = kconfiggroup.readEntry("Timeout", s_startuptimeout);
    m_startuptimeout = (qMax(busytimeout, tasktimeout) * 1000);
}

KLauncherAdaptor::~KLauncherAdaptor()
{
    kDebug() << "terminating processes" << m_processes.size();
    cleanup();
}

void KLauncherAdaptor::autoStart(int phase)
{
    if (m_autostart.isEmpty()) {
        kDebug() << "finding autostart desktop files" << phase;
        m_autostart = KGlobal::dirs()->findAllResources(
            "autostart",
            QString::fromLatin1("*.desktop"),
            KStandardDirs::NoDuplicates
        );
    }

    kDebug() << "autostart phase" << phase;
    foreach(const QString &it, m_autostart) {
        kDebug() << "checking autostart" << it;
        KAutostart kautostart(it);
        if (kautostart.startPhase() != phase) {
            continue;
        }
        if (!kautostart.autostarts(QString::fromLatin1("KDE"), KAutostart::CheckAll)) {
            kDebug() << "not autostarting" << it;
            continue;
        }
        QStringList programandargs = KShell::splitArgs(kautostart.command());
        if (programandargs.isEmpty()) {
            kWarning() << "could not process autostart" << it;
            continue;
        }
        const QString program = programandargs.takeFirst();
        const QStringList programargs = programandargs;
        exec_blind(program, programargs);
    }
    switch (phase) {
        case 0: {
            emit autoStart0Done();
            break;
        }
        case 1: {
            emit autoStart1Done();
            break;
        }
        case 2: {
            emit autoStart2Done();
            break;
        }
    }
}

void KLauncherAdaptor::exec_blind(const QString &name, const QStringList &arg_list)
{
    const QString appexe = findExe(name);
    if (appexe.isEmpty()) {
        kWarning() << "could not find" << name;
        return;
    }

    const QStringList envlist = m_environment.toStringList();
    kDebug() << "blind starting" << appexe << arg_list << envlist;
    const QString envexe = findExe("env");
    if (envexe.isEmpty()) {
        kWarning() << "env program not found";
        QProcess::startDetached(appexe, arg_list);
        return;
    }

    QStringList envargs = envlist;
    envargs += appexe;
    envargs += arg_list;
    QProcess::startDetached(envexe, envargs);
}

void KLauncherAdaptor::cleanup()
{
    while (!m_processes.isEmpty()) {
        QProcess* process = m_processes.takeLast();
        disconnect(process, 0, this, 0);
        process->terminate();
        if (!process->waitForFinished(s_processtimeout)) {
            kWarning() << "process still running" << process->pid();
            // SIGKILL is non-ignorable
            process->kill();
        }
    }
}

int KLauncherAdaptor::kdeinit_exec(const QString &app, const QStringList &args, const QStringList &envs, const QString& startup_id)
{
    return kdeinit_exec_with_workdir(app, args, envs, startup_id, QDir::currentPath());
}

int KLauncherAdaptor::kdeinit_exec_wait(const QString &app, const QStringList &args, const QStringList &envs, const QString &startup_id)
{
    QMutexLocker locker(&m_mutex);
    qint64 pid = 0;
    int result = startProgram(app, args, envs, startup_id, QDir::currentPath(), pid);
    if (result != KLauncherAdaptor::NoError) {
        return result;
    }
    kDebug() << "waiting for" << pid;
    while (isPIDAlive(pid)) {
        QApplication::processEvents(QEventLoop::AllEvents, s_eventstime);
        QThread::msleep(s_sleeptime);
    }
    result = getExitStatus(pid);
    kDebug() << "done waiting for" << pid << ", exit status" << result;
    return result;
}

int KLauncherAdaptor::kdeinit_exec_with_workdir(const QString &app, const QStringList &args, const QStringList &envs, const QString &startup_id, const QString &workdir)
{
    QMutexLocker locker(&m_mutex);
    qint64 pid = 0;
    return startProgram(app, args, envs, startup_id, workdir, pid);
}

void KLauncherAdaptor::setLaunchEnv(const QString &name, const QString &value)
{
    if (name.isEmpty()) {
        kWarning() << "attempting to set empty environment variable to" << value;
        return;
    }
    kDebug() << "setting environment variable" << name << "to" << value;
    m_environment.insert(name, value);
}

int KLauncherAdaptor::start_service_by_desktop_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id)
{
    KService::Ptr kservice = KService::serviceByDesktopName(serviceName);
    if (!kservice) {
        kWarning() << "invalid service name" << serviceName;
        return KLauncherAdaptor::ServiceError;
    }
    return start_service_by_desktop_path(kservice->entryPath(), urls, envs, startup_id);
}

int KLauncherAdaptor::start_service_by_desktop_path(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id)
{
    QMutexLocker locker(&m_mutex);
    KService::Ptr kservice = KService::serviceByStorageId(serviceName);
    if (!kservice) {
        kWarning() << "invalid service path" << serviceName;
        return KLauncherAdaptor::ServiceError;
    }
    const KService::DBusStartupType dbusstartuptype = kservice->dbusStartupType();
    QString dbusServiceName = kservice->property(QString::fromLatin1("X-DBUS-ServiceName"), QVariant::String).toString();
    // any unique Katana application/service checks if another instance is running, if it is
    // already running starting it may raise its window instead (if it uses KUniqueApplication)
    if (dbusstartuptype == KService::DBusUnique && !dbusServiceName.startsWith(QLatin1String("org.kde."))) {
        QDBusReply<bool> sessionreply = m_dbusconnectioninterface->isServiceRegistered(dbusServiceName);
        if (!sessionreply.isValid()) {
            kWarning() << "invalid D-Bus reply for" << dbusServiceName;
            return KLauncherAdaptor::DBusError;
        }
        if (sessionreply.value() == true) {
            kDebug() << "service already started" << dbusServiceName;
            return KLauncherAdaptor::NoError;
        }
    }
    if (urls.size() > 1 && !kservice->allowMultipleFiles()) {
        // TODO: start multiple instances for each URL
        kWarning() << "service does not support multiple files" << serviceName;
        return KLauncherAdaptor::ServiceError;
    }
    QStringList programandargs = KRun::processDesktopExec(*kservice, urls);
    if (programandargs.isEmpty()) {
        kWarning() << "could not process service" << kservice->entryPath();
        return KLauncherAdaptor::ArgumentsError;
    }
    kDebug() << "starting" << kservice->entryPath() << urls << dbusServiceName;
    const QString program = programandargs.takeFirst();
    const QStringList programargs = programandargs;
    Q_ASSERT(m_kstartupinfoid.none() == true);
    m_kstartupinfoid = KStartupInfoId();
    m_kstartupinfodata = KStartupInfoData();
    bool startupsilent = false;
    QByteArray startupwmclass;
    if (KRun::checkStartupNotify(kservice.data(), &startupsilent, &startupwmclass)) {
        m_kstartupinfoid.initId(startup_id.toLatin1());
        m_kstartupinfodata.setBin(QFileInfo(program).fileName());
        m_kstartupinfodata.setDescription(i18n("Launching %1", kservice->name()));
        m_kstartupinfodata.setIcon(kservice->icon());
        m_kstartupinfodata.setApplicationId(kservice->entryPath());
        m_kstartupinfodata.setSilent(startupsilent ? KStartupInfoData::Yes : KStartupInfoData::No);
        m_kstartupinfodata.setWMClass(startupwmclass);
        sendSIStart();
    } else {
        kDebug() << "no ASN for" << kservice->entryPath();
    }
    qint64 pid = 0;
    int result = startProgram(program, programargs, envs, QString(), QDir::currentPath(), pid);
    if (result != KLauncherAdaptor::NoError) {
        // sendSIFinish() is called on exec error
        return result;
    }
    if (dbusstartuptype == KService::DBusNone) {
        sendSIFinish();
        return result;
    } else if (dbusstartuptype != KService::DBusNone && dbusServiceName.isEmpty()) {
        // not going to guess what service to wait for, bud
        kWarning() << "X-DBUS-ServiceName not specified in" << kservice->entryPath();
        sendSIFinish();
        return result;
    } else if (dbusstartuptype == KService::DBusMulti) {
        dbusServiceName.append(QLatin1Char('-'));
        dbusServiceName.append(QString::number(pid));
    }
    kDebug() << "waiting for" << pid << dbusServiceName << m_startuptimeout;
    QElapsedTimer elapsedtime;
    elapsedtime.start();
    while (true) {
        QDBusReply<bool> sessionreply = m_dbusconnectioninterface->isServiceRegistered(dbusServiceName);
        if (!sessionreply.isValid()) {
            kWarning() << "invalid D-Bus reply for" << dbusServiceName;
            sendSIFinish();
            return KLauncherAdaptor::DBusError;
        }
        // the service unregistered
        if (sessionreply.value() == false && dbusstartuptype == KService::DBusWait) {
            kDebug() << "service unregistered" << dbusServiceName;
            break;
        }
        // the service registered
        if (sessionreply.value() == true && dbusstartuptype != KService::DBusWait) {
            kDebug() << "service registered" << dbusServiceName;
            break;
        }
        // or the program is just not registering the service at all
        if (elapsedtime.elapsed() >= m_startuptimeout && dbusstartuptype != KService::DBusWait) {
            kWarning() << "timed out while waiting for service" << dbusServiceName;
            break;
        }
        // or the program is not even running
        if (!isPIDAlive(pid)) {
            kWarning() << "service process is not running" << dbusServiceName;
            result = getExitStatus(pid);
            break;
        }
        QApplication::processEvents(QEventLoop::AllEvents, s_eventstime);
        QThread::msleep(s_sleeptime);
    }
    sendSIFinish();
    return result;
}

#ifdef KLAUNCHER_DEBUG
QStringList KLauncherAdaptor::environment() const
{
    return m_environment.toStringList();
}
#endif

void KLauncherAdaptor::slotProcessStateChanged(QProcess::ProcessState state)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    kDebug() << "process state changed" << process << state;
    if (state == QProcess::Starting && !m_kstartupinfoid.none()) {
        m_kstartupinfodata.addPid(process->pid());
        sendSIChange();
    } else if (state == QProcess::NotRunning && m_kstartupinfodata.is_pid(process->pid())) {
        sendSIFinish();
    }
}

void KLauncherAdaptor::slotProcessFinished(int exitcode)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    kDebug() << "process finished" << process << exitcode;
    m_processes.removeAll(process);
}

QString KLauncherAdaptor::findExe(const QString &app) const
{
    if (QDir::isAbsolutePath(app)) {
        if (!QFile::exists(app)) {
            // return empty string if it does not exists (like KStandardDirs::findExe())
            return QString();
        }
        return app;
    }
    const QString environmentpath = m_environment.value(QString::fromLatin1("PATH"), QString());
    return KStandardDirs::findExe(app, environmentpath);
}

int KLauncherAdaptor::startProgram(const QString &app, const QStringList &args, const QStringList &envs, const QString &startup_id, const QString &workdir, qint64 &pid)
{
    const QString appexe = findExe(app);
    if (appexe.isEmpty()) {
        kWarning() << "could not find" << app;
        return KLauncherAdaptor::FindError;
    }

    QProcess* process = new QProcess(this);
    connect(process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(slotProcessStateChanged(QProcess::ProcessState)));
    connect(process, SIGNAL(finished(int)), this, SLOT(slotProcessFinished(int)));
    m_processes.append(process);
    QProcessEnvironment processenv = m_environment;
    foreach (const QString &env, envs) {
        const int equalindex = env.indexOf(QLatin1Char('='));
        if (equalindex <= 0) {
            kWarning() << "invalid environment variable" << env;
            continue;
        }
        const QString environmentvar = env.mid(0, equalindex);
        const QString environmentvalue = env.mid(equalindex + 1, env.size() - equalindex - 1);
        kDebug() << "adding to environment" << environmentvar << environmentvalue;
        processenv.insert(environmentvar, environmentvalue);
    }
    process->setProcessEnvironment(processenv);
    process->setWorkingDirectory(workdir);
    kDebug() << "starting" << appexe << args << envs;
    if (!startup_id.isEmpty()) {
        Q_ASSERT(m_kstartupinfoid.none() == true);
        m_kstartupinfoid = KStartupInfoId();
        m_kstartupinfodata = KStartupInfoData();
        m_kstartupinfoid.initId(startup_id.toLatin1());
        m_kstartupinfodata.setBin(QFileInfo(appexe).fileName());
        m_kstartupinfodata.setDescription(i18n("Launching %1", m_kstartupinfodata.bin()));
        sendSIStart();
    }
    process->start(appexe, args);
    while (process->state() == QProcess::Starting) {
        QApplication::processEvents(QEventLoop::AllEvents, s_eventstime);
        QThread::msleep(s_sleeptime);
    }
    if (process->error() == QProcess::FailedToStart || process->error() == QProcess::Crashed) {
        sendSIFinish();
        kWarning() << "could not start" << appexe;
        return KLauncherAdaptor::ExecError;
    }
    if (!startup_id.isEmpty()) {
        sendSIFinish();
    }

    pid = process->pid();
    return KLauncherAdaptor::NoError;
}

void KLauncherAdaptor::sendSIStart() const
{
    if (m_kstartupinfoid.none()) {
        return;
    }
    kDebug() << "sending ASN start for" << m_kstartupinfodata.bin();
    KStartupInfo::sendStartup(m_kstartupinfoid, m_kstartupinfodata);
}

void KLauncherAdaptor::sendSIChange()
{
    if (m_kstartupinfoid.none()) {
        return;
    }
    kDebug() << "sending ASN change for" << m_kstartupinfodata.bin();
    KStartupInfo::sendChange(m_kstartupinfoid, m_kstartupinfodata);
}

void KLauncherAdaptor::sendSIFinish()
{
    if (m_kstartupinfoid.none()) {
        return;
    }
    kDebug() << "sending ASN finish for" << m_kstartupinfodata.bin();
    KStartupInfo::sendFinish(m_kstartupinfoid, m_kstartupinfodata);
    m_kstartupinfoid = KStartupInfoId();
    m_kstartupinfodata = KStartupInfoData();
}

#include "moc_klauncher_adaptor.cpp"
