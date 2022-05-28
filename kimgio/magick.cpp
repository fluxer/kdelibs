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

static const char* const magickpluginformat = "magick";
static const uchar icoheader[] = { 0x0, 0x0, 0x1, 0x0, 0x0 };

int initMagick()
{
    Magick::InitializeMagick(magickpluginformat);
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
            if (qstrncmp(data.constData(), reinterpret_cast<const char*>(icoheader), 5) == 0) {
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
    return magickpluginformat;
}

bool MagickHandler::canRead(QIODevice *device, QByteArray *actualformat)
{
    if (Q_UNLIKELY(!device)) {
        kWarning() << "Called with no device";
        return false;
    }

    const QFile *file = qobject_cast<QFile*>(device);
    if (file) {
        QFileInfo fileinfo(file->fileName());
        const QByteArray filesuffix = fileinfo.suffix().toLatin1();
        if (!filesuffix.isEmpty()) {
            kDebug() << "Using QFile shortcut for" << file->fileName() << "with extension" << filesuffix;
            try {
                const Magick::CoderInfo magickcoderinfo(std::string(filesuffix.constData()));
                const std::string magickcodername = magickcoderinfo.name();
                if (magickcoderinfo.isReadable() && (qstrnicmp(magickcodername.c_str(), "png", 3) != 0)) {
                    kDebug() << "Shortcut says it is supported";
                    actualformat->append(magickcodername.c_str(), magickcodername.size());
                    return true;
                }
                kDebug() << "Shortcut says it is not supported";
            } catch(Magick::Exception &err) {
                kWarning() << err.what();
            } catch(std::exception &err) {
                kWarning() << err.what();
            } catch (...) {
                ;
            }
        }
    }

    const qint64 oldpos = device->pos();

    bool isvalid = false;
    const QByteArray data = device->readAll();

    if (Q_UNLIKELY(data.isEmpty())) {
        device->seek(oldpos);
        return false;
    }

    try {
        Magick::Blob magickinblob(data.constData(), data.size());
        Magick::Image magickimage;
        magickimage.read(magickinblob);
        // PNG handler used by this plugin
        const std::string magickmagick = magickimage.magick();
        isvalid = (magickimage.isValid() && (qstrnicmp(magickmagick.c_str(), "png", 3) != 0));
        if (isvalid) {
            actualformat->append(magickmagick.c_str(), magickmagick.size());
        }
    } catch(Magick::Exception &err) {
        kWarning() << err.what();
    } catch(std::exception &err) {
        kWarning() << err.what();
    } catch (...) {
        kWarning() << "Exception raised";
    }

    if (qstrncmp(data.constData(), reinterpret_cast<const char*>(icoheader), 5) == 0) {
        kDebug() << "ICO header detected";
        actualformat->append("ico");
        isvalid = true;
    }

    device->seek(oldpos);

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
    return QStringList() << magickpluginformat;
}

QList<QByteArray> MagickPlugin::mimeTypes() const
{
    static const QList<QByteArray> list = QList<QByteArray>()
        << "image/bmp"
        << "image/x-dds"
        << "image/x-eps"
        << "image/x-exr"
        << "image/gif"
        << "image/vnd.microsoft.icon"
        << "image/jp2"
        << "image/jpeg"
        << "image/x-portable-bitmap"
        << "image/x-pcx"
        << "image/x-portable-graymap"
        << "image/x-portable-pixmap"
        << "image/x-psd"
        << "image/x-tga"
        << "image/tiff"
        << "image/x-dcraw"
        << "image/x-xbitmap"
        << "image/x-xcf";
    return list;
}

QImageIOPlugin::Capabilities MagickPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == magickpluginformat)
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    QByteArray actualformat;
    if (device->isReadable() && MagickHandler::canRead(device, &actualformat))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *MagickPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new MagickHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN2(magick, MagickPlugin)
