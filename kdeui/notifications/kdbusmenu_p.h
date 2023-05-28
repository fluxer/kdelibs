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

#ifndef KDBUSMENU_H
#define KDBUSMENU_H

#include "kiconloader.h"
#include "kdebug.h"

#include <QString>
#include <QStringList>
#include <QDBusArgument>
#include <QAction>
#include <QMenu>
#include <QImageWriter>
#include <QBuffer>
#include <QPixmap>

static const int s_kdbusmenuiconsize = KIconLoader::SizeSmall;
static const QByteArray s_kdbusmenuiconformat = QImageWriter::defaultImageFormat();
// see kdebug.areas
static const int s_kdbusmenuarea = 301;

struct KDBusMenu
{
    QString icon;
    QString title;
    QByteArray icondata;
};
Q_DECLARE_METATYPE(KDBusMenu);

QDBusArgument& operator<<(QDBusArgument &argument, const KDBusMenu &kdbusmenu);
const QDBusArgument& operator>>(const QDBusArgument &argument, KDBusMenu &kdbusmenu);

struct KDBusMenuAction
{
    quint64 id;
    QString text;
    QString icon;
    QString tooltip;
    QString statustip;
    QString whatsthis;
    bool separator;
    bool checkable;
    bool checked;
    bool enabled;
    bool visible;
    QStringList shortcuts;
    QByteArray icondata;
    bool title;
    bool exclusive;
    bool submenu;
};
Q_DECLARE_METATYPE(KDBusMenuAction);
Q_DECLARE_METATYPE(QList<KDBusMenuAction>);

QDBusArgument& operator<<(QDBusArgument &argument, const KDBusMenuAction &kdbusmenuaction);
const QDBusArgument& operator>>(const QDBusArgument &argument, KDBusMenuAction &kdbusmenuaction);

static quint64 kDBusMenuActionID(const QAction *action)
{
    return quint64(quintptr(action));
}

static QByteArray kDBusMenuIconData(const QIcon &actionicon)
{
    QBuffer iconbuffer;
    QPixmap iconpixmap = actionicon.pixmap(s_kdbusmenuiconsize);
    if (iconpixmap.isNull()) {
        // it is common for actions to not have a icon
        kDebug(s_kdbusmenuarea) << "Action/menu icon name is empty and icon pixmap is null";
        return QByteArray();
    }
    const bool iconsaved = iconpixmap.save(&iconbuffer, s_kdbusmenuiconformat);
    if (!iconsaved) {
        kWarning(s_kdbusmenuarea) << "Could not save action/menu icon pixmap";
        return QByteArray();
    }
    return iconbuffer.data();
}

static QIcon kDBusMenuIcon(const QByteArray &actionicondata)
{
    if (actionicondata.isEmpty()) {
        // see kDBusMenuIconData()
        return QIcon();
    }
    QPixmap actionpixmap;
    const bool pixmaploaded = actionpixmap.loadFromData(actionicondata, s_kdbusmenuiconformat);
    if (!pixmaploaded) {
        kWarning(s_kdbusmenuarea) << "Could not load action/menu icon pixmap";
        return QIcon();
    }
    return QIcon(actionpixmap);
}

static QAction* kDBusMenuAction(QMenu *menu, const KDBusMenuAction &actionproperties)
{
    QAction* action = new QAction(menu);
    action->setText(actionproperties.text);
    action->setToolTip(actionproperties.tooltip);
    action->setStatusTip(actionproperties.statustip);
    action->setWhatsThis(actionproperties.whatsthis);
    action->setSeparator(actionproperties.separator);
    action->setCheckable(actionproperties.checkable);
    action->setChecked(actionproperties.checked);
    action->setEnabled(actionproperties.enabled);
    action->setVisible(actionproperties.visible);
    QList<QKeySequence> shortcuts;
    shortcuts.reserve(actionproperties.shortcuts.size());
    foreach (const QString &keysequence, actionproperties.shortcuts) {
        shortcuts.append(QKeySequence::fromString(keysequence));
    }
    action->setShortcuts(shortcuts);
    if (actionproperties.exclusive) {
        QActionGroup *actiongroup = new QActionGroup(action);
        actiongroup->addAction(action);
    }
    action->setData(QVariant(actionproperties.id));
    return action;
}

#endif // KDBUSMENU_H
