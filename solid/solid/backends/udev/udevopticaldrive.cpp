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

#include <QDebug>

#include "udevopticaldrive.h"

using namespace Solid::Backends::UDev;

// TODO: Q_CONSTRUCTOR_FUNCTION() for cdio_init()? cdio_open() is supposed to call it

OpticalDrive::OpticalDrive(UDevDevice *device)
    : StorageDrive(device),
    p_cdio(Q_NULLPTR)
{
    const QByteArray devicename = m_device->deviceName().toLocal8Bit();
    p_cdio = cdio_open(devicename.constData(), DRIVER_UNKNOWN);
    if (!p_cdio) {
        qWarning() << "Could not open" << devicename;
    }

    m_device->registerAction("eject", this,
                             SLOT(slotEjectRequested()),
                             SLOT(slotEjectDone(int,QString)));
}

OpticalDrive::~OpticalDrive()
{
    if (p_cdio) {
        cdio_destroy(p_cdio);
    }
}

bool OpticalDrive::eject()
{
    m_device->broadcastActionRequested("eject");

    const QByteArray devicename = m_device->deviceName().toLocal8Bit();
    const driver_return_code_t result = cdio_eject_media_drive(devicename.constData());
    if (result == DRIVER_OP_SUCCESS) {
        m_device->broadcastActionDone("setup", Solid::NoError, QString());
        return true;
    }

    /*
        TODO: check result and call broadcastActionDone with one of:
        UnauthorizedOperation
        DeviceBusy
        OperationFailed
        UserCanceled
        InvalidOption
        MissingDriver
    */
    const QString ejecterror = QString::fromLatin1(cdio_driver_errmsg(result));;
    m_device->broadcastActionDone("eject", Solid::UnauthorizedOperation, ejecterror);
    return false;
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
    Solid::OpticalDrive::MediumTypes result = 0;
    if (!p_cdio) {
        return result;
    }

    cdio_drive_read_cap_t  reacap;
    cdio_drive_write_cap_t writecap;
    cdio_drive_misc_cap_t  misccap;
    cdio_get_drive_cap(p_cdio, &reacap, &writecap, &misccap);
    // ignoring read capabilities on purpose
    Q_UNUSED(reacap);
    Q_UNUSED(misccap);

    /*
        TODO: not supported by libcdio:
        Dvdplusr, Dvdplusdl, Dvdplusdlrw, Bd, Bdr, Bdre, HdDvd, HdDvdr, HdDvdrw
    */

    if (writecap == CDIO_DRIVE_CAP_ERROR) {
        qWarning() << "Could not obtain write capabilities";
    } else {
        if (writecap & CDIO_DRIVE_CAP_WRITE_CD_R) {
            result |= Solid::OpticalDrive::Cdr;
        }
        if (writecap & CDIO_DRIVE_CAP_WRITE_CD_RW) {
            result |= Solid::OpticalDrive::Cdrw;
        }
        if (writecap & CDIO_DRIVE_CAP_WRITE_DVD_R) {
            result |= Solid::OpticalDrive::Dvd;
        }
        if (writecap & CDIO_DRIVE_CAP_WRITE_DVD_PR) {
            result |= Solid::OpticalDrive::Dvdr;
        }
        if (writecap & CDIO_DRIVE_CAP_WRITE_DVD_RW) {
            result |= Solid::OpticalDrive::Dvdrw;
        }
        if (writecap & CDIO_DRIVE_CAP_WRITE_DVD_RAM) {
            result |= Solid::OpticalDrive::Dvdram;
        }
        if (writecap & CDIO_DRIVE_CAP_WRITE_DVD_RPW) {
            result |= Solid::OpticalDrive::Dvdplusrw;
        }
        if (writecap & CDIO_DRIVE_CAP_WRITE_DVD_RPW) {
            result |= Solid::OpticalDrive::Dvdplusrw;
        }
    }

    return result;
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
