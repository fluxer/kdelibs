/*
 *  Copyright (C) 2002 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */


#include <kstandarddirs.h>
#include <QApplication>
#include <QFile>
#include <kprotocolmanager.h>
#include <kglobalsettings.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <assert.h>

int main(int argc, char **argv) {
    KAboutData aboutData(QByteArray("kprotocolinfotest"), QByteArray(), ki18n("KProtocolinfo Test"), QByteArray("1.0"));

    KComponentData componentData(&aboutData);
    QCoreApplication app(argc,argv); // needed by QEventLoop in ksycoca.cpp

    KUrl url("/tmp");
    assert( KProtocolManager::supportsListing( KUrl( "ftp://10.1.1.10") ) );
    assert( KProtocolManager::supportsReading(url) == true );

    assert( KProtocolInfo::showFilePreview( "file" ) == true );
    assert( KProtocolInfo::showFilePreview( "http" ) == false );
    assert( KGlobalSettings::showFilePreview( KUrl( "http:/" ) ) == false );

    QString proxy;
    QString protocol = KProtocolManager::slaveProtocol( KUrl( "http://bugs.kde.org" ), proxy );
    assert( protocol == "http" );

    return 0;
}
