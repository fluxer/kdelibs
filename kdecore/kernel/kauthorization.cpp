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
#include "klocale.h"
#include "klockfile.h"
#include "kglobal.h"
#include "kstandarddirs.h"
#include "kdebug.h"

#include <QDir>
#include <QThread>
#include <QTimer>
#include <QDBusAbstractAdaptor>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <syslog.h>

#define KAUTHORIZATION_TIMEOUT 250

// see kdebug.areas
static const int s_kauthorizationarea = 185;
static const qint64 s_servicetimeout = 30000;

void kAuthMessageHandler(QtMsgType type, const char *msg)
{
    // NOTE: cannot use KDebug because if it triggers a warning the program will dead-lock
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

static QString kGetLockFile(const QString &helper)
{
    const QString lockdir = KGlobal::dirs()->saveLocation("tmp");
    return lockdir + helper;
}

class KAuthorizationAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kauthorization")
public:
    KAuthorizationAdaptor(QObject *parent);

public Q_SLOTS:
    Q_SCRIPTABLE int execute(const QString &method, const QVariantMap &arguments);
    Q_SCRIPTABLE void stop();

private:
    void delayedStop();
};

KAuthorizationAdaptor::KAuthorizationAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
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
        kWarning(s_kauthorizationarea) << "Invalid method" << method;
        delayedStop();
        return KAuthorization::MethodError;
    }
    delayedStop();
    return result;
}

void KAuthorizationAdaptor::stop()
{
    qApp->quit();
}

void KAuthorizationAdaptor::delayedStop()
{
    QTimer::singleShot(100, this, SLOT(stop()));
}


KAuthorization::KAuthorization(const char* const helper, QObject *parent)
    : QObject(parent ? parent : qApp)
{
    Q_ASSERT(helper);
    setObjectName(QString::fromLatin1(helper));
    new KAuthorizationAdaptor(this);
}

KAuthorization::~KAuthorization()
{
    const QString helper = objectName();
    if (!helper.isEmpty()) {
        QDBusConnection::systemBus().unregisterService(helper);
        QDBusConnection::systemBus().unregisterObject(QString::fromLatin1("/KAuthorization"));
    }
}

bool KAuthorization::isAuthorized(const QString &helper)
{
    kDebug() << "Checking if" << helper << "is authorized";
    KLockFile authorizationlock(kGetLockFile(helper));
    authorizationlock.lock();
    QDBusInterface kauthorizationinterface(
        helper, QString::fromLatin1("/KAuthorization"), QString::fromLatin1("org.kde.kauthorization"),
        QDBusConnection::systemBus()
    );
    QDBusReply<void> reply = kauthorizationinterface.call(QString::fromLatin1("stop"));
    bool result = true;
    if (!reply.isValid()) {
        kWarning(s_kauthorizationarea) << reply.error().message();
        result = false;
    }
    kDebug(s_kauthorizationarea) << "Result is" << result;
    return result;
}

int KAuthorization::execute(const QString &helper, const QString &method, const QVariantMap &arguments)
{
    kDebug(s_kauthorizationarea) << "Executing" << helper << "method" << method;
    KLockFile authorizationlock(kGetLockFile(helper));
    authorizationlock.lock();
    QDBusInterface kauthorizationinterface(
        helper, QString::fromLatin1("/KAuthorization"), QString::fromLatin1("org.kde.kauthorization"),
        QDBusConnection::systemBus()
    );
    QDBusReply<int> reply = kauthorizationinterface.call(QString::fromLatin1("execute"), method, arguments);
    int result = KAuthorization::NoError;
    if (!reply.isValid()) {
        result = KAuthorization::DBusError;
        kWarning(s_kauthorizationarea) << reply.error().message();
        if (reply.error().type() == QDBusError::AccessDenied) {
            result = KAuthorization::AuthorizationError;
        }
    } else {
        result = reply.value();
    }
    kDebug(s_kauthorizationarea) << "Result is" << result;
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
    if (!QDBusConnection::systemBus().registerObject(QString::fromLatin1("/KAuthorization"), object)) {
        qApp->exit(2);
        return;
    }

    // in case the process executing the helper crashes
    QTimer::singleShot(s_servicetimeout, qApp, SLOT(quit()));
}

#include "kauthorization.moc"
#include "moc_kauthorization.cpp"
