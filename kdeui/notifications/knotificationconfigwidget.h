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

#ifndef KNOTIFICATIONCONFIGWIDGET_H
#define KNOTIFICATIONCONFIGWIDGET_H

#include <kdeui_export.h>

#include <QWidget>
#include <QTreeWidgetItem>

class KNotificationConfigWidgetPrivate;

/*!
    Class to configure user notification events.

    @since 4.24
    @warning the API is subject to change
*/
class KDEUI_EXPORT KNotificationConfigWidget : public QWidget
{
    Q_OBJECT
public:
    KNotificationConfigWidget(const QString &notification, QWidget *parent = nullptr);
    ~KNotificationConfigWidget();

    void setNotification(const QString &notification);

    static void configure(const QString &notification, QWidget *parent = nullptr);

public Q_SLOTS:
    void save();

Q_SIGNALS:
    void changed(bool state);

private:
    friend KNotificationConfigWidgetPrivate;
    Q_DISABLE_COPY(KNotificationConfigWidget);
    KNotificationConfigWidgetPrivate *d;

    Q_PRIVATE_SLOT(d, void _k_slotItemChanged(QTreeWidgetItem *item, int column));
    Q_PRIVATE_SLOT(d, void _k_slotSoundChanged(int index));
};

#endif // KNOTIFICATIONCONFIGWIDGET_H
