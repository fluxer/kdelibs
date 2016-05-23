/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KFILTERBASE_H
#define KFILTERBASE_H

#include <kdecore_export.h>

#include <QtCore/qiodevice.h>

class QIODevice;

/**
 * This is the base class for compression filters
 * such as gzip and bzip2. It's pretty much internal.
 * Don't use directly, use KFilterDev instead.
 * @internal
 */
class KDECORE_EXPORT KFilterBase
{
public:
    KFilterBase();
    virtual ~KFilterBase();

    /**
     * Sets the device on which the filter will work
     * @param dev the device on which the filter will work
     * @param autodelete if true, @p dev is deleted when the filter is deleted
     */
    void setDevice( QIODevice * dev, bool autodelete = false );
    // Note that this isn't in the constructor, because of KPluginFactory::create,
    // but it should be called before using the filterbase !

    /**
     * Returns the device on which the filter will work.
     * @returns the device on which the filter will work
     */
    QIODevice * device();
    /** \internal */
    virtual bool init( int mode ) = 0;
    /** \internal */
    virtual int mode() const = 0;
    /** \internal */
    virtual bool terminate();
    /** \internal */
    virtual bool reset();
    /** \internal */
    virtual bool readHeader() = 0;
    /** \internal */
    virtual bool writeHeader( const QByteArray & filename ) = 0;
    /** \internal */
    virtual void setOutBuffer( char * data, uint maxlen ) = 0;
    /** \internal */
    virtual void setInBuffer( const char * data, uint size ) = 0;
    /** \internal */
    virtual bool inBufferEmpty() const;
    /** \internal */
    virtual int  inBufferAvailable() const = 0;
    /** \internal */
    virtual bool outBufferFull() const;
    /** \internal */
    virtual int  outBufferAvailable() const = 0;

    /** \internal */
    enum Result { Ok, End, Error };
    /** \internal */
    virtual Result uncompress() = 0;
    /** \internal */
    virtual Result compress( bool finish ) = 0;

    /**
     * \internal
     * \since 4.3
     */
    enum FilterFlags {
        NoHeaders = 0,
        WithHeaders = 1
    };
    /**
     * \internal
     * \since 4.3
     */
    void setFilterFlags(FilterFlags flags);
    FilterFlags filterFlags() const;

    /**
     * Call this to create the appropriate filter for the file
     * named @p fileName.
     * @param fileName the name of the file to filter
     * @return the filter for the @p fileName, or 0 if not found
     */
    static KFilterBase * findFilterByFileName( const QString & fileName );

    /**
     * Call this to create the appropriate filter for the mimetype
     * @p mimeType. For instance application/x-gzip.
     * @param mimeType the mime type of the file to filter
     * @return the filter for the @p mimeType, or 0 if not found
     */
    static KFilterBase * findFilterByMimeType( const QString & mimeType );

protected: // TODO KDE5: move to d pointer
    QIODevice * m_dev;
    bool m_bAutoDel;

private:
    Q_DISABLE_COPY( KFilterBase )
    class Private;
    Private * const d;
};

#endif
