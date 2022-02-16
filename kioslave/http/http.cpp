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

#include "http.h"
#include "kdebug.h"
#include "kcomponentdata.h"

#include <QCoreApplication>

#include <sys/types.h>
#include <unistd.h>

extern "C" int Q_DECL_EXPORT kdemain( int argc, char **argv )
{
    QCoreApplication app(argc, argv);
    KComponentData componentData( "kio_http", "kdelibs4" );
    ( void ) KGlobal::locale();

    kDebug(7103) << "Starting " << getpid();

    if (argc != 4)
    {
        fprintf(stderr, "Usage: kio_http protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    HttpProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kDebug(7103) << "Done";
    return 0;
}

HttpProtocol::HttpProtocol( const QByteArray &pool, const QByteArray &app )
    : SlaveBase( "http", pool, app )
{
}

HttpProtocol::~HttpProtocol()
{
    kDebug(7103);
}

void HttpProtocol::setHost( const QString& host, quint16 port, const QString& user, const QString& pass )
{
    Q_UNUSED(port);
    Q_UNUSED(user);
    Q_UNUSED(pass);

    kDebug(7103);
    error(KIO::ERR_UNSUPPORTED_ACTION, host);
}

void HttpProtocol::stat( const KUrl &url )
{
    kDebug(7103);
    error(KIO::ERR_UNSUPPORTED_ACTION, url.prettyUrl());
}

void HttpProtocol::get( const KUrl& url )
{
    kDebug(7103);
    error(KIO::ERR_UNSUPPORTED_ACTION, url.prettyUrl());
}

void HttpProtocol::put( const KUrl& url, int permissions, KIO::JobFlags flags )
{
    Q_UNUSED(permissions);
    Q_UNUSED(flags);

    kDebug(7103);
    error(KIO::ERR_UNSUPPORTED_ACTION, url.prettyUrl());
}
