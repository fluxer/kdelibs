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

#ifndef KDBUSMENUEXPORTER_H
#define KDBUSMENUEXPORTER_H

#include "kdeui_export.h"

#include <QDBusConnection>
#include <QMenu>
#include <QAction>

class KDBusMenuExporterPrivate;

/*!
    D-Bus menu exporting class

    KDBusMenuExporter class can be used to export menu (QMenu or KMenu) from one application
    to other applications.

    @since 4.23
    @see KDBusMenuImporter
*/
class KDEUI_EXPORT KDBusMenuExporter : public QObject
{
    Q_OBJECT
public:
    /*!
        @brief Creates menu exporter for the given object path @p objectpath, path @p path and
        connection @p connection.
        @note The exporter is parented to the menu @p menu thus when the menu is destroyed the
        exporter becomes non-operational and should not be used.
    */
    KDBusMenuExporter(const QString &objectpath, QMenu *menu, const QDBusConnection &connection = QDBusConnection::sessionBus());
    virtual ~KDBusMenuExporter();

    /*!
        @brief Returns the exported menu status, usually "normal" or "notice".
        @note It may be other string, including empty, but that should not be considered valid.
    */
    QString status() const;
    /*!
        @brief Sets the exported menu status to @p status.
    */
    void setStatus(const QString &status);

    /*!
        @brief Activates (triggers) the action. If the action is invalid nothing happens.
    */
    void activateAction(QAction *action);

protected:
    /*!
        @brief Reimplement to provide custom icons for actions and menus. The default
        implementation returns the action icon name.
    */
    virtual QString iconNameForAction(QAction *action);

private:
    KDBusMenuExporterPrivate *d;
    Q_DISABLE_COPY(KDBusMenuExporter);

    friend class KDBusMenuAdaptor;
};

#endif // KDBUSMENUEXPORTER_H
