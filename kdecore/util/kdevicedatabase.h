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

#ifndef KDEVICEDATABASE_H
#define KDEVICEDATABASE_H

#include <kdecore_export.h>

#include <QString>

/*!
    Class for obtaining device product and vendor strings from their IDs

    @since 4.21
*/
class KDECORE_EXPORT KDeviceDatabase
{
public:
    /*!
        @return The vendor for @p vendorid, empty string if @p vendorid is unknown
    */
    static QString lookupPCIVendor(const QByteArray &vendorid);
    /*!
        @return The device for @p vendorid and @p deviceid, empty string if @p vendorid is unknown
    */
    static QString lookupPCIDevice(const QByteArray &vendorid, const QByteArray &deviceid);

    /*!
        @return The vendor for @p vendorid or empty string if @p vendorid is unknown
    */
    static QString lookupUSBVendor(const QByteArray &vendorid);
    /*!
        @return The device for @p vendorid and @p deviceid, empty string if @p vendorid is unknown
    */
    static QString lookupUSBDevice(const QByteArray &vendorid, const QByteArray &deviceid);
};

#endif // KDEVICEDATABASE_H
