/*  This file is part of the KDE libraries
    Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "magick.h"

#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <ktemporaryfile.h>
#include <kdebug.h>

#include <Magick++/Functions.h>
#include <Magick++/Blob.h>
#include <Magick++/CoderInfo.h>
#include <Magick++/STL.h>

static const char* const s_magickpluginformat = "magick";

static const ushort s_peekbuffsize = 32;
// for reference:
// https://en.wikipedia.org/wiki/List_of_file_signatures
static const uchar s_jp2header[] = { 0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50, 0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A };
static const uchar s_jpgjfifheader[] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01 };
static const uchar s_gif87aheader[] = { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 };
static const uchar s_gif89aheader[] = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 };
static const uchar s_icoheader[] = { 0x0, 0x0, 0x1, 0x0, 0x0 };
static const uchar s_jpgheader[] = { 0xFF, 0xD8, 0xFF, 0xE0 };
static const uchar s_jpg2header[] = { 0xFF, 0xD8, 0xFF, 0xEE };
static const uchar s_bmpheader[] = { 0x42, 0x4D };

static const struct HeadersTblData {
    const uchar *header;
    const int headersize;
    const char *format;
} HeadersTbl[] = {
    { s_jp2header, 12, "jp2" },
    { s_jpgjfifheader, 12, "jpg" },
    { s_gif87aheader, 6, "gif" },
    { s_gif89aheader, 6, "gif" },
    { s_icoheader, 5, "ico" },
    { s_jpgheader, 4, "jpg" },
    { s_jpg2header, 4, "jpg" },
    { s_bmpheader , 2, "bmp" }
};
static const qint16 HeadersTblSize = sizeof(HeadersTbl) / sizeof(HeadersTblData);

static QList<std::string> s_whitelist = QList<std::string>()
    << std::string("PGM")
    << std::string("XBM")
    << std::string("BMP")
    << std::string("ICO")
    << std::string("JPEG")
    << std::string("JP2")
    << std::string("GIF")
    << std::string("PNG");

int initMagick()
{
    Magick::InitializeMagick(s_magickpluginformat);
    std::list<Magick::CoderInfo> magickcoderlist;
    try {
        Magick::coderInfoList(
            &magickcoderlist,
            Magick::CoderInfo::AnyMatch,
            Magick::CoderInfo::AnyMatch,
            Magick::CoderInfo::AnyMatch
        );
    } catch(Magick::Exception &err) {
        kWarning() << err.what();
    } catch(std::exception &err) {
        kWarning() << err.what();
    } catch (...) {
        kWarning() << "Exception raised";
    }
    foreach (const Magick::CoderInfo &magickcoder, magickcoderlist) {
        if (s_whitelist.contains(magickcoder.name())) {
            continue;
        }
        try {
            kDebug() << "Blacklisting coder" << magickcoder.name().c_str();
            magickcoder.unregister();
        } catch(Magick::Exception &err) {
            kWarning() << err.what();
        } catch(std::exception &err) {
            kWarning() << err.what();
        } catch (...) {
            kWarning() << "Exception raised";
        }
    }
    return 0;
}
Q_CONSTRUCTOR_FUNCTION(initMagick);

MagickHandler::MagickHandler()
    : m_loopcount(0),
    m_imagecount(1),
    m_imagedelay(0),
    m_currentimage(0),
    m_device(nullptr)
{
}

MagickHandler::~MagickHandler()
{
    Magick::TerminateMagick();
}

bool MagickHandler::canRead() const
{
    // QMovie will probe on each frame
    if (device() == m_device && m_magickimages.size() > 0) {
        return true;
    }

    m_magickimages.clear();
    QByteArray actualformat;
    if (MagickHandler::canRead(device(), &actualformat)) {
        m_device = device();
        setFormat(actualformat.toLower());
        return true;
    }
    return false;
}

bool MagickHandler::read(QImage *image)
{
    try {
        // QMovie will continuously call read() to get each frame
        if (m_magickimages.size() == 0) {
            const QByteArray data = device()->readAll();
            // some ImageMagick coders fail to load from blob (e.g. icon)
            if (qstrncmp(data.constData(), reinterpret_cast<const char*>(s_icoheader), 5) == 0) {
                kDebug() << "ICO workaround";
                KTemporaryFile tempblobfile;
                tempblobfile.setFileTemplate("XXXXXXXXXX.ico");
                if (!tempblobfile.open()) {
                    kWarning() << "Could not open temporary file";
                    return false;
                }
                tempblobfile.write(data.constData(), data.size());
                const QByteArray tmpblob = tempblobfile.fileName().toLocal8Bit();
                Magick::readImages(&m_magickimages, std::string(tmpblob.constData()));
            } else {
                Magick::Blob magickinblob(data.constData(), data.size());
                Magick::readImages(&m_magickimages, magickinblob);
            }
        }

        if (Q_UNLIKELY(m_magickimages.size() == 0)) {
            kWarning() << "Image is not valid";
            return false;
        } else if (Q_UNLIKELY(m_currentimage >= int(m_magickimages.size()))) {
            kWarning() << "Invalid image index";
            return false;
        }

        Magick::Image magickinimage = m_magickimages.at(m_currentimage);
        m_loopcount = magickinimage.animationIterations();
        m_imagecount = m_magickimages.size();
        m_imagedelay = magickinimage.animationDelay();

        Magick::Blob magickoutblob;
        magickinimage.quality(100);
        magickinimage.write(&magickoutblob, "PNG");

        const Magick::Geometry magicksize = magickinimage.size();
        const size_t magickwidth = magicksize.width();
        const size_t magickheight = magicksize.height();
        if (Q_UNLIKELY(magickwidth > INT_MAX || magickheight > INT_MAX)) {
            kWarning() << "Image is too big";
            return false;
        }

        image->loadFromData(reinterpret_cast<const char*>(magickoutblob.data()), magickoutblob.length(), "png");
        const bool result = !image->isNull();
        if (!result) {
            m_loopcount = 0;
            m_imagecount = 0;
            m_imagedelay = 0;
            m_currentimage = 0;
        } else {
            m_currentimage++;
            if (m_currentimage >= m_imagecount) {
                m_currentimage = 0;
            }
        }
        return result;
    } catch(Magick::Exception &err) {
        kWarning() << err.what();
        return false;
    } catch(std::exception &err) {
        kWarning() << err.what();
        return false;
    } catch (...) {
        kWarning() << "Exception raised";
        return false;
    }

    return false;
}

bool MagickHandler::write(const QImage &image)
{
    // this plugin is a read-only catch-all kind of plugin
    return false;
}

QByteArray MagickHandler::name() const
{
    return s_magickpluginformat;
}

bool MagickHandler::canRead(QIODevice *device, QByteArray *actualformat)
{
    if (Q_UNLIKELY(!device)) {
        kWarning() << "Called with no device";
        return false;
    }

    const QByteArray data = device->peek(s_peekbuffsize);

    if (Q_UNLIKELY(data.isEmpty())) {
        return false;
    }

    for (int i = 0; i < HeadersTblSize; i++) {
        if (qstrncmp(data.constData(), reinterpret_cast<const char*>(HeadersTbl[i].header), HeadersTbl[i].headersize) == 0) {
            kDebug() << "Header detected" << HeadersTbl[i].format;
            actualformat->append(HeadersTbl[i].format);
            return true;
        }
    }

    bool isvalid = false;
    try {
        char magickformat[s_peekbuffsize];
        ::memset(magickformat, '\0', s_peekbuffsize * sizeof(char));
        isvalid = (
            MagickCore::GetImageMagick(reinterpret_cast<const uchar*>(data.constData()), data.size(), magickformat)
            // PNG handler used by this plugin
            && (qstrnicmp(magickformat, "png", 3) != 0)
        );
        if (isvalid) {
            kDebug() << "Magick format detected" << magickformat;
            if (s_whitelist.contains(std::string(magickformat, qstrlen(magickformat)))) {
                actualformat->append(magickformat);
            } else {
                isvalid = false;
            }
        }
    } catch(Magick::Exception &err) {
        kWarning() << err.what();
    } catch(std::exception &err) {
        kWarning() << err.what();
    } catch (...) {
        kWarning() << "Exception raised";
    }

    return isvalid;
}

bool MagickHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
    return (option == QImageIOHandler::Animation);
}

QVariant MagickHandler::option(QImageIOHandler::ImageOption option) const
{
    if (option == QImageIOHandler::Animation) {
        return QVariant(bool(m_imagecount > 1));
    }
    return QVariant();
}

bool MagickHandler::jumpToNextImage()
{
    return jumpToImage(m_currentimage + 1);
}

bool MagickHandler::jumpToImage(int imageNumber)
{
    if (imageNumber >= int(m_magickimages.size())) {
        return false;
    }
    m_currentimage = imageNumber;
    return true;
}

int MagickHandler::loopCount() const
{
    return m_loopcount;
}

int MagickHandler::imageCount() const
{
    return m_imagecount;
}

int MagickHandler::nextImageDelay() const
{
    return m_imagedelay;
}

int MagickHandler::currentImageNumber() const
{
    return m_currentimage;
}

QStringList MagickPlugin::keys() const
{
    return QStringList() << s_magickpluginformat;
}

QList<QByteArray> MagickPlugin::mimeTypes() const
{
    static const QList<QByteArray> list = QList<QByteArray>()
        << "image/bmp"
        << "image/gif"
        << "image/vnd.microsoft.icon"
        << "image/jp2"
        << "image/jpeg"
        << "image/x-portable-bitmap"
        << "image/x-portable-graymap"
        << "image/x-portable-pixmap"
        << "image/x-xbitmap";
    return list;
}

QImageIOPlugin::Capabilities MagickPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == s_magickpluginformat)
        return QImageIOPlugin::Capabilities(QImageIOPlugin::CanRead);
    if (!device->isOpen())
        return 0;

    QImageIOPlugin::Capabilities cap;
    QByteArray actualformat;
    if (device->isReadable() && MagickHandler::canRead(device, &actualformat))
        cap |= QImageIOPlugin::CanRead;
    return cap;
}

QImageIOHandler *MagickPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new MagickHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN2(magick, MagickPlugin)
