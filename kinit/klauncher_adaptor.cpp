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
#include "kautostart.h"
#include "kshell.h"
#include "kconfiggroup.h"
#include "kdebug.h"

#include <QDir>
#include <QApplication>
#include <QThread>

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

static inline bool isASNValid(const QByteArray &asn)
{
    return (!asn.isEmpty() && asn != "0");
}

KLauncherProcess::KLauncherProcess(QObject *parent)
    : QProcess(parent),
    m_kstartupinfo(nullptr),
    m_startuptimer(nullptr)
{
    connect(
        this, SIGNAL(stateChanged(QProcess::ProcessState)),
        this, SLOT(slotProcessStateChanged(QProcess::ProcessState))
    );
}

void KLauncherProcess::setupStartup(const QByteArray &startup_id, const QString &appexe,
                                    const KService::Ptr kservice, const qint64 timeout)
{
    Q_ASSERT(m_kstartupinfoid.none() == true);
    bool startupsilent = false;
    QByteArray startupwmclass;
    if (KRun::checkStartupNotify(kservice.data(), &startupsilent, &startupwmclass)) {
        m_kstartupinfoid.initId(!isASNValid(startup_id) ? KStartupInfo::createNewStartupId() : startup_id);
        kDebug() << "setting up ASN for" << kservice->entryPath() << m_kstartupinfoid.id();
        m_kstartupinfodata.setHostname();
        m_kstartupinfodata.setBin(QFileInfo(appexe).fileName());
        m_kstartupinfodata.setDescription(i18n("Launching %1", kservice->name()));
        m_kstartupinfodata.setIcon(kservice->icon());
        m_kstartupinfodata.setApplicationId(kservice->entryPath());
        m_kstartupinfodata.setSilent(startupsilent ? KStartupInfoData::Yes : KStartupInfoData::No);
        m_kstartupinfodata.setWMClass(startupwmclass);
        QProcessEnvironment processenv = QProcess::processEnvironment();
        processenv.insert(QString::fromLatin1("DESKTOP_STARTUP_ID"), m_kstartupinfoid.id());
        QProcess::setProcessEnvironment(processenv);
        sendSIStart(timeout);
    } else if (isASNValid(startup_id)) {
        kDebug() << "setting up ASN for" << startup_id;
        m_kstartupinfoid.initId(startup_id);
        m_kstartupinfodata.setHostname();
        m_kstartupinfodata.setBin(QFileInfo(appexe).fileName());
        m_kstartupinfodata.setDescription(i18n("Launching %1", m_kstartupinfodata.bin()));
        QProcessEnvironment processenv = QProcess::processEnvironment();
        processenv.insert(QString::fromLatin1("DESKTOP_STARTUP_ID"), QString::fromLatin1(startup_id.constData(), startup_id.size()));
        QProcess::setProcessEnvironment(processenv);
        sendSIStart(timeout);
    } else {
        kDebug() << "no ASN for" << appexe;
    }
}

void KLauncherProcess::slotProcessStateChanged(QProcess::ProcessState state)
{
    kDebug() << "process state changed" << this << state;
    if (state == QProcess::Starting && !m_kstartupinfoid.none()) {
        m_kstartupinfodata.addPid(QProcess::pid());
        sendSIChange();
    } else if (state == QProcess::NotRunning && !m_kstartupinfoid.none()) {
        sendSIFinish();
    }
}

void KLauncherProcess::slotStartupRemoved(const KStartupInfoId &kstartupinfoid,
                                          const KStartupInfoData &kstartupinfodata)
{
    if (m_kstartupinfoid.none()) {
        return;
    }

    kDebug() << "startup removed" << kstartupinfoid.id() << m_kstartupinfoid.id();
    if (kstartupinfoid.id() == m_kstartupinfoid.id() || kstartupinfodata.is_pid(QProcess::pid())) {
        kDebug() << "startup done for process" << this;
        sendSIFinish();
    }
}

void KLauncherProcess::slotStartupTimeout()
{
    kWarning() << "timed out while waiting for process" << this;
    sendSIFinish();
}

