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

#ifndef KNOTIFICATION_H
#define KNOTIFICATION_H

#include <kdeui_export.h>

#include <QWidget>
#include <QEvent>

class KNotificationPrivate;

/*!
    Class to notify the user of an event.

    Notification configuration file consist of main group and a group for each event, for example:
    @code
    [mynotification]
    Name=My name
    Comment=My comment
    IconName=my-icon

    [mynotification/myevent]
    Name=My event name
    Actions=Sound,Popup,Taskbar
    Sound=KDE-Im-Contact-Out.ogg
    @endcode

    The main configuration group ("mynotification" in this case) is used as fallback for key that
    is not set in the event group ("mynotification/myevent" in this case). The "Name" key specifies
    the event text while the "Comment" key specifies the event title. The "Actions" key speicifes
    what the notification event will do:
    @li Sound - play the sound from the "Sound" key
    @li Popup - show a passive popup
    @li Taskbar - mark the event widget as requiring attention 

    Notification configuration files should be installed from the build system like this:
    @code
    install(FILES foo.notifyrc DESTINATION ${KDE4_CONFIG_INSTALL_DIR}/notifications)
    @endcode

    @since 4.24
    @warning the API is subject to change
*/
class KDEUI_EXPORT KNotification : public QObject
{
    Q_OBJECT
public:
    enum NotificationFlag {
        /**
         * The notification will be automatically closed after a timeout. (this is the default)
         */
        CloseOnTimeout = 0x00,

        /**
         * The notification will NOT be automatically closed after a timeout. You will have to
         * track the notification, and close it with the close function manually when the event is
         * done, otherwise there will be a memory leak
         */
        Persistent = 0x02,

        /**
         * The notification will be automatically closed if the widget() becomes activated.
         *
         * If the widget is already activated when the notification occurs, the notification will
         * be closed after a small timeout. This only works if the widget is the toplevel widget
         */
        CloseWhenWidgetActivated = 0x03,
            
    };
    Q_DECLARE_FLAGS(NotificationFlags , NotificationFlag)

    KNotification(QObject *parent = nullptr);
    ~KNotification();

    QString eventID() const;
    void setEventID(const QString &eventid);

    QString title() const;
    void setTitle(const QString &title);

    QString text() const;
    void setText(const QString &text);

    QString icon() const;
    void setIcon(const QString &icon);

    QWidget *widget() const;
    void setWidget(QWidget *widget);

    QStringList actions() const;
    void setActions(const QStringList &actions);

    NotificationFlags flags() const;
    void setFlags(const NotificationFlags &flags);

    /**
     * Convenience method - creates KNotification, sets it up and automatically sends the event.
     */
    static void event(const QString &eventid,
                      const QString &title = QString(), const QString &text = QString(),
                      const QString &icon = QString(), QWidget *widget = nullptr,
                      const NotificationFlags &flags = CloseOnTimeout);

    /**
     * This is a simple substitution for QApplication::beep().
     */
    static void beep(const QString &reason = QString(), QWidget *widget = nullptr);

    // prevent warning
    using QObject::event;

public Q_SLOTS:
    /**
     * Sends the event.
     */
    void send();

    /**
     * Activate the specified @p action.
     */
    void activate(unsigned int action);
    /**
     * Closes the notification and deletes it.
     */
    void close();

Q_SIGNALS:
    /**
     * Signals that the first action is activated.
     */
    void action1Activated();
    /**
     * Signals that the second action is activated.
     */
    void action2Activated();
    /**
     * Signals that the third action is activated.
     */
    void action3Activated();

    /**
     * Signals that the notification is closed.
     */
    void closed();

protected:
    /**
     * Reimplemented for internal reasons
     */
    virtual bool eventFilter(QObject *watched, QEvent *event);

private:
    Q_DISABLE_COPY(KNotification);
    KNotificationPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KNotification::NotificationFlags)

#endif
