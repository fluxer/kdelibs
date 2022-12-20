/* This file is part of the KDE libraries
    Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
    Copyright (C) 2006 Thiago Macieira <thiago@kde.org>

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

#include "ktoolinvocation.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kstandarddirs.h"
#include "kcomponentdata.h"
#include "kurl.h"
#include "kmessage.h"
#include "kservice.h"
#include "klocale.h"
#include "kglobalsettings.h"

#include <QtCore/QThread>
#include <QtCore/QProcess>
#include <QtCore/QCoreApplication>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnectionInterface>

#define KTOOLINVOCATION_TIMEOUT 250
#define KTOOLINVOCATION_SLEEPTIME 50

// NOTE: keep in sync with:
// kdelibs/kinit/klauncher_adaptor.h
static inline QString getKLauncherError(const int result, const QString &app)
{
    switch (result) {
        case -1: {
            return i18n("Application service is not valid or does not support multiple files: %1.", app);
        }
        case -2: {
            return i18n("Application not found: %1.", app);
        }
        case -3: {
            return i18n("Application could not be processed: %1.", app);
        }
        case -4: {
            return i18n("Application failed to start: %1.", app);
        }
        case -5: {
            return i18n("D-Bus error occured while starting application: %1.", app);
        }
    }
    return i18n("Unknown KLauncher error for application: %1.", app);
}

static inline void printError(const QString &text, QString *error)
{
    if (error)
        *error = text;
    else
        kError() << text;
}

KToolInvocation *KToolInvocation::self()
{
    K_GLOBAL_STATIC(KToolInvocation, s_self)
    return s_self;
}

KToolInvocation::KToolInvocation()
    : QObject(0),
    klauncherIface(nullptr)
{
    klauncherIface = new QDBusInterface(
        QString::fromLatin1("org.kde.klauncher"),
        QString::fromLatin1("/KLauncher"),
        QString::fromLatin1("org.kde.KLauncher"),
        QDBusConnection::sessionBus(),
        this
    );
}

KToolInvocation::~KToolInvocation()
{
    delete klauncherIface;
}

void KToolInvocation::setLaunchEnv(const QString &name, const QString &value)
{
    self()->klauncherIface->asyncCall(QString::fromLatin1("setLaunchEnv"), name, value);
}

int KToolInvocation::startServiceInternal(const char *_function,
                                          const QString &name, const QStringList &URLs,
                                          QString *error,
                                          const QByteArray &startup_id,
                                          const QString &workdir)
{
    QString function = QString::fromLatin1(_function);
    // make sure there is id, so that user timestamp exists
    QStringList envs;
    QByteArray asn = startup_id;
    emit kapplication_hook(envs, asn);

    QDBusPendingReply<int> reply;
    if (qstrcmp(_function, "kdeinit_exec_with_workdir") == 0) {
        reply = klauncherIface->asyncCall(
            function, name, URLs, envs, QString::fromLatin1(asn, asn.size()), workdir
        );
    } else {
        reply = klauncherIface->asyncCall(
            function, name, URLs, envs, QString::fromLatin1(asn, asn.size())
        );
    }
    kDebug() << "Waiting for klauncher call to finish" << function;
    while (!reply.isFinished()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KTOOLINVOCATION_TIMEOUT);
        QThread::msleep(KTOOLINVOCATION_SLEEPTIME);
    }
    kDebug() << "Done waiting for klauncher call to finish" << function;
    if (!reply.isValid()) {
        printError(
            i18n("KLauncher error: %1.", reply.error().message()),
            error
        );
        return EINVAL;
    }

    const int result = reply.value();
    if (result < 0) {
        printError(
            getKLauncherError(result, name),
            error
        );
        // compat
        return -result;
    } else if (result != 0) {
        printError(
            i18n("Application failed to start: %1.", name),
            error
        );
    }
    return result;
}

int KToolInvocation::startServiceByDesktopPath(const QString &name, const QString &URL,
                                               QString *error,
                                               const QByteArray &startup_id)
{
    QStringList URLs;
    if (!URL.isEmpty())
        URLs.append(URL);
    return self()->startServiceInternal("start_service_by_desktop_path",
                                        name, URLs, error, startup_id);
}

int KToolInvocation::startServiceByDesktopPath(const QString &name, const QStringList &URLs,
                                               QString *error,
                                               const QByteArray &startup_id)
{
    return self()->startServiceInternal("start_service_by_desktop_path",
                                        name, URLs, error, startup_id);
}

int KToolInvocation::startServiceByDesktopName(const QString &name, const QString &URL,
                                               QString *error,
                                               const QByteArray &startup_id)
{
    QStringList URLs;
    if (!URL.isEmpty())
        URLs.append(URL);
    return self()->startServiceInternal("start_service_by_desktop_name",
                                        name, URLs, error, startup_id);
}

int KToolInvocation::startServiceByDesktopName(const QString &name, const QStringList &URLs,
                                               QString *error,
                                               const QByteArray &startup_id)
{
    return self()->startServiceInternal("start_service_by_desktop_name",
                                        name, URLs, error, startup_id);
}

int KToolInvocation::kdeinitExec(const QString &name, const QStringList &args,
                                 QString *error,
                                 const QByteArray &startup_id)
{
    return self()->startServiceInternal("kdeinit_exec",
                                        name, args, error, startup_id);
}


int KToolInvocation::kdeinitExecWait(const QString &name, const QStringList &args,
                                     QString *error,
                                     const QByteArray &startup_id)
{
    return self()->startServiceInternal("kdeinit_exec_wait",
                                        name, args, error, startup_id);
}

void KToolInvocation::invokeHelp(const QString &anchor,
                                 const QString &_appname,
                                 const QByteArray &startup_id)
{
    KUrl url;
    QString appname;
    QString docPath;
    if (_appname.isEmpty()) {
        appname = QCoreApplication::instance()->applicationName();
    } else {
        appname = _appname;
    }

    KService::Ptr service(KService::serviceByDesktopName(appname));
    if (service) {
        docPath = service->docPath();
    }

    if (!docPath.isEmpty()) {
        url = KUrl(KUrl(QString::fromLatin1(KDE_HELP_URL)), docPath);
    } else {
        url = QString::fromLatin1(KDE_HELP_URL);
    }

    if (!anchor.isEmpty()) {
        url.addQueryItem(QString::fromLatin1("anchor"), anchor);
    }

    invokeBrowser(url.url());
}

void KToolInvocation::invokeMailer(const QString &address, const QString &subject, const QByteArray &startup_id)
{
    invokeMailer(address, QString(), subject, QString(), QStringList(), startup_id );
}

void KToolInvocation::invokeMailer(const KUrl &mailtoURL, const QByteArray& startup_id, bool allowAttachments)
{
    QString address = mailtoURL.path();
    QString subject;
    QString cc;
    QString body;

    const QStringList queries = mailtoURL.query().mid(1).split(QLatin1Char('&'));
    const QChar comma = QChar::fromLatin1(',');
    QStringList attachURLs;
    for (QStringList::ConstIterator it = queries.begin(); it != queries.end(); ++it)
    {
        QString q = (*it).toLower();
        if (q.startsWith(QLatin1String("subject=")))
            subject = KUrl::fromPercentEncoding((*it).mid(8).toLatin1());
        else
            if (q.startsWith(QLatin1String("cc=")))
                cc = cc.isEmpty()? KUrl::fromPercentEncoding((*it).mid(3).toLatin1()): cc + comma + KUrl::fromPercentEncoding((*it).mid(3).toLatin1());
            else
                if (q.startsWith(QLatin1String("body=")))
                    body = KUrl::fromPercentEncoding((*it).mid(5).toLatin1());
                else
                    if (allowAttachments && q.startsWith(QLatin1String("attach=")))
                        attachURLs.push_back(KUrl::fromPercentEncoding((*it).mid(7).toLatin1()));
                    else
                        if (allowAttachments && q.startsWith(QLatin1String("attachment=")))
                            attachURLs.push_back(KUrl::fromPercentEncoding((*it).mid(11).toLatin1()));
                        else
                            if (q.startsWith(QLatin1String("to=")))
                                address = address.isEmpty()? KUrl::fromPercentEncoding((*it).mid(3).toLatin1()): address + comma + KUrl::fromPercentEncoding((*it).mid(3).toLatin1());
    }

    invokeMailer(address, cc, subject, body, attachURLs, startup_id);
}

#include "moc_ktoolinvocation.cpp"
