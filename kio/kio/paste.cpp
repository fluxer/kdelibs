/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "paste.h"
#include "pastedialog.h"

#include "kio/job.h"
#include "kio/copyjob.h"
#include "kio/deletejob.h"
#include "kio/global.h"
#include "kio/netaccess.h"
#include "kio/renamedialog.h"
#include "kio/kprotocolmanager.h"
#include "jobuidelegate.h"

#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmimetype.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

static bool decodeIsCutSelection(const QMimeData *mimeData)
{
    const QByteArray data = mimeData->data("application/x-kde-cutselection");
    return data.isEmpty() ? false : data.at(0) == '1';
}

static KIO::Job *pasteClipboardUrls(const QMimeData* mimeData, const KUrl& destDir)
{
    const KUrl::List urls = KUrl::List::fromMimeData(mimeData, KUrl::List::PreferLocalUrls);
    if (!urls.isEmpty()) {
        const bool move = decodeIsCutSelection(mimeData);
        KIO::Job *job = 0;
        if (move) {
            job = KIO::move(urls, destDir);
        } else {
            job = KIO::copy(urls, destDir);
        }
        return job;
    }
    return 0;
}

static KUrl getNewFileName(const KUrl &u, const QString& text, const QString& suggestedFileName, QWidget *widget)
{
    bool ok;
    QString dialogText(text);
    if (dialogText.isEmpty()) {
        dialogText = i18n("Filename for clipboard content:");
    }
    QString file = KInputDialog::getText(QString(), dialogText, suggestedFileName, &ok, widget);
    if (!ok) {
        return KUrl();
    }

    KUrl myurl(u);
    myurl.addPath(file);

    // Check for existing destination file.
    // When we were using CopyJob, we couldn't let it do that (would expose
    // an ugly tempfile name as the source URL)
    // And now we're using a put job anyway, no destination checking included.
    if (KIO::NetAccess::exists(myurl, KIO::NetAccess::DestinationSide, widget)) {
        kDebug(7007) << "Paste will overwrite file.  Prompting...";
        KIO::RenameDialog dlg(widget,
                              i18n("File Already Exists"),
                              u.pathOrUrl(),
                              myurl.pathOrUrl(),
                              (KIO::RenameDialog_Mode) (KIO::M_OVERWRITE | KIO::M_SINGLE) );
        KIO::RenameDialog_Result res = static_cast<KIO::RenameDialog_Result>(dlg.exec());
        if (res == KIO::R_RENAME) {
            myurl = dlg.newDestUrl();
        } else if (res == KIO::R_CANCEL) {
            return KUrl();
        }
    }

    return myurl;
}

static KIO::Job* putDataAsyncTo(const KUrl& url, const QByteArray& data, QWidget* widget, KIO::JobFlags flags)
{
    KIO::Job* job = KIO::storedPut(data, url, -1, flags);
    job->ui()->setWindow(widget);
    return job;
}

static QByteArray chooseFormatAndUrl(const KUrl& u, const QMimeData* mimeData,
                                     const QStringList& formats,
                                     const QString& text,
                                     const QString& suggestedFileName,
                                     QWidget* widget,
                                     bool clipboard,
                                     KUrl* newUrl)
{
    QStringList formatLabels;
    for (int i = 0; i < formats.size(); i++) {
        const QString& fmt = formats[i];
        KMimeType::Ptr mime = KMimeType::mimeType(fmt, KMimeType::ResolveAliases);
        if (mime) {
            formatLabels.append(i18n("%1 (%2)", mime->comment(), fmt));
        } else {
            formatLabels.append(fmt);
        }
    }

    QString dialogText(text);
    if (dialogText.isEmpty()) {
        dialogText = i18n("Filename for clipboard content:");
    }
    KIO::PasteDialog dlg(QString(), dialogText, suggestedFileName, formatLabels, widget, clipboard);

    if (dlg.exec() != KDialog::Accepted) {
        return QByteArray();
    }

    if (clipboard && dlg.clipboardChanged()) {
        KMessageBox::sorry(widget,
                           i18n("The clipboard has changed since you used 'paste': "
                                "the chosen data format is no longer applicable. "
                                "Please copy again what you wanted to paste."));
        return QByteArray();
    }

    const QString result = dlg.lineEditText();
    const QString chosenFormat = formats[dlg.comboItem()];

    kDebug() << " result=" << result << " chosenFormat=" << chosenFormat;
    *newUrl = KUrl(u);
    newUrl->addPath(result);
    // if "data" came from QClipboard, then it was deleted already - by a nice 0-seconds timer
    // In that case, get it again. Let's hope the user didn't copy something else meanwhile :/
    // #### QT4/KDE4 TODO: check that this is still the case
    if (clipboard) {
        mimeData = QApplication::clipboard()->mimeData();
    }
    return mimeData->data(chosenFormat);
}

