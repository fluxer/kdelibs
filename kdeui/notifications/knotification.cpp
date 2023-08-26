/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "knotification.h"
#include "kglobal.h"
#include "kcomponentdata.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kstandarddirs.h"
#include "kwindowsystem.h"
#include "kdbusconnectionpool.h"
#include "kiconloader.h"
#include "kpassivepopup.h"
#include "kdebug.h"

#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTimer>

// for reference:
// https://specifications.freedesktop.org/notification-spec/notification-spec-latest.html

// see kdebug.areas
static const int s_knotificationarea = 299;
static const QString s_notifications = QString::fromLatin1("org.freedesktop.Notifications");

class KNotificationManager : public QObject
{
    Q_OBJECT
public:
    KNotificationManager();

    void send(KNotification *notification, const bool persistent);
    void close(KNotification *notification);

private Q_SLOTS:
    void slotNotificationClosed(uint eventid, uint reason);
    void slotActionInvoked(uint eventid, const QString &action);

private:
    KConfig m_config;
    QDBusInterface* m_notificationsiface;
    QDBusInterface* m_kaudioplayeriface;
    QMap<KNotification*,uint> m_notifications;
};
K_GLOBAL_STATIC(KNotificationManager, kNotificationManager);

KNotificationManager::KNotificationManager()
    : m_config("knotificationrc", KConfig::NoGlobals),
    m_notificationsiface(nullptr),
    m_kaudioplayeriface(nullptr)
{
    // TODO: config watch
    const QStringList notifyconfigs = KGlobal::dirs()->findAllResources("config", "notifications/*.notifyrc");
    if (!notifyconfigs.isEmpty()) {
        m_config.addConfigSources(notifyconfigs);
    }
    // qDebug() << Q_FUNC_INFO << notifyconfigs;
}

