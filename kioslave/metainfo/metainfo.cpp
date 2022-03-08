/*  This file is part of the KDE libraries
    Copyright (C) 2002 Rolf Magnus <ramagnus@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation version 2.0

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "metainfo.h"

#include <kurl.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kfilemetainfo.h>
#include <klocale.h>
#include <stdlib.h>

// Recognized metadata entries:
// what         - what to load

using namespace KIO;

extern "C" int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    KAboutData about("kio_metainfo", 0, ki18n("kio_metainfo"), 0);
    KCmdLineArgs::init(&about);

    KApplication app;

    //KApplication app(argc, argv, "kio_metainfo", false, true);

    if (argc != 4)
    {
        kError() << "Usage: kio_metainfo protocol domain-socket1 domain-socket2" << endl;
        exit(-1);
    }

    MetaInfoProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    return 0;
}

MetaInfoProtocol::MetaInfoProtocol(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("metainfo", pool, app)
{
}

MetaInfoProtocol::~MetaInfoProtocol()
{
}

void MetaInfoProtocol::get(const KUrl &url)
{
    KFileMetaInfo info(url.toLocalFile());

    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly);

    stream << info;

    data(arr);
    finished();
}
