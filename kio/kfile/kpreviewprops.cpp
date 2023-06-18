/* This file is part of the KDE libraries
    Copyright (C) 2005 Stephan Binner <binner@kde.org>

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

#include "kpreviewprops.h"
#include <kio/previewjob.h>

#include <QtGui/QLayout>

#include <kimagefilepreview.h>
#include <kglobalsettings.h>
#include <klocale.h>

KPreviewPropsPlugin::KPreviewPropsPlugin(KPropertiesDialog *props)
    : KPropertiesDialogPlugin(props)
{
    if (properties->items().count() > 1) {
        return;
    }
    createLayout();
}

void KPreviewPropsPlugin::createLayout()
{
    // let the dialog create the page frame
    QFrame* topframe = new QFrame();
    properties->addPage(topframe, i18n("P&review"));
    topframe->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout* tmp = new QVBoxLayout(topframe);
    tmp->setMargin(0);

    preview = new KImageFilePreview(topframe);

    tmp->addWidget(preview);
    connect(
        properties, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
        this, SLOT(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*))
    );
}

bool KPreviewPropsPlugin::supports(const KFileItemList &items)
{
    if (items.count() != 1) {
        return false;
    }
    const bool metaDataEnabled = KGlobalSettings::showFilePreview(items.first().url());
    if (!metaDataEnabled) {
        return false;
    }
    const KMimeType::Ptr itemMime = items.first().mimeTypePtr();
    foreach(const QString &it, KIO::PreviewJob::supportedMimeTypes()) {
        if (itemMime->is(it)) {
            return true;
        }

        // glob match for certain thumbnailers, same matching is done in:
        // kdelibs/kio/kio/previewjob.cpp
        // kde-workspace/kioslave/thumbnail/thumbnail.cpp
        if (it.endsWith('*')) {
            const QString mimeGroup = it.left(it.length()-1);
            const QString mimeName = itemMime->name();
            if (mimeName.startsWith(mimeGroup)) {
                return true;
            }
        }
    }
    return false;
}

void KPreviewPropsPlugin::currentPageChanged(KPageWidgetItem *current, KPageWidgetItem *)
{
    if (current->widget() != preview->parent()) {
        return;
    }

    disconnect(
        properties, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
        this, SLOT(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*))
    );
    preview->showPreview(properties->item().url());
}

#include "moc_kpreviewprops.cpp"
