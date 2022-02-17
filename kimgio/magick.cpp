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
#include <kdebug.h>

#include <Magick++/Functions.h>
#include <Magick++/Blob.h>
#include <Magick++/CoderInfo.h>
#include <Magick++/STL.h>

static const char* const magickpluginformat = "magick";

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
    if (MagickHandler::canRead(device())) {
        m_device = device();
        setFormat(magickpluginformat);
        return true;
    }
    return false;
}

bool MagickHandler::read(QImage *image)
{
    try {
        // QMovie will continuously call read() to get each frame
        if (m_magickimages.size() == 0) {
            const QFile *file = qobject_cast<QFile*>(device());
            if (file) {
                // some ImageMagick coders fail to load from blob (e.g. icon), this workaround does
                // not work for resource files tho
                const std::string filename = file->fileName().toStdString();
                Magick::readImages(&m_magickimages, filename);
            } else {
                const QByteArray data = device()->readAll();
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

bool MagickHandler::canRead(QIODevice *device)
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
                if (magickcoderinfo.isReadable() && (qstrnicmp(magickcoderinfo.name().c_str(), "png", 3) != 0)) {
                    kDebug() << "Shortcut says it is supported";
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
        isvalid = (magickimage.isValid() && (qstrnicmp(magickimage.magick().c_str(), "png", 3) != 0));
    } catch(Magick::Exception &err) {
        kWarning() << err.what();
    } catch(std::exception &err) {
        kWarning() << err.what();
    } catch (...) {
        kWarning() << "Exception raised";
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
    return QVariant(false);
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

QImageIOPlugin::Capabilities MagickPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == magickpluginformat)
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && MagickHandler::canRead(device))
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
