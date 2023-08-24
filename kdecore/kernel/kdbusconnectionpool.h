/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KDBUSCONNECTIONPOOL_H
#define KDBUSCONNECTIONPOOL_H

#include <kdecore_export.h>

#include <QtDBus/qdbusconnection.h>

namespace KDBusConnectionPool
{

/**
 * The KDBusConnectionPool works around the problem of QDBusConnection not being thread-safe.
 * @since 4.10.4
 */
KDECORE_EXPORT QDBusConnection threadConnection();

/**
 * Checks if service is registered on the give connection.
 * @since 4.24.0
 */
KDECORE_EXPORT bool isServiceRegistered(const QString &service, const QDBusConnection &connection);

}

#endif

