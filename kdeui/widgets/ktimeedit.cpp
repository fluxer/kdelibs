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

#include "ktimeedit.h"
#include "klocale.h"
#include "kdebug.h"

#include <QSpinBox>
#include <QHBoxLayout>

class KTimeEditPrivate;

class KTimeBox : public QSpinBox
{
    Q_OBJECT
public:
    KTimeBox(KTimeEditPrivate *privateparent, QWidget *parent);

protected:
    QValidator::State validate(QString &input, int &pos) const final;

private:
    KTimeEditPrivate* ktimeeditprivate;
};

KTimeBox::KTimeBox(KTimeEditPrivate *privateparent, QWidget *parent)
    : QSpinBox(parent),
    ktimeeditprivate(privateparent)
{
}


class KTimeEditPrivate
{
public:
    KTimeEditPrivate(KTimeEdit *parent);

    void updateWidgets();
    void slotValueChanged(const int value);

    KTimeEdit* ktimeedit;
    KTimeBox* hourbox;
    KTimeBox* minutebox;
    KTimeBox* secondbox;
    QTime mintime;
    QTime maxtime;
};


QValidator::State KTimeBox::validate(QString &input, int &pos) const
{
    const int hour = ktimeeditprivate->hourbox->value();
    const int minute = ktimeeditprivate->minutebox->value();
    const int second = ktimeeditprivate->secondbox->value();
    const QTime currenttime = QTime(hour, minute, second);
    if (!currenttime.isValid()) {
        return QValidator::Invalid;
    }
    if (currenttime < ktimeeditprivate->mintime || currenttime > ktimeeditprivate->maxtime) {
        return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}


KTimeEditPrivate::KTimeEditPrivate(KTimeEdit *parent)
    : ktimeedit(parent),
    hourbox(nullptr),
    minutebox(nullptr),
    secondbox(nullptr),
    mintime(0, 0, 0),
    maxtime(23, 59, 59)
{
}

void KTimeEditPrivate::updateWidgets()
{
    const int hour = hourbox->value();
    const int minute = minutebox->value();
    const int second = secondbox->value();
    hourbox->setSuffix(i18np(" hour", " hours", hour));
    minutebox->setSuffix(i18np(" minute", " minutes", minute));
    secondbox->setSuffix(i18np(" second", " seconds", second));
    if (hour == 0 && minute == 0 && second == 0) {
        hourbox->setSpecialValueText(i18n("never"));
        minutebox->setSpecialValueText(i18n("never"));
        secondbox->setSpecialValueText(i18n("never"));
    } else {
        hourbox->setSpecialValueText(QString());
        minutebox->setSpecialValueText(QString());
        secondbox->setSpecialValueText(QString());
    }
    hourbox->setMinimum(mintime.hour());
    hourbox->setMaximum(maxtime.hour());
    if (hour > 0) {
        minutebox->setMinimum(0);
    } else {
        minutebox->setMinimum(mintime.minute());
    }
    if (hour > 0 || minute > 0) {
        secondbox->setMinimum(0);
    } else {
        secondbox->setMinimum(mintime.second());
    }
}

void KTimeEditPrivate::slotValueChanged(const int value)
{
    updateWidgets();
    const QTime currenttime = QTime(hourbox->value(), minutebox->value(), secondbox->value());
    emit ktimeedit->timeChanged(currenttime);
}


KTimeEdit::KTimeEdit(QWidget *parent)
    : QWidget(parent),
    d(new KTimeEditPrivate(this))
{
    QHBoxLayout* timelayout = new QHBoxLayout(this);
    d->hourbox = new KTimeBox(d, this);
    d->hourbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    timelayout->addWidget(d->hourbox);
    d->minutebox = new KTimeBox(d, this);
    d->minutebox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    timelayout->addWidget(d->minutebox);
    d->secondbox = new KTimeBox(d, this);
    d->secondbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    timelayout->addWidget(d->secondbox);
    setLayout(timelayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    d->updateWidgets();

    connect(d->hourbox, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
    connect(d->minutebox, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
    connect(d->secondbox, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
}

KTimeEdit::~KTimeEdit()
{
    delete d;
}

QTime KTimeEdit::minimumTime() const
{
    return d->mintime;
}

void KTimeEdit::setMinimumTime(const QTime &time)
{
    d->mintime = time;
    d->updateWidgets();
}

QTime KTimeEdit::maximumTime() const
{
    return d->maxtime;
}

void KTimeEdit::setMaximumTime(const QTime &time)
{
    d->maxtime = time;
    d->updateWidgets();
}

QTime KTimeEdit::time() const
{
    return QTime(d->hourbox->value(), d->minutebox->value(), d->secondbox->value());
}

void KTimeEdit::setTime(const QTime &time)
{
    d->hourbox->setValue(time.hour());
    d->minutebox->setValue(time.minute());
    d->secondbox->setValue(time.second());
}

#include "moc_ktimeedit.cpp"
#include "ktimeedit.moc"
