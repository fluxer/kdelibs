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

#ifndef KLAUNCHER_ADAPTOR_H
#define KLAUNCHER_ADAPTOR_H

#include "kstartupinfo.h"
#include "kservice.h"

#include <QDBusAbstractAdaptor>
#include <QProcess>
#include <QTimer>

// #define KLAUNCHER_DEBUG

class KLauncherProcess : public QProcess
{
    Q_OBJECT
public:
    explicit KLauncherProcess(QObject *parent);

    void setupStartup(const QByteArray &startup_id, const QString &appexe,
                      const KService::Ptr kservice, const qint64 timeout);

private Q_SLOTS:
    void slotProcessStateChanged(QProcess::ProcessState state);
    void slotStartupRemoved(const KStartupInfoId &id, const KStartupInfoData &data);
    void slotStartupTimeout();

private:
    Q_DISABLE_COPY(KLauncherProcess);

    void sendSIStart(const qint64 timeout);
    void sendSIChange();
    void sendSIFinish();

    KStartupInfo* m_kstartupinfo;
    QTimer* m_startuptimer;
    KStartupInfoId m_kstartupinfoid;
    KStartupInfoData m_kstartupinfodata;
};

// Adaptor class for interface org.kde.KLauncher
class KLauncherAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KLauncher")
public:
    enum KLauncherError {
        NoError = 0,
        ServiceError = -1,
        FindError = -2,
        ArgumentsError = -3,
        ExecError = -4
    };

    KLauncherAdaptor(QObject *parent);
    ~KLauncherAdaptor();

public:
public Q_SLOTS:
    // used by ksmserver
    void autoStart(int phase);

    // used by ksmserver and klauncher itself
    void exec_blind(const QString &name, const QStringList &arg_list);
    void cleanup();

    // used by KToolInvocation
    void setLaunchEnv(const QString &name, const QString &value);
    int kdeinit_exec(const QString &app, const QStringList &args, const QStringList &envs, const QString &startup_id);
    int kdeinit_exec_wait(const QString &app, const QStringList &args, const QStringList &envs, const QString &startup_id);
    int kdeinit_exec_with_workdir(const QString &app, const QStringList &args, const QStringList &envs, const QString &startup_id, const QString &workdir);
    int start_service_by_desktop_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id);
    int start_service_by_desktop_path(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id);

    // for debugging
#ifdef KLAUNCHER_DEBUG
    QStringList environment() const;
#endif

Q_SIGNALS:
    void autoStart0Done();
    void autoStart1Done();
    void autoStart2Done();

private Q_SLOTS:
    void slotProcessFinished(int exitcode);

private:
    QString findExe(const QString &app) const;
    int startProgram(const QString &app, const QStringList &args, const QStringList &envs,
                     const QString &startup_id, const QString &workdir, qint64 &pid,
                     const qint64 timeout, const KService::Ptr kservice = KService::Ptr(nullptr));

    QProcessEnvironment m_environment;
    qint64 m_startuptimeout;
    QList<KLauncherProcess*> m_processes;
    QStringList m_autostart;
};

#endif // KLAUNCHER_ADAPTOR_H
