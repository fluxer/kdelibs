/*
    Copyright 2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "solidnamespace.h"
#include "kglobal.h"
#include "klocale.h"

static const KCatalogLoader loader("solid_qt");

static int registerSolidMetaTypes()
{
    qRegisterMetaType<Solid::ErrorType>();
    return 0; // something
}
Q_CONSTRUCTOR_FUNCTION(registerSolidMetaTypes)

QString Solid::errorString(const ErrorType error)
{
    switch (error) {
        case Solid::NoError:
            return QString();
        case Solid::UnauthorizedOperation:
            return i18n("Unauthorized operation");
        case Solid::DeviceBusy:
            return i18n("Device is busy");
        case Solid::OperationFailed:
            return i18n("Operation failed");
        case Solid::UserCanceled:
            return i18n("Canceled by user");
        case Solid::InvalidOption:
            return i18n("Invalid option");
        case Solid::MissingDriver:
            return i18n("Missing driver");
        case Solid::Insecure:
            return i18n("The filesystem type has been deemed insecure");
    }
    return QString();
}
