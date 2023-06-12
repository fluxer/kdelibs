/*******************************************************************************
 *   Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>                      *
 *                                                                             *
 *   This library is free software; you can redistribute it and/or             *
 *   modify it under the terms of the GNU Library General Public               *
 *   License as published by the Free Software Foundation; either              *
 *   version 2 of the License, or (at your option) any later version.          *
 *                                                                             *
 *   This library is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *   Library General Public License for more details.                          *
 *                                                                             *
 *   You should have received a copy of the GNU Library General Public License *
 *   along with this library; see the file COPYING.LIB.  If not, write to      *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                               *
 *******************************************************************************/

#include "kdefaultviewadapter_p.h"

#include <QAbstractItemView>
#include <QScrollBar>

KDefaultViewAdapter::KDefaultViewAdapter(QAbstractItemView* view, QObject* parent) :
    KAbstractViewAdapter(parent),
    m_view(view)
{
}

QAbstractItemModel *KDefaultViewAdapter::model() const
{
    return m_view->model();
}

QSize KDefaultViewAdapter::iconSize() const
{
    return m_view->iconSize();
}

QPalette KDefaultViewAdapter::palette() const
{
    return m_view->palette();
}

QRect KDefaultViewAdapter::visibleArea() const
{
    return m_view->viewport()->rect();
}

QRect KDefaultViewAdapter::visualRect(const QModelIndex& index) const
{
    return m_view->visualRect(index);
}

void KDefaultViewAdapter::connectScrollBar(QObject* receiver, const char* slot)
{
    QObject::connect(m_view->horizontalScrollBar(), SIGNAL(valueChanged(int)), receiver, slot);
    QObject::connect(m_view->verticalScrollBar(), SIGNAL(valueChanged(int)), receiver, slot);
}
