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

#ifndef SOLID_IFACES_GRAPHIC_H
#define SOLID_IFACES_GRAPHIC_H

#include <solid/ifaces/deviceinterface.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on graphic devices.
     *
     * A graphic device is typically used to render and output data to display.
     */
    class Graphic : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a Graphic object.
         */
        virtual ~Graphic();
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Graphic, "org.kde.Solid.Ifaces.Graphic/0.1")

#endif
