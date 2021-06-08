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

#include "udevopticaldrive.h"

using namespace Solid::Backends::UDev;

OpticalDrive::OpticalDrive(UDevDevice *device)
    : StorageDrive(device)
{
    m_device->registerAction("eject", this,
                             SLOT(slotEjectRequested()),
                             SLOT(slotEjectDone(int,QString)));
}

OpticalDrive::~OpticalDrive()
{
}

bool OpticalDrive::eject()
{
    m_device->broadcastActionRequested("eject");

#if 0
    Solid::DeviceBusy
    Solid::OperationFailed
    Solid::UserCanceled
    Solid::InvalidOption
    Solid::MissingDriver
    Solid::UnauthorizedOperation

    if (true) {
        m_device->broadcastActionDone("setup", Solid::NoError, QString());
    } else {
        const QString ejecterror;
        m_device->broadcastActionDone("eject", Solid::UnauthorizedOperation, ejecterror);
    }
#endif

    const QString ejecterror;
    m_device->broadcastActionDone("eject", Solid::UnauthorizedOperation, ejecterror);
    return false; // TODO:
}

QList<int> OpticalDrive::writeSpeeds() const
{
    return QList<int>(); // TODO:
}

int OpticalDrive::writeSpeed() const
{
    return 0; // TODO:
}

int OpticalDrive::readSpeed() const
{
    return 0; // TODO:
}

Solid::OpticalDrive::MediumTypes OpticalDrive::supportedMedia() const
{
    return 0; // TODO:
}

void OpticalDrive::slotEjectRequested()
{
    emit ejectRequested(m_device->udi());
}

void OpticalDrive::slotEjectDone(int error, const QString &errorString)
{
    emit ejectDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}

#include "moc_udevopticaldrive.cpp"
