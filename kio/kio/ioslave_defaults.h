/* This file is part of the KDE libraries
   Copyright (C) 2001 Waldo Bastian <bastian@kde.org>

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

#ifndef KIO_IOSLAVE_DEFAULTS_H
#define KIO_IOSLAVE_DEFAULTS_H

// TIMEOUT VALUES
#define DEFAULT_RESPONSE_TIMEOUT           600     // 10 min.
#define DEFAULT_CONNECT_TIMEOUT             20     // 20 secs.
#define DEFAULT_READ_TIMEOUT                15     // 15 secs.
#define DEFAULT_PROXY_CONNECT_TIMEOUT       10     // 10 secs.
#define MIN_TIMEOUT_VALUE                    2     //  2 secs.

// MINMUM SIZE FOR ABORTED DOWNLOAD TO BE KEPT
#define DEFAULT_MINIMUM_KEEP_SIZE         5120  //  5 Kbs

// PORT DEFAULTS
#define DEFAULT_FTP_PORT                    21
#define DEFAULT_SFTP_PORT                   22

// DEFAULT USER AGENT KEY - ENABLES OS NAME
#define DEFAULT_USER_AGENT_KEYS         "om"            // Show OS, Machine

#endif // KIO_IOSLAVE_DEFAULTS_H
