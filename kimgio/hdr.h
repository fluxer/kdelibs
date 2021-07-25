/* This file is part of the KDE project
   Copyright (C) 2005 Christoph Hormann <chris_hormann@gmx.de>
   Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KIMG_HDR_H
#define KIMG_HDR_H

#include <QtGui/qimageiohandler.h>

class HDRHandler : public QImageIOHandler
{
public:
    HDRHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);
};

class HDRPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

#endif
 
