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

class KTimeEditPrivate
{
public:
    KTimeEditPrivate(KTimeEdit *parent);

    void updateSuffixes();
    void slotValueChanged(const int value);

    KTimeEdit* ktimeedit;
    QSpinBox* hourbox;
    QSpinBox* minutebox;
    QSpinBox* secondbox;
};

KTimeEditPrivate::KTimeEditPrivate(KTimeEdit *parent)
    : ktimeedit(parent),
    hourbox(nullptr),
    minutebox(nullptr),
    secondbox(nullptr)
{
}

void KTimeEditPrivate::updateSuffixes()
{
    hourbox->setSuffix(i18np(" hour", " hours", hourbox->value()));
    minutebox->setSuffix(i18np(" minute", " minutes", minutebox->value()));
    secondbox->setSuffix(i18np(" second", " seconds", secondbox->value()));
}

void KTimeEditPrivate::slotValueChanged(const int value)
{
    updateSuffixes();
    const QTime currenttime = QTime(hourbox->value(), minutebox->value(), secondbox->value());
    emit ktimeedit->timeChanged(currenttime);
}


KTimeEdit::KTimeEdit(QWidget *parent)
    : QWidget(parent),
    d(new KTimeEditPrivate(this))
{
    QHBoxLayout* timelayout = new QHBoxLayout(this);
    d->hourbox = new QSpinBox(this);
    d->hourbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->hourbox->setMaximum(23);
    timelayout->addWidget(d->hourbox);
    d->minutebox = new QSpinBox(this);
    d->minutebox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->minutebox->setMaximum(59);
    timelayout->addWidget(d->minutebox);
    d->secondbox = new QSpinBox(this);
    d->secondbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->secondbox->setMaximum(59);
    timelayout->addWidget(d->secondbox);
    setLayout(timelayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    d->updateSuffixes();

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
    return QTime(d->hourbox->minimum(), d->minutebox->minimum(), d->secondbox->minimum());
}

void KTimeEdit::setMinimumTime(const QTime &time)
{
    d->hourbox->setMinimum(time.hour());
    d->minutebox->setMinimum(time.minute());
    d->secondbox->setMinimum(time.second());
}

QTime KTimeEdit::maximumTime() const
{
    return QTime(d->hourbox->maximum(), d->minutebox->maximum(), d->secondbox->maximum());
}

void KTimeEdit::setMaximumTime(const QTime &time)
{
    d->hourbox->setMaximum(time.hour());
    d->minutebox->setMaximum(time.minute());
    d->secondbox->setMaximum(time.second());
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
