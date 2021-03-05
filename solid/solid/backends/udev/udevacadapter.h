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

#ifndef UDEVACADAPTER_H
#define UDEVACADAPTER_H

#include <solid/ifaces/acadapter.h>
#include "udevdeviceinterface.h"
#include "../shared/udevqt.h"

namespace Solid
{
namespace Backends
{
namespace UDev
{
class AcAdapter : public DeviceInterface, virtual public Solid::Ifaces::AcAdapter
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::AcAdapter)

public:
    AcAdapter(UDevDevice *device);
    virtual ~AcAdapter();

    virtual bool isPlugged() const;

Q_SIGNALS:
    void plugStateChanged(bool newState, const QString &udi);

private Q_SLOTS:
    void slotEmitSignals(const UdevQt::Device &device);

private:
    UdevQt::Client *m_client;
    bool m_isplugged;
};
}
}
}

#endif // UDEVACADAPTER_H
