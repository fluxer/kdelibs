// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  1999,2000,2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>
                  2003 Clarence Dang <dang@kde.org>
                  2008 Jarosław Staniek <staniek@kde.org>
                  2009 David Jarvie <djarvie@kde.org>

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

#include "kfiledialog.h"

#include <QtCore/QTextStream>
#include <QtGui/QCheckBox>
#include <QtGui/qevent.h>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include <kimageio.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <config-kfile.h>
#include <krecentdocument.h>
#include <kdebug.h>
#include <kwindowsystem.h>
#include "kabstractfilewidget.h"
#include "kabstractfilemodule.h"
#include "krecentdirs.h"
#include "kservice.h"

static KAbstractFileModule* s_module = 0;
static const char s_defaultFileModuleName[] = "kfilemodule";

static KAbstractFileModule* loadFileModule( const QString& moduleName )
{
    KService::Ptr fileModuleService = KService::serviceByDesktopName(moduleName);
    if (fileModuleService) {
        return fileModuleService->createInstance<KAbstractFileModule>();
    }
    return nullptr;
}

static KAbstractFileModule* fileModule()
{
    if (!s_module) {
        QString moduleName = KConfig("kdeglobals").group(ConfigGroup).readEntry("file module", s_defaultFileModuleName);
        if (!(s_module = loadFileModule(moduleName))) {
            kDebug() << "Failed to load configured file module" << moduleName;
            if (moduleName != s_defaultFileModuleName) {
                kDebug() << "Falling back to default file module.";
                s_module = loadFileModule(s_defaultFileModuleName);
            }
        }
    }
    return s_module;
}

class KFileDialogPrivate
{
public:
    KFileDialogPrivate()
      : w(nullptr),
        cfgGroup(KGlobal::config(), ConfigGroup)
    {
    }

    KAbstractFileWidget* w;
    KConfigGroup cfgGroup;
};

KFileDialog::KFileDialog(const KUrl &startDir, const QString &filter,
                         QWidget* parent, QWidget* customWidget)
    : KDialog(parent),
    d(new KFileDialogPrivate())

{
    QWidget* fileQWidget = fileModule()->createFileWidget(startDir, this);
    d->w = ::qobject_cast<KAbstractFileWidget *>(fileQWidget);

    setButtons( KDialog::None );
    restoreDialogSize(d->cfgGroup); // call this before the fileQWidget is set as the main widget.
                                   // otherwise the sizes for the components are not obeyed (ereslibre)

    d->w->setFilter(filter);
    setMainWidget(fileQWidget);

    d->w->okButton()->show();
    connect(d->w->okButton(), SIGNAL(clicked()), SLOT(slotOk()));
    d->w->cancelButton()->show();
    connect(d->w->cancelButton(), SIGNAL(clicked()), SLOT(slotCancel()));

    // Publish signals
    // TODO: Move the relevant signal declarations from KFileWidget to the
    //       KAbstractFileWidget interface?
    //       Else, all of these connects (including "accepted") are not typesafe.
    // Answer: you cannot define signals in a non-qobject base class (DF).
    //         I simply documentde them in kabstractfilewidget.h now.
    kDebug (kfile_area) << "KFileDialog connecting signals";
    connect(fileQWidget, SIGNAL(fileSelected(KUrl)),
                         SIGNAL(fileSelected(KUrl)));
    connect(fileQWidget, SIGNAL(fileHighlighted(KUrl)),
                         SIGNAL(fileHighlighted(KUrl)));
    connect(fileQWidget, SIGNAL(selectionChanged()),
                         SIGNAL(selectionChanged()));
    connect(fileQWidget, SIGNAL(filterChanged(QString)),
                         SIGNAL(filterChanged(QString)));

    connect(fileQWidget, SIGNAL(accepted()), SLOT(accept()));
    //connect(fileQWidget, SIGNAL(canceled()), SLOT(slotCancel()));

    if (customWidget) {
        d->w->setCustomWidget(QString(), customWidget);
    }
}


KFileDialog::~KFileDialog()
{
    delete d;
}

void KFileDialog::setLocationLabel(const QString &text)
{
    d->w->setLocationLabel(text);
}

void KFileDialog::setFilter(const QString &filter)
{
    d->w->setFilter(filter);
}

QString KFileDialog::currentFilter() const
{
    return d->w->currentFilter();
}

void KFileDialog::setMimeFilter(const QStringList &mimeTypes, const QString &defaultType)
{
    d->w->setMimeFilter(mimeTypes, defaultType);
}

void KFileDialog::clearFilter()
{
    d->w->clearFilter();
}

QString KFileDialog::currentMimeFilter() const
{
    return d->w->currentMimeFilter();
}

KMimeType::Ptr KFileDialog::currentFilterMimeType()
{
    return KMimeType::mimeType(currentMimeFilter());
}

