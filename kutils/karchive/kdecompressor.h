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

#ifndef KDECOMPRESSOR_H
#define KDECOMPRESSOR_H

#include <karchive_export.h>

#include <QString>

class KDecompressorPrivate;

/*!
    Class to decompress data in various formats.

    Example:
    \code
    KDecompressor kdecompressor;
    kdecompressor.setType(KDecompressor::TypeDeflate);
    if (!kdecompressor.process(mydata)) {
        kWarning() << kdecompressor.errorString();
        return;
    }
    kDebug() << kdecompressor.result().toHex();
    \endcode

    @since 4.22
    @see KCompressor
*/
class KARCHIVE_EXPORT KDecompressor
{
public:
    enum KDecompressorType {
        TypeUnknown = 0,
        TypeDeflate = 1,
        TypeZlib = 2,
        TypeGZip = 3,
        TypeBZip2 = 4,
        TypeXZ = 5
    };

    KDecompressor();
    ~KDecompressor();

    KDecompressorType type() const;
    /*!
        @brief Set the type of decompression to perform
        @note By default the type is none (i.e. @p KDecompressorType::TypeUnknown). The type must be
        set before processing data and can determined via @p KDecompressor::typeForMime or
        @p KDecompressor::typeForFile depending if the input is file or data in memory.
    */
    bool setType(const KDecompressorType type);

    /*!
        @brief Decompresses @p data in the set decompression type
        @note Multiple calls with different data are allowed
    */
    bool process(const QByteArray &data);
    /*!
        @brief Returns the decompressed data
        @note May be empty if nothing was processed or error occurred
    */
    QByteArray result() const;

    //! @brief Returns human-readable description of the error that occured
    QString errorString() const;

    static KDecompressorType typeForMime(const QString &mime);
    static KDecompressorType typeForFile(const QString &filepath);

private:
    Q_DISABLE_COPY(KDecompressor);
    KDecompressorPrivate* d;
};

#endif // KDECOMPRESSOR_H
