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

#include "udevinput.h"

using namespace Solid::Backends::UDev;

Input::Input(UDevDevice *device)
    : DeviceInterface(device)
{
}

Input::~Input()
{
}

QString Input::driver() const
{
    const QString idusbdriver = m_device->deviceProperty("ID_USB_DRIVER");
    if (!idusbdriver.isEmpty()) {
        return idusbdriver;
    }
    return m_device->udevDevice().driver();
}

Solid::Input::InputType Input::inputType() const
{
    const bool ismouse = (m_device->deviceProperty("ID_INPUT_MOUSE").toInt() == 1);
    if (ismouse) {
        return Solid::Input::Mouse;
    }
    const bool iskeyboard = (m_device->deviceProperty("ID_INPUT_KEYBOARD").toInt() == 1);
    if (iskeyboard) {
        return Solid::Input::Keyboard;
    }
    const bool isjoystick = (m_device->deviceProperty("ID_INPUT_JOYSTICK").toInt() == 1);
    if (isjoystick) {
        return Solid::Input::Joystick;
    }
    return Solid::Input::UnknownInput;
}

#include "moc_udevinput.cpp"
