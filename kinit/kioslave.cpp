/*
 * This file is part of the KDE libraries
 * Copyright (c) 1999-2000 Waldo Bastian <bastian@kde.org>
 *           (c) 1999 Mario Weilguni <mweilguni@sime.com>
 *           (c) 2001 Lubos Lunak <l.lunak@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <kdebug.h>
#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <locale.h>

#include <QtCore/QString>
#include <QtCore/QLibrary>
#include <QtCore/QFile>

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
#define USE_KPROCESS_FOR_KIOSLAVES
#endif

#ifdef USE_KPROCESS_FOR_KIOSLAVES
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include "kstandarddirs.h"
#endif

/* These are to link libkio even if 'smart' linker is used */
#include <kio/authinfo.h>
extern "C" KIO::AuthInfo* _kioslave_init_kio() { return new KIO::AuthInfo(); }

int main(int argc, char **argv)
{
     if (argc < 5)
     {
        fprintf(stderr, "Usage: kioslave <slave-lib> <protocol> <klauncher-socket> <app-socket>\n\nThis program is part of KDE.\n");
        exit(1);
     }
#ifndef _WIN32_WCE
     setlocale(LC_ALL, "");
#endif
     QString libpath = QFile::decodeName(argv[1]);

     if (libpath.isEmpty())
     {
        fprintf(stderr, "library path is empty.\n");
        exit(1); 
     }

     QLibrary lib(libpath);
#ifdef USE_KPROCESS_FOR_KIOSLAVES
     qDebug("trying to load '%s'", qPrintable(libpath));
#endif
     if ( !lib.load() || !lib.isLoaded() )
     {
#ifdef USE_KPROCESS_FOR_KIOSLAVES
        libpath = KStandardDirs::installPath("module") + QFileInfo(libpath).fileName();
        lib.setFileName( libpath );
        if(!lib.load() || !lib.isLoaded())
        {
            QByteArray kdedirs = qgetenv("KDEDIRS");
            if (!kdedirs.size()) {
              qDebug("not able to find '%s' because KDEDIRS environment variable is not set.\n"
                     "Set KDEDIRS to the KDE installation root dir and restart klauncher to fix this problem.",
                     qPrintable(libpath));
              exit(1);
            }
            QString paths = QString::fromLocal8Bit(kdedirs);
            QStringList pathlist = paths.split(';');
            Q_FOREACH(const QString &path, pathlist) {
              QString slave_path = path + QLatin1String("/lib/kde4/") + QFileInfo(libpath).fileName();
              qDebug("trying to load '%s'",slave_path.toLatin1().data());
              lib.setFileName(slave_path);
              if (lib.load() && lib.isLoaded() )
                break;
            }
            if (!lib.isLoaded())
            {
              qWarning("could not open %s: %s", libpath.data(), qPrintable (lib.errorString()) );
              exit(1);
            }
        }
#else
        fprintf(stderr, "could not open %s: %s", qPrintable(libpath),
                qPrintable (lib.errorString()) );
        exit(1);
#endif
     }  

     void* sym = lib.resolve("kdemain");
     if (!sym )
     {
        fprintf(stderr, "Could not find kdemain: %s\n", qPrintable(lib.errorString() ));
        exit(1);
     }


     int (*func)(int, char *[]) = (int (*)(int, char *[])) sym;

     exit( func(argc-1, argv+1)); /* Launch! */
}
