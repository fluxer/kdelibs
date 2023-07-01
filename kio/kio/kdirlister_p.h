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

#ifndef KDIRLISTER_P_H
#define KDIRLISTER_P_H

#include "kfileitem.h"
#include "kdirwatch.h"
#include "kdirnotify.h"
#include "kio/job.h"

#include <QRegExp>
#include <QTimer>

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

    KDirWatch* dirWatch;
    OrgKdeKDirNotifyInterface* dirNotify;
    QTimer* pendingUpdateTimer;

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
    void _k_slotUpdateDirectory();

private:
    KDirLister *m_parent;
    KUrl::List m_desktopUrls;
};

#endif // KDIRLISTER_P_H
