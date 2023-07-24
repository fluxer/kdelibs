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

#ifndef KCALENDARWIDGET_H
#define KCALENDARWIDGET_H

#include <kdeui_export.h>

#include <QCalendarWidget>
#include <QEvent>

class KCalendarWidgetPrivate;

/*!
    Class to pick a date.

    @since 4.23
    @warning the API is subject to change
*/
class KDEUI_EXPORT KCalendarWidget: public QCalendarWidget
{
    Q_OBJECT
public:
    KCalendarWidget(QWidget *parent = nullptr);
    KCalendarWidget(const QDate &date, QWidget *parent = nullptr);

protected:
    // QWidget reimplementation (not reimplemented by QCalendarWidget)
    virtual void changeEvent(QEvent *event);
};

#endif //  KCALENDARWIDGET_H
