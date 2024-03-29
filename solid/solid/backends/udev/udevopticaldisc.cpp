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

#include "udevopticaldisc.h"

#include <QDebug>

#include <cdio/cd_types.h>

using namespace Solid::Backends::UDev;

OpticalDisc::OpticalDisc(UDevDevice *device)
    : StorageVolume(device),
    p_cdio(nullptr)
{
    const QByteArray devicename(m_device->deviceProperty("DEVNAME").toLocal8Bit());
    p_cdio = cdio_open(devicename.constData(), DRIVER_UNKNOWN);
    if (!p_cdio) {
        qWarning() << "Could not open" << devicename;
    }

    // qDebug() << "OpticalDisc" << devicename << capacity() << isRewritable() << isAppendable() << discType() << availableContent();
}

OpticalDisc::~OpticalDisc()
{
    if (p_cdio) {
        cdio_destroy(p_cdio);
    }
}

qulonglong OpticalDisc::capacity() const
{
    switch (discType()) {
        case Solid::OpticalDisc::CdRom:
        case Solid::OpticalDisc::CdRecordable:
        case Solid::OpticalDisc::CdRewritable:
            return (qulonglong(700) * 1024 * 1024);
        // for reference:
        // https://www.blue-ray.com/aboutBlu-ray.html
        case Solid::OpticalDisc::DvdRom:
        case Solid::OpticalDisc::DvdRam:
        case Solid::OpticalDisc::DvdRecordable:
        case Solid::OpticalDisc::DvdRewritable:
        case Solid::OpticalDisc::DvdPlusRecordable:
        case Solid::OpticalDisc::DvdPlusRewritable:
            return (qulonglong(4700) * 1024 * 1024);
        case Solid::OpticalDisc::DvdPlusRecordableDuallayer:
        case Solid::OpticalDisc::DvdPlusRewritableDuallayer:
            return (qulonglong(9400) * 1024 * 1024);
        case Solid::OpticalDisc::HdDvdRom:
        case Solid::OpticalDisc::HdDvdRecordable:
        case Solid::OpticalDisc::HdDvdRewritable:
            return (qulonglong(15000) * 1024 * 1024);
        // no dual layer enums
        case Solid::OpticalDisc::BluRayRom:
        case Solid::OpticalDisc::BluRayRecordable:
        case Solid::OpticalDisc::BluRayRewritable:
            return (qulonglong(25000) * 1024 * 1024);
        // no dual/tripple/quadruple layer enums
        default:
            return 0;
    }
    Q_UNREACHABLE();
}

bool OpticalDisc::isRewritable() const
{
    switch (discType()) {
        case Solid::OpticalDisc::CdRewritable:
        case Solid::OpticalDisc::BluRayRewritable:
        case Solid::OpticalDisc::HdDvdRewritable:
        case Solid::OpticalDisc::DvdRewritable:
        case Solid::OpticalDisc::DvdPlusRewritable:
        case Solid::OpticalDisc::DvdPlusRewritableDuallayer:
            return true;
        default:
            return false;
    }
    Q_UNREACHABLE();
}

bool OpticalDisc::isBlank() const
{
    return (availableContent() == Solid::OpticalDisc::NoContent);
}

bool OpticalDisc::isAppendable() const
{
    switch (discType()) {
        case Solid::OpticalDisc::DvdPlusRecordable:
        case Solid::OpticalDisc::DvdPlusRewritable:
        case Solid::OpticalDisc::DvdPlusRewritableDuallayer:
        case Solid::OpticalDisc::DvdPlusRecordableDuallayer:
            return true;
        default:
            return false;
    }
    Q_UNREACHABLE();
}

