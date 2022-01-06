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

#include "kaboutapplicationpersonlistdelegate_p.h"

#include "kaboutapplicationpersonmodel_p.h"
#include "kaboutapplicationpersonlistview_p.h"
#include "kdeui/widgets/ktoolbar.h"
#include "kdeui/actions/kaction.h"
#include "kdeui/icons/kicon.h"

#include <kdecore/io/kdebug.h>
#include <kdecore/kernel/kstandarddirs.h>
#include <kdecore/kernel/ktoolinvocation.h>

#include <QtGui/QApplication>
#include <QtGui/QPainter>

namespace KDEPrivate
{

static const int MAIN_LINKS_HEIGHT = 32;

KAboutApplicationPersonListDelegate::KAboutApplicationPersonListDelegate(
        QAbstractItemView *itemView,
        QObject *parent )
    : KWidgetItemDelegate( itemView, parent )
{
}

QList< QWidget *> KAboutApplicationPersonListDelegate::createItemWidgets() const
{
    QList< QWidget *> list;

    QLabel *textLabel = new QLabel( itemView() );
    list.append( textLabel );


    KToolBar *mainLinks = new KToolBar( itemView(), false, false );

    KAction *emailAction = new KAction( KIcon( "internet-mail" ),
                                        i18nc( "Action to send an email to a contributor", "Email contributor" ),
                                        mainLinks );
    emailAction->setVisible( false );
    mainLinks->addAction( emailAction );
    KAction *homepageAction = new KAction( KIcon( "applications-internet" ),
                                           i18n( "Visit contributor's homepage" ),
                                           mainLinks );
    homepageAction->setVisible( false );
    mainLinks->addAction( homepageAction );

    list.append( mainLinks );

    connect( mainLinks, SIGNAL(actionTriggered(QAction*)),
             this, SLOT(launchUrl(QAction*)) );

    return list;
}

void KAboutApplicationPersonListDelegate::updateItemWidgets( const QList<QWidget *> widgets,
                                                             const QStyleOptionViewItem &option,
                                                             const QPersistentModelIndex &index ) const
{
    const int margin = option.fontMetrics.height() / 2;

    KAboutApplicationPersonProfile profile = index.data().value< KAboutApplicationPersonProfile >();

    QRect wRect = widgetsRect( option, index );

    //Let's fill in the text first...
    QLabel *label = qobject_cast< QLabel * >( widgets.at( TextLabel ) );
    label->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    QString text = buildTextForProfile( profile );

    label->move( wRect.left(), wRect.top() );
    label->resize( wRect.width(), heightForString( text, wRect.width() - margin, option ) + margin );
    label->setWordWrap( true );
    label->setContentsMargins( 0, 0, 0, 0 );
    label->setAlignment( Qt::AlignBottom | Qt::AlignLeft );
    label->setForegroundRole( QPalette::WindowText );

    label->setText( text );

    //And now we fill in the main links (email + homepage)...
    KToolBar *mainLinks = qobject_cast< KToolBar * >( widgets.at( MainLinks ) );
    mainLinks->setIconSize( QSize( 22, 22 ) );
    mainLinks->setContentsMargins( 0, 0, 0, 0 );
    mainLinks->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    KAction *action;
    if( !profile.email().isEmpty() ) {
        action = qobject_cast< KAction * >( mainLinks->actions().at( EmailAction ) );
        action->setToolTip( i18nc( "Action to send an email to a contributor",
                                   "Email contributor\n%1", profile.email() ) );
        action->setData( QString( QLatin1String( "mailto:") + profile.email() ) );
        action->setVisible( true );
    }
    if( !profile.homepage().isEmpty() ) {
        action = qobject_cast< KAction * >( mainLinks->actions().at( HomepageAction ) );
        action->setToolTip( i18n( "Visit contributor's homepage\n%1", profile.homepage().url() ) );
        action->setData( profile.homepage().url() );
        action->setVisible( true );
    }

    mainLinks->resize( QSize( mainLinks->sizeHint().width(), MAIN_LINKS_HEIGHT ) );
    mainLinks->move( wRect.left(), wRect.top() + label->height() );

    itemView()->reset();
}

QSize KAboutApplicationPersonListDelegate::sizeHint( const QStyleOptionViewItem &option,
                                                     const QModelIndex &index ) const
{
    int height = widgetsRect( option, index ).height();

    return QSize( option.fontMetrics.height() * 7, height );
}

void KAboutApplicationPersonListDelegate::paint( QPainter *painter,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index) const
{
    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_Widget, &option, painter, 0);
}

void KAboutApplicationPersonListDelegate::launchUrl( QAction *action ) const
{
    QString url = action->data().toString();
    if( !url.isEmpty() ) {
        if( url.startsWith( "mailto:" ) )
            KToolInvocation::invokeMailer( KUrl( url ) );
        else
            KToolInvocation::invokeBrowser( url );
    }
}

int KAboutApplicationPersonListDelegate::heightForString( const QString &string,
                                                          int lineWidth,
                                                          const QStyleOptionViewItem &option) const
{
    QFontMetrics fm = option.fontMetrics;
    QRect boundingRect = fm.boundingRect(QRect(0, 0, lineWidth, 9999), Qt::AlignLeft |
                                          Qt::AlignBottom | Qt::TextWordWrap, string );
    return boundingRect.height();
}

QString KAboutApplicationPersonListDelegate::buildTextForProfile( const KAboutApplicationPersonProfile &profile ) const
{
    QString text;
    text += QLatin1String("<b>");
    text += i18nc("@item Contributor name in about dialog.", "%1", profile.name());
    text += QLatin1String("</b>");

    if( !profile.task().isEmpty() ) {
        text += QLatin1String("\n<br><i>");
        text += profile.task();
        text += QLatin1String("</i>");
    }

    return text;
}

QRect KAboutApplicationPersonListDelegate::widgetsRect( const QStyleOptionViewItem &option,
                                                        const QPersistentModelIndex &index ) const
{
    KAboutApplicationPersonProfile profile = index.data().value< KAboutApplicationPersonProfile >();
    int margin = option.fontMetrics.height() / 2;

    QRect widgetsRect = QRect( option.rect.left() + margin,
                               margin/2,
                               option.rect.width() - 2*margin,
                               0 );

    int textHeight = heightForString( buildTextForProfile( profile ), widgetsRect.width() - margin, option );
    widgetsRect.setHeight( textHeight + MAIN_LINKS_HEIGHT + 1.5*margin );

    return widgetsRect;
}

} //namespace KDEPrivate
