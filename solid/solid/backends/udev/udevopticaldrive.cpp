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

#include <cdio/mmc.h>
#include <cdio/mmc_cmds.h>

// for reference:
// https://www.t10.org/ftp/t10/document.97/97-108r0.pdf

// magic bits taken from libcdio examples
enum LibCDIOMagic {
    ReadSpeed = 14,
    MaximumWriteSpeed = 18,
    CurrentWriteSpeed = 28,
};

using namespace Solid::Backends::UDev;

OpticalDrive::OpticalDrive(UDevDevice *device)
    : StorageDrive(device),
    p_cdio(Q_NULLPTR)
{
    const QByteArray devicename = m_device->property("DEVNAME").toString().toLocal8Bit();
    p_cdio = cdio_open(devicename.constData(), DRIVER_UNKNOWN);
    if (!p_cdio) {
        qWarning() << "Could not open" << devicename;
    }

    m_device->registerAction("eject", this,
                             SLOT(slotEjectRequested()),
                             SLOT(slotEjectDone(int,QString)));

    // qDebug() << "OpticalDrive" << devicename << writeSpeeds() << writeSpeed() << readSpeed() << supportedMedia();
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

    const QByteArray devicename = m_device->property("DEVNAME").toString().toLocal8Bit();
    const driver_return_code_t result = cdio_eject_media_drive(devicename.constData());
    // not supported by libcdio: UserCanceled
    switch(result) {
        case DRIVER_OP_SUCCESS: {
            m_device->broadcastActionDone("setup", Solid::NoError, QString());
            return true;
        }
        case DRIVER_OP_NOT_PERMITTED: {
            const QString ejecterror = QString::fromLatin1(cdio_driver_errmsg(result));;
            m_device->broadcastActionDone("eject", Solid::UnauthorizedOperation, ejecterror);
            return false;
        }
        case DRIVER_OP_BAD_PARAMETER:
        case DRIVER_OP_BAD_POINTER: {
            const QString ejecterror = QString::fromLatin1(cdio_driver_errmsg(result));;
            m_device->broadcastActionDone("eject", Solid::InvalidOption, ejecterror);
            return false;
        }
        case DRIVER_OP_NO_DRIVER: {
            const QString ejecterror = QString::fromLatin1(cdio_driver_errmsg(result));;
            m_device->broadcastActionDone("eject", Solid::MissingDriver, ejecterror);
            return false;
        }
        case DRIVER_OP_UNINIT: {
            const QString ejecterror = QString::fromLatin1(cdio_driver_errmsg(result));;
            m_device->broadcastActionDone("eject", Solid::DeviceBusy, ejecterror);
            return false;
        }
        default: {
            const QString ejecterror = QString::fromLatin1(cdio_driver_errmsg(result));;
            m_device->broadcastActionDone("eject", Solid::OperationFailed, ejecterror);
            return false;
        }
    }

    Q_UNREACHABLE();
}

QList<int> OpticalDrive::writeSpeeds() const
{
    QList<int> result;

    // TODO: obtain minimum and maximum, calculate from the results?
    result << writeSpeed();

    return result;
}

int OpticalDrive::writeSpeed() const
{
    int result = 0;
    if (!p_cdio) {
        return result;
    }

    // should be atleast 18
    uint8_t cdiobuf[30];
    ::memset(cdiobuf, 0, sizeof(cdiobuf) * sizeof(uint8_t));
    driver_return_code_t mmcresult = mmc_mode_sense_6(p_cdio, cdiobuf, sizeof(cdiobuf), CDIO_MMC_CAPABILITIES_PAGE);
    if (mmcresult != DRIVER_OP_SUCCESS) {
        mmcresult = mmc_mode_sense_10(p_cdio, cdiobuf, sizeof(cdiobuf), CDIO_MMC_CAPABILITIES_PAGE);
        if (mmcresult != DRIVER_OP_SUCCESS) {
            qWarning() << "Could not obtain mode sense data";
            return result;
        }
    }

    result = CDIO_MMC_GETPOS_LEN16(cdiobuf, LibCDIOMagic::CurrentWriteSpeed);

    return result;
}

int OpticalDrive::readSpeed() const
{
    int result = 0;
    if (!p_cdio) {
        return result;
    }

    uint8_t cdiobuf[30];
    ::memset(cdiobuf, 0, sizeof(cdiobuf) * sizeof(uint8_t));
    driver_return_code_t mmcresult = mmc_mode_sense_6(p_cdio, cdiobuf, sizeof(cdiobuf), CDIO_MMC_CAPABILITIES_PAGE);
    if (mmcresult != DRIVER_OP_SUCCESS) {
        mmcresult = mmc_mode_sense_10(p_cdio, cdiobuf, sizeof(cdiobuf), CDIO_MMC_CAPABILITIES_PAGE);
        if (mmcresult != DRIVER_OP_SUCCESS) {
            qWarning() << "Could not obtain mode sense data";
            return result;
        }
    }

    result = CDIO_MMC_GETPOS_LEN16(cdiobuf, LibCDIOMagic::ReadSpeed);

    return result;
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
    // ignoring read and misc capabilities on purpose
    Q_UNUSED(reacap);
    Q_UNUSED(misccap);

    // not supported by libcdio: Dvdplusdl, Dvdplusdlrw, Bd, Bdr, Bdre, HdDvd, HdDvdr, HdDvdrw
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
            result |= Solid::OpticalDrive::Dvdr;
        }
        if (writecap & CDIO_DRIVE_CAP_WRITE_DVD_PR) {
            result |= Solid::OpticalDrive::Dvdplusr;
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
