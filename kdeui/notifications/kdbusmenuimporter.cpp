/* This file is part of the KDE libraries
   Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "kdbusmenuimporter.h"
#include "kmenu.h"
#include "kdebug.h"
#include "kdbusmenu_p.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <qdbusmetatype.h>

class KDBusMenuImporterPrivate
{
public:
    KDBusMenuImporterPrivate();

    QDBusInterface* interface;
    QMenu* menu;
};

KDBusMenuImporterPrivate::KDBusMenuImporterPrivate()
    : interface(nullptr),
    menu(nullptr)
{
}


KDBusMenuImporter::KDBusMenuImporter(const QString &service, const QString &path, QObject *parent)
    : QObject(parent),
    d(new KDBusMenuImporterPrivate())
{
    kDebug(s_kdbusmenuarea) << "Importing menu" << service << path;

    qDBusRegisterMetaType<KDBusMenu>();
    qDBusRegisterMetaType<KDBusMenuAction>();
    qDBusRegisterMetaType<QList<KDBusMenuAction>>();

    QDBusConnection dbusconnection = QDBusConnection::sessionBus();
    d->interface = new QDBusInterface(service, path, QString::fromLatin1("org.kde.DBusMenu"), dbusconnection, this);
}

KDBusMenuImporter::~KDBusMenuImporter()
{
    delete d;
}

QMenu* KDBusMenuImporter::menu() const
{
    return d->menu;
}

void KDBusMenuImporter::updateMenu()
{
    kDebug(s_kdbusmenuarea) << "Updating menu";
    if (!d->menu) {
        d->menu = KDBusMenuImporter::createMenu(nullptr);
    } else {
        d->menu->clear();
    }
    QDBusReply<KDBusMenu> menureply = d->interface->call("menu");
    if (!menureply.isValid()) {
        kWarning(s_kdbusmenuarea) << "Invalid menu reply" << menureply.error();
        return;
    }
    const KDBusMenu menuproperties = menureply.value();
    d->menu->setTitle(menuproperties.title);
    QIcon menuicon = KDBusMenuImporter::iconForName(menuproperties.icon);
    if (menuicon.isNull()) {
        menuicon = kDBusMenuIcon(menuproperties.icondata);
    }
    d->menu->setIcon(menuicon);
    
    QDBusReply<QList<KDBusMenuAction>> actionsreply = d->interface->call("actions");
    if (!actionsreply.isValid()) {
        kWarning(s_kdbusmenuarea) << "Invalid actions reply" << actionsreply.error();
        return;
    }
    foreach (const KDBusMenuAction &actionproperties, actionsreply.value()) {
        const quint64 actionid = actionproperties.id;
        kDebug(s_kdbusmenuarea) << "Importing action" << actionid;
        QIcon actionicon = KDBusMenuImporter::iconForName(actionproperties.icon);
        if (actionicon.isNull()) {
            actionicon = kDBusMenuIcon(actionproperties.icondata);
        }
        QAction* action = nullptr;
        if (actionproperties.title) {
            // see kdeui/widgets/kmenu.cpp
            action = KMenu::titleAction(actionicon, actionproperties.text, d->menu);
        } else {
            action = kDBusMenuAction(d->menu, actionproperties);
        }
        if (actionproperties.submenu) {
            QDBusReply<QList<KDBusMenuAction>> subactionsreply = d->interface->call("actions", actionid);
            if (!subactionsreply.isValid()) {
                kWarning(s_kdbusmenuarea) << "Invalid sub-actions reply" << subactionsreply.error();
                return;
            }
            QMenu* subactionmenu = KDBusMenuImporter::createMenu(d->menu);
            foreach (const KDBusMenuAction &subactionproperties, subactionsreply.value()) {
                QIcon subactionicon = KDBusMenuImporter::iconForName(subactionproperties.icon);
                if (subactionicon.isNull()) {
                    subactionicon = kDBusMenuIcon(subactionproperties.icondata);
                }
                QAction* subaction = kDBusMenuAction(subactionmenu, subactionproperties);
                subaction->setIcon(subactionicon);
                connect(subaction, SIGNAL(triggered()), this, SLOT(slotActionTriggered()));
                subactionmenu->addAction(subaction);
            }
            action->setMenu(subactionmenu);
        }
        action->setIcon(actionicon);
        connect(action, SIGNAL(triggered()), this, SLOT(slotActionTriggered()));
        d->menu->addAction(action);
    }
    emit menuUpdated();
}

QMenu* KDBusMenuImporter::createMenu(QWidget *parent)
{
    return new QMenu(parent);
}

QIcon KDBusMenuImporter::iconForName(const QString &name)
{
    return QIcon::fromTheme(name);
}

void KDBusMenuImporter::slotActionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    const quint64 actionid = action->data().toULongLong();
    kDebug(s_kdbusmenuarea) << "Action triggered" << actionid;
    QDBusReply<bool> reply = d->interface->call("triggerAction", actionid);
    if (!reply.isValid()) {
        kWarning(s_kdbusmenuarea) << "Invalid action trigger reply" << reply.error();
        return;
    }
    if (reply.value() != true) {
        kWarning(s_kdbusmenuarea) << "Could not trigger action" << actionid;
        return;
    }
    emit actionActivationRequested(action);
}
