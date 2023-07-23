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
    const QString klocalelanguage = KGlobal::locale()->language();
    QLocale calendarlocale = QLocale(klocalelanguage);
    const QLocale systemlocale = QLocale::system();
    if (calendarlocale.name() == QLatin1String("C") && calendarlocale.name() != systemlocale.name()) {
        calendarlocale = systemlocale;
        kWarning() << "Could not create KCalendarWidget locale for" << klocalelanguage;
    }

    kcalendarwidget->setLocale(calendarlocale);
    kcalendarwidget->setSelectedDate(date);
    kcalendarwidget->setCalendar(KGlobal::locale()->calendar());
}

class KCalendarWidgetPrivate
{
public:
    KCalendarWidgetPrivate();

    const KCalendarSystem *calendar;
};

KCalendarWidgetPrivate::KCalendarWidgetPrivate()
    : calendar(nullptr)
{
}

KCalendarWidget::KCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent),
    d(new KCalendarWidgetPrivate())
{
    setupCalendarWidget(this, QDate::currentDate());
}

KCalendarWidget::KCalendarWidget(const QDate &date, QWidget *parent)
    : QCalendarWidget(parent),
    d(new KCalendarWidgetPrivate())
{
    setupCalendarWidget(this, date);
}

KCalendarWidget::~KCalendarWidget()
{
    delete d;
}

const KCalendarSystem* KCalendarWidget::calendar() const
{
    return d->calendar;
}

void KCalendarWidget::setCalendar(const KCalendarSystem *calendar)
{
    d->calendar = calendar;
    if (!calendar) {
        kWarning() << "Attempting to set KCalendarWidget calendar to null";
        return;
    }
    setMinimumDate(calendar->earliestValidDate());
    setMaximumDate(calendar->latestValidDate());
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
