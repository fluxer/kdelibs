/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Norbert Frese <nf2@scheinwelt.at>
   Copyright (C) 2007 Thiago Macieira <thiago@kde.org>

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


#ifndef UDSENTRY_H
#define UDSENTRY_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QSharedData>

#include <kio/kio_export.h>

namespace KIO
{
    class UDSEntryPrivate;
    /**
     * Universal Directory Service
     *
     * UDS entry is the data structure representing all the fields about a given URL
     * (file or directory).
     *
     * The KIO::listDir() and KIO:stat() operations use this data structure.
     *
     * KIO defines a number of standard fields, see the UDS_XXX enums (see StandardFieldTypes).
     * at the moment UDSEntry only provides fields with numeric indexes,
     * but there might be named fields with string indexes in the future.
     *
     * For instance, to retrieve the name of the entry, use:
     * \code
     * QString displayName = entry.stringValue( KIO::UDSEntry::UDS_NAME );
     * \endcode
     *
     * To know the modification time of the file/url:
     * \code
     *  time_t mtime = entry.numberValue( KIO::UDSEntry::UDS_MODIFICATION_TIME, -1 );
     *  if ( mtime != -1 )
     *      ...
     * \endcode
     */
    class KIO_EXPORT UDSEntry
    {
    public:

        UDSEntry();
        UDSEntry(const UDSEntry &other);
        ~UDSEntry();
        UDSEntry &operator=(const UDSEntry &other);

        /**
         * @return value of a textual field
         */
        QString stringValue( uint field ) const;

        /**
         * @return value of a numeric field
         */
        long long numberValue( uint field, long long defaultValue = 0 ) const;

        // Convenience methods.
        // Let's not add one method per field, only methods that have some more logic
        // than just calling stringValue(field) or numberValue(field).

        /// @return true if this entry is a directory (or a link to a directory)
        bool isDir() const;
        /// @return true if this entry is a link
        bool isLink() const;

        /**
         * insert field with numeric value
         * @param field numeric field id
         * @param value
         */
        void insert(uint field, const QString& value);

        /**
         * insert field with string value
         * @param field numeric tield id
         * @param l value to set
         */
        void insert(uint field, long long l);

        /**
         * count fields
         * @return the number of fields
         */
        int count() const;

        /**
         * check existence of a field
         * @param field
         */
        bool contains(uint field) const;

        /**
         * remove a field with a certain numeric id
         * @param field numeric type id
         */
        bool remove(uint field);

        /**
         * lists all fields
         */
        QList<uint> listFields() const;

        /**
         * remove all fields
         */
        void clear();

        /**
         * Constants used to specify the type of a UDSField.
         */
        enum StandardFieldTypes {
            // First let's define the item types: bit field

            /// Indicates that the field is a QString
            UDS_STRING = 0x01000000,
            /// Indicates that the field is a number (long long)
            UDS_NUMBER   = 0x02000000,
            /// Indicates that the field represents a time, which is modelled by a long long
            UDS_TIME   = 0x04000000 | UDS_NUMBER,

            // The rest isn't a bit field