void KFileDialog::setPreviewWidget(KPreviewWidgetBase *w)
{
    d->w->setPreviewWidget(w);
}

void KFileDialog::setInlinePreviewShown(bool show)
{
    d->w->setInlinePreviewShown(show);
}

// This is only used for the initial size when no configuration has been saved
QSize KFileDialog::sizeHint() const
{
    int fontSize = fontMetrics().height();
    QSize goodSize(48 * fontSize, 30 * fontSize);
    QSize screenSize = QApplication::desktop()->availableGeometry(this).size();
    QSize minSize(screenSize / 2);
    QSize maxSize(screenSize * qreal(0.9));
    return (goodSize.expandedTo(minSize).boundedTo(maxSize));
}

// This slot still exists mostly for compat purposes; for subclasses which reimplement slotOk
void KFileDialog::slotOk()
{
    d->w->slotOk();
}

// This slot still exists mostly for compat purposes; for subclasses which reimplement accept
void KFileDialog::accept()
{
    setResult(QDialog::Accepted); // keep old behavior; probably not needed though
    d->w->accept();
    KConfigGroup cfgGroup(KGlobal::config(), ConfigGroup);
    KDialog::accept();
    emit okClicked();
}

// This slot still exists mostly for compat purposes; for subclasses which reimplement slotCancel
void KFileDialog::slotCancel()
{
    d->w->slotCancel();
    reject();
}

void KFileDialog::setUrl(const KUrl &url, bool clearforward)
{
    d->w->setUrl(url, clearforward);
}

void KFileDialog::setSelection(const QString &name)
{
    d->w->setSelection(name);
}

QString KFileDialog::getOpenFileName(const KUrl &startDir,
                                     const QString &filter,
                                     QWidget *parent, const QString &caption)
{
    KFileDialog dlg(startDir, filter, parent);

    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setMode(KFile::File | KFile::LocalOnly | KFile::ExistingOnly);
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();
    return dlg.selectedFile();
}

QString KFileDialog::getOpenFileNameWId(const KUrl& startDir,
                                        const QString &filter,
                                        WId parent_id, const QString &caption)
{
    QWidget* parent = QWidget::find(parent_id);
    KFileDialog dlg(startDir, filter, parent);
    if (parent == NULL && parent_id != 0) {
        KWindowSystem::setMainWindow(&dlg, parent_id);
    }

    dlg.setOperationMode(KFileDialog::Opening );
    dlg.setMode( KFile::File | KFile::LocalOnly | KFile::ExistingOnly);
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();

    return dlg.selectedFile();
}

QStringList KFileDialog::getOpenFileNames(const KUrl &startDir,
                                          const QString &filter,
                                          QWidget *parent,
                                          const QString &caption)
{
    KFileDialog dlg(startDir, filter, parent);

    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setMode(KFile::Files | KFile::LocalOnly | KFile::ExistingOnly);
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();
    return dlg.selectedFiles();
}

KUrl KFileDialog::getOpenUrl(const KUrl &startDir, const QString &filter,
                             QWidget *parent, const QString &caption)
{
    KFileDialog dlg(startDir, filter, parent);

    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setMode(KFile::File | KFile::ExistingOnly);
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();
    return dlg.selectedUrl();
}

KUrl::List KFileDialog::getOpenUrls(const KUrl &startDir,
                                    const QString &filter,
                                    QWidget *parent,
                                    const QString &caption)
{
    KFileDialog dlg(startDir, filter, parent);

    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setMode( KFile::Files | KFile::ExistingOnly);
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();
    return dlg.selectedUrls();
}

void KFileDialog::setConfirmOverwrite(bool enable)
{
    if (operationMode() == KFileDialog::Saving) {
        d->w->setConfirmOverwrite(enable);
    }
}

KUrl KFileDialog::getExistingDirectoryUrl(const KUrl &startDir,
                                          QWidget *parent,
                                          const QString &caption)
{
    return fileModule()->selectDirectory(startDir, false, parent, caption);
}

QString KFileDialog::getExistingDirectory(const KUrl &startDir,
                                          QWidget *parent,
                                          const QString &caption)
{
    KUrl url = fileModule()->selectDirectory(startDir, true, parent, caption);
    if (url.isValid()) {
        return url.toLocalFile();
    }
    return QString();
}

KUrl KFileDialog::getImageOpenUrl(const KUrl &startDir, QWidget *parent,
                                  const QString &caption)
{
    const QStringList mimetypes = KImageIO::mimeTypes(KImageIO::Reading);
    KFileDialog dlg(startDir, mimetypes.join(" "), parent);

    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setMode(KFile::File | KFile::ExistingOnly);
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);
    dlg.setInlinePreviewShown(true);

    dlg.exec();

    return dlg.selectedUrl();
}

