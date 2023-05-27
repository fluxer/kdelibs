/* This file is part of the KDE libraries
   Copyright (C) 2023 smil3y <xakepa10@gmail.com>

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
#include "kicon.h"
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
    if (d->menu) {
        delete d->menu;
        d->menu = nullptr;
    }
    d->menu = KDBusMenuImporter::createMenu(nullptr);
    QDBusReply<KDBusMenu> menureply = d->interface->call("menu");
    if (!menureply.isValid()) {
        kWarning(s_kdbusmenuarea) << "Invalid menu reply" << menureply.error();
        return;
    }
    const KDBusMenu menuproperties = menureply.value();
    d->menu->setTitle(menuproperties.title);
    QIcon menuicon = KDBusMenuImporter::iconForName(menuproperties.icon);
    if (menuicon.isNull() && !menuproperties.icondata.isEmpty()) {
        QPixmap menupixmap;
        const bool pixmaploaded = menupixmap.loadFromData(menuproperties.icondata, s_kdbusmenuiconformat);
        if (!pixmaploaded) {
            kWarning(s_kdbusmenuarea) << "Could not load menu icon pixmap";
        } else {
            menuicon = QIcon(menupixmap);
        }
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
        if (actionicon.isNull() && !actionproperties.icondata.isEmpty()) {
            QPixmap actionpixmap;
            const bool pixmaploaded = actionpixmap.loadFromData(actionproperties.icondata, s_kdbusmenuiconformat);
            if (!pixmaploaded) {
                kWarning(s_kdbusmenuarea) << "Could not load icon pixmap" << actionid;
            } else {
                actionicon = QIcon(actionpixmap);
            }
        }
        QAction* action = nullptr;
        if (actionproperties.title) {
            // see kdeui/widgets/kmenu.cpp
            action = KMenu::titleAction(actionicon, actionproperties.text, d->menu);
        } else {
            action = new QAction(d->menu);
            action->setText(actionproperties.text);
            action->setIcon(actionicon);
            action->setToolTip(actionproperties.tooltip);
            action->setStatusTip(actionproperties.statustip);
            action->setWhatsThis(actionproperties.whatsthis);
            action->setSeparator(actionproperties.separator);
            action->setCheckable(actionproperties.checkable);
            action->setChecked(actionproperties.checked);
            action->setEnabled(actionproperties.enabled);
            action->setVisible(actionproperties.visible);
            QList<QKeySequence> shortcuts;
            foreach (const QString &keysequence, actionproperties.shortcuts) {
                shortcuts.append(QKeySequence::fromString(keysequence));
            }
            action->setShortcuts(shortcuts);
            if (actionproperties.exclusive) {
                QActionGroup *actiongroup = new QActionGroup(action);
                actiongroup->addAction(action);
            }
        }
        action->setData(QVariant(actionid));
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
