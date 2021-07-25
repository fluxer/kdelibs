/* This file is part of the KDE project
   Copyright (C) 2005 Christoph Hormann <chris_hormann@gmx.de>
   Copyright (C) 2005 Ignacio Castaño <castanyo@yahoo.es>
   Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "hdr.h"

#include <QtGui/QImage>
#include <QtCore/QDataStream>
#include <QtCore/QVarLengthArray>

#include <kdebug.h>

namespace { // Private.

#define MAXLINE 1024
#define MINELEN 8       // minimum scanline length for encoding
#define MAXELEN 0x7fff  // maximum scanline length for encoding

    static inline uchar ClipToByte(float value)
    {
        if (value > 255.0f) {
            return 255;
        }
        // else if (value < 0.0f) return 0; // we know value is positive.
        return uchar(value);
    }

    // read an old style line from the hdr image file
    // if 'first' is true the first byte is already read
    static bool Read_Old_Line(uchar *image, int width, QDataStream &s)
    {
        int  rshift = 0;
        int  i;

        while (width > 0) {
            s >> image[0];
            s >> image[1];
            s >> image[2];
            s >> image[3];

            if (s.atEnd()) {
                return false;
            }

            if ((image[0] == 1) && (image[1] == 1) && (image[2] == 1)) {
                for (i = image[3] << rshift; i > 0; i--) {
                    // memcpy(image, image-4, 4);
                    (uint &)image[0] = (uint &)image[0-4];
                    image += 4;
                    width--;
                }
                rshift += 8;
            } else {
                image += 4;
                width--;
                rshift = 0;
            }
        }
        return true;
    }

    static void RGBE_To_QRgbLine(uchar *image, QRgb *scanline, int width)
    {
        for (int j = 0; j < width; j++) {
            // v = ldexp(1.0, int(image[3]) - 128);
            float v;
            int e = int(image[3]) - 128;
            if (e > 0) {
                v = float(1 << e);
            } else {
                v = 1.0f / float(1 << -e);
            }
            
            scanline[j] = qRgb(ClipToByte(float(image[0]) * v),
                               ClipToByte(float(image[1]) * v),
                               ClipToByte(float(image[2]) * v));

            image += 4;
        }
    }

    // Load the HDR image.
    static bool LoadHDR(QDataStream &s, const int width, const int height, QImage &img)
    {
        uchar val, code;

        // Create dst image.
        img = QImage(width, height, QImage::Format_RGB32);
        if (img.isNull()) {
            kDebug(399) << "Could not create image.";
            return false;
        }

        QVarLengthArray<uchar> image(width * 4);

        for (int cline = 0; cline < height; cline++) {
            QRgb *scanline = (QRgb *)img.scanLine(cline);

            // determine scanline type
            if ((width < MINELEN) || (MAXELEN < width)) {
                Read_Old_Line(image.data(), width, s);
                RGBE_To_QRgbLine(image.data(), scanline, width);
                continue;
            }

            s >> val;

            if (s.atEnd()) {
                return true;
            }

            if (val != 2) {
                s.device()->seek(s.device()->pos() - 1);
                Read_Old_Line(image.data(), width, s);
                RGBE_To_QRgbLine(image.data(), scanline, width);
                continue;
            }

            s >> image[1];
            s >> image[2];
            s >> image[3];

            if (s.atEnd()) {
                return true;
            }

            if ((image[1] != 2) || (image[2] & 128)) {
                image[0] = 2;
                Read_Old_Line(image.data()+4, width-1, s);
                RGBE_To_QRgbLine(image.data(), scanline, width);
                continue;
            }

            if ((image[2] << 8 | image[3]) != width) {
                return false;
            }

            // read each component
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < width; ) {
                    s >> code;
                    if (s.atEnd()) {
                        return false;
                    }
                    if (code > 128) {
                        // run
                        code &= 127;
                        s >> val;
                        while(code != 0) {
                            image[i + j * 4] = val;
                            j++;
                            code--;
                        }
                    } else {
                        // non-run
                        while(code != 0) {
                            s >> image[i +  j * 4];
                            j++;
                            code--;
                        }
                    }
                }
            }

            RGBE_To_QRgbLine(image.data(), scanline, width);
        }

        return true;
    }
            
} // namespace

HDRHandler::HDRHandler()
{
}

bool HDRHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("hdr");
        return true;
    }
    return false;
}

bool HDRHandler::read(QImage *image)
{
    int len;
    char line[MAXLINE];

    // Skip to size, parse it
    char s1[3], s2[3];
    int width, height;
    bool validSize = false;
    int linecount = 0;
    do {
        len = device()->readLine(line, MAXLINE);

        if (sscanf(line, "%2[+-XY] %d %2[+-XY] %d", s1, &height, s2, &width) == 4) {
            validSize = true;
            break;
        }
        linecount++;
    } while (linecount < 10);

    if (!validSize) {
        kDebug(399) << "Invalid HDR file.";
        return false;
    }

    QDataStream s(device());

    QImage img;
    if (!LoadHDR(s, width, height, img)) {
        kDebug(399) << "Error loading HDR file.";
        return false;
    }

    *image = img;
    return true;
}

bool HDRHandler::write(const QImage &image)
{
    // intentionally not implemented (since writing low dynamic range data to a HDR file is nonsense.)
    return false;
}

QByteArray HDRHandler::name() const
{
    return "hdr";
}

bool HDRHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("HDRHandler::canRead() called with no device");
        return false;
    }

    int len;
    char line[MAXLINE];
    bool validHeader = false;
    bool validFormat = false;
    int linecount = 0;
    // Parse header
    do {
        len = device->readLine(line, MAXLINE);

        if (qstrncmp(line, "#?RADIANCE", len) == 0 || qstrncmp(line, "#?RGBE", len) == 0) {
            validHeader = true;
        } else if (qstrncmp(line, "FORMAT=32-bit_rle_rgbe", len) == 0) {
            validFormat = true;
        }

        if (validHeader && validFormat) {
            break;
        }
        linecount++;
    } while (linecount < 10);
    
    if (!validHeader || !validFormat) {
        kDebug(399) << "Unknown HDR format.";
        return false;
    }

    return true;
}

QStringList HDRPlugin::keys() const
{
    return QStringList() << "hdr" << "HDR";
}

QImageIOPlugin::Capabilities HDRPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "hdr" || format == "HDR")
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && HDRHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *HDRPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new HDRHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN2(hdr, HDRPlugin)