void KLauncherProcess::sendSIStart(const qint64 timeout)
{
    if (m_kstartupinfoid.none()) {
        return;
    }

    kDebug() << "sending ASN start for" << m_kstartupinfodata.bin();
    m_kstartupinfo = new KStartupInfo(this);
    connect(
        m_kstartupinfo, SIGNAL(gotRemoveStartup(KStartupInfoId,KStartupInfoData)),
        this, SLOT(slotStartupRemoved(KStartupInfoId,KStartupInfoData))
    );

    m_startuptimer = new QTimer(this);
    m_startuptimer->setSingleShot(true);
    m_startuptimer->setInterval(timeout);
    connect(m_startuptimer, SIGNAL(timeout()), this, SLOT(slotStartupTimeout()));
    m_startuptimer->start();

    KStartupInfo::sendStartup(m_kstartupinfoid, m_kstartupinfodata);
}

void KLauncherProcess::sendSIChange()
{
    if (m_kstartupinfoid.none()) {
        return;
    }
    kDebug() << "sending ASN change for" << m_kstartupinfodata.bin();
    KStartupInfo::sendChange(m_kstartupinfoid, m_kstartupinfodata);
}

void KLauncherProcess::sendSIFinish()
{
    if (m_kstartupinfoid.none()) {
        return;
    }
    kDebug() << "sending ASN finish for" << m_kstartupinfodata.bin();
    KStartupInfo::sendFinish(m_kstartupinfoid, m_kstartupinfodata);
    m_kstartupinfoid = KStartupInfoId();
    m_kstartupinfodata = KStartupInfoData();
    if (m_startuptimer) {
        m_startuptimer->stop();
    }
}

KLauncherAdaptor::KLauncherAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent),
    m_startuptimeout(0)
{
    m_environment = QProcessEnvironment::systemEnvironment();

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
    kDebug() << "terminating processes" << m_processes.size();
    while (!m_processes.isEmpty()) {
        KLauncherProcess* process = m_processes.takeLast();
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
    qint64 pid = 0;
    int result = startProgram(app, args, envs, startup_id, QDir::currentPath(), pid, m_startuptimeout);
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
    qint64 pid = 0;
    return startProgram(app, args, envs, startup_id, workdir, pid, m_startuptimeout);
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
    KService::Ptr kservice = KService::serviceByStorageId(serviceName);
    if (!kservice) {
        kWarning() << "invalid service path" << serviceName;
        return KLauncherAdaptor::ServiceError;
    }
    if (urls.size() > 1 && !kservice->allowMultipleFiles()) {
        kWarning() << "service does not support multiple files" << serviceName;
        return KLauncherAdaptor::ServiceError;
    }
    QStringList programandargs = KRun::processDesktopExec(*kservice, urls);
    if (programandargs.isEmpty()) {
        kWarning() << "could not process service" << kservice->entryPath();
        return KLauncherAdaptor::ArgumentsError;
    }
    kDebug() << "starting" << kservice->entryPath() << urls;
    const QString program = programandargs.takeFirst();
    qint64 pid = 0;
    return startProgram(program, programandargs, envs, QString(), QDir::currentPath(), pid, m_startuptimeout, kservice);
}

#ifdef KLAUNCHER_DEBUG
QStringList KLauncherAdaptor::environment() const
{
    return m_environment.toStringList();
}
#endif

void KLauncherAdaptor::slotProcessFinished(int exitcode)
{
    KLauncherProcess* process = qobject_cast<KLauncherProcess*>(sender());
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

int KLauncherAdaptor::startProgram(const QString &app, const QStringList &args, const QStringList &envs,
                                   const QString &startup_id, const QString &workdir, qint64 &pid,
                                   const qint64 timeout, const KService::Ptr kservice)
{
    const QString appexe = findExe(app);
    if (appexe.isEmpty()) {
        kWarning() << "could not find" << app;
        return KLauncherAdaptor::FindError;
    }

    KLauncherProcess* process = new KLauncherProcess(this);
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
    process->setupStartup(startup_id.toLatin1(), appexe, kservice, timeout);
    kDebug() << "starting" << appexe << args << envs << workdir;
    process->start(appexe, args);
    while (process->state() == QProcess::Starting) {
        QApplication::processEvents(QEventLoop::AllEvents, s_eventstime);
        QThread::msleep(s_sleeptime);
    }
    if (process->error() == QProcess::FailedToStart || process->error() == QProcess::Crashed) {
        kWarning() << "could not start" << appexe;
        return KLauncherAdaptor::ExecError;
    }

    pid = process->pid();
    return KLauncherAdaptor::NoError;
}

#include "moc_klauncher_adaptor.cpp"
