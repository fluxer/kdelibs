/*
    Copyright 2021 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef SOLID_GRAPHIC_H
#define SOLID_GRAPHIC_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class GraphicPrivate;
    class Device;

    /**
     * This device interface is available on graphic devices.
     *
     * A graphic device is typically used to render and output data to display.
     */
    class SOLID_EXPORT Graphic : public DeviceInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(Graphic)
        friend class Device;

    private:
        /**
         * Creates a new Graphic object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Graphic(QObject *backendObject);

    public:
        /**
         * Destroys a Graphic object.
         */
        virtual ~Graphic();


        /**
         * Get the Solid::DeviceInterface::Type of the Graphic device interface.
         *
         * @return the Graphic device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Graphic; }
    };
}

#endif
