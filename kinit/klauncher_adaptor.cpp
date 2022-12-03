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
#include "kprotocolmanager.h"
#include "krun.h"
#include "kstandarddirs.h"
#include "kautostart.h"
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

static const int s_eventstime = 250;
static const int s_sleeptime = 50;
static const qint64 s_servicetimeout = 10000; // 10sec

KLauncherAdaptor::KLauncherAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    m_environment = QProcessEnvironment::systemEnvironment();
}

KLauncherAdaptor::~KLauncherAdaptor()
{
    kDebug() << "terminating processes" << m_processes.size();
    while (!m_processes.isEmpty()) {
        QProcess* process = m_processes.takeLast();
        disconnect(process, 0, this, 0);
        process->terminate();
        process->waitForFinished();
    }
}

void KLauncherAdaptor::autoStart(int phase)
{
    if (m_autostart.isEmpty()) {
        m_autostart = KGlobal::dirs()->findAllResources(
            "autostart",
            QString::fromLatin1("*.desktop"),
            KStandardDirs::NoDuplicates
        );
    }

    kDebug() << "autostart, phase" << phase;
    foreach(const QString &it, m_autostart) {
        KAutostart kautostart(it);
        if (!kautostart.autostarts(QString::fromLatin1("KDE"), KAutostart::CheckAll)) {
            continue;
        }
        if (kautostart.startPhase() != phase) {
            continue;
        }
        KService kservice(it);
        QStringList programandargs = KRun::processDesktopExec(kservice, KUrl::List());
        if (programandargs.isEmpty()) {
            kWarning() << "could not process service" << kservice.entryPath();
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
        kWarning() << "Could not find" << name;
        return;
    }

    kDebug() << "blind starting" << appexe << arg_list << m_environment.toStringList();
    const QString envexe = findExe("env");
    if (envexe.isEmpty()) {
        kWarning() << "env program not found";
        QProcess::startDetached(appexe, arg_list);
        return;
    }

    QStringList envargs = m_environment.toStringList();
    envargs += appexe;
    envargs += arg_list;
    QProcess::startDetached(envexe, envargs);
}

int KLauncherAdaptor::kdeinit_exec(const QString &app, const QStringList &args, const QStringList &env, const QString& startup_id,
                                   const QDBusMessage &msg, QString &dbusServiceName, QString &error, qint64 &pid)
{
    return kdeinit_exec_with_workdir(app, args, QDir::currentPath(), env, startup_id, msg, dbusServiceName, error, pid);
}

int KLauncherAdaptor::kdeinit_exec_wait(const QString &app, const QStringList &args, const QStringList &env, const QString& startup_id,
                                        const QDBusMessage &msg, QString &dbusServiceName, QString &error, qint64 &pid)
{
    const int result = kdeinit_exec(app, args, env, startup_id, msg, dbusServiceName, error, pid);
    if (result != KLauncherAdaptor::NoError) {
        return result;
    }
    kDebug() << "waiting for" << pid;
    while (::kill(pid, 0) >= 0) {
        QApplication::processEvents(QEventLoop::AllEvents, s_eventstime);
        QThread::msleep(s_sleeptime);
    }
    int pidstate = 0;
    ::waitpid(pid, &pidstate, WNOHANG);
    kDebug() << "done waiting for" << pid << ", exit status" << WEXITSTATUS(pidstate);
    return WEXITSTATUS(pidstate);
}

int KLauncherAdaptor::kdeinit_exec_with_workdir(const QString &app, const QStringList &args, const QString& workdir, const QStringList &env, const QString& startup_id,
                                                const QDBusMessage &msg, QString &dbusServiceName, QString &error, qint64 &pid)
{
    const QString appexe = findExe(app);
    if (appexe.isEmpty()) {
        error = i18n("Could not find: %1", app);
        return KLauncherAdaptor::FindError;
    }

    QProcess* process = new QProcess(this);
    connect(process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(slotProcessStateChanged(QProcess::ProcessState)));
    connect(process, SIGNAL(finished(int)), this, SLOT(slotProcessFinished(int)));
    m_processes.append(process);
    QProcessEnvironment processenv = m_environment;
    foreach (const QString &it, env) {
        const int equalindex = it.indexOf(QLatin1Char('='));
        if (equalindex <= 0) {
            kWarning() << "Invalid environment variable" << it;
            continue;
        }
        const QString environmentvar = it.mid(0, equalindex);
        const QString environmentvalue = it.mid(equalindex + 1, it.size() - equalindex - 1);
        kDebug() << "adding to environment" << environmentvar << environmentvalue;
        processenv.insert(environmentvar, environmentvalue);
    }
    process->setProcessEnvironment(processenv);
    process->setWorkingDirectory(workdir);
    kDebug() << "starting" << appexe << args << env << processenv.toStringList();
    m_kstartupinfoid = KStartupInfoId();
    m_kstartupinfoid.initId(startup_id.toLatin1());
    m_kstartupinfodata = KStartupInfoData();
    m_kstartupinfodata.setBin(appexe);
    process->start(appexe, args);
    sendSIStart();
    while (process->state() == QProcess::Starting) {
        QApplication::processEvents(QEventLoop::AllEvents, s_eventstime);
        QThread::msleep(s_sleeptime);
    }
    if (process->error() == QProcess::FailedToStart || process->error() == QProcess::Crashed) {
        sendSIFinish();
        error = i18n("Could not start: %1", appexe);
        return KLauncherAdaptor::ExecError;
    }
    if (dbusServiceName.isEmpty()) {
        // if service name is not empty it will be send when the service is up
        sendSIFinish();
    }

    error.clear();
    pid = process->pid();
    return KLauncherAdaptor::NoError;
}

void KLauncherAdaptor::reparseConfiguration()
{
    kDebug() << "reparsing configuration";
    KProtocolManager::reparseConfiguration();
}

void KLauncherAdaptor::setLaunchEnv(const QString &name, const QString &value)
{
    kDebug() << "setting environment variable" << name << "to" << value;
    m_environment.insert(name, value);
}

int KLauncherAdaptor::start_service_by_desktop_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind,
                                                    const QDBusMessage &msg, QString &dbusServiceName, QString &error, qint64 &pid)
{
    KService::Ptr kservice = KService::serviceByDesktopName(serviceName);
    if (!kservice) {
        error = i18n("Invalid service name: %1", serviceName);
        return KLauncherAdaptor::ServiceError;
    }
    return start_service_by_desktop_path(kservice->entryPath(), urls, envs, startup_id, blind, msg, dbusServiceName, error, pid);
}

