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
                                                            const QString &providerUrl,
                                                            QObject *parent )
    : QAbstractListModel( parent )
    , m_personList( personList )
    , m_hasAvatarPixmaps( false )
    , m_providerUrl( providerUrl )
{
    if( m_providerUrl.isEmpty() )
        m_providerUrl = QString( "https://api.opendesktop.org/v1/" );

    foreach( const KAboutPerson person, personList )
    {
        KAboutApplicationPersonProfile profile =
                KAboutApplicationPersonProfile( person.name(),
                                                person.task(),
                                                person.emailAddress(),
                                                person.ocsUsername() );
        profile.setHomepage( person.webAddress() );
        m_profileList.append( profile );
    }

    m_ocsLinkIcons.insert( KAboutApplicationPersonProfileOcsLink::Other, KIcon( "applications-internet").pixmap( 16 ) );
    m_ocsLinkIcons.insert( KAboutApplicationPersonProfileOcsLink::Blog, KIcon( "applications-internet" ).pixmap( 16 ) );
    m_ocsLinkIcons.insert( KAboutApplicationPersonProfileOcsLink::Homepage, KIcon( "applications-internet" ).pixmap( 16 ) );

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

void KAboutApplicationPersonModel::onProvidersLoaded()   //SLOT
{
}

void KAboutApplicationPersonModel::onPersonJobFinished( Attica::BaseJob *job )   //SLOT
{
    Q_UNUSED( job )
}

void KAboutApplicationPersonModel::onAvatarJobFinished( QNetworkReply *reply )  //SLOT
{
    Q_UNUSED( reply )
}

void KAboutApplicationPersonModel::fetchOcsLinkIcons( int personProfileListIndex )
{
    KAboutApplicationPersonProfile profile = m_profileList.value( personProfileListIndex );
    QList< KAboutApplicationPersonProfileOcsLink > ocsLinks = profile.ocsLinks();

    KAboutApplicationPersonIconsJob *job =
            new KAboutApplicationPersonIconsJob( this, personProfileListIndex );
    connect( job, SIGNAL(finished(KAboutApplicationPersonIconsJob*)),
             this, SLOT(onOcsLinksJobFinished(KAboutApplicationPersonIconsJob*)) );
    job->start();
}

void KAboutApplicationPersonModel::onOcsLinksJobFinished( KAboutApplicationPersonIconsJob *job )   //SLOT
{
    int personProfileListIndex = job->personProfileListIndex();
    KAboutApplicationPersonProfile profile = m_profileList.value( personProfileListIndex );

    profile.setOcsLinks( job->ocsLinks() );

    m_profileList.replace( personProfileListIndex, profile );

    emit dataChanged( index( personProfileListIndex ), index( personProfileListIndex ) );
    emit layoutChanged();
}

KAboutApplicationPersonProfileOcsLink::Type KAboutApplicationPersonProfileOcsLink::typeFromAttica( const QString &atticaType )
{
    int index = -1;
    for( int i = 0; i < NUM_ATTICA_LINK_TYPES; ++i ) {
        if( atticaType == QString( s_personOcsLinkAtticaTypes[i] ) ) {
            index = i;
            break;
        }
    }
    if( index == -1 )
        return Other;
    else
        return static_cast< Type >( index );
}

QString KAboutApplicationPersonProfileOcsLink::prettyType() const
{
    //This can't be static like the other lists because of i18n
    switch( m_type ) {
    case Other:
        return i18nc( "A generic social network or homepage link of an unlisted type.", "Other" );
    case Blog:
        return i18nc( "A type of link.", "Blog" );
    case Delicious:
        return "Delicious";
    case Digg:
        return "Digg";
    case Facebook:
        return "Facebook";
    case Homepage:
        return i18nc( "A type of link.", "Homepage" );
    case Identica:
        return "Identi.ca";
    case LibreFm:
        return "Libre.fm";
    case LinkedIn:
        return "LinkedIn";
    case MySpace:
        return "MySpace";
    case Reddit:
        return "Reddit";
    case StackOverflow:
        return "Stack Overflow";
    case Twitter:
        return "Twitter";
    case Wikipedia:
        return "Wikipedia";
    case Xing:
        return "Xing";
    case YouTube:
        return "YouTube";
    case NUM_ATTICA_LINK_TYPES: // silence compiler warning
        break;
    }
    return QString();
}

KAboutApplicationPersonIconsJob::KAboutApplicationPersonIconsJob( KAboutApplicationPersonModel *model,
                                                                  int personProfileListIndex )
    : QObject( model )
    , m_personProfileListIndex( personProfileListIndex )
    , m_model( model )
{
    m_manager = new QNetworkAccessManager( this );
    connect( m_manager, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(onJobFinished(QNetworkReply*)) );
    m_ocsLinks = model->m_profileList.value( personProfileListIndex ).ocsLinks();
}

void KAboutApplicationPersonIconsJob::start()
{
    getIcons( 0 );
}

void KAboutApplicationPersonIconsJob::getIcons( int i )
{
    for( QList< KAboutApplicationPersonProfileOcsLink >::iterator it = m_ocsLinks.begin() + i;
         it != m_ocsLinks.end(); ++it ) {
        if( m_model->m_ocsLinkIcons.contains( it->type() ) ) {
            it->setIcon( m_model->m_ocsLinkIcons.value( it->type() ) );
        }
        else if( m_model->m_ocsLinkIconUrls.contains( it->type() ) )
        {
            QNetworkReply *reply =
                m_manager->get( QNetworkRequest( QUrl( m_model->m_ocsLinkIconUrls.value( it->type() ) ) ) );
            reply->setProperty( "linkIndex", i );
            return;
        }
        ++i;
    }
    emit finished( this );
}

void KAboutApplicationPersonIconsJob::onJobFinished( QNetworkReply *reply ) //SLOT
{
    int i = reply->property( "linkIndex" ).toInt();
    KAboutApplicationPersonProfileOcsLink::Type type = m_ocsLinks.at( i ).type();

    if( reply->error() != QNetworkReply::NoError ) {
        kDebug() << "Could not fetch OCS link icon.";
        reply->deleteLater();
        getIcons( i + 1 );
        return;
    }
    QByteArray data = reply->readAll();
    QPixmap pixmap;
    pixmap.loadFromData( data );

    if( !pixmap.isNull() && !m_model->m_ocsLinkIcons.contains( type ) ) {
        m_model->m_ocsLinkIcons.insert( type, pixmap );
    }
    reply->deleteLater();
    getIcons( i );
}

} //namespace KDEPrivate

#include "moc_kaboutapplicationpersonmodel_p.cpp"
