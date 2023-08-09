/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2001 Dawit Alemayehu <adawit@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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

#ifndef KIO_AUTHINFO_H
#define KIO_AUTHINFO_H

#include "kio_export.h"

#include <kurl.h>

#include <QDataStream>
#include <QVariant>
#include <QMap>

namespace KIO {

class AuthInfoPrivate;

/**
 * This class is intended to make it easier to prompt for, cache
 * and retrieve authorization information.
 *
 * When using this class to cache, retrieve or prompt authentication
 * information, you only need to set the necessary attributes. For
 * example, to check whether a password is already cached, the only
 * required information is the URL of the resource. Similarly, to
 * prompt for password you only need to optionally set the prompt,
 * username (if already supplied), comment and commentLabel fields.
 *
 * @short A two way messaging class for passing authentication information.
 * @author Dawit Alemayehu <adawit@kde.org>
 */
class KIO_EXPORT AuthInfo
{
    KIO_EXPORT friend QDataStream& operator<< (QDataStream& s, const AuthInfo& a);
    KIO_EXPORT friend QDataStream& operator>> (QDataStream& s, AuthInfo& a);

public:
    
   /**
    * Default constructor.
    */
   AuthInfo();

   /**
    * Copy constructor.
    */
   AuthInfo(const AuthInfo &info);

   /**
    * Destructor
    * @since 4.1
    */ 
   ~AuthInfo();

   /**
    * Custom assignment operator.
    */
   AuthInfo& operator=(const AuthInfo &info);

   /**
    * The URL for which authentication is to be stored.
    *
    * This field is required when attempting to cache authorization
    * and retrieve it.  However, it is not needed when prompting
    * the user for authorization info.
    *
    * This setting is @em required except when prompting the
    * user for password.
    */
   KUrl url;

   /**
    * This is @em required for caching.
    */
   QString username;

   /**
    * This is @em required for caching.
    */
   QString password;

   /**
    * Information to be displayed when prompting
    * the user for authentication information.
    *
    * @note If this field is not set, the authentication
    *    dialog simply displays the preset default prompt.
    *
    * This setting is @em optional and empty by default.
    */
   QString prompt;

   /**
    * The text to displayed in the title bar of
    * the password prompting dialog.
    *
    * @note If this field is not set, the authentication
    *    dialog simply displays the preset default caption.
    *
    * This setting is @em optional and empty by default.
    */
   QString caption;

   /**
    * Additional comment to be displayed when prompting
    * the user for authentication information.
    *
    * This field allows you to display a short (no more than
    * 80 characters) extra description in the password prompt
    * dialog.  For example, this field along with the
    * commentLabel can be used to describe the server that
    * requested the authentication:
    *
    *  \code
    *  Server:   Squid Proxy @ foo.com
    *  \endcode
    *
    * where "Server:" is the commentLabel and the rest is the
    * actual comment.  Note that it is always better to use
    * the @p commentLabel field as it will be placed properly
    * in the dialog rather than to include it within the actual
    * comment.
    *
    * This setting is @em optional and empty by default.
    */
   QString comment;

   /**
    * Descriptive label to be displayed in front of the
    * comment when prompting the user for password.
    *
    * This setting is @em optional and only applicable when
    * the comment field is also set.
    */
   QString commentLabel;

   /**
    * Flag which if set forces the username field to be read-only.
    *
    * This setting is @em optional and false by default.
    */
   bool readOnly;

   /**
    * Flag to indicate the persistence of the given password.
    *
    * This is a two-way flag, when set before calling openPasswordDialog
    * it makes the "keep Password" check box visible to the user.
    * In return the flag will indicate the state of the check box.
    * By default if the flag is checked the password will be cached
    * for the entire life of the current KDE session otherwise the
    * cached password is deleted right after the application using
    * it has been closed.
    */
   bool keepPassword;

   /**
    * Set Extra Field Value. 
    * Currently supported extra-fields: 
    *    "domain" (QString), 
    *    "anonymous" (bool)
    *    "hide-username-line" (bool)
    * Setting it to an invalid QVariant() will disable the field.
    * Extra Fields are disabled by default.
    * @since 4.1
    */
   void setExtraField(const QString &fieldName, const QVariant &value);

   /**
    * Get Extra Field Value
    * Check QVariant::isValid() to find out if the field exists.
    * @since 4.1 
    */
   QVariant getExtraField(const QString &fieldName) const;

private:
    friend class ::KIO::AuthInfoPrivate;
    AuthInfoPrivate * const d;
};

KIO_EXPORT QDataStream& operator<< (QDataStream &s, const AuthInfo &a);
KIO_EXPORT QDataStream& operator>> (QDataStream &s, AuthInfo &a);

}

#endif // KIO_AUTHINFO_H
