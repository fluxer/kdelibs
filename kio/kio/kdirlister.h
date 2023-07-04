/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KDIRLISTER_H
#define KDIRLISTER_H

#include "kfileitem.h"
#include <kurl.h>

class KJob;
namespace KIO { class Job; class ListJob; }

class KDirListerPrivate;

/**
 * This clas lists and emits updates for directory. It is independent from the
 * graphical representation of the dir (icon container, tree view, ...), it
 * stores and emits the items as KFileItems.
 *
 * Typical usage:
 * @li Create an instance.
 * @li Connect to at least clear, itemsAdded, and itemsDeleted.
 * @li Call openUrl, signals will be emitted.
 * @li Reuse the instance when opening a new URL (see openUrl).
 * @li Destroy the instance when not needed anymore (usually destructor).
 *
 * @author Ivailo Monev <xakepa10@gmail.com>
 * @author Michael Brade <brade@kde.org>
 */
class KIO_EXPORT KDirLister : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate)
    Q_PROPERTY(bool showingDotFiles READ showingDotFiles WRITE setShowingDotFiles)
    Q_PROPERTY(bool dirOnlyMode READ dirOnlyMode WRITE setDirOnlyMode)
    Q_PROPERTY(bool autoErrorHandlingEnabled READ autoErrorHandlingEnabled)
    Q_PROPERTY(bool delayedMimeTypes READ delayedMimeTypes WRITE setDelayedMimeTypes)
    Q_PROPERTY(QString nameFilter READ nameFilter WRITE setNameFilter)
    Q_PROPERTY(QStringList mimeFilter READ mimeFilters WRITE setMimeFilter RESET clearMimeFilter)

