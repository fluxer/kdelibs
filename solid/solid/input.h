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

#ifndef SOLID_INPUT_H
#define SOLID_INPUT_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class InputPrivate;
    class Device;

    /**
     * This device interface is available on graphic devices.
     *
     * A graphic device is typically used to render and output data to display.
     */
    class SOLID_EXPORT Input : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(InputType)
        Q_PROPERTY(QString driver READ driver)
        Q_PROPERTY(InputType inputType READ inputType)
        Q_DECLARE_PRIVATE(Input)
        friend class Device;

    private:
        /**
         * Creates a new Input object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Input(QObject *backendObject);

    public:
        /**
         * This enum type defines the type of drive a storage device can be.
         *
         * - UnknownInput : An unknown input type
         * - Mouse : A mouse
         * - Keyboard : A keyboard
         * - Joystick : A joystick
         */
        enum InputType { UnknownInput, Mouse, Keyboard, Joystick };

        /**
         * Destroys a Input object.
         */
        virtual ~Input();

        /**
         * Get the Solid::DeviceInterface::Type of the Input device interface.
         *
         * @return the Input device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Input; }

        /**
         * Retrieves the driver used by the device.
         *
         * @return the driver name
         */
        QString driver() const;

        /**
         * Retrieves the type of input device this device is.
         *
         * @return the input type
         * @see Solid::Input::InputType
         */
        Solid::Input::InputType inputType() const;
    };
}

#endif
