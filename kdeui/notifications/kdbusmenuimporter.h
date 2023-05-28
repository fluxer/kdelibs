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

#ifndef KDBUSMENUIMPORTER_H
#define KDBUSMENUIMPORTER_H

#include "kdeui_export.h"

#include <QMenu>
#include <QIcon>
#include <QWidget>
#include <QAction>

class KDBusMenuImporterPrivate;

/*!
    D-Bus menu importing class

    KDBusMenuImporter class can be used to import menu (QMenu or KMenu) from other application
    exported via KDBusMenuExporter.

    @since 4.23
    @see KDBusMenuExporter
*/
class KDEUI_EXPORT KDBusMenuImporter : public QObject
{
    Q_OBJECT
public:
    /*!
        @brief Creates menu importer for the given service @p service and path @p path.
    */
    KDBusMenuImporter(const QString &service, const QString &path, QObject *parent = 0);
    virtual ~KDBusMenuImporter();

    /*!
        @brief Returns nearly perfect copy of the exported menu, if any.
    */
    QMenu* menu() const;

public Q_SLOTS:
    /*!
        @brief Forces update of the menu, must be called for @p KDBusMenuImporter::menu() to return
        valid menu.
    */
    void updateMenu();

Q_SIGNALS:
    /*!
        @brief Signals that the menu was updated.
    */
    void menuUpdated();
    /*!
        @brief Signals that menu action @p action was activated.
    */
    void actionActivationRequested(QAction *action);

protected:
    /*!
        @brief Reimplement to provide custom menus, affects the root menu and sub-menus. The
        default implementation simply creates QMenu parented to @p parent.
    */
    virtual QMenu* createMenu(QWidget *parent);
    /*!
        @brief Reimplement to provide custom icons for actions and menus. The default
        implementation returns the result of QIcon::fromTheme() for @p name.
    */
    virtual QIcon iconForName(const QString &name);

private Q_SLOTS:
    void slotActionTriggered();

private:
    KDBusMenuImporterPrivate *d;
    Q_DISABLE_COPY(KDBusMenuImporter);
};

#endif // KDBUSMENUIMPORTER_H