            /// Size of the file
            UDS_SIZE = 1 | UDS_NUMBER,
            /// User ID of the file owner
            UDS_USER = 2 | UDS_STRING,
            /// Name of the icon, that should be used for displaying.
            /// It overrides all other detection mechanisms
            UDS_ICON_NAME = 3 | UDS_STRING,
            /// Group ID of the file owner
            UDS_GROUP = 4 | UDS_STRING,
            /// Filename - as displayed in directory listings etc.
            /// "." has the usual special meaning of "current directory"
            /// UDS_NAME must always be set and never be empty, neither contain '/'.
            ///
            /// Note that KIO will append the UDS_NAME to the url of their
            /// parent directory, so all kioslaves must use that naming scheme
            /// ("url_of_parent/filename" will be the full url of that file).
            /// To customize the appearance of files without changing the url
            /// of the items, use UDS_DISPLAY_NAME.
            UDS_NAME = 5 | UDS_STRING,
            /// A local file path if the ioslave display files sitting
            /// on the local filesystem (but in another hierarchy, e.g. settings:/ or remote:/)
            UDS_LOCAL_PATH = 6 | UDS_STRING,
            /// Access permissions (part of the mode returned by stat)
            UDS_ACCESS = 7 | UDS_NUMBER,
            /// The last time the file was modified
            UDS_MODIFICATION_TIME = 8 | UDS_TIME,
            /// The last time the file was opened
            UDS_ACCESS_TIME = 9 | UDS_TIME,
            /// The time the file was created
            UDS_CREATION_TIME = 10 | UDS_TIME,
            /// File type, part of the mode returned by stat
            /// (for a link, this returns the file type of the pointed item)
            /// check UDS_LINK_DEST to know if this is a link
            UDS_FILE_TYPE = 11 | UDS_NUMBER,
            /// Name of the file where the link points to
            /// Allows to check for a symlink (don't use S_ISLNK !)
            UDS_LINK_DEST = 12 | UDS_STRING,
            /// An alternative URL (If different from the caption).
            /// Can be used to mix different hierarchies.
            ///
            /// Use UDS_DISPLAY_NAME if you simply want to customize the user-visible filenames, or use
            /// UDS_TARGET_URL if you want "links" to unrelated urls.
            UDS_URL = 13 | UDS_STRING,
            /// A mime type; the slave should set it if it's known.
            UDS_MIME_TYPE = 14 | UDS_STRING,
            /// A mime type to be used for displaying only.
            /// But when 'running' the file, the mimetype is re-determined
            /// This is for special cases like symlinks in FTP; you probably don't want to use this one.
            UDS_GUESSED_MIME_TYPE = 15 | UDS_STRING,

            /// Indicates that the entry has extended ACL entries
            UDS_EXTENDED_ACL = 16 | UDS_NUMBER,
            /// The access control list serialized into a single string.
            UDS_ACL_STRING = 17 | UDS_STRING,
            /// The default access control list serialized into a single string.
            /// Only available for directories.
            UDS_DEFAULT_ACL_STRING = 18 | UDS_STRING,

            /// If set, contains the label to display instead of
            /// the 'real name' in UDS_NAME
            /// @since 4.1
            UDS_DISPLAY_NAME = 19 | UDS_STRING,
            /// This file is a shortcut or mount, pointing to an
            /// URL in a different hierarchy
            /// @since 4.1
            UDS_TARGET_URL = 20 | UDS_STRING,

            /// User-readable type of file (if not specified,
            /// the mimetype's description is used)
            /// @since 4.4
            UDS_DISPLAY_TYPE = 21 | UDS_STRING,

            /// A comma-separated list of supplementary icon overlays
            /// which will be added to the list of overlays created
            /// by KFileItem.
            ///
            /// @since 4.5
            UDS_ICON_OVERLAY_NAMES = 22 | UDS_STRING,

            /// A comment which will be displayed as is to the user. The string
            /// value may contain plain text or Qt-style rich-text extensions.
            ///
            /// @since 4.6
            UDS_COMMENT = 23 | UDS_STRING,

            /// Device number for this file, used to detect hardlinks
            /// @since 4.7.3
            UDS_DEVICE_ID = 24 | UDS_NUMBER,
            /// Inode number for this file, used to detect hardlinks
            /// @since 4.7.3
            UDS_INODE = 25 | UDS_NUMBER
        };

    private:
        friend class UDSEntryPrivate;
        QSharedDataPointer<UDSEntryPrivate> d;
    };

    /**
     * A directory listing is a list of UDSEntry instances.
     *
     * To list the name and size of all the files in a directory listing you would do:
     * \code
     *   KIO::UDSEntryList::ConstIterator it = entries.begin();
     *   const KIO::UDSEntryList::ConstIterator end = entries.end();
     *   for (; it != end; ++it) {
     *     const KIO::UDSEntry& entry = *it;
     *     QString name = entry.stringValue( KIO::UDSEntry::UDS_NAME );
     *     bool isDir = entry.isDir();
     *     KIO::filesize_t size = entry.numberValue( KIO::UDSEntry::UDS_SIZE, -1 );
     *     ...
     *   }
     * \endcode
     */
    typedef QList<UDSEntry> UDSEntryList;
} // end namespace

QT_BEGIN_NAMESPACE
KIO_EXPORT QDataStream & operator<< ( QDataStream & s, const KIO::UDSEntry & a );
KIO_EXPORT QDataStream & operator>> ( QDataStream & s, KIO::UDSEntry & a );
QT_END_NAMESPACE

#endif /*UDSENTRY_H*/
