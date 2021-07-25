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

#include <QtGui/QImage>
#include <QtCore/QDataStream>
#include <kdebug.h>

#include <Magick++/Functions.h>
#include <Magick++/Blob.h>
#include <Magick++/Image.h>

static const char* const magickpluginformat = "magick";

MagickHandler::MagickHandler()
{
    Magick::InitializeMagick(magickpluginformat);
}

MagickHandler::~MagickHandler()
{
    Magick::TerminateMagick();
}

bool MagickHandler::canRead() const
{
    if (canRead(device())) {
        setFormat(magickpluginformat);
        return true;
    }
    return false;
}

bool MagickHandler::read(QImage *image)
{
    const QByteArray data = device()->readAll();

    try {
        Magick::Blob magickinblob(data.constData(), data.size()); 
        Magick::Image magickinimage; 
        magickinimage.read(magickinblob);

        if (Q_UNLIKELY(!magickinimage.isValid())) {
            kWarning() << "image is not valid";
            return false;
        }

        Magick::Blob magickoutblob;
        magickinimage.write(&magickoutblob, "PNG");

        const Magick::Geometry magicksize = magickinimage.size();
        const size_t magickwidth = magicksize.width();
        const size_t magickheight = magicksize.height();
        if (Q_UNLIKELY(magickwidth > INT_MAX || magickheight > INT_MAX)) {
            kWarning() << "image is too big";
            return false;
        }

        image->loadFromData(reinterpret_cast<const char*>(magickoutblob.data()), magickoutblob.length(), "png");
        return !image->isNull();
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
        kWarning("MagickHandler::canRead() called with no device");
        return false;
    }

    qint64 oldPos = device->pos();

    bool isvalid = false;
    const QByteArray data = device->readAll();

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
        kWarning() << "exception raised";
    }

    device->seek(oldPos);

    return isvalid;
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
