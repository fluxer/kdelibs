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

#include <karchive_export.h>

#include <QString>

class KCompressorPrivate;

/*!
    Class to compress data in various formats.

    Example:
    \code
    KCompressor kcompressor;
    kcompressor.setType(KCompressor::TypeDeflate);
    kcompressor.setLevel(5);
    if (!kcompressor.process(mydata)) {
        kWarning() << kcompressor.errorString();
        return;
    }
    kDebug() << kcompressor.result().toHex();
    \endcode

    @since 4.22
    @see KDecompressor
*/
class KARCHIVE_EXPORT KCompressor
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
    /*!
        @brief Set the type of compression to perform
        @note By default the type is none (i.e. @p KCompressorType::TypeUnknown). The type must be
        set before processing data and can determined via @p KCompressor::typeForMime or
        @p KCompressor::typeForFile depending if the input is file or data in memory
        @see KMimeType::findByPath, KMimeType::findByContent
    */
    bool setType(const KCompressorType type);
    /*!
        @note By default the level is 1 which is good balance between ratio and resources
        requirements for the processing operation
    */
    int level() const;
    bool setLevel(const int level);

    /*!
        @brief Compresses @p data to the set compression type
        @note Multiple calls with different data are allowed
    */
    bool process(const QByteArray &data);
    /*!
        @brief Returns the compressed data
        @note May be empty if nothing was processed or error occurred
    */
    QByteArray result() const;

    //! @brief Returns human-readable description of the error that occured
    QString errorString() const;

    static KCompressorType typeForMime(const QString &mime);
    static KCompressorType typeForFile(const QString &filepath);

private:
    Q_DISABLE_COPY(KCompressor);
    KCompressorPrivate* d;
};

#endif // KCOMPRESSOR_H
