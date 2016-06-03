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

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <locale.h>

#include <QtCore/QString>
#include <QtCore/QLibrary>
#include <QtCore/QFile>

int main(int argc, char **argv)
{
     if (argc < 5)
     {
        fprintf(stderr, "Usage: kioslave <slave-lib> <protocol> <klauncher-socket> <app-socket>\n\nThis program is part of KDE.\n");
        exit(1);
     }
     setlocale(LC_ALL, "");
     QString libpath = QFile::decodeName(argv[1]);

     if (libpath.isEmpty())
     {
        fprintf(stderr, "library path is empty.\n");
        exit(1); 
     }

     QLibrary lib(libpath);

     if ( !lib.load() || !lib.isLoaded() )
     {
        fprintf(stderr, "could not open %s: %s", qPrintable(libpath),
                qPrintable (lib.errorString()) );
        exit(1);
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
