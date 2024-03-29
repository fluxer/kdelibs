/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KLINKITEMSELECTIONMODEL_H
#define KLINKITEMSELECTIONMODEL_H

#include <QtGui/QItemSelectionModel>
#include <QtGui/QAbstractProxyModel>

#include "kdeui_export.h"

class KLinkItemSelectionModelPrivate;

/**
  @brief Makes it possible to share a selection in multiple views which do not have the same source model

  Although <a href="http://doc.trolltech.com/4.6/model-view-view.html#sharing-selections-between-views">multiple views can share the same QItemSelectionModel</a>, the views then need to have the same source model.

  If there is a proxy model between the model and one of the views, or different proxy models in each, this class makes
  it possible to share the selection between the views.

  @image html kproxyitemselectionmodel-simple.png "Sharing a QItemSelectionModel between views on the same model is trivial"
  @image html kproxyitemselectionmodel-error.png "If a proxy model is used, it is no longer possible to share the QItemSelectionModel directly"
  @image html kproxyitemselectionmodel-solution.png "A KLinkItemSelectionModel can be used to map the selection through the proxy model"

  @code
    QAbstractItemModel *model = getModel();

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel();
    proxy->setSourceModel(model);

    QTreeView *view1 = new QTreeView(splitter);
    view1->setModel(model);

    KLinkItemSelectionModel *view2SelectionModel = new KLinkItemSelectionModel( proxy, view1->selectionModel());

    QTreeView *view2 = new QTreeView(splitter);
    // Note that the QAbstractItemModel passed to KLinkItemSelectionModel must be the same as what is used in the view
    view2->setModel(proxy);
    view2->setSelectionModel( view2SelectionModel );
  @endcode

  @image html kproxyitemselectionmodel-complex.png "Arbitrarily complex proxy configurations on the same root model can be used"

  @code
    QAbstractItemModel *model = getModel();

    QSortFilterProxyModel *proxy1 = new QSortFilterProxyModel();
    proxy1->setSourceModel(model);
    QSortFilterProxyModel *proxy2 = new QSortFilterProxyModel();
    proxy2->setSourceModel(proxy1);
    QSortFilterProxyModel *proxy3 = new QSortFilterProxyModel();
    proxy3->setSourceModel(proxy2);

    QTreeView *view1 = new QTreeView(splitter);
    view1->setModel(proxy3);

    QSortFilterProxyModel *proxy4 = new QSortFilterProxyModel();
    proxy4->setSourceModel(model);
    QSortFilterProxyModel *proxy5 = new QSortFilterProxyModel();
    proxy5->setSourceModel(proxy4);

    KLinkItemSelectionModel *view2SelectionModel = new KLinkItemSelectionModel( proxy5, view1->selectionModel());

    QTreeView *view2 = new QTreeView(splitter);
    // Note that the QAbstractItemModel passed to KLinkItemSelectionModel must be the same as what is used in the view
    view2->setModel(proxy5);
    view2->setSelectionModel( view2SelectionModel );
  @endcode

  @since 4.5
  @author Stephen Kelly <steveire@gmail.com>

*/
class KDEUI_EXPORT KLinkItemSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    /**
      Constructor.
    */
    KLinkItemSelectionModel(QAbstractItemModel *targetModel, QItemSelectionModel *linkedItemSelectionModel, QObject *parent = 0);
    ~KLinkItemSelectionModel();
    /* reimp */ void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command);
    /* reimp */ void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command);

protected:
    KLinkItemSelectionModelPrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE(KLinkItemSelectionModel)
    Q_PRIVATE_SLOT( d_func(), void sourceSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected))
    Q_PRIVATE_SLOT( d_func(), void sourceCurrentChanged(const QModelIndex &current))
    Q_PRIVATE_SLOT( d_func(), void slotCurrentChanged(const QModelIndex &current))
};

#endif