public:
    /**
     * Create a directory lister.
     */
    KDirLister(QObject *parent = nullptr);

    /**
     * Destroy the directory lister.
     */
    virtual ~KDirLister();

    /**
     * Run the directory lister on the given URL.
     *
     * This method causes KDirLister to emit _all_ the items of @p url, in any
     * case. clear() will be emitted first for view models to remove all of
     * their indexes for example.
     *
     * The itemsAdded() signal may be emitted once listing is done and then the
     * completed() signal is emitted (and isFinished() returns true).
     *
     * @param url the directory URL.
     * @param recursive whether to list directory recursively.
     * @return true if successful, false otherwise (e.g. if invalid @p url) was
     *         passed.
     * @see KIO::listDir()
     * @see KIO::listRecursive()
     */
    bool openUrl(const KUrl &url, bool recursive = false);

    /**
     * Stop listing the current directory URL. Emits canceled() if there was
     * job running.
     */
    void stop();

    /**
     * @return true if the "delayed mimetypes" feature was enabled
     * @see setDelayedMimeTypes()
     */
    bool delayedMimeTypes() const;

    /**
     * If enabled, mime types will be fetched on demand, which leads to a
     * faster initial directory listing, where icons get progressively replaced
     * with the correct one while KMimeType is going through the items with
     * unknown or imprecise mimetype (e.g. files with no extension or an
     * unknown extension).
     */
    void setDelayedMimeTypes(bool delayedMimeTypes);

    /**
     * @return true if KDirWatch is used to automatically update directories.
     */
    bool autoUpdate() const;

    /**
     * Enable/disable automatic directory updating, when a directory changes.
     * This is enabled by default and even the URLs of .desktop files are being
     * watched.
     *
     * @param enable true to enable, false to disable
     * @note Call updateDirectory() afterwards for the changes to take effect.
     */
    void setAutoUpdate(bool enable);

    /**
     * @return true if auto error handling is enabled, false otherwise
     * @see setAutoErrorHandlingEnabled()
     */
    bool autoErrorHandlingEnabled() const;

    /**
     * Enable or disable auto error handling is enabled. If enabled, it will
     * show an error dialog to the user when an error occurs. It is turned on
     * by default.
     *
     * @param enable true to enable auto error handling, false to disable
     * @param parent the parent widget for the error dialogs, can be null
     * @see autoErrorHandlingEnabled()
     */
    void setAutoErrorHandlingEnabled(bool enable, QWidget *parent);

    /**
     * @return true if dot files are shown, false otherwise
     * @see setShowingDotFiles()
     */
    bool showingDotFiles() const;

    /**
     * Changes the "is viewing dot files" setting. By default this option is
     * disabled (hidden files will not be shown).
     *
     * @param showDotFiles true to enable showing hidden files, false to
     *        disable
     * @note Call updateDirectory() afterwards for the changes to take effect.
     * @see showingDotFiles()
     */
    void setShowingDotFiles(bool showDotFiles);

    /**
     * @return true if setDirOnlyMode(true) was called
     */
    bool dirOnlyMode() const;

    /**
     * Call this to list only directories. By default this option is disabled
     * (both files and directories will be shown).
     *
     * @param dirsOnly true to list only directories
     * @note Call updateDirectory() afterwards for the changes to take effect.
     */
    void setDirOnlyMode(bool dirsOnly);

    /**
     * @return the URL used by this instance to list the directory.
     */
    KUrl url() const;

    /**
     * This method causes KDirLister to re-list the directory. It might be
     * useful to force an update manually or in case automatic updates are
     * disabled.
     */
    void updateDirectory();

    /**
     * Returns true if no listing operation is currently in progress.
     * @return true if finished, false otherwise
     */
    bool isFinished() const;

    /**
     * Returns the root item of the URL.
     *
     * @return the file item for url() itself (".")
     */
    KFileItem rootItem() const;

    /**
     * Find an item by its URL.
     * @param url the item URL
     * @return the KFileItem
     */
    KFileItem findByUrl(const KUrl &url) const;

    /**
     * Find an item by its name.
     * @param name the item name
     * @return the KFileItem
     */
    KFileItem findByName(const QString &name) const;

    /**
     * Set a name filter to only list items matching this name, e.g. "*.cpp".
     * More than one filter may be set by separating them with whitespace, e.g
     * "*.cpp *.h". By default the filter is empty.
     *
     * @param filter the new filter, empty QString() to disable filtering
     * @note Call updateDirectory() afterwards for the changes to take effect.
     * @see matchesFilter()
     */
    void setNameFilter(const QString &filter);

    /**
     * Returns the current name filter, as set via setNameFilter()
     * @return the current name filter. Empty, when no mime filter is set
     */
    QString nameFilter() const;

    /**
     * Set MIME-based filter to only list items matching the given mimetypes.
     * Calling this function will not affect any named filter already set.
     *
     * @param mimeList a list of MIME-types.
     *
     * @note Call updateDirectory() afterwards for the changes to take effect.
     * @see clearMimeFilter()
     * @see matchesMimeFilter()
     */
    void setMimeFilter(const QStringList &mimeList);

    /**
     * Clears the mime based filter.
     *
     * @note Call updateDirectory() afterwards for the changes to take effect.
     * @see setMimeFilter()
     */
    void clearMimeFilter();

    /**
     * Returns the list of mime based filters, as set via setMimeFilter().
     * @return the list of mime based filters. Empty, when no mime filter is set
     */
    QStringList mimeFilters() const;

    /**
     * @return true if @p name matches a filter in the list, otherwise false.
     * @see setNameFilter()
     */
    bool matchesFilter(const QString &name) const;

    /**
     * @param mime the mimetype to find in the filter list.
     * @return true if @p name matches a filter in the list, otherwise false.
     * @see setMimeFilter()
     */
    bool matchesMimeFilter(const QString &mime) const;

    /**
     * Pass the main window this object is associated with, the window is used
     * for caching authentication data.
     *
     * @param window the window to associate with, null to disassociate
     */
    void setMainWindow(QWidget *window);

    /**
     * @return the associated main window, or null if there is none
     */
    QWidget* mainWindow();

    /**
     * Used by items() to specify whether you want all items for a directory
     * or just the filtered ones.
     */
    enum WhichItems
    {
        AllItems = 0,
        FilteredItems = 1
    };

    /**
     * Returns the items listed for the current url(). This method will NOT
     * start listing a directory, only call this when listing is finished.
     *
     * @param which specifies whether the returned list will contain all
     *              entries or only the ones that passed the nameFilter() and
     *              mimeFilter().
     * @return the items listed for the current url()
     */
    KFileItemList items(WhichItems which = FilteredItems) const;

