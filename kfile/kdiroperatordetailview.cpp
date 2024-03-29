/*****************************************************************************
 * Copyright (C) 2007 by Peter Penz <peter.penz@gmx.at>                      *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License version 2 as published by the Free Software Foundation.           *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "kdiroperatordetailview_p.h"

#include <kdirlister.h>
#include <kdirmodel.h>
#include <kdebug.h>

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QAbstractProxyModel>
#include <QtGui/QApplication>
#include <QtGui/QHeaderView>
#include <QtGui/QListView>
#include <QtGui/qevent.h>
#include <QtGui/QScrollBar>

KDirOperatorDetailView::KDirOperatorDetailView(QWidget *parent) :
    QTreeView(parent),
    m_resizeColumns(true), m_hideDetailColumns(false)
{
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setSortingEnabled(true);
    setUniformRowHeights(true);
    setDragDropMode(QListView::DragOnly);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QListView::ScrollPerPixel);
    setHorizontalScrollMode(QListView::ScrollPerPixel);
}

void KDirOperatorDetailView::setModel(QAbstractItemModel *model)
{
    if (model->rowCount() == 0) {
        // The model is empty. Assure that the columns get automatically resized
        // until the loading has been finished.
        QAbstractProxyModel *proxyModel = static_cast<QAbstractProxyModel*>(model);
        KDirModel *dirModel = static_cast<KDirModel*>(proxyModel->sourceModel());
        connect(dirModel->dirLister(), SIGNAL(completed()),
                this, SLOT(resetResizing()));
    } else {
        resetResizing();
    }

    connect(model, SIGNAL(layoutChanged()), this, SLOT(slotLayoutChanged()));

    QTreeView::setModel(model);
}

bool KDirOperatorDetailView::setViewMode(KFile::FileView viewMode)
{
    if (KFile::isDetailView(viewMode)) {
        m_hideDetailColumns = false;
    } else {
        return false;
    }

    return true;
}

bool KDirOperatorDetailView::event(QEvent *event)
{
    if (event->type() == QEvent::Polish) {
        QHeaderView *headerView = header();
        headerView->setResizeMode(QHeaderView::Interactive);
        headerView->setStretchLastSection(true);
        headerView->setMovable(false);

        setColumnHidden(KDirModel::Size, m_hideDetailColumns);
        setColumnHidden(KDirModel::ModifiedTime, m_hideDetailColumns);
        hideColumn(KDirModel::Type);
        hideColumn(KDirModel::Permissions);
        hideColumn(KDirModel::Owner);
        hideColumn(KDirModel::Group);
    }
    else if (event->type() == QEvent::UpdateRequest) {
        // A wheel movement will scroll 4 items
        if (model()->rowCount())
            verticalScrollBar()->setSingleStep((sizeHintForRow(0) / 3) * 4);
    }

    return QTreeView::event(event);
}

void KDirOperatorDetailView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void KDirOperatorDetailView::resizeEvent(QResizeEvent *event)
{
    QTreeView::resizeEvent(event);
    slotLayoutChanged();
}

void KDirOperatorDetailView::mousePressEvent(QMouseEvent *event)
{
    QTreeView::mousePressEvent(event);

    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid() || (index.column() != KDirModel::Name)) {
        const Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
        if (!(modifiers & Qt::ShiftModifier) && !(modifiers & Qt::ControlModifier)) {
            clearSelection();
        }
    }
}

void KDirOperatorDetailView::resetResizing()
{
    QTimer::singleShot(300, this, SLOT(disableColumnResizing()));
}

void KDirOperatorDetailView::disableColumnResizing()
{
    m_resizeColumns = false;
}

void KDirOperatorDetailView::slotLayoutChanged()
{
    if (m_resizeColumns) {
        QHeaderView *headerView = header();
        headerView->resizeSections(QHeaderView::ResizeToContents);

        // calculate the required width for all columns except the name column
        int requiredWidth = 0;
        const int count = headerView->count();
        for (int i = 1; i < count; ++i) {
            requiredWidth += headerView->sectionSize(i);
        }

        // try to stretch the name column if enough width is available
        int oldNameColumnWidth = headerView->sectionSize(KDirModel::Name);
        int nameColumnWidth = viewport()->width() - requiredWidth;
        if (nameColumnWidth < oldNameColumnWidth) {
            nameColumnWidth = oldNameColumnWidth;
        }
        headerView->resizeSection(KDirModel::Name, nameColumnWidth);
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
