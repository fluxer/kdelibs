/* This file is part of the KDE project

   Copyright 2009 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSYCOCADEVICES_P_H
#define KSYCOCADEVICES_P_H

#include "ksycoca_p.h"

class KSycocaAbstractDevice
{
public:
    KSycocaAbstractDevice() : m_stream(0)
    {
    }

    virtual ~KSycocaAbstractDevice() { delete m_stream; }

    virtual QIODevice* device() const = 0;

    QDataStream* & stream() {
        if (!m_stream) {
            m_stream = new QDataStream(device());
        }
        return m_stream;
    }

private:
    QDataStream* m_stream;
};

// Reading from a QFile
class KSycocaFileDevice : public KSycocaAbstractDevice
{
public:
    KSycocaFileDevice(const QString& path) {
        m_database = new QFile(path);
    }
    ~KSycocaFileDevice() {
        delete m_database;
    }
    virtual QIODevice* device() const {
        return m_database;
    }
private:
    QFile* m_database;
};

// Reading from a dummy memory buffer
class KSycocaBufferDevice : public KSycocaAbstractDevice
{
public:
    KSycocaBufferDevice() {
        m_buffer = new QBuffer();
        m_buffer->open(QIODevice::ReadWrite); // can't fail
        QDataStream str(m_buffer);
        str << qint32(KSYCOCA_VERSION);
        str << qint32(0);
    }
    ~KSycocaBufferDevice() {
        delete m_buffer;
    }
    virtual QIODevice* device() const {
        return m_buffer;
    }
private:
    QBuffer* m_buffer;
};

#endif /* KSYCOCADEVICES_P_H */