Q_SIGNALS:
    /**
     * Signals that listing has started.
     */
    void started();

    /**
     * Signals that listing has finished.
     */
    void completed();

    /**
     * Signals that listing has been canceled, either by a call to stop() or
     * openUrl() while listing was in progress.
     */
    void canceled();

    /**
     * Signals that redirection has occurred.
     * @param url the new URL
     */
    void redirection(const KUrl &url);

    /**
     * Signals to clear all items. Make sure to connect to this signal to avoid
     * doubled items, or just clear the items yourself.
     */
    void clear();

    /**
     * Signals that new items were found during directory listing.
     *
     * @param items a list of new items
     * @since 4.2
     */
    void itemsAdded(const KFileItemList &items);

    /**
     * Signals that items have been deleted.
     *
     * @since 4.1.2
     * @param items the list of deleted items
     */
    void itemsDeleted(const KFileItemList &items);

    /**
     * Signals an item to refresh (its mimetype/icon/name has changed).
     *
     * @param items the items to refresh. This is a list of pairs, where
     *              the first item in the pair is the OLD item, and the second
     *              item is the NEW item. This allows to track which item has
     *              changed, especially after a renaming.
     * @note KFileItem::refresh has already been called on those items.
     */
    void refreshItems(const QList<QPair<KFileItem, KFileItem>> &items);

    /**
     * Signals to display information about running list job. Examples of
     * message are "Resolving host", "Connecting to host...", etc.
     *
     * @param msg the info message
     */
    void infoMessage(const QString &msg);

    /**
     * Signals the overall progress of the KDirLister. This can be connected
     * to progress bar very easily. (see QProgressBar)
     *
     * @param percent the progress in percent
     */
    void percent(ulong percent);

    /**
     * Signals the total size of the job.
     *
     * @param size the total size in bytes
     */
    void totalSize(qulonglong size);

    /**
     * Signals the processed size of the running job, emitted regularly.
     *
     * @param size the processed size in bytes
     */
    void processedSize(qulonglong size);

    /**
     * Signals to display information about the speed of the job.
     *
     * @param bytes_per_second the speed in bytes/s
     */
    void speed(ulong bytes_per_second);

protected:
   /**
    * Called for every new item to decide if it shall be filtered or not.
    * Reimplement this method in a subclass to implement your own filtering.
    * The default implementation filters out ".." and everything not matching
    * the name filter(s)
    *
    * @return true if the item is "ok". false if the item shall not be shown in
    *         a view, e.g. files not matching a *.cpp pattern
    * @see matchesFilter()
    * @see setNameFilter()
    */
   virtual bool matchesFilter(const KFileItem &item) const;

   /**
    * Called for every new item to decide if it shall be filtered or not.
    * Reimplement this method in a subclass to implement your own filtering.
    * The default implementation filters out everything not matching the MIME
    * filter(s)
    *
    * @return true if the item is "ok". false if the item shall not be shown in
    *         a view, e.g. files not matching a text/plain MIME
    * @see matchesMimeFilter()
    * @see setMimeFilter()
    */
   virtual bool matchesMimeFilter(const KFileItem &item) const;

   /**
    * Called by the protected matchesFilter() to do the actual filtering.
    *
    * @param name the name to filter
    * @param filters a list of regular expressions for filtering
    */
   virtual bool doNameFilter(const QString &name, const QList<QRegExp> &filters) const;

   /**
    * Called by the protected matchesMimeFilter() to do the actual filtering.
    *
    * @param mime the mime type to filter
    * @param filters the list of mime types to filter
     */
    virtual bool doMimeFilter(const QString &mime, const QStringList &filters) const;

    /**
     * Called whenever list job error occurs. Reimplement to customize error
     * handling. The default implementation uses the job UI delegate.
     */
    virtual void handleError(KIO::Job *job);

private:
    KDirListerPrivate* const d;
    friend KDirListerPrivate;

    Q_PRIVATE_SLOT(d, void _k_slotInfoMessage(KJob *job, const QString &msg));
    Q_PRIVATE_SLOT(d, void _k_slotPercent(KJob *job, ulong value));
    Q_PRIVATE_SLOT(d, void _k_slotTotalSize(KJob *job, qulonglong value));
    Q_PRIVATE_SLOT(d, void _k_slotProcessedSize(KJob *job, qulonglong value));
    Q_PRIVATE_SLOT(d, void _k_slotSpeed(KJob *job, ulong value));
    Q_PRIVATE_SLOT(d, void _k_slotRedirection(KIO::Job *job, const KUrl &url));

    Q_PRIVATE_SLOT(d, void _k_slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries));
    Q_PRIVATE_SLOT(d, void _k_slotResult(KJob *job));

    Q_PRIVATE_SLOT(d, void _k_slotDirty(const QString &path));
    Q_PRIVATE_SLOT(d, void _k_slotFileRenamed(const QString &path, const QString &path2));
    Q_PRIVATE_SLOT(d, void _k_slotFilesAdded(const QString &path));
    Q_PRIVATE_SLOT(d, void _k_slotFilesChangedOrRemoved(const QStringList &paths));
    Q_PRIVATE_SLOT(d, void _k_slotUpdateDirectory());
};

#endif // KDIRLISTER_H
