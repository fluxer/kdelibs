// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
                  2003 Andras Mantia <amantia@freemail.hu>

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

#include "kencodingfiledialog.h"

#include <config-kfile.h>

#include <kabstractfilewidget.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>

#include <QTextCodec>

struct KEncodingFileDialogPrivate
{
    KComboBox *encoding;
};

KEncodingFileDialog::KEncodingFileDialog(const QString &startDir, const QString &encoding , const QString &filter,
                                         const QString &caption, KFileDialog::OperationMode type, QWidget *parent)
   : KFileDialog(startDir, filter, parent),
   d(new KEncodingFileDialogPrivate())
{
    setCaption(caption);

    setOperationMode(type);

    d->encoding = new KComboBox(this);
    fileWidget()->setCustomWidget(i18n("Encoding:"), d->encoding);

    d->encoding->clear();
    QString sEncoding = encoding;
    QByteArray systemEncoding = QTextCodec::codecForLocale()->name();
    if (sEncoding.isEmpty() || sEncoding == QLatin1String("System")) {
        sEncoding = systemEncoding;
    }

    int insert = 0;
    int system = 0;
    bool foundRequested = false;
    foreach (const QString &encoding, KGlobal::charsets()->availableEncodingNames()) {
        bool found = false;
        QTextCodec *codecForEnc = KGlobal::charsets()->codecForName(encoding, found);

        if (found) {
            d->encoding->addItem (encoding);
            if (codecForEnc->name() == sEncoding || encoding == sEncoding) {
                d->encoding->setCurrentIndex(insert);
                foundRequested=true;
            }

            if (codecForEnc->name() == systemEncoding || encoding == systemEncoding) {
                system = insert;
            }
            insert++;
        }
    }
  
    if (!foundRequested) {
        d->encoding->setCurrentIndex(system); 
    }
}

KEncodingFileDialog::~KEncodingFileDialog()
{
    delete d;
}

QString KEncodingFileDialog::selectedEncoding() const
{
    if (d->encoding) {
        return d->encoding->currentText();
    }
    return QString();
}

KEncodingFileDialog::Result KEncodingFileDialog::getOpenFileNameAndEncoding(const QString &encoding,
                                                                            const QString &startDir,
                                                                            const QString &filter,
                                                                            QWidget *parent, const QString &caption)
{
    KEncodingFileDialog dlg(
        startDir, encoding, filter,
        caption.isNull() ? i18n("Open") : caption,
        KFileDialog::Opening, parent
    );
    dlg.setMode(KFile::File | KFile::LocalOnly);
    dlg.exec();

    Result res;
    res.fileNames << dlg.selectedFile();
    res.encoding = dlg.selectedEncoding();
    return res;
}

KEncodingFileDialog::Result KEncodingFileDialog::getOpenFileNamesAndEncoding(const QString &encoding,
                                                                             const QString &startDir,
                                                                             const QString &filter,
                                                                             QWidget *parent,
                                                                             const QString &caption)
{
    KEncodingFileDialog dlg(
            startDir, encoding, filter, caption.isNull() ? i18n("Open") : caption,
            KFileDialog::Opening, parent
    );
    dlg.setMode(KFile::Files | KFile::LocalOnly);
    dlg.exec();

    Result res;
    res.fileNames = dlg.selectedFiles();
    res.encoding = dlg.selectedEncoding();
    return res;
}

KEncodingFileDialog::Result KEncodingFileDialog::getOpenUrlAndEncoding(const QString &encoding, const QString &startDir,
                                                                       const QString &filter, QWidget *parent, const QString &caption)
{
    KEncodingFileDialog dlg(
        startDir, encoding, filter,
        caption.isNull() ? i18n("Open") : caption,
        KFileDialog::Opening, parent
    );
    dlg.setMode(KFile::File);
    dlg.exec();

    Result res;
    res.URLs = dlg.selectedUrl();
    res.encoding = dlg.selectedEncoding();
    return res;
}

KEncodingFileDialog::Result KEncodingFileDialog::getOpenUrlsAndEncoding(const QString &encoding, const QString &startDir,
                                                                        const QString &filter,
                                                                        QWidget *parent,
                                                                        const QString &caption)
{
    KEncodingFileDialog dlg(
        startDir, encoding, filter,
        caption.isNull() ? i18n("Open") : caption,
        KFileDialog::Opening, parent
    );
    dlg.setMode(KFile::Files);
    dlg.exec();

    Result res;
    res.URLs = dlg.selectedUrls();
    res.encoding = dlg.selectedEncoding();
    return res;
}

KEncodingFileDialog::Result KEncodingFileDialog::getSaveFileNameAndEncoding(const QString &encoding,
                                                                            const QString &dir,
                                                                            const QString &filter,
                                                                            QWidget *parent,
                                                                            const QString &caption)
{
    KEncodingFileDialog dlg(
        dir, encoding, filter,
        caption.isNull() ? i18n("Save As") : caption,
        KFileDialog::Saving, parent
    );
    dlg.setMode(KFile::File);
    dlg.exec();

    Result res;
    res.fileNames << dlg.selectedFile();
    res.encoding = dlg.selectedEncoding();
    return res;
}


KEncodingFileDialog::Result KEncodingFileDialog::getSaveUrlAndEncoding(const QString &encoding,
                                                                       const QString &dir, const  QString &filter,
                                                                       QWidget *parent, const QString &caption)
{
    KEncodingFileDialog dlg(
        dir, encoding, filter,
        caption.isNull() ? i18n("Save As") : caption,
        KFileDialog::Saving, parent
    );
    dlg.setMode(KFile::File);

    Result res;
    if (dlg.exec() == QDialog::Accepted) {
        res.URLs << dlg.selectedUrl();
        res.encoding = dlg.selectedEncoding();
    }
    return res;
}

#include "moc_kencodingfiledialog.cpp"