KUrl KFileDialog::selectedUrl() const
{
    return d->w->selectedUrl();
}

KUrl::List KFileDialog::selectedUrls() const
{
    return d->w->selectedUrls();
}

QString KFileDialog::selectedFile() const
{
    return d->w->selectedFile();
}

QStringList KFileDialog::selectedFiles() const
{
    return d->w->selectedFiles();
}

KUrl KFileDialog::baseUrl() const
{
    return d->w->baseUrl();
}

QString KFileDialog::getSaveFileName(const KUrl &dir, const QString &filter,
                                     QWidget *parent,
                                     const QString &caption, Options options)
{
    KFileDialog dlg(dir, filter, parent);

    dlg.setOperationMode(KFileDialog::Saving);
    dlg.setMode(KFile::File | KFile::LocalOnly);
    dlg.setConfirmOverwrite(options & KFileDialog::ConfirmOverwrite);
    dlg.setInlinePreviewShown(options & KFileDialog::ShowInlinePreview);
    dlg.setCaption(caption.isEmpty() ? i18n("Save As") : caption);

    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty()) {
        KRecentDocument::add(filename);
    }

    return filename;
}

QString KFileDialog::getSaveFileNameWId(const KUrl &dir, const QString &filter,
                                        WId parent_id,
                                        const QString &caption, Options options)
{
    QWidget* parent = QWidget::find(parent_id);
    KFileDialog dlg(dir, filter, parent);
    if (parent == NULL && parent_id != 0) {
        KWindowSystem::setMainWindow(&dlg, parent_id);
    }

    dlg.setOperationMode(KFileDialog::Saving);
    dlg.setMode(KFile::File | KFile::LocalOnly);
    dlg.setConfirmOverwrite(options & ConfirmOverwrite);
    dlg.setInlinePreviewShown(options & ShowInlinePreview);
    dlg.setCaption(caption.isEmpty() ? i18n("Save As") : caption);

    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty()) {
        KRecentDocument::add(filename);
    }

    return filename;
}

KUrl KFileDialog::getSaveUrl(const KUrl &dir, const QString &filter,
                             QWidget *parent, const QString &caption, Options options)
{
    KFileDialog dlg(dir, filter, parent);

    dlg.setOperationMode(KFileDialog::Saving);
    dlg.setMode(KFile::File);
    dlg.setConfirmOverwrite(options & KFileDialog::ConfirmOverwrite);
    dlg.setInlinePreviewShown(options & KFileDialog::ShowInlinePreview);
    dlg.setCaption(caption.isEmpty() ? i18n("Save As") : caption);

    dlg.exec();

    KUrl url = dlg.selectedUrl();
    if (url.isValid()) {
        KRecentDocument::add(url);
    }

    return url;
}

void KFileDialog::setMode(KFile::Modes m)
{
    d->w->setMode(m);
}

KFile::Modes KFileDialog::mode() const
{
    return d->w->mode();
}

KPushButton * KFileDialog::okButton() const
{
    return d->w->okButton();
}

KPushButton * KFileDialog::cancelButton() const
{
    return d->w->cancelButton();
}

KUrlComboBox* KFileDialog::locationEdit() const
{
    return d->w->locationEdit();
}

KFileFilterCombo* KFileDialog::filterWidget() const
{
    return d->w->filterWidget();
}

KActionCollection * KFileDialog::actionCollection() const
{
    return d->w->actionCollection();
}

void KFileDialog::setKeepLocation(bool keep)
{
    d->w->setKeepLocation(keep);
}

bool KFileDialog::keepsLocation() const
{
    return d->w->keepsLocation();
}

void KFileDialog::setOperationMode(OperationMode mode)
{
    d->w->setOperationMode(static_cast<KAbstractFileWidget::OperationMode>(mode));
}

KFileDialog::OperationMode KFileDialog::operationMode() const
{
    return static_cast<KFileDialog::OperationMode>(d->w->operationMode());
}

void KFileDialog::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        e->accept();
        d->w->cancelButton()->animateClick();
    } else {
        KDialog::keyPressEvent(e);
    }
}

void KFileDialog::hideEvent(QHideEvent* e)
{
    saveDialogSize(d->cfgGroup, KConfigBase::Persistent);

    KDialog::hideEvent(e);
}

// static
KUrl KFileDialog::getStartUrl(const KUrl &startDir, QString &recentDirClass)
{
    return fileModule()->getStartUrl(startDir, recentDirClass);
}

void KFileDialog::setStartDir(const KUrl& directory)
{
    fileModule()->setStartDir(directory);
}

KToolBar* KFileDialog::toolBar() const
{
    return d->w->toolBar();
}

KAbstractFileWidget* KFileDialog::fileWidget()
{
    return d->w;
}

#include "moc_kfiledialog.cpp"
