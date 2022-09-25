/*
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

#include "BackendsManager.h"

// Include dbus backends
#include "backends/dbus/DBusBackend.h"
#include "backends/dbus/DBusHelperProxy.h"

#include <kdebug.h>

namespace KAuth
{

AuthBackend *BackendsManager::auth = 0;
HelperProxy *BackendsManager::helper = 0;

BackendsManager::BackendsManager()
{
}

void BackendsManager::init()
{
    if (!auth) {
        auth = new DBusBackend();
    }

    if (!helper) {
        helper = new DBusHelperProxy();
    }
}

AuthBackend *BackendsManager::authBackend()
{
    if (!auth) {
        init();
    }

    return auth;
}

HelperProxy *BackendsManager::helperProxy()
{
    if (!helper) {
        init();
    }

    return helper;
}

} // namespace Auth
