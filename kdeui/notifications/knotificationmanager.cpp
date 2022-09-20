/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>

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

#include <QWidget>
#include <QImageWriter>
#include <QBuffer>

#include "knotificationmanager_p.h"
#include <ktoolinvocation.h>
#include "knotification.h"
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <klocale.h>

#include "knotify_interface.h"

typedef QHash<QString,QString> Dict;

static const QByteArray imageFormat = QImageWriter::defaultImageFormat();

struct KNotificationManager::Private
{
    QHash<int , KNotification*> notifications;
    org::kde::KNotify *knotify;
};

KNotificationManager * KNotificationManager::self()
{
    K_GLOBAL_STATIC(KNotificationManager, s_self)
    return s_self;
}


KNotificationManager::KNotificationManager()
    : d(new Private)
{
    QDBusConnectionInterface* sessionIface = QDBusConnection::sessionBus().interface();
    if (!sessionIface->isServiceRegistered("org.kde.knotify")) {
        QDBusReply<void> sessionReply = sessionIface->startService("org.kde.knotify");
        if (!sessionReply.isValid()) {
            kError() << "Couldn't start knotify service" << sessionReply.error();
        }
    }
    d->knotify =
        new org::kde::KNotify(QLatin1String("org.kde.knotify"), QLatin1String("/Notify"), QDBusConnection::sessionBus(), this);
    connect(d->knotify, SIGNAL(notificationClosed(int)),
                           this, SLOT(notificationClosed(int)));
    connect(d->knotify, SIGNAL(notificationActivated(int,int)),
                           this, SLOT(notificationActivated(int,int)));
}


KNotificationManager::~KNotificationManager()
{
    delete d->knotify;
    delete d;
}

void KNotificationManager::notificationActivated( int id, int action )
{
    if(d->notifications.contains(id))
    {
        kDebug(299) << id << " " << action;
        KNotification *n = d->notifications[id];
        d->notifications.remove(id);
        n->activate( action );
    }
}

void KNotificationManager::notificationClosed( int id )
{
    if(d->notifications.contains(id))
    {
        kDebug( 299 ) << id;
        KNotification *n = d->notifications[id];
        d->notifications.remove(id);
        n->close();
    }
}


void KNotificationManager::close( int id, bool force )
{
    if(force || d->notifications.contains(id)) {
        d->notifications.remove(id);
        kDebug( 299 ) << id;
        d->knotify->closeNotification(id);
    }
}

bool KNotificationManager::notify( KNotification* n, const QPixmap &pix,
                                           const QStringList &actions,
                                           const KNotification::ContextList & contexts,
                                           const QString &appname)
{
    WId winId=n->widget() ? n->widget()->window()->winId()  : 0;

    QByteArray pixmapData;
    QBuffer buffer(&pixmapData);
    buffer.open(QIODevice::WriteOnly);
    pix.save(&buffer, imageFormat);

    QVariantList contextList;
    typedef QPair<QString,QString> Context;
    foreach (const Context& ctx, contexts) {
        QVariantList vl;
        vl << ctx.first << ctx.second;
        contextList << vl;
    }

    // Persistent     => 0  == infinite timeout
    // CloseOnTimeout => -1 == let the server decide
    int timeout = (n->flags() & KNotification::Persistent) ? 0 : -1;

    QList<QVariant>  args;
    args << n->eventId() << (appname.isEmpty() ? KGlobal::mainComponent().componentName() : appname);
    args.append(QVariant(contextList)); 
    args << n->title() << n->text() <<  pixmapData << QVariant(actions) << timeout << qlonglong(winId) ;
    return d->knotify->callWithCallback( "event", args, n, SLOT(slotReceivedId(int)), SLOT(slotReceivedIdError(QDBusError)));
}

void KNotificationManager::insert(KNotification *n, int id)
{
    d->notifications.insert(id, n);
}

void KNotificationManager::update(KNotification * n, int id)
{
    if (id <= 0)
        return;

    QByteArray pixmapData;
    if(!n->pixmap().isNull()) {
        QBuffer buffer(&pixmapData);
        buffer.open(QIODevice::WriteOnly);
        n->pixmap().save(&buffer, imageFormat);
    }

    d->knotify->update(id, n->title(), n->text(), pixmapData , n->actions() );
}

void KNotificationManager::reemit(KNotification * n, int id)
{
    QVariantList contextList;
    typedef QPair<QString,QString> Context;
    foreach (const Context& ctx, n->contexts())
    {
//      kDebug(299) << "add context " << ctx.first << "-" << ctx.second;
        QVariantList vl;
        vl << ctx.first << ctx.second;
        contextList << vl;
    }

    d->knotify->reemit(id, contextList);
}


#include "moc_knotificationmanager_p.cpp"
