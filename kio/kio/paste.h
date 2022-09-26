/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

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

#ifndef KIO_PASTE_H
#define KIO_PASTE_H

#include <kio/kio_export.h>
#include <kurl.h>

#include <QWidget>

namespace KIO {
    class Job;

  /**
   * Pastes the content of the clipboard to the given destination URL.
   * URLs are treated separately (performing a file copy)
   * from other data (which is saved into a file after asking the user
   * to choose a filename and the preferred data format)
   *
   * @param destURL the URL to receive the data
   * @param widget parent widget to use for dialogs
   * @param move true to move the data, false to copy -- now ignored and handled automatically
   * @return the job that handles the operation
   */
  KIO_EXPORT Job *pasteClipboard(const KUrl& destURL, QWidget* widget, bool move = false);

  /**
   * Save the given mime @p data to the given destination URL
   * after offering the user to choose a data format.
   * This is the method used when handling drops (of anything else than URLs)
   * onto dolphin and konqueror.
   *
   * @param data the QMimeData, usually from a QDropEvent
   * @param destUrl the URL of the directory where the data will be pasted.
   * The filename to use in that directory is prompted by this method.
   * @param dialogText the text to show in the dialog
   * @param widget parent widget to use for dialogs
   * @param clipboard whether the QMimeData comes from QClipboard. If you
   * use pasteClipboard for that case, you never have to worry about this parameter.
   *
   * @see pasteClipboard()
   */
  KIO_EXPORT Job* pasteMimeData(const QMimeData* data, const KUrl& destUrl,
                                const QString& dialogText, QWidget* widget);

  /**
   * Returns true if pasteMimeSource finds any interesting format in @p data.
   * You can use this method to enable/disable the paste action appropriately.
   * @since 4.3
   */
  KIO_EXPORT bool canPasteMimeSource(const QMimeData* data);

  /**
   * Returns the text to use for the Paste action, when the application supports
   * pasting files, urls, and clipboard data, using pasteClipboard().
   * @return a string suitable for KAction::setText, or an empty string if pasting
   * isn't possible right now.
   */
  KIO_EXPORT QString pasteActionText();
}

#endif
