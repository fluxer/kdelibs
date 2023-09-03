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

#ifndef SOLID_IFACES_INPUT_H
#define SOLID_IFACES_INPUT_H

#include <solid/input.h>
#include <solid/ifaces/deviceinterface.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on input devices.
     *
     * A input device is typically used to input data on display.
     */
    class Input : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a Input object.
         */
        virtual ~Input();

        /**
         * Retrieves the driver used by the device.
         *
         * @return the driver name
         */
        virtual QString driver() const = 0;

        /**
         * Retrieves the type of input device this device is.
         *
         * @return the input type
         * @see Solid::Input::InputType
         */
        virtual Solid::Input::InputType inputType() const = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Input, "org.kde.Solid.Ifaces.Input/0.1")

#endif
