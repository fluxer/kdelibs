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

#ifndef KDELIBS_HTTP_H
#define KDELIBS_HTTP_H

#include <config.h>

#include <kurl.h>
#include <kio/slavebase.h>

#include <curl/curl.h>

class HttpProtocol : public QObject, public KIO::SlaveBase
{
    Q_OBJECT

public:
    HttpProtocol(const QByteArray &pool, const QByteArray &app);
    virtual ~HttpProtocol();

    virtual void get(const KUrl &url);

    void slotMIME();
    void slotData(const char* curldata, const size_t curldatasize);
    void slotProgress(KIO::filesize_t received, KIO::filesize_t total);

    bool firstchunk;
    QByteArray headerdata;
private:
    CURL* m_curl;
};

#endif // KDELIBS_HTTP_H
