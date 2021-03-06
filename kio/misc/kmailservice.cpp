/* This file is part of the KDE libraries
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <ktoolinvocation.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kurl.h>

#include <QCoreApplication>

int main( int argc, char **argv )
{
    KCmdLineOptions options;
    options.add("+url");

    KCmdLineArgs::init( argc, argv, "kmailservice", "kdelibs4", ki18n("KMailService"), "unknown", ki18n("Mail service") );
    KCmdLineArgs::addCmdLineOptions( options );

    QCoreApplication app(argc, argv);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( args->count() != 1 )
        return 1;

    KToolInvocation::invokeMailer(KUrl(args->arg(0)), QByteArray(), true);

    return 0;
}
