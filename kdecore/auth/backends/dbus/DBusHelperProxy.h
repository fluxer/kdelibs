/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Dario Freddi <drf@kde.org>
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

#ifndef DBUS_HELPER_PROXY_H
#define DBUS_HELPER_PROXY_H

#include "HelperProxy.h"
#include "kauthactionreply.h"

namespace KAuth
{

class DBusHelperProxy : public HelperProxy
{
    Q_OBJECT
    Q_INTERFACES(KAuth::HelperProxy)

    QObject *responder;
    QString m_name;
    QString m_currentAction;
    bool m_stopRequest;
    QList<QString> m_actionsInProgress;

    enum SignalType {
        ActionStarted, // The blob argument is empty
        ActionPerformed, // The blob argument contains the ActionReply
        DebugMessage, // The blob argument contains the debug level and the message (in this order)
        ProgressStepIndicator, // The blob argument contains the step indicator
        ProgressStepData    // The blob argument contains the QVariantMap
    };

public:
    DBusHelperProxy() : responder(0), m_stopRequest(false) {}

    bool executeActions(const QList<QPair<QString, QVariantMap> > &list, const QString &helperID) final;
    ActionReply executeAction(const QString &action, const QString &helperID, const QVariantMap &arguments) final;
    Action::AuthStatus authorizeAction(const QString& action, const QString& helperID) final;
    void stopAction(const QString &action, const QString &helperID) final;

    bool initHelper(const QString &name) final;
    void setHelperResponder(QObject *o) final;
    bool hasToStopAction() final;
    void sendDebugMessage(int level, const char *msg) final;
    void sendProgressStep(int step) final;
    void sendProgressStep(const QVariantMap &data) final;

public slots:
    void stopAction(const QString &action);
    void performActions(QByteArray blob);
    QByteArray performAction(const QString &action, QByteArray arguments);
    uint authorizeAction(const QString &action);

signals:
    void remoteSignal(int type, const QString &action, const QByteArray &blob); // This signal is sent from the helper to the app

private slots:
    void remoteSignalReceived(int type, const QString &action, QByteArray blob);
};

} // namespace Auth

#endif
