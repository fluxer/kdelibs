/*
    Copyright (c) 2013 Aurélien Gateau <agateau@kde.org>

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
#ifndef KLINKITEMSELECTIONMODELTEST_H
#define KLINKITEMSELECTIONMODELTEST_H

#include <QtCore/QObject>

#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class KLinkItemSelectionModel;

class KLinkItemSelectionModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void testToggle();
    void testMainSetCurrent();
    void testSubSetCurrent();

private:
    QStandardItemModel *m_mainModel;
    QItemSelectionModel *m_mainSelectionModel;
    QSortFilterProxyModel *m_subModel;
    KLinkItemSelectionModel *m_subSelectionModel;
};

#endif /* KLINKITEMSELECTIONMODELTEST_H */
