// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                  2000-2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KIO_JOB_H
#define KIO_JOB_H

#include <kio/jobclasses.h>

namespace KIO {

    enum LoadType { Reload, NoReload };

    /**
     * Creates a single directory.
     *
     *
     *
     *
     * @param url The URL of the directory to create.
     * @param permissions The permissions to set after creating the
     *                    directory (unix-style), -1 for default permissions.
     * @return A pointer to the job handling the operation.
     */
    KIO_EXPORT SimpleJob * mkdir( const KUrl& url, int permissions = -1 ); // TODO KDE5: return a MkdirJob and make that class public again

    /**
     * Removes a single directory.
     *
     * The directory is assumed to be empty.
     * The job will fail if the directory is not empty.
     * Use KIO::del() (DeleteJob) to delete non-empty directories.
     *
     * @param url The URL of the directory to remove.
     * @return A pointer to the job handling the operation.
     */
    KIO_EXPORT SimpleJob * rmdir( const KUrl& url );

    /**
     * Changes permissions on a file or directory.
     * See the other chmod in chmodjob.h for changing many files
     * or directories.
     *
     * @param url The URL of file or directory.
     * @param permissions The permissions to set.
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * chmod( const KUrl& url, int permissions );

    /**
     * Changes ownership and group of a file or directory.
     *
     * @param url The URL of file or directory.
     * @param owner the new owner
     * @param group the new group
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * chown( const KUrl& url, const QString& owner, const QString& group );

    /**
     * Changes the modification time on a file or directory.
     *
     * @param url The URL of file or directory.
     * @param permissions The permissions to set.
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob *setModificationTime( const KUrl& url, const QDateTime& mtime );


    /**
     * Rename a file or directory.
     * Warning: this operation fails if a direct renaming is not
     * possible (like with files or dirs on separate partitions)
     * Use move or file_move in this case.
     *
     * @param src The original URL
     * @param dest The final URL
     * @param flags Can be Overwrite here
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * rename( const KUrl& src, const KUrl & dest, JobFlags flags = DefaultFlags );

    /**
     * Create or move a symlink.
     * This is the lowlevel operation, similar to file_copy and file_move.
     * It doesn't do any check (other than those the slave does)
     * and it doesn't show rename and skip dialogs - use KIO::link for that.
     * @param target The string that will become the "target" of the link (can be relative)
     * @param dest The symlink to create.
     * @param flags Can be Overwrite and HideProgressInfo
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * symlink( const QString & target, const KUrl& dest, JobFlags flags = DefaultFlags );

    /**
     * Execute any command that is specific to one slave (protocol).
     *
     * @param url The URL isn't passed to the slave, but is used to know
     *        which slave to send it to :-)
     * @param data Packed data.  The meaning is completely dependent on the
     *        slave, but usually starts with an int for the command number.
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * special( const KUrl& url, const QByteArray & data, JobFlags flags = DefaultFlags );

    /**
     * Find all details for one file or directory.
     *
     * @param url the URL of the file
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT StatJob * stat( const KUrl& url, JobFlags flags = DefaultFlags );
    /**
     * Find all details for one file or directory.
     * This version of the call includes two additional booleans, @p sideIsSource and @p details.
     *
     * @param url the URL of the file
     * @param side is SourceSide when stating a source file (we will do a get on it if
     * the stat works) and DestinationSide when stating a destination file (target of a copy).
     * The reason for this parameter is that in some cases the kioslave might not
     * be able to determine a file's existence (e.g. HTTP doesn't allow it, FTP
     * has issues with case-sensitivity on some systems).
     * When the slave can't reliably determine the existence of a file, it will:
     * @li be optimistic if SourceSide, i.e. it will assume the file exists,
     * and if it doesn't this will appear when actually trying to download it
     * @li be pessimistic if DestinationSide, i.e. it will assume the file
     * doesn't exist, to prevent showing "about to overwrite" errors to the user.
     * If you simply want to check for existence without downloading/uploading afterwards,
     * then you should use DestinationSide.
     *
     * @param details selects the level of details we want.
     * By default this is 2 (all details wanted, including modification time, size, etc.),
     * setDetails(1) is used when deleting: we don't need all the information if it takes
     * too much time, no need to follow symlinks etc.
     * setDetails(0) is used for very simple probing: we'll only get the answer
     * "it's a file or a directory or a symlink, or it doesn't exist". This is used by KRun and DeleteJob.
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT StatJob * stat( const KUrl& url, KIO::StatJob::StatSide side,
                               short int details, JobFlags flags = DefaultFlags );
    /**
     * Get (a.k.a. read).
     * This is the job to use in order to "download" a file into memory.
     * The slave emits the data through the data() signal.
     *
     * @param url the URL of the file
     * @param reload: Reload to reload the file, NoReload if it can be taken from the cache
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT TransferJob *get( const KUrl& url, LoadType reload = NoReload, JobFlags flags = DefaultFlags );

    /**
     * Put (a.k.a. write)
     *
     * @param url Where to write data.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param flags Can be HideProgressInfo, Overwrite and Resume here. WARNING:
     * Setting Resume means that the data will be appended to @p dest if @p dest exists.
     * @return the job handling the operation.
     * @see get()
     */
    KIO_EXPORT TransferJob *put( const KUrl& url, int permissions,
                                 JobFlags flags = DefaultFlags );

