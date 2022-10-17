/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KCOMPRESSOR_H
#define KCOMPRESSOR_H

#include <kdecore_export.h>

#include <QString>

class KCompressorPrivate;

class KDECORE_EXPORT KCompressor
{
public:
    enum KCompressorType {
        TypeUnknown = 0,
        TypeDeflate = 1,
        TypeZlib = 2,
        TypeGZip = 3,
        TypeBZip2 = 4,
        TypeXZ = 5
    };

    KCompressor();
    ~KCompressor();

    KCompressorType type() const;
    bool setType(const KCompressorType type);
    int level() const;
    bool setLevel(const int level);

    bool process(const QByteArray &data);
    QByteArray result() const;

    QString errorString() const;

    static KCompressorType typeForMime(const QString &mime);
    static KCompressorType typeForFile(const QString &filepath);

private:
    KCompressorPrivate* d;
};

#endif // KCOMPRESSOR_H