static QStringList extractFormats(const QMimeData* mimeData)
{
    QStringList formats;
    Q_FOREACH(const QString& format, mimeData->formats()) {
        if (format == QLatin1String("application/x-kde-cutselection")) { // see KonqDrag
            continue;
        }
        if (format == QLatin1String("application/x-kde-suggestedfilename")) {
            continue;
        }
        if (format.startsWith(QLatin1String("application/x-qt-"))) { // Katie-internal
            continue;
        }
        if (!format.contains(QLatin1Char('/'))) { // e.g. TARGETS, MULTIPLE, TIMESTAMP
            continue;
        }
        formats.append(format);
    }
    return formats;
}

KIO_EXPORT bool KIO::canPasteMimeSource(const QMimeData* data)
{
    return data->hasText() || !extractFormats(data).isEmpty();
}

KIO::Job* pasteMimeDataImpl(const QMimeData* mimeData, const KUrl& destUrl,
                            const QString& dialogText, QWidget* widget,
                            bool clipboard)
{
    QByteArray ba;
    const QString suggestedFilename = QString::fromUtf8(mimeData->data("application/x-kde-suggestedfilename"));

    // Now check for plain text
    // We don't want to display a mimetype choice for a QTextDrag, those mimetypes look ugly.
    if (mimeData->hasText()) {
        ba = mimeData->text().toLocal8Bit(); // encoding OK?
    } else {
        const QStringList formats = extractFormats(mimeData);
        if (formats.isEmpty()) {
            return 0;
        } else if (formats.size() > 1) {
            KUrl newUrl;
            ba = chooseFormatAndUrl(destUrl, mimeData, formats, dialogText, suggestedFilename, widget, clipboard, &newUrl);
            if (ba.isEmpty()) {
                return 0;
            }
            return putDataAsyncTo(newUrl, ba, widget, KIO::Overwrite);
        }
        ba = mimeData->data(formats.first());
    }
    if (ba.isEmpty()) {
        return 0;
    }

    const KUrl newUrl = getNewFileName(destUrl, dialogText, suggestedFilename, widget);
    if (newUrl.isEmpty()) {
        return 0;
    }

    return putDataAsyncTo(newUrl, ba, widget, KIO::Overwrite);
}

// The main method for pasting
KIO_EXPORT KIO::Job *KIO::pasteClipboard( const KUrl& destUrl, QWidget* widget, bool move )
{
    Q_UNUSED(move);

    if (!destUrl.isValid()) {
        KMessageBox::error(widget, i18n("Malformed URL\n%1", destUrl.prettyUrl()));
        return 0;
    }

    // TODO: if we passed mimeData as argument, we could write unittests that don't
    // mess up the clipboard and that don't need QtGui.
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();

    if (KUrl::List::canDecode(mimeData)) {
        // We can ignore the bool move, KIO::paste decodes it
        KIO::Job* job = pasteClipboardUrls(mimeData, destUrl);
        if (job) {
            job->ui()->setWindow(widget);
            return job;
        }
    }

    return pasteMimeDataImpl(mimeData, destUrl, QString(), widget, true /*clipboard*/);
}

// NOTE: DolphinView::pasteInfo() has a better version of this
// (but which requires KonqFileItemCapabilities)
KIO_EXPORT QString KIO::pasteActionText()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    const KUrl::List urls = KUrl::List::fromMimeData(mimeData);
    if (!urls.isEmpty()) {
        if (urls.first().isLocalFile()) {
            return i18np("&Paste File", "&Paste %1 Files", urls.count());
        }
        return i18np("&Paste URL", "&Paste %1 URLs", urls.count());
    } else if ( !mimeData->formats().isEmpty() ) {
        return i18n( "&Paste Clipboard Contents" );
    }
    return QString();
}

// The [new] main method for dropping
KIO_EXPORT KIO::Job* KIO::pasteMimeData(const QMimeData* mimeData, const KUrl& destUrl,
                                        const QString& dialogText, QWidget* widget)
{
    return pasteMimeDataImpl(mimeData, destUrl, dialogText, widget, false /*not clipboard*/);
}
