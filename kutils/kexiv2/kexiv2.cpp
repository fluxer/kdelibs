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
#endif // HAVE_EXIV2

class KExiv2Private
{
public:
    KExiv2Private(const QString &path);

#if defined(HAVE_EXIV2)
    Exiv2::Image::AutoPtr m_exiv2image;
#endif
    const QByteArray m_path;
};

KExiv2Private::KExiv2Private(const QString &path)
    : m_path(path.toLocal8Bit())
{
#if defined(HAVE_EXIV2)
    try {
        kDebug() << "Reading Exiv2 metadata from" << m_path;
        m_exiv2image = Exiv2::ImageFactory::open(m_path.constData());
        if (!m_exiv2image.get()) {
            kWarning() << "Image pointer is null";
            return;
        }
        m_exiv2image->readMetadata();
    } catch(Exiv2::Error &err) {
        kWarning() << err.what() << err.code();
    } catch(std::exception &err) {
        kWarning() << err.what();
    } catch (...) {
        kWarning() << "exception raised";
    }
#endif // HAVE_EXIV2
}

KExiv2::KExiv2(const QString &path)
    : d(new KExiv2Private(path))
{
}

bool KExiv2::isSupported()
{
#if defined(HAVE_EXIV2)
    return true;
#else
    return false;
#endif
}

QImage KExiv2::preview() const
{
    QImage result;
#if defined(HAVE_EXIV2)
    if (d->m_exiv2image.get()) {
        try {
            kDebug() << "Obtaninig Exiv2 preview for" << d->m_path;
            Exiv2::PreviewManager exiv2previewmanager(*d->m_exiv2image);
            Exiv2::PreviewPropertiesList exiv2previewpropertieslist = exiv2previewmanager.getPreviewProperties();
            // reverse iteration to get the largest preview
            for (size_t i = exiv2previewpropertieslist.size(); i > 0; i--) {
                const Exiv2::PreviewProperties exiv2previewproperties = exiv2previewpropertieslist.at(i - 1);
                Exiv2::PreviewImage exiv2previewimage = exiv2previewmanager.getPreviewImage(exiv2previewproperties);
                std::string imageextension = exiv2previewimage.extension();
                if (imageextension.size() > 0 && imageextension.at(0) == '.') {
                    imageextension = imageextension.substr(1, imageextension.size() - 1);
                }
                result.loadFromData(
                    reinterpret_cast<const char*>(exiv2previewimage.pData()), exiv2previewimage.size(),
                    imageextension.c_str()
                );
                if (!result.isNull()) {
                    break;
                }
            }
        } catch(Exiv2::Error &err) {
            kWarning() << err.what() << err.code();
        } catch(std::exception &err) {
            kWarning() << err.what();
        } catch (...) {
            kWarning() << "Exception raised";
        }
    }
#endif // HAVE_EXIV2
    return result;
}

bool KExiv2::rotateImage(QImage &image) const
{
#if defined(HAVE_EXIV2)
    // for reference:
    // https://exiv2.org/tags-xmp-tiff.html
    int orientation = 0;
    if (d->m_exiv2image.get()) {
        static const std::string s_orientationkey = std::string("Exif.Image.Orientation");
        try {
            kDebug() << "Checking for orientation Exif data for" << d->m_path;
            const Exiv2::ExifData exiv2data = d->m_exiv2image->exifData();
            for (Exiv2::ExifData::const_iterator it = exiv2data.begin(); it != exiv2data.end(); it++) {
                const std::string key = (*it).key();
                if (key != s_orientationkey) {
                    continue;
                }
                orientation = (*it).value().toLong();
                kDebug() << "Found orientation Exif data" << orientation;
                break;
            }
        } catch(Exiv2::Error &err) {
            kWarning() << err.what() << err.code();
        } catch(std::exception &err) {
            kWarning() << err.what();
        } catch (...) {
            kWarning() << "Exception raised";
        }
    }
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
            kWarning() << "Unknown orientation" << orientation;
            return false;
        }
    }
    Q_UNREACHABLE();
#else
    kWarning() << "KExiv2 is a stub";
    return false;
#endif // HAVE_EXIV2
}

KExiv2PropertyList KExiv2::metadata() const
{
    KExiv2PropertyList result;
#if defined(HAVE_EXIV2)
    if (d->m_exiv2image.get()) {
        try {
            KExiv2Property kexiv2property;
            kDebug() << "Mapping Exif data for" << d->m_path;
            const Exiv2::ExifData exiv2data = d->m_exiv2image->exifData();
            for (Exiv2::ExifData::const_iterator it = exiv2data.begin(); it != exiv2data.end(); it++) {
                const std::string key = (*it).key();
                const std::string value = (*it).value().toString();
                const std::string taglabel = (*it).tagLabel();
                kDebug() << "Key" << key.c_str() << "value" << value.c_str() << "tag label" << taglabel.c_str();
                kexiv2property.name = QByteArray(key.c_str(), key.size());
                kexiv2property.value = QString::fromStdString(value);
                kexiv2property.label = QString::fromStdString(taglabel);
                result.append(kexiv2property);
            }

            kDebug() << "Mapping IPTC data for" << d->m_path;
            const Exiv2::IptcData iptcdata = d->m_exiv2image->iptcData();
            for (Exiv2::IptcData::const_iterator it = iptcdata.begin(); it != iptcdata.end(); it++) {
                const std::string key = (*it).key();
                const std::string value = (*it).value().toString();
                const std::string taglabel = (*it).tagLabel();
                kDebug() << "Key" << key.c_str() << "value" << value.c_str() << "tag label" << taglabel.c_str();
                kexiv2property.name = QByteArray(key.c_str(), key.size());
                kexiv2property.value = QString::fromStdString(value);
                kexiv2property.label = QString::fromStdString(taglabel);
                result.append(kexiv2property);
            }

            kDebug() << "Mapping XMP data for" << d->m_path;
            const Exiv2::XmpData xmpdata = d->m_exiv2image->xmpData();
            for (Exiv2::XmpData::const_iterator it = xmpdata.begin(); it != xmpdata.end(); it++) {
                const std::string key = (*it).key();
                const std::string value = (*it).value().toString();
                const std::string taglabel = (*it).tagLabel();
                kDebug() << "Key" << key.c_str() << "value" << value.c_str() << "tag label" << taglabel.c_str();
                kexiv2property.name = QByteArray(key.c_str(), key.size());
                kexiv2property.value = QString::fromStdString(value);
                kexiv2property.label = QString::fromStdString(taglabel);
                result.append(kexiv2property);
            }
        } catch(Exiv2::Error &err) {
            kWarning() << err.what() << err.code();
        } catch(std::exception &err) {
            kWarning() << err.what();
        } catch (...) {
            kWarning() << "Exception raised";
        }
    }
#else
    kWarning() << "KExiv2 is a stub";
#endif // HAVE_EXIV2
    return result;
}

