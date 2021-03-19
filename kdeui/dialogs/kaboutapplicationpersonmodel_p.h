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

#ifndef KABOUT_APPLICATION_PERSON_MODEL_H
#define KABOUT_APPLICATION_PERSON_MODEL_H

#include "kdeui/icons/kicon.h"

#include <kaboutdata.h>
#include <kurl.h>

#include <QtCore/qabstractitemmodel.h>
#include <QtGui/QPixmap>
#include <QtNetwork/QNetworkReply>

// Forward declarations to make Attica-related members work
namespace Attica {
class BaseJob;
}

namespace KDEPrivate
{

class KAboutApplicationPersonProfile;

class KAboutApplicationPersonModel : public QAbstractListModel
{
    Q_OBJECT
public:
    KAboutApplicationPersonModel( const QList< KAboutPerson > &personList,
                                  QObject *parent = 0 );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount(const QModelIndex &parent = QModelIndex() ) const { Q_UNUSED( parent ) return 1; }
    QVariant data( const QModelIndex &index, int role ) const;

    Qt::ItemFlags flags( const QModelIndex &index ) const;

private:
    QList< KAboutPerson > m_personList;
    QList< KAboutApplicationPersonProfile > m_profileList;
};

class KAboutApplicationPersonProfile
{
public:
    KAboutApplicationPersonProfile()
        : m_name()
        , m_task()
        , m_email()
    {} //needed for QVariant

    KAboutApplicationPersonProfile( const QString &name,
                                    const QString &task,
                                    const QString &email)
        : m_name( name )
        , m_task( task )
        , m_email( email )
    {}

    void setHomepage( const KUrl &url ) { m_homepage = url; }

    const QString & name() const { return m_name; }
    const QString & task() const { return m_task; }
    const QString & email() const { return m_email; }
    const KUrl & homepage() const { return m_homepage; }

private:
    QString m_name;
    QString m_task;
    QString m_email;
    KUrl m_homepage;
};

} //namespace KDEPrivate

Q_DECLARE_METATYPE( KDEPrivate::KAboutApplicationPersonProfile )
#endif // KABOUT_APPLICATION_PERSON_MODEL_H
