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

#include "kexiv2.h"
#include "kdebug.h"

#include <QMatrix>

#if defined(HAVE_EXIV2)
#  include <exiv2/exiv2.hpp>
#endif

#if defined(HAVE_EXIV2)
static void KExiv2MsgHandler(int level, const char* message)
{
    switch (level) {
        case Exiv2::LogMsg::debug:
        case Exiv2::LogMsg::info: {
            kDebug() << message;
            break;
        }
        case Exiv2::LogMsg::warn: {
            kWarning() << message;
            break;
        }
        case Exiv2::LogMsg::error: {
            kError() << message;
            break;
        }
    }
}

static int KExiv2Init()
{
    if (Exiv2::XmpParser::initialize()) {
        Exiv2::LogMsg::setHandler(KExiv2MsgHandler);
        return 0;
    }
    return 1;
}
Q_CONSTRUCTOR_FUNCTION(KExiv2Init);

static int KExiv2Deinit()
{
    Exiv2::XmpParser::terminate();
    return 0;
}
Q_DESTRUCTOR_FUNCTION(KExiv2Deinit);
#endif

class KExiv2Private
{
public:
    KExiv2Private(const QString &path);

    QImage m_preview;
    KExiv2::DataMap m_datamap;
};

KExiv2Private::KExiv2Private(const QString &path)
{
#if defined(HAVE_EXIV2)
    try {
        kDebug() << "reading Exiv2 metadata from" << path;
        Exiv2::Image::AutoPtr exiv2image = Exiv2::ImageFactory::open(path.toLocal8Bit().constData());
        if (!exiv2image.get()) {
            kWarning() << "image pointer is null";
            return;
        }
        exiv2image->readMetadata();

        kDebug() << "mapping Exiv2 data for" << path;
        const Exiv2::ExifData exiv2data = exiv2image->exifData();
        for (Exiv2::ExifData::const_iterator it = exiv2data.begin(); it != exiv2data.end(); it++) {
            const std::string key = (*it).key();
            const std::string value = (*it).value().toString();
            kDebug() << "key" << key.c_str() << "value" << value.c_str();
            m_datamap.insert(QByteArray(key.c_str(), key.size()), QString::fromUtf8(value.c_str(), value.size()));
        }

        kDebug() << "obtaninig Exiv2 preview for" << path;
        Exiv2::PreviewManager exiv2previewmanager(*exiv2image);
        Exiv2::PreviewPropertiesList exiv2previewpropertieslist = exiv2previewmanager.getPreviewProperties();
        // reverse iteration to get the largest preview
        for (size_t i = exiv2previewpropertieslist.size(); i > 0; i--) {
            const Exiv2::PreviewProperties exiv2previewproperties = exiv2previewpropertieslist.at(i - 1);
            Exiv2::PreviewImage exiv2previewimage = exiv2previewmanager.getPreviewImage(exiv2previewproperties);
            m_preview.loadFromData(reinterpret_cast<const char*>(exiv2previewimage.pData()),
                exiv2previewimage.size(), exiv2previewimage.extension().c_str());
            if (!m_preview.isNull()) {
                break;
            }
        }
    } catch(Exiv2::Error &err) {
        kWarning() << err.what() << err.code();
    } catch(std::exception &err) {
        kWarning() << err.what();
    } catch (...) {
        kWarning() << "exception raised";
    }
#endif
}

KExiv2::KExiv2(const QString &path)
    : d(new KExiv2Private(path))
{
}

QImage KExiv2::preview() const
{
    return d->m_preview;
}

bool KExiv2::rotateImage(QImage &image) const
{
    // for reference:
    // https://exiv2.org/tags-xmp-tiff.html
    const int orientation = d->m_datamap.value("Exif.Image.Orientation").toInt();
    switch (orientation) {
        case 0: // not documented, nothing to do I guess
        case 1: { // normal orientation
            return true;
        }
        case 2: {
            image = image.mirrored(true, false);
            return true;
        }
        case 3: {
            image = image.mirrored(true, true);
            return true;
        }
        case 4: {
            image = image.mirrored(false, true);
            return true;
        }
        case 5: {
            QMatrix matrix;
            matrix.rotate(90.0);
            image = image.transformed(matrix);
            image = image.mirrored(true, false);
            return true;
        }
        case 6: {
            QMatrix matrix;
            matrix.rotate(90.0);
            image = image.transformed(matrix);
            return true;
        }
        case 7: {
            QMatrix matrix;
            matrix.rotate(-90.0);
            image = image.transformed(matrix);
            image = image.mirrored(true, false);
            return true;
        }
        case 8: {
            QMatrix matrix;
            matrix.rotate(-90.0);
            image = image.transformed(matrix);
            return true;
        }
        default: {
            kWarning() << "unknown orientation" << orientation;
            return false;
        }
    }
    Q_UNREACHABLE();
}

KExiv2::DataMap KExiv2::data() const
{
    return d->m_datamap;
}