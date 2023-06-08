//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright 2007 Daniel Teske <teske@squorn.de>

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
#ifndef KBOOKMARKDIALOG_H
#define KBOOKMARKDIALOG_H

#include "kbookmark.h"

#include <kdialog.h>
#include <kurl.h>

#include <QWidget>
#include <QTreeWidgetItem>

class KBookmarkManager;
class KBookmarkDialogPrivate;

/**
 * This class provides a Dialog for editing properties, adding Bookmarks and creating new folders.
 * It can be used to show dialogs for common tasks with bookmarks.
 *
 * It is used by KBookmarkMenu to show a dialog for "Properties", "Add Bookmark" and "Create New Folder".
 * If you want to customize those dialogs, derive from KBookmarkOwner and reimplement bookmarkDialog(),
 * return a KBookmarkDialog subclass and reimplement initLayout(), aboutToShow() and save().
**/
class KIO_EXPORT KBookmarkDialog : public KDialog
{
    Q_OBJECT
public:
    /**
     * Creates a new KBookmarkDialog
     */
    KBookmarkDialog(KBookmarkManager *mgr, QWidget *parent = 0);
    /**
     * Destructor
     */
    ~KBookmarkDialog();
    /**
     * shows a propeties dialog
     * @note This updates the bookmark and calls KBookmarkManager::emitChanged
     */
    KBookmark editBookmark(const KBookmark &bm);
    /**
     * shows a add Bookmark dialog
     * @note This updates the bookmark and calls KBookmarkManager::emitChanged   
     */
    KBookmark addBookmark(const QString &title, const KUrl &url, KBookmark parent = KBookmark());
    /**
     * Creates a folder from a list of bookmarks
     * @note This updates the bookmark and calls KBookmarkManager::emitChanged
     */
    KBookmarkGroup addBookmarks(const QList<QPair<QString, QString>> &list, const QString &name = QString(),  KBookmarkGroup parent = KBookmarkGroup());
    /**
     * A dialog to create a new folder.
     */
    KBookmarkGroup createNewFolder(const QString &name, KBookmark parent = KBookmark());
    /**
     * A dialog to select a folder.
     */
    KBookmarkGroup selectFolder(KBookmark start = KBookmark());

protected:
    typedef enum { NewFolder, NewBookmark, EditBookmark, NewMultipleBookmarks, SelectFolder } BookmarkDialogMode;
    /**
     * initLayout is called to set the dialog up, indepent from the mode
     * If you want to add widgets or a custom layout, reimplement this function.
     * The default implementation is rather simple, take a look at the source.
     * 
     */
    virtual void initLayout();
    /**
     *  aboutToShow is called immediately before exec()
     *  Reimplement this to show or hide UI elements for certain modes.
     *  
     */
    virtual void aboutToShow(BookmarkDialogMode mode);
    /** 
     * save all your custom data in this method
     * This is called after the users has accepted() the dialog.
     *
     */
    virtual void save(BookmarkDialogMode mode, const KBookmark &bm);

    /**
     * selects the specified bookmark in the folder tree
     */
    void setParentBookmark(const KBookmark &bm);
    /**
     * returns the selected bookmark in the folder tree, or the root (top-level)
     * bookmark if none was selected
     */
    KBookmarkGroup parentBookmark();

    void slotButtonClicked(int);

protected Q_SLOTS:
    void newFolderButton();

private:
    void initLayoutPrivate();
    void fillGroup(QTreeWidgetItem *parentItem, const KBookmarkGroup &group);
    void fillGroup(QTreeWidgetItem *parentItem, const KBookmarkGroup &group, const KBookmarkGroup &selectGroup);

    KBookmarkDialogPrivate* const d;
    friend KBookmarkDialogPrivate;
    Q_DISABLE_COPY(KBookmarkDialog);
};

#endif // KBOOKMARKDIALOG_H