Solid::OpticalDisc::DiscType OpticalDisc::discType() const
{
    if (!p_cdio) {
        return Solid::OpticalDisc::UnknownDiscType;
    }

    // not supported by libcdio: BluRayRom, BluRayRecordable, BluRayRewritable
    /*
        TODO: not implemented by libcdio/needs rw query on CDIO_DISC_MODE_CD_*?
        CdRecordable, CdRewritable, HdDvdRewritable
    */

    const discmode_t discmode = cdio_get_discmode(p_cdio);
    switch(discmode) {
        case CDIO_DISC_MODE_CD_DA:
        case CDIO_DISC_MODE_CD_DATA:
        case CDIO_DISC_MODE_CD_XA:
        case CDIO_DISC_MODE_CD_MIXED:
            return Solid::OpticalDisc::CdRom;
        case CDIO_DISC_MODE_DVD_ROM:
        case CDIO_DISC_MODE_DVD_RAM:
            return Solid::OpticalDisc::DvdRom;
        case CDIO_DISC_MODE_DVD_R:
            return Solid::OpticalDisc::DvdRecordable;
        case CDIO_DISC_MODE_DVD_RW:
            return Solid::OpticalDisc::DvdRewritable;
        case CDIO_DISC_MODE_HD_DVD_ROM:
            return Solid::OpticalDisc::HdDvdRom;
        case CDIO_DISC_MODE_HD_DVD_RAM:
            return Solid::OpticalDisc::DvdRam;
        case CDIO_DISC_MODE_HD_DVD_R:
            return Solid::OpticalDisc::HdDvdRecordable;
        case CDIO_DISC_MODE_DVD_PR:
            return Solid::OpticalDisc::DvdPlusRecordable;
        case CDIO_DISC_MODE_DVD_PRW:
            return Solid::OpticalDisc::DvdPlusRewritable;
        case CDIO_DISC_MODE_DVD_PRW_DL:
            return Solid::OpticalDisc::DvdPlusRewritableDuallayer;
        case CDIO_DISC_MODE_DVD_PR_DL:
            return Solid::OpticalDisc::DvdPlusRecordableDuallayer;
        case CDIO_DISC_MODE_DVD_OTHER:
        case CDIO_DISC_MODE_CD_I: {
            qWarning() << "Unhandled disc mode" << discmode;
            return Solid::OpticalDisc::UnknownDiscType;
        }
        case CDIO_DISC_MODE_NO_INFO: {
            qDebug() << "No information about disc mode";
            return Solid::OpticalDisc::UnknownDiscType;
        }
        case CDIO_DISC_MODE_ERROR: {
            qWarning() << "Disc mode error";
            return Solid::OpticalDisc::UnknownDiscType;
        }
        default: {
            qWarning() << "Unknown disc mode" << discmode;
            return Solid::OpticalDisc::UnknownDiscType;
        }
    }

    Q_UNREACHABLE();
}

Solid::OpticalDisc::ContentTypes OpticalDisc::availableContent() const
{
    Solid::OpticalDisc::ContentTypes result = Solid::OpticalDisc::NoContent;
    if (!p_cdio) {
        return result;
    }

    // not implemented by libcdio: VideoDvd, VideoBluRay
    const track_t totaltracks = cdio_get_num_tracks(p_cdio);
    for (track_t tcount = 0; tcount < totaltracks; tcount++) {
        cdio_iso_analysis_t analysis;
        ::memset(&analysis, 0, sizeof(analysis));
        const cdio_fs_anal_t guessresult = cdio_guess_cd_type(p_cdio, 0, tcount, &analysis);
        switch(CDIO_FSTYPE(guessresult)) {
            case CDIO_FS_AUDIO: {
                result |= Solid::OpticalDisc::Audio;
                break;
            }
            case CDIO_FS_ANAL_VIDEOCD: {
                result |= Solid::OpticalDisc::VideoCd;
                break;
            }
            case CDIO_FS_ANAL_SVCD: {
                result |= Solid::OpticalDisc::SuperVideoCd;
                break;
            }
            default: {
                result |= Solid::OpticalDisc::Data;
                break;
            }
        }
    }

    return result;
}

#include "moc_udevopticaldisc.cpp"
