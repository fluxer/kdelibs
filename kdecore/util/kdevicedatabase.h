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

class KDeviceDatabasePrivate;

/*!
    Class for obtaining device product and vendor strings from their IDs

    Vendor and device IDs should be either 4 or 6 characters long, e.g. "1d14" or "0x1d14". Class,
    sub-class and protocol IDs should be 2 characters long, e.g. "0b". If they are not zero is
    prepended to the IDs.

    @since 4.21
*/
class KDECORE_EXPORT KDeviceDatabase
{
public:
    KDeviceDatabase();

    /*!
        @return The vendor for @p vendorid, empty string if unknown
    */
    QString lookupPCIVendor(const QByteArray &vendorid);
    /*!
        @return The device for @p vendorid and @p deviceid, empty string if unknown
    */
    QString lookupPCIDevice(const QByteArray &vendorid, const QByteArray &deviceid);
    /*!
        @return The class for @p classid, empty string if unknown
    */
    QString lookupPCIClass(const QByteArray &classid);
    /*!
        @return The subclass for @p classid and @p subclassid, empty string if unknown
    */
    QString lookupPCISubClass(const QByteArray &classid, const QByteArray &subclassid);
    /*!
        @return The protocol for @p classid, @p subclassid and @p protocolid, empty string if unknown
    */
    QString lookupPCIProtocol(const QByteArray &classid, const QByteArray &subclassid, const QByteArray &protocolid);

    /*!
        @return The vendor for @p vendorid, empty string if unknown
    */
    QString lookupUSBVendor(const QByteArray &vendorid);
    /*!
        @return The device for @p vendorid and @p deviceid, empty string if unknown
    */
    QString lookupUSBDevice(const QByteArray &vendorid, const QByteArray &deviceid);
    /*!
        @return The class for @p classid, empty string if unknown
    */
    QString lookupUSBClass(const QByteArray &classid);
    /*!
        @return The subclass for @p classid and @p subclassid, empty string if unknown
    */
    QString lookupUSBSubClass(const QByteArray &classid, const QByteArray &subclassid);
    /*!
        @return The protocol for @p classid, @p subclassid and @p protocolid, empty string if unknown
    */
    QString lookupUSBProtocol(const QByteArray &classid, const QByteArray &subclassid, const QByteArray &protocolid);

private:
    Q_DISABLE_COPY(KDeviceDatabase);
    KDeviceDatabasePrivate *d;
};

#endif // KDEVICEDATABASE_H