void KNotificationManager::send(KNotification *notification, const bool persistent)
{
    const QString eventid = notification->eventID();
    const QStringList spliteventid = eventid.split(QLatin1Char('/'));
    // qDebug() << Q_FUNC_INFO << spliteventid;
    if (spliteventid.size() != 2) {
        kWarning(s_knotificationarea) << "invalid notification ID" << eventid;
        return;
    }
    KConfigGroup globalgroup(&m_config, spliteventid.at(0));
    KConfigGroup eventgroup(&m_config, eventid);
    QString eventtitle = notification->title();
    if (eventtitle.isEmpty()) {
        eventtitle = eventgroup.readEntry("Comment");
    }
    if (eventtitle.isEmpty()) {
        eventtitle = globalgroup.readEntry("Comment");
    }

    QString eventtext = notification->text();
    if (eventtext.isEmpty()) {
        eventtext = eventgroup.readEntry("Name");
    }
    if (eventtext.isEmpty()) {
        eventtext = globalgroup.readEntry("Name");
    }

    QString eventicon = notification->icon();
    if (eventicon.isEmpty()) {
        eventicon = eventgroup.readEntry("IconName");
    }
    if (eventicon.isEmpty()) {
        eventicon = globalgroup.readEntry("IconName");
    }

    QStringList eventactions = eventgroup.readEntry("Actions", QStringList());
    if (eventactions.isEmpty()) {
        eventactions = globalgroup.readEntry("Actions", QStringList());
    }
    // qDebug() << Q_FUNC_INFO << eventactions << notification->actions();
    if (eventactions.contains(QString::fromLatin1("Popup"))) {
        if (!m_notificationsiface
            && KDBusConnectionPool::isServiceRegistered(s_notifications, QDBusConnection::sessionBus())) {
            m_notificationsiface = new QDBusInterface(
                s_notifications, "/org/freedesktop/Notifications", s_notifications,
                QDBusConnection::sessionBus(), this
            );
            connect(m_notificationsiface, SIGNAL(NotificationClosed(uint,uint)), this, SLOT(slotNotificationClosed(uint,uint)));
            connect(m_notificationsiface, SIGNAL(ActionInvoked(uint,QString)), this, SLOT(slotActionInvoked(uint,QString)));
        }
        const int eventtimeout = (persistent ? 0 : -1);
        if (!m_notificationsiface || !m_notificationsiface->isValid()) {
            kWarning(s_knotificationarea) << "notifications interface is not valid";
            const QPixmap eventpixmap = KIconLoader::global()->loadIcon(eventicon, KIconLoader::Small);
            KPassivePopup* kpassivepopup = new KPassivePopup(notification->widget());
            kpassivepopup->setTimeout(eventtimeout);
            kpassivepopup->setView(eventtitle, eventtext, eventpixmap);
            kpassivepopup->setAutoDelete(true);
            // NOTE: KPassivePopup positions itself depending on the windows
            kpassivepopup->show();
        } else {
            const uint eventid = m_notifications.value(notification, 0);
            // NOTE: there has to be id for each action, starting from 1
            int actionscounter = 1;
            QStringList eventactions;
            foreach (const QString &eventaction, notification->actions()) {
                eventactions.append(QString::number(actionscounter));
                eventactions.append(eventaction);
                actionscounter++;
            }
            const QString eventapp = KGlobal::mainComponent().componentName();
            QVariantMap eventhints;
            // NOTE: has to be set to be configurable via plasma notifications applet
            eventhints.insert("x-kde-appname", eventapp);
            QDBusReply<uint> notifyreply = m_notificationsiface->call(
                QString::fromLatin1("Notify"),
                eventapp,
                eventid,
                eventicon,
                eventtitle,
                eventtext,
                eventactions,
                eventhints,
                eventtimeout
            );
            if (!notifyreply.isValid()) {
                kWarning(s_knotificationarea) << "invalid notify reply" << notifyreply.error().message();
            } else {
                m_notifications.insert(notification, notifyreply.value());
            }
        }
    }

    if (eventactions.contains(QString::fromLatin1("Sound"))) {
        QString eventsound = notification->icon();
        if (eventsound.isEmpty()) {
            eventsound = eventgroup.readEntry("Sound");
        }
        if (eventsound.isEmpty()) {
            eventsound = globalgroup.readEntry("Sound");
        }
        const QStringList eventsoundfiles = KGlobal::dirs()->findAllResources("sound", eventsound, KStandardDirs::Recursive);
        if (eventsoundfiles.isEmpty()) {
            kWarning(s_knotificationarea) << "sound not found" << eventsound;
        } else {
            kDebug(s_knotificationarea) << "playing notification sound" << eventsound;
            if (!m_kaudioplayeriface) {
                m_kaudioplayeriface = new QDBusInterface(
                    "org.kde.kded", "/modules/kaudioplayer", "org.kde.kaudioplayer",
                    QDBusConnection::sessionBus(), this
                );
            }
            // the sound player is configurable and is used by the bball plasma applet for example
            QDBusReply<void> playreply = m_kaudioplayeriface->call(QString::fromLatin1("play"), eventsoundfiles.first());
            if (!playreply.isValid()) {
                kWarning(s_knotificationarea) << "invalid play reply" << playreply.error().message();
            }
        }
    }

    if (eventactions.contains(QString::fromLatin1("Taskbar"))) {
        const QWidget* eventwidget = notification->widget();
        if (!eventwidget) {
            kWarning(s_knotificationarea) << "taskbar event with no widget set" << eventid;
        } else {
            const WId eventwidgetid = eventwidget->winId();
            kDebug(s_knotificationarea) << "marking notification task" << eventid << eventwidgetid;
            KWindowSystem::demandAttention(eventwidgetid);
        }
    }
}

void KNotificationManager::close(KNotification *notification)
{
    QMutableMapIterator<KNotification*,uint> iter(m_notifications);
    while (iter.hasNext()) {
        iter.next();
        if (iter.key() == notification) {
            iter.remove();
                QDBusReply<void> closereply = m_notificationsiface->call(
                QString::fromLatin1("CloseNotification"),
                iter.value()
            );
            if (!closereply.isValid()) {
                kWarning(s_knotificationarea) << "invalid close reply" << closereply.error().message();
            }
        }
    }
}

void KNotificationManager::slotNotificationClosed(uint eventid, uint reason)
{
    kDebug(s_knotificationarea) << "closing notifications due to interface" << reason;
    QMutableMapIterator<KNotification*,uint> iter(m_notifications);
    while (iter.hasNext()) {
        iter.next();
        if (iter.value() == eventid) {
            KNotification* notification = iter.key();
            notification->close();
        }
    }
}

