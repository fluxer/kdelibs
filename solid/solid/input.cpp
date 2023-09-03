/*
    Copyright 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "input.h"
#include "input_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/input.h>

Solid::Input::Input(QObject *backendObject)
    : DeviceInterface(*new InputPrivate(), backendObject)
{
}

Solid::Input::~Input()
{
}

QString Solid::Input::driver() const
{
    Q_D(const Input);
    return_SOLID_CALL(Ifaces::Input *, d->backendObject(), QString(), driver());
}

Solid::Input::InputType Solid::Input::inputType() const
{
    Q_D(const Input);
    return_SOLID_CALL(Ifaces::Input *, d->backendObject(), Solid::Input::UnknownInput, inputType());
}

#include "moc_input.cpp"
