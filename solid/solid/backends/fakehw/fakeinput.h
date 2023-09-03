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

#ifndef SOLID_BACKENDS_FAKEHW_FAKEINPUT_H
#define SOLID_BACKENDS_FAKEHW_FAKEINPUT_H

#include "fakedeviceinterface.h"
#include <solid/ifaces/input.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeInput : public FakeDeviceInterface, virtual public Solid::Ifaces::Input
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Input)

public:
    explicit FakeInput(FakeDevice *device);
    ~FakeInput();

public Q_SLOTS:
    virtual QString driver() const;
    virtual Solid::Input::InputType inputType() const;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEGRAPHIC_H
