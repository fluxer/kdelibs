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

#include "kauthorization.h"

#include <QThread>
#include <QTimer>
#include <QDBusAbstractAdaptor>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <klocale.h>
#include <kdebug.h>

#include <syslog.h>

#define KAUTHORIZATION_TIMEOUT 150
#define KAUTHORIZATION_SLEEPTIME 150

void kAuthMessageHandler(QtMsgType type, const char *msg)
{
    switch (type) {
        case QtDebugMsg: {
            ::syslog(LOG_DEBUG, "%s", msg);
            break;
        }
        case QtWarningMsg: {
            ::syslog(LOG_WARNING, "%s", msg);
            break;
        }
        case QtCriticalMsg: {
            ::syslog(LOG_CRIT, "%s", msg);
            break;
        }
        case QtFatalMsg: {
            ::syslog(LOG_ERR, "%s", msg);
            break;
        }
    }
}

static bool isDBusServiceRegistered(const QString &helper)
{
    QDBusConnectionInterface* dbusinterface = QDBusConnection::systemBus().interface();
    if (!dbusinterface) {
        kDebug() << "Null D-Bus interface" << helper;
        return false;
    }
    QDBusReply<bool> reply = dbusinterface->isServiceRegistered(helper);
    if (reply.value() == false) {
        kDebug() << "Service not registered" << helper;
        return false;
    }
    return true;
}

static void killDBusService(const QString &helper, QDBusInterface *kauthorizationinterface)
{
    if (!isDBusServiceRegistered(helper)) {
        return;
    }

    QDBusReply<void> reply = kauthorizationinterface->call(QString::fromLatin1("stop"));
    if (!reply.isValid()) {
        kWarning() << reply.error().message();
    }
}

class KAuthorizationAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kauthorization")
public:
    KAuthorizationAdaptor(QObject *parent);

public Q_SLOTS:
    Q_SCRIPTABLE bool ping();
    Q_SCRIPTABLE int execute(const QString &method, const QVariantMap &arguments);
    Q_SCRIPTABLE void stop();
};

KAuthorizationAdaptor::KAuthorizationAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

bool KAuthorizationAdaptor::ping()
{
    return true;
}

int KAuthorizationAdaptor::execute(const QString &method, const QVariantMap &arguments)
{
    const QByteArray bytemethod = method.toLatin1();
    int result = KAuthorization::HelperError;
    const bool success = QMetaObject::invokeMethod(
        parent(), bytemethod.constData(), Qt::DirectConnection,
        Q_RETURN_ARG(int, result), Q_ARG(QVariantMap, arguments)
    );
    if (!success) {
        kWarning() << "Invalid method" << method;
        return KAuthorization::MethodError;
    }
    return result;
}

void KAuthorizationAdaptor::stop()
{
    qApp->quit();
}


KAuthorization::KAuthorization(QObject *parent)
    : QObject(parent)
{
    new KAuthorizationAdaptor(this);
}

bool KAuthorization::isAuthorized(const QString &helper)
{
    QDBusInterface kauthorizationinterface(
        helper, QString::fromLatin1("/"), QString::fromLatin1("org.kde.kauthorization"),
        QDBusConnection::systemBus()
    );
    QDBusReply<bool> reply = kauthorizationinterface.call(QString::fromLatin1("ping"));
    if (!reply.isValid()) {
        kWarning() << reply.error().message();
        killDBusService(helper, &kauthorizationinterface);
        return false;
    }
    const bool result = reply.value();
    killDBusService(helper, &kauthorizationinterface);
    return result;
}

int KAuthorization::execute(const QString &helper, const QString &method, const QVariantMap &arguments)
{
    if (!KAuthorization::isAuthorized(helper)) {
        return KAuthorization::AuthorizationError;
    }

    while (isDBusServiceRegistered(helper)) {
        kDebug() << "Waiting for service to unregister" << helper;
        QCoreApplication::processEvents(QEventLoop::AllEvents, KAUTHORIZATION_TIMEOUT);
        QThread::msleep(KAUTHORIZATION_SLEEPTIME);
    }

    QDBusInterface kauthorizationinterface(
        helper, QString::fromLatin1("/"), QString::fromLatin1("org.kde.kauthorization"),
        QDBusConnection::systemBus()
    );
    QDBusReply<int> reply = kauthorizationinterface.call(QString::fromLatin1("execute"), method, arguments);
    int result = KAuthorization::DBusError;
    if (!reply.isValid()) {
        kWarning() << reply.error().message();
    } else {
        result = reply.value();
    }
    killDBusService(helper, &kauthorizationinterface);
    kDebug() << "Result" << helper << method << result;
    return result;
}

QString KAuthorization::errorString(const int status)
{
    switch (status) {
        case KAuthorization::NoError: {
            return i18n("No error");
        }
        case KAuthorization::HelperError: {
            return i18n("Helper error");
        }
        case KAuthorization::DBusError: {
            return i18n("D-Bus error");
        }
        case KAuthorization::MethodError: {
            return i18n("Method error");
        }
        case KAuthorization::AuthorizationError: {
            return i18n("Authorization error");
        }
    }
    if (status > KAuthorization::NoError) {
        return i18n("Custom error");
    }
    return i18n("Unknown error");
}

void KAuthorization::helperMain(const char* const helper, KAuthorization *object)
{
    ::openlog(helper, 0, LOG_USER);

    qInstallMsgHandler(kAuthMessageHandler);

    if (!QDBusConnection::systemBus().registerService(QString::fromLatin1(helper))) {
        qApp->exit(1);
        return;
    }
    if (!QDBusConnection::systemBus().registerObject(QString::fromLatin1("/"), object)) {
        qApp->exit(2);
        return;
    }

    // in case the process executing the helper crashes
    QTimer::singleShot(30000, qApp, SLOT(quit()));
}

#include "kauthorization.moc"
#include "moc_kauthorization.cpp"
