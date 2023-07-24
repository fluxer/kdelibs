/*
 *   Copyright 2023 Ivailo Monev <xakepa10@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "calendarwidget.h"
#include "private/style_p.h"
#include "private/themedwidgetinterface_p.h"

#include "kcalendarwidget.h"
#include "kdebug.h"

#include <QToolButton>

namespace Plasma
{

class CalendarWidgetPrivate: public ThemedWidgetInterface<CalendarWidget>
{
public:
    CalendarWidgetPrivate(CalendarWidget *calendarwidget)
        : ThemedWidgetInterface<CalendarWidget>(calendarwidget)
    {
    }

    Plasma::Style::Ptr style;
};

CalendarWidget::CalendarWidget(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new CalendarWidgetPrivate(this))
{
    KCalendarWidget *native = new KCalendarWidget();
    setWidget(native);
    native->setWindowIcon(QIcon());
    native->setAttribute(Qt::WA_NoSystemBackground);
    connect(native, SIGNAL(clicked(QDate)), this, SIGNAL(clicked(QDate)));
    connect(native, SIGNAL(activated(QDate)), this, SIGNAL(activated(QDate)));

    d->style = Plasma::Style::sharedStyle();
    native->setStyle(d->style.data());

    d->initTheming();

    QToolButton* nativemonthbutton = native->findChild<QToolButton*>("qt_calendar_monthbutton");
    if (nativemonthbutton) {
        // FIXME: the popup menu outside color is not transparent
        nativemonthbutton->setEnabled(false);
        nativemonthbutton->setMenu(nullptr);
    } else {
        kWarning() << "Could not find the QCalendarWidget month button" << native;
    }
}

CalendarWidget::~CalendarWidget()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void CalendarWidget::setSelectedDate(const QDate &date)
{
    nativeWidget()->setSelectedDate(date);
}

QDate CalendarWidget::selectedDate() const
{
    return nativeWidget()->selectedDate();
}

void CalendarWidget::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString CalendarWidget::styleSheet()
{
    return widget()->styleSheet();
}

KCalendarWidget* CalendarWidget::nativeWidget() const
{
    return static_cast<KCalendarWidget*>(widget());
}

void CalendarWidget::focusInEvent(QFocusEvent *event)
{
    QGraphicsProxyWidget::focusInEvent(event);
    if (!nativeWidget()->hasFocus()) {
        // as of Qt 4.7, apparently we have a bug here in QGraphicsProxyWidget
        nativeWidget()->setFocus(event->reason());
    }
}

}

#include "moc_calendarwidget.cpp"
