/*  This file is part of the KDE libraries
    Copyright (C) 2018 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KARCHIVE_H
#define KARCHIVE_H

#include "karchive_export.h"

#include <QStringList>

#include <sys/types.h>

/*!
    Archive entry information holder, valid object is obtained via @p KArchive::entry

    @note It is up to the programmer to keep the integrity of the structure
    @ingroup Types

    @see KArchive
    @since 4.22
*/
class KARCHIVE_EXPORT KArchiveEntry
{
public:
    KArchiveEntry();

    bool encrypted;
    qint64 size;
    qint64 gid;
    qint64 uid;
    mode_t mode;
    time_t atime;
    time_t ctime;
    time_t mtime;
    QByteArray hardlink;
    QByteArray symlink;
    QByteArray pathname;
    QByteArray groupname;
    QByteArray username;

    //! @brief Returns if the entry is valid or not
    bool isNull() const;

    //! @brief Fancy encrypted for the purpose of widgets
    QString fancyEncrypted() const;
    //! @brief Fancy size for the purpose of widgets
    QString fancySize() const;
    //! @brief Fancy mode for the purpose of widgets
    QString fancyMode() const;
    //! @brief Fancy access time for the purpose of widgets
    QString fancyATime() const;
    //! @brief Fancy creation time for the purpose of widgets
    QString fancyCTime() const;
    //! @brief Fancy modification time for the purpose of widgets
    QString fancyMTime() const;
    //! @brief Fancy type for the purpose of widgets
    QString fancyType() const;

    bool operator==(const KArchiveEntry &karchiveentry) const;
};
#ifndef QT_NO_DEBUG_STREAM
KARCHIVE_EXPORT QDebug operator<<(QDebug, const KArchiveEntry &karchiveentry);
#endif
QT_BEGIN_NAMESPACE
Q_DECLARE_TYPEINFO(KArchiveEntry, Q_PRIMITIVE_TYPE);
QT_END_NAMESPACE

class KArchivePrivate;


/*!
    Class for archives management.

    Example:
    \code
    KArchive archive("/home/joe/archive.tar.gz");
    kDebug() << archive.list();

    QDir::mkpath("/tmp/destination");
    archive.extract(QStringList() << "dir/in/archive/", "/tmp/destination");
    archive.remove(QStringList() << "file/in/archive.txt");
    \endcode

    @note Paths ending with "/" will be considered as directories
    @warning The operations are done on temporary file, copy of the orignal, which after
    successfull operation (add or remove) replaces the orignal thus if it is interrupted the
    source may get corrupted

    @see KArchiveEntry
    @since 4.22
*/
class KARCHIVE_EXPORT KArchive : public QObject
{
    Q_OBJECT
public:
    KArchive(const QString &path, QObject *parent = nullptr);
    ~KArchive();

    static bool isSupported();
    
    /*!
        @brief Add paths to the archive
        @param strip string to remove from the start of every path
        @param destination relative path where paths should be added to
    */
    bool add(const QStringList &paths, const QByteArray &strip = "/", const QByteArray &destination = "") const;
    //! @brief Remove paths from the archive
    bool remove(const QStringList &paths) const;
    /*!
        @brief Extract paths to destination
        @param destination existing directory, you can use @p QDir::mkpath(QString)
        @param preserve preserve advanced attributes (ACL/ATTR)
    */
    bool extract(const QStringList &paths, const QString &destination, const bool preserve = true) const;

    /*!
        @brief List the content of the archive
        @param path filter, anything not starting with @p path will not be listed
        @note May return empty list on both failure and success
        @note Some formats list directories, some do not
    */
    QList<KArchiveEntry> list(const QString &path = QString()) const;
    //! @brief Get entry information for path in archive
    KArchiveEntry entry(const QString &path) const;
    //! @brief Get data for path in archive
    QByteArray data(const QString &path) const;

    //! @brief Returns if path is readable archive
    bool isReadable() const;
    //! @brief Returns if path is writable archive
    bool isWritable() const;

    //! @brief Returns if path has encrypted entries
    bool requiresPassphrase() const;
    //! @brief Sets the passphrase to be used when reading archive
    void setReadPassphrase(const QString &passphrase);
    //! @brief Sets the passphrase to be used when writing archive
    void setWritePassphrase(const QString &passphrase);

    //! @brief Returns human-readable description of the error that occured
    QString errorString() const;

    //! @brief Returns list of archive MIME types that are readable
    static QStringList readableMimeTypes();
    //! @brief Returns list of archive MIME types that are writable
    static QStringList writableMimeTypes();

Q_SIGNALS:
    /*!
        @brief Signals how far operation (add, remove or extract) is from completing
        @note The progress value is between 0.0 and 1.0
        @note If you are connecting the progress signal to QProgressBar, derived or similar class
        make sure to set its initial value, progress is emited only for @b valid items. For example
        progress will note be emited for paths that are not in the archive from
        @p KArchive::remove()
    */
    void progress(const qreal value) const;

private:
    Q_DISABLE_COPY(KArchive);
    KArchivePrivate* const d;
};

#endif // KARCHIVE_H
