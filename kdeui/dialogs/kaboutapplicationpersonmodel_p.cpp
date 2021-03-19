/* This file is part of the KDE libraries
   Copyright (C) 2010 Teo Mrnjavac <teo@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kaboutapplicationpersonmodel_p.h"


#include <kdebug.h>
#include <kaboutdata.h>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>

namespace KDEPrivate
{

KAboutApplicationPersonModel::KAboutApplicationPersonModel( const QList< KAboutPerson > &personList,
                                                            QObject *parent )
    : QAbstractListModel( parent )
    , m_personList( personList )
{
    foreach( const KAboutPerson person, personList )
    {
        KAboutApplicationPersonProfile profile =
                KAboutApplicationPersonProfile( person.name(),
                                                person.task(),
                                                person.emailAddress());
        profile.setHomepage( person.webAddress() );
        m_profileList.append( profile );
    }

}

int KAboutApplicationPersonModel::rowCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent )
    return m_personList.count();
}

QVariant KAboutApplicationPersonModel::data( const QModelIndex &index, int role ) const
{
    if( !index.isValid() ) {
        kWarning()<<"ERROR: invalid index";
        return QVariant();
    }
    if( index.row() >= rowCount() ) {
        kWarning()<<"ERROR: index out of bounds";
        return QVariant();
    }
    if( role == Qt::DisplayRole ) {
//        kDebug() << "Spitting data for name " << m_profileList.at( index.row() ).name();
        QVariant var;
        var.setValue( m_profileList.at( index.row() ) );
        return var;
    }
    else {
        return QVariant();
    }
}

Qt::ItemFlags KAboutApplicationPersonModel::flags( const QModelIndex &index ) const
{
    if( index.isValid() )
        return Qt::ItemIsEnabled;
    return QAbstractListModel::flags( index ) | Qt::ItemIsEditable;
}

} //namespace KDEPrivate

#include "moc_kaboutapplicationpersonmodel_p.cpp"
