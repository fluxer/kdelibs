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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <locale.h>

int main(int argc, char **argv)
{
     if (argc != 3) {
        ::fprintf(stderr, "Usage: kioslave <slave-exe> <app-socket>\n\nThis program is part of KDE.\n");
        ::exit(1);
     }

     ::setlocale(LC_ALL, "");
     if (!argv[1])
     {
        fprintf(stderr, "slave executable path is empty.\n");
        ::exit(1); 
     }

     /* Launch! */
     return ::execv(argv[1], argv+1);
}