    /**
     * Get (a.k.a. read), into a single QByteArray.
     * @see StoredTransferJob
     *
     * @param url the URL of the file
     * @param reload: Reload to reload the file, NoReload if it can be taken from the cache
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT StoredTransferJob *storedGet( const KUrl& url, LoadType reload = NoReload, JobFlags flags = DefaultFlags );

    /**
     * Put (a.k.a. write) data from a single QByteArray.
     * @see StoredTransferJob
     *
     * @param arr The data to write
     * @param url Where to write data.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param flags Can be HideProgressInfo, Overwrite and Resume here. WARNING:
     * Setting Resume means that the data will be appended to @p dest if @p dest exists.
     * @return the job handling the operation.
     */
    KIO_EXPORT StoredTransferJob *storedPut( const QByteArray& arr, const KUrl& url, int permissions,
                                             JobFlags flags = DefaultFlags );

    /**
     * Find mimetype for one file or directory.
     *
     * If you are going to download the file right after determining its mimetype,
     * then don't use this, prefer using a KIO::get() job instead. See the note
     * about putting the job on hold once the mimetype is determined.
     *
     * @param url the URL of the file
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT MimetypeJob * mimetype( const KUrl& url,
                                       JobFlags flags = DefaultFlags );

    /**
     * Copy a single file.
     *
     * Uses either SlaveBase::copy() if the slave supports that
     * or get() and put() otherwise.
     * @param src Where to get the file.
     * @param dest Where to put the file.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param flags Can be HideProgressInfo, Overwrite and Resume here. WARNING:
     * Setting Resume means that the data will be appended to @p dest if @p dest exists.
     * @return the job handling the operation.
     */
    KIO_EXPORT FileCopyJob *file_copy( const KUrl& src, const KUrl& dest, int permissions=-1,
                                       JobFlags flags = DefaultFlags );

    /**
     * Overload for catching code mistakes. Do NOT call this method (it is not implemented),
     * insert a value for permissions (-1 by default) before the JobFlags.
     * @since 4.5
     */
    FileCopyJob *file_copy( const KUrl& src, const KUrl& dest, JobFlags flags ); // not implemented - on purpose.

    /**
     * Move a single file.
     *
     * Use either SlaveBase::rename() if the slave supports that,
     * or copy() and del() otherwise, or eventually get() & put() & del()
     * @param src Where to get the file.
     * @param dest Where to put the file.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param flags Can be HideProgressInfo, Overwrite and Resume here. WARNING:
     * Setting Resume means that the data will be appended to @p dest if @p dest exists.
     * @return the job handling the operation.
     */
    KIO_EXPORT FileCopyJob *file_move( const KUrl& src, const KUrl& dest, int permissions=-1,
                                       JobFlags flags = DefaultFlags );

    /**
     * Overload for catching code mistakes. Do NOT call this method (it is not implemented),
     * insert a value for permissions (-1 by default) before the JobFlags.
     * @since 4.3
     */
    FileCopyJob *file_move( const KUrl& src, const KUrl& dest, JobFlags flags ); // not implemented - on purpose.


    /**
     * Delete a single file.
     *
     * @param src File to delete.
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob *file_delete( const KUrl& src, JobFlags flags = DefaultFlags );

    /**
     * List the contents of @p url, which is assumed to be a directory.
     *
     * "." and ".." are returned, filter them out if you don't want them.
     *
     *
     * @param url the url of the directory
     * @param flags Can be HideProgressInfo here
     * @param includeHidden true for all files, false to cull out UNIX hidden
     *                      files/dirs (whose names start with dot)
     * @return the job handling the operation.
     */
    KIO_EXPORT ListJob *listDir( const KUrl& url, JobFlags flags = DefaultFlags,
                                 bool includeHidden = true );

    /**
     * The same as the previous method, but recurses subdirectories.
     * Directory links are not followed.
     *
     * "." and ".." are returned but only for the toplevel directory.
     * Filter them out if you don't want them.
     *
     * @param url the url of the directory
     * @param flags Can be HideProgressInfo here
     * @param includeHidden true for all files, false to cull out UNIX hidden
     *                      files/dirs (whose names start with dot)
     * @return the job handling the operation.
     */
    KIO_EXPORT ListJob *listRecursive( const KUrl& url, JobFlags flags = DefaultFlags,
                            bool includeHidden = true );

    /**
     * Tries to map a local URL for the given URL, using a KIO job.
     *
     * Starts a (stat) job for determining the "most local URL" for a given URL.
     * Retrieve the result with StatJob::mostLocalUrl in the result slot.
     * @param url The URL we are testing.
     * \since 4.4
     */
    KIO_EXPORT StatJob* mostLocalUrl(const KUrl& url, JobFlags flags = DefaultFlags);

}

#endif

