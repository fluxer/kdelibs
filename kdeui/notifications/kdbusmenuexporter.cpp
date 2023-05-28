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

#include "kdbusmenuexporter.h"
#include "kdebug.h"
#include "kdbusmenu_p.h"

#include <QDBusAbstractAdaptor>
#include <QPointer>
#include <QBuffer>
#include <QToolButton>
#include <QWidgetAction>
#include <qdbusmetatype.h>

class KDBusMenuAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.DBusMenu")
public:
    KDBusMenuAdaptor(KDBusMenuExporter *exporter, QMenu* menu);

public Q_SLOTS:
    QString status() const;
    void setStatus(const QString &status);

    KDBusMenu menu() const;

    QList<KDBusMenuAction> actions() const;
    bool triggerAction(quint64 actionid) const;

private:
    KDBusMenuExporter* m_exporter;
    QString m_status;
    QPointer<QMenu> m_menu;
};

KDBusMenuAdaptor::KDBusMenuAdaptor(KDBusMenuExporter *exporter, QMenu* menu)
    : QDBusAbstractAdaptor(exporter),
    m_exporter(exporter),
    m_status(QString::fromLatin1("normal")),
    m_menu(menu)
{
    qDBusRegisterMetaType<KDBusMenu>();
    qDBusRegisterMetaType<KDBusMenuAction>();
    qDBusRegisterMetaType<QList<KDBusMenuAction>>();
}

QString KDBusMenuAdaptor::status() const
{
    return m_status;
}

void KDBusMenuAdaptor::setStatus(const QString &status)
{
    kDebug(s_kdbusmenuarea) << "Changing status to" << status;
    if (status != QLatin1String("normal") && status != QLatin1String("notice")) {
        kWarning(s_kdbusmenuarea) << "Invalid status" << status;
    }
    m_status = status;
}

KDBusMenu KDBusMenuAdaptor::menu() const
{
    KDBusMenu result;
    if (m_menu.isNull()) {
        kWarning(s_kdbusmenuarea) << "Menu is null";
        return result;
    }
    // NOTE: KStatusNotifierItem adds actions to the menu when aboutToShow is emitted
    const bool menusignaled = QMetaObject::invokeMethod(m_menu.data(), "aboutToShow", Qt::DirectConnection);
    if (!menusignaled) {
        kWarning(s_kdbusmenuarea) << "Could not invoke menu aboutToShow() signal";
    }
    const QIcon menuicon = m_menu->icon();
    result.icon = menuicon.name();
    result.title = m_menu->title();
    if (result.icon.isEmpty()) {
        QBuffer iconbuffer;
        QPixmap iconpixmap = menuicon.pixmap(s_kdbusmenuiconsize);
        if (iconpixmap.isNull()) {
            kWarning(s_kdbusmenuarea) << "Menu icon name is empty and icon pixmap is null";
        } else {
            const bool savedicon = iconpixmap.save(&iconbuffer, s_kdbusmenuiconformat);
            if (!savedicon) {
                kWarning(s_kdbusmenuarea) << "Could not save menu icon pixmap";
            } else {
                result.icondata = iconbuffer.data();
            }
        }
    }
    return result;
}