int KLauncherAdaptor::start_service_by_desktop_path(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind,
                                                    const QDBusMessage &msg, QString &dbusServiceName, QString &error, qint64 &pid)
{
    KService::Ptr kservice = KService::serviceByStorageId(serviceName);
    if (!kservice) {
        error = i18n("Invalid service path: %1", serviceName);
        return KLauncherAdaptor::ServiceError;
    }
    if (urls.size() > 1 && !kservice->allowMultipleFiles()) {
        // TODO: start multiple instances for each URL
        error = i18n("Service does not support multiple files: %1", serviceName);
        return KLauncherAdaptor::ServiceError;
    }
    QStringList programandargs = KRun::processDesktopExec(*kservice, urls);
    if (programandargs.isEmpty()) {
        error = i18n("Could not process service: %1", kservice->entryPath());
        return KLauncherAdaptor::ArgumentsError;
    }
    const QString program = programandargs.takeFirst();
    const QStringList programargs = programandargs;
    const KService::DBusStartupType dbusstartuptype = kservice->dbusStartupType();
    dbusServiceName = kservice->property(QString::fromLatin1("X-DBUS-ServiceName"), QVariant::String).toString();
    const int result = kdeinit_exec(program, programargs, envs, startup_id, msg, dbusServiceName, error, pid);
    if (result != KLauncherAdaptor::NoError) {
        // sendSIFinish() is called on exec error
        return result;
    }
    // blind means do not wait, even if its type is wait
    if (blind || dbusstartuptype == KService::DBusNone) {
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
    kDebug() << "waiting for" << pid << dbusServiceName;
    QDBusConnection session = QDBusConnection::sessionBus();
    QElapsedTimer elapsedtime;
    elapsedtime.start();
    while (true) {
        QDBusReply<bool> sessionreply = session.interface()->isServiceRegistered(dbusServiceName);
        if (!sessionreply.isValid()) {
            sendSIFinish();
            error = i18n("Invalid D-Bus reply for: %1", dbusServiceName);
            return KLauncherAdaptor::DBusError;
        }
        // the service unregistered
        if (sessionreply.value() == false && dbusstartuptype == KService::DBusWait) {
            break;
        }
        // the service registered
        if (sessionreply.value() == true && dbusstartuptype != KService::DBusWait) {
            break;
        }
        // or the program is just not registering the service at all
        if (elapsedtime.elapsed() >= s_servicetimeout && dbusstartuptype != KService::DBusWait) {
            kWarning() << "timed out for" << pid << ", service name" << dbusServiceName;
            break;
        }
        QApplication::processEvents(QEventLoop::AllEvents, s_eventstime);
        QThread::msleep(s_sleeptime);
    }
    kDebug() << "done waiting for" << pid << ", service name" << dbusServiceName;
    sendSIFinish();
    return result;
}

void KLauncherAdaptor::slotProcessStateChanged(QProcess::ProcessState state)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    kDebug() << "process state changed" << process << state;
    if (state == QProcess::Starting && !m_kstartupinfoid.none()) {
        m_kstartupinfodata.addPid(process->pid());
        sendSIUpdate();
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
        return app;
    }
    const QString environmentpath = m_environment.value(QString::fromLatin1("PATH"), QString());
    return KStandardDirs::findExe(app, environmentpath);
}

void KLauncherAdaptor::sendSIStart() const
{
    kDebug() << "sending ASN start";
    KStartupInfo::sendStartup(m_kstartupinfoid, m_kstartupinfodata);
}

void KLauncherAdaptor::sendSIUpdate()
{
    if (m_kstartupinfoid.none()) {
        return;
    }
    kDebug() << "sending ASN update";
    KStartupInfo::sendChange(m_kstartupinfoid, m_kstartupinfodata);
}

void KLauncherAdaptor::sendSIFinish()
{
    if (m_kstartupinfoid.none()) {
        return;
    }
    kDebug() << "sending ASN finish";
    KStartupInfo::sendFinish(m_kstartupinfoid, m_kstartupinfodata);
    m_kstartupinfoid = KStartupInfoId();
    m_kstartupinfodata = KStartupInfoData();
}

#include "moc_klauncher_adaptor.cpp"
