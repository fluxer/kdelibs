/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 2002 Hamish Rodda <rodda@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "ktimerdialog.h"

#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QTimer>
#include <QProgressBar>

#include <kvbox.h>
#include <khbox.h>
#include <kwindowsystem.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

class KTimerDialogPrivate
{
public:
    KTimerDialogPrivate();

    QTimer *totalTimer;
    QTimer *updateTimer;
    int msecRemaining, updateInterval, msecTotal;

    KDialog::ButtonCode buttonOnTimeout;
    KTimerDialog::TimerStyle tStyle;

    KHBox *timerWidget;
    QProgressBar *timerProgress;
    QLabel *timerLabel;
    KVBox *mainWidget;
};

KTimerDialogPrivate::KTimerDialogPrivate()
    : totalTimer(nullptr),
    updateTimer(nullptr),
    msecRemaining(0),
    updateInterval(0),
    msecTotal(0),
    buttonOnTimeout(KDialog::None),
    tStyle(KTimerDialog::CountDown),
    timerWidget(nullptr),
    timerProgress(nullptr),
    timerLabel(nullptr),
    mainWidget(nullptr)
{
}

KTimerDialog::KTimerDialog(int msec, TimerStyle style, QWidget *parent,
                           const QString &caption,
                           int buttonMask, ButtonCode defaultButton,
                           bool separator,
                           const KGuiItem &user1,
                           const KGuiItem &user2,
                           const KGuiItem &user3)
    : KDialog(parent),
    d(new KTimerDialogPrivate())
{
    setCaption(caption);
    setButtons((ButtonCodes)buttonMask);
    setDefaultButton(defaultButton);
    setButtonFocus(defaultButton);   // setDefaultButton() doesn't do this
    showButtonSeparator(separator);
    setButtonGuiItem(User1, user1);
    setButtonGuiItem(User2, user2);
    setButtonGuiItem(User3, user3);

    d->totalTimer = new QTimer(this);
    d->totalTimer->setSingleShot(true);
    d->updateTimer = new QTimer(this);
    d->updateTimer->setSingleShot(false);
    d->msecTotal = d->msecRemaining = msec;
    d->updateInterval = 1000;
    d->tStyle = style;
    KWindowSystem::setIcons(winId(), DesktopIcon("randr"), SmallIcon("randr"));
    // default to canceling the dialog on timeout
    if (buttonMask & KDialog::Cancel) {
        d->buttonOnTimeout = KDialog::Cancel;
    }

    connect(d->totalTimer, SIGNAL(timeout()), SLOT(slotInternalTimeout()));
    connect(d->updateTimer, SIGNAL(timeout()), SLOT(slotUpdateTime()));

    // create the widgets
    d->mainWidget = new KVBox(this);
    d->timerWidget = new KHBox(d->mainWidget);
    d->timerWidget->setSpacing(-1);
    d->timerLabel = new QLabel(d->timerWidget);
    d->timerProgress = new QProgressBar(d->timerWidget);
    d->timerProgress->setRange(0, d->msecTotal);
    d->timerProgress->setTextVisible(false);

    KDialog::setMainWidget(d->mainWidget);

    slotUpdateTime(false);
}

KTimerDialog::~KTimerDialog()
{
    delete d;
}

void KTimerDialog::setVisible(bool visible)
{
    KDialog::setVisible(visible);

    if (visible) {
        d->totalTimer->start(d->msecTotal);
        d->updateTimer->start(d->updateInterval);
    }
}

int KTimerDialog::exec()
{
    d->totalTimer->start(d->msecTotal);
    d->updateTimer->start(d->updateInterval);
    return KDialog::exec();
}

void KTimerDialog::setMainWidget(QWidget *widget)
{
    // yuck, here goes.
    KVBox *newWidget = new KVBox(this);
    newWidget->setSpacing(-1);

    if (widget->parentWidget() != d->mainWidget) {
        widget->setParent(newWidget);
    }
    d->timerWidget->setParent(newWidget);

    delete d->mainWidget;
    d->mainWidget = newWidget;
    KDialog::setMainWidget(d->mainWidget);
}

void KTimerDialog::setRefreshInterval(int msec)
{
    d->updateInterval = msec;
    if (d->updateTimer->isActive()) {
        d->updateTimer->start(d->updateInterval);
    }
}

int KTimerDialog::timeoutButton() const
{
    return d->buttonOnTimeout;
}

void KTimerDialog::setTimeoutButton(const ButtonCode newButton)
{
    d->buttonOnTimeout = newButton;
}

int KTimerDialog::timerStyle() const
{
    return d->tStyle;
}

void KTimerDialog::setTimerStyle(const TimerStyle newStyle)
{
    d->tStyle = newStyle;
}

void KTimerDialog::slotUpdateTime(bool update)
{
    if (update)
        switch(d->tStyle) {
        case CountDown:
            d->msecRemaining -= d->updateInterval;
            break;
        case CountUp:
            d->msecRemaining += d->updateInterval;
            break;
        case Manual:
            break;
        }

    d->timerProgress->setValue(d->msecRemaining);

    d->timerLabel->setText(i18np("1 second remaining:", "%1 seconds remaining:", d->msecRemaining / 1000));
}

void KTimerDialog::slotInternalTimeout()
{
    emit timerTimeout();
    slotButtonClicked(d->buttonOnTimeout);
}

#include "moc_ktimerdialog.cpp"