QList<KDBusMenuAction> KDBusMenuAdaptor::actions() const
{
    QList<KDBusMenuAction> result;
    if (m_menu.isNull()) {
        kWarning(s_kdbusmenuarea) << "Menu is null";
        return result;
    }
    foreach (QAction* action, m_menu->actions()) {
        const quint64 actionid = kDBusMenuActionID(action);
        kDebug(s_kdbusmenuarea) << "Exporting action" << actionid;
        const QMenu* actionmenu = action->menu();
        if (actionmenu) {
            kWarning(s_kdbusmenuarea) << "Menu actions are not supported";
            continue;
        }
        KDBusMenuAction actionproperties;
        // see kdeui/widgets/kmenu.cpp
        actionproperties.title = (action->objectName() == QLatin1String("kmenu_title"));
        if (actionproperties.title) {
            const QWidgetAction *actionwidget = qobject_cast<QWidgetAction*>(action);
            if (!actionwidget) {
                kWarning(s_kdbusmenuarea) << "Title has no widget" << actionid;
            } else {
                const QToolButton *actionbutton = qobject_cast<QToolButton*>(actionwidget->defaultWidget());
                if (!actionbutton) {
                    kWarning(s_kdbusmenuarea) << "Title has no button" << actionid;
                } else {
                    action = actionbutton->defaultAction();
                }
            }
        }
        actionproperties.id = actionid;
        actionproperties.text = action->text();
        actionproperties.icon = m_exporter->iconNameForAction(action);
        actionproperties.tooltip = action->toolTip();
        actionproperties.statustip = action->statusTip();
        actionproperties.whatsthis = action->whatsThis();
        actionproperties.separator = action->isSeparator();
        actionproperties.checkable = action->isCheckable();
        actionproperties.checked = action->isChecked();
        actionproperties.enabled = action->isEnabled();
        actionproperties.visible = action->isVisible();
        QStringList shortcuts;
        foreach (const QKeySequence &keysequence, action->shortcuts()) {
            shortcuts.append(keysequence.toString());
        }
        actionproperties.shortcuts = shortcuts;
        if (actionproperties.icon.isEmpty()) {
            QBuffer iconbuffer;
            QPixmap iconpixmap = action->icon().pixmap(s_kdbusmenuiconsize);
            if (iconpixmap.isNull()) {
                kWarning(s_kdbusmenuarea) << "Icon name is empty and icon pixmap is null" << actionid;
            } else {
                const bool savedicon = iconpixmap.save(&iconbuffer, s_kdbusmenuiconformat);
                if (!savedicon) {
                    kWarning(s_kdbusmenuarea) << "Could not save icon pixmap" << actionid;
                } else {
                    actionproperties.icondata = iconbuffer.data();
                }
            }
        }
        const QActionGroup* actiongroup = action->actionGroup();
        actionproperties.exclusive = (actiongroup && actiongroup->isExclusive());
        result.append(actionproperties);
    }
    return result;
}

bool KDBusMenuAdaptor::triggerAction(quint64 actionid) const
{
    kDebug(s_kdbusmenuarea) << "Triggering action" << actionid;
    if (m_menu.isNull()) {
        kWarning(s_kdbusmenuarea) << "Menu is null";
        return false;
    }
    foreach (QAction* action, m_menu->actions()) {
        const quint64 itactionid = kDBusMenuActionID(action);
        if (itactionid == actionid) {
            return QMetaObject::invokeMethod(action, "triggered");
        }
    }
    kWarning(s_kdbusmenuarea) << "Could not find action for" << actionid;
    return false;
}


class KDBusMenuExporterPrivate
{
public:
    KDBusMenuExporterPrivate();

    KDBusMenuAdaptor* adaptor;
    QString objectpath;
    QString connectionname;
};

KDBusMenuExporterPrivate::KDBusMenuExporterPrivate()
    : adaptor(nullptr)
{
}


KDBusMenuExporter::KDBusMenuExporter(const QString &objectpath, QMenu *menu, const QDBusConnection &connection)
    : QObject(menu),
    d(new KDBusMenuExporterPrivate())
{
    kDebug(s_kdbusmenuarea) << "Exporting menu" << objectpath;

    d->adaptor = new KDBusMenuAdaptor(this, menu);
    d->objectpath = objectpath;
    d->connectionname = connection.name();

    QDBusConnection dbusconnection(connection.name());
    dbusconnection.registerObject(objectpath, this);
}

KDBusMenuExporter::~KDBusMenuExporter()
{
    QDBusConnection dbusconnection(d->connectionname);
    dbusconnection.unregisterObject(d->objectpath);
    delete d;
}

QString KDBusMenuExporter::status() const
{
    return d->adaptor->status();
}

void KDBusMenuExporter::setStatus(const QString &status)
{
    d->adaptor->setStatus(status);
}

void KDBusMenuExporter::activateAction(QAction *action)
{
    const quint64 actionid = kDBusMenuActionID(action);
    d->adaptor->triggerAction(actionid);
}

QString KDBusMenuExporter::iconNameForAction(QAction *action)
{
    if (!action) {
        return QString();
    }
    const QIcon icon = action->icon();
    if (action->isIconVisibleInMenu() && !icon.isNull()) {
        return icon.name();
    }
    return QString();
}

#include "kdbusmenuexporter.moc"