void KNotificationManager::slotActionInvoked(uint eventid, const QString &action)
{
    kDebug(s_knotificationarea) << "notification action invoked" << action;
    QMutableMapIterator<KNotification*,uint> iter(m_notifications);
    while (iter.hasNext()) {
        iter.next();
        if (iter.value() == eventid) {
            KNotification* notification = iter.key();
            notification->activate(action.toUInt());
        }
    }
}

class KNotificationPrivate
{
public:
    KNotificationPrivate();

    QString eventid;
    QString title;
    QString text;
    QString icon;
    QWidget *widget;
    QStringList actions;
    KNotification::NotificationFlags flags;
};

KNotificationPrivate::KNotificationPrivate()
    : widget(nullptr)
{
}


KNotification::KNotification(QObject *parent)
    : QObject(parent),
    d(new KNotificationPrivate())
{
}

KNotification::~KNotification()
{
    close();
    delete d;
}

QString KNotification::eventID() const
{
    return d->eventid;
}

void KNotification::setEventID(const QString &eventid)
{
    d->eventid = eventid;
}

QString KNotification::title() const
{
    return d->title;
}

void KNotification::setTitle(const QString &title)
{
    d->title = title;
}

QString KNotification::text() const
{
    return d->text;
}

void KNotification::setText(const QString &text)
{
    d->text = text;
}

QString KNotification::icon() const
{
    return d->icon;
}

void KNotification::setIcon(const QString &icon)
{
    d->icon = icon;
}

QWidget* KNotification::widget() const
{
    return d->widget;
}

void KNotification::setWidget(QWidget *widget)
{
    d->widget = widget;
    setParent(widget);
    if (widget && (d->flags & KNotification::CloseWhenWidgetActivated)) {
        widget->installEventFilter(this);
    }
}

QStringList KNotification::actions() const
{
    return d->actions;
}

void KNotification::setActions(const QStringList &actions)
{
    d->actions = actions;
}

KNotification::NotificationFlags KNotification::flags() const
{
    return d->flags;
}

void KNotification::setFlags(const NotificationFlags &flags)
{
    d->flags = flags;
}

void KNotification::send()
{
    kDebug(s_knotificationarea) << "sending notification" << d->eventid;
    const bool persistent = (flags() & KNotification::Persistent);
    kNotificationManager->send(this, persistent);
    if (!persistent) {
        QTimer::singleShot(500, this, SLOT(close()));
    }
}

void KNotification::activate(unsigned int action)
{
    kDebug(s_knotificationarea) << "activating notification action" << d->eventid << action;
    switch (action) {
        case 1: {
            emit action1Activated();
            break;
        }
        case 2: {
            emit action2Activated();
            break;
        }
        case 3: {
            emit action3Activated();
            break;
        }
        default: {
            kWarning(s_knotificationarea) << "invalid action" << action;
            break;
        }
    }
    close();
}

void KNotification::close()
{
    kDebug(s_knotificationarea) << "closing notification" << d->eventid;
    kNotificationManager->close(this);
    emit closed();
    deleteLater();
}

bool KNotification::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->widget) {
        if (event->type() == QEvent::WindowActivate
            && d->flags & KNotification::CloseWhenWidgetActivated) {
            kDebug(s_knotificationarea) << "closing due to widget activation" << d->eventid;
            QTimer::singleShot(500, this, SLOT(close()));
        }
    }
    return false;
}

KNotification* KNotification::event(const QString &eventid, const QString &title, const QString &text,
                                    const QString &icon, QWidget *widget,
                                    const NotificationFlags &flags)
{
    KNotification* knotification = new KNotification(widget);
    knotification->setEventID(eventid);
    knotification->setTitle(title);
    knotification->setText(text);
    knotification->setIcon(icon);
    knotification->setWidget(widget);
    knotification->setFlags(flags);
    QTimer::singleShot(0, knotification, SLOT(send()));
    return knotification;
}

void KNotification::beep(const QString &reason, QWidget *widget)
{
    event(
        QString::fromLatin1("kde/beep"), QString(), reason, QString(), widget,
        KNotification::CloseOnTimeout
    );
}

#include "moc_knotification.cpp"
#include "knotification.moc"
