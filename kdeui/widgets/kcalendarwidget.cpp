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

#include "kcalendarwidget.h"
#include "kglobal.h"
#include "klocale.h"
#include "kdebug.h"

#include <QLocale>

static void setupCalendarWidget(KCalendarWidget *kcalendarwidget, const QDate &date)
{
    kcalendarwidget->setLocale(KGlobal::locale()->toLocale());
    kcalendarwidget->setSelectedDate(date);
}

KCalendarWidget::KCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    setupCalendarWidget(this, QDate::currentDate());
}

KCalendarWidget::KCalendarWidget(const QDate &date, QWidget *parent)
    : QCalendarWidget(parent)
{
    setupCalendarWidget(this, date);
}

void KCalendarWidget::changeEvent(QEvent *event)
{
    switch (event->type()) {
        // NOTE: QCalendarWidget adapts on QEvent::LocaleChange, KSwitchLanguageDialog sends
        // QEvent::LanguageChange event
        case QEvent::LocaleChange:
        case QEvent::LanguageChange: {
            setupCalendarWidget(this, selectedDate());
            break;
        }
        default: {
            break;
        }
    }
}
