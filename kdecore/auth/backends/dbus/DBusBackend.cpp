/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#include "DBusBackend.h"

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusPendingCall>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>

namespace KAuth
{

DBusBackend::DBusBackend()
    : AuthBackend()
{
}

Action::AuthStatus DBusBackend::authorizeAction(const QString &action)
{
    return actionStatus(action);
}

Action::AuthStatus DBusBackend::actionStatus(const QString &action)
{
    if (isCallerAuthorized(action)) {
        return Action::Authorized;
    } else {
        return Action::Denied;
    }
}

bool DBusBackend::isCallerAuthorized(const QString &action)
{
    QDBusMessage message;

    QRegExp rx(QLatin1String("(\\S+\\.\\S+\\.\\S+\\.\\S+)\\."));
    int pos = rx.indexIn(action);
    Q_UNUSED(pos);

    message = QDBusMessage::createMethodCall(rx.capturedTexts()[1],
                                             QLatin1String("/"),
                                             QLatin1String("org.freedesktop.DBus.Introspectable"),
                                             QLatin1String("Introspect"));

    QDBusPendingCall reply = QDBusConnection::systemBus().asyncCall(message);
    reply.waitForFinished();
    if (reply.reply().type() == QDBusMessage::ErrorMessage) {
        return false;
    }

    return true;
}

} // namespace Auth
