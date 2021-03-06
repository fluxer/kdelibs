/* -*- C++ -*-

   This file declares the SMIVModel class. 

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SMIVModel.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef SMIVMODEL_H
#define SMIVMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/QList>

class SMIVItem;

class SMIVModel : public QAbstractListModel
{
    Q_OBJECT
public:
    SMIVModel( QObject* parent = 0 );
    ~SMIVModel();

    void insert ( const SMIVItem *item);

    QVariant headerData ( int section, Qt::Orientation orientation,
                          int role = Qt::DisplayRole ) const ;

    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

    const SMIVItem* data ( int index ) const;

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

private:
    QList<const SMIVItem*> m_data;
};

#endif // SMIVMODEL_H
