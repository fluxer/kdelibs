/* This file is part of the KDE project
   Copyright (C) 2002-2006 Michael Brade <brade@kde.org>

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

#ifndef KDIRLISTER_P_H
#define KDIRLISTER_P_H

#include "kfileitem.h"
#include <kurl.h>
#include <kio/global.h>
#include <kdirwatch.h>
#include <kdirnotify.h>

class KDirListerPrivate
{
public:
    KDirListerPrivate(KDirLister *parent);

    // toplevel URL
    KUrl url;
    bool autoUpdate;
    bool delayedMimeTypes;
    bool autoErrorHandling;
    bool showingDotFiles;
    bool dirOnlyMode;
    bool complete;
    QWidget* window;
    KIO::ListJob* listJob;
    // file item for the root itself (".")
    KFileItem rootFileItem;
    KFileItemList allItems;
    KFileItemList filteredItems;
    QString nameFilter;
    QStringList mimeFilter;
    QList<QRegExp> nameFilters;

    void _k_slotInfoMessage(KJob *job, const QString &msg);
    void _k_slotPercent(KJob *job, ulong value);
    void _k_slotTotalSize(KJob *job, qulonglong value);
    void _k_slotProcessedSize(KJob *job, qulonglong value);
    void _k_slotSpeed(KJob *job, ulong value);

    void _k_slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries);
    void _k_slotRedirection(KIO::Job *job, const KUrl &url);
    void _k_slotResult(KJob *job);

    void _k_slotDirty(const QString &path);
    void _k_slotFileRenamed(const QString &path, const QString &path2);
    void _k_slotFilesAdded(const QString &path);
    void _k_slotFilesChanged(const QStringList &paths);
    void _k_slotFilesRemoved(const QStringList &paths);

private:
    KDirLister *m_parent;
    KDirWatch* m_dirwatch;
    OrgKdeKDirNotifyInterface* m_dirnotify;
};

#endif // KDIRLISTER_P_H
