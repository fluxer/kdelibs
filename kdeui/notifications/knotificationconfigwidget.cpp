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

#include "knotificationconfigwidget.h"
#include "kdialog.h"
#include "klineedit.h"
#include "klocale.h"

#include <QGridLayout>
#include <QTreeWidget>
#include <QCheckBox>

class KNotificationConfigWidgetPrivate
{
public:
    KNotificationConfigWidgetPrivate(KNotificationConfigWidget *q);

    void _k_slotChanged();

    KNotificationConfigWidget* parent;
    KConfig* config;
    QGridLayout* layout;
    QTreeWidget* treewidget;
    QCheckBox* soundbox;
    KLineEdit* soundedit;
    QCheckBox* popupbox;
    QCheckBox* taskbarbox;
};

void KNotificationConfigWidgetPrivate::_k_slotChanged()
{
    emit parent->changed(true);
}

KNotificationConfigWidgetPrivate::KNotificationConfigWidgetPrivate(KNotificationConfigWidget *q)
    : parent(q),
    config(nullptr),
    layout(nullptr),
    treewidget(nullptr),
    soundbox(nullptr),
    soundedit(nullptr),
    popupbox(nullptr),
    taskbarbox(nullptr)
{
}

KNotificationConfigWidget::KNotificationConfigWidget(const QString &app, QWidget *parent)
    : QWidget(parent),
    d(new KNotificationConfigWidgetPrivate(this))
{
    d->layout = new QGridLayout(this);
    setLayout(d->layout);

    d->treewidget = new QTreeWidget(this);
    d->layout->addWidget(d->treewidget, 0, 0, 1, 2);

    d->soundbox = new QCheckBox(this);
    d->soundbox->setText(i18n("Play a sound"));
    d->soundbox->setIcon(KIcon("media-playback-start"));
    connect(d->soundbox, SIGNAL(toggled(bool)), this, SLOT(_k_slotChanged()));
    d->layout->addWidget(d->soundbox, 1, 0);
    d->soundedit = new KLineEdit(this);
    connect(d->soundedit, SIGNAL(textChanged(QString)), this, SLOT(_k_slotChanged()));
    d->layout->addWidget(d->soundedit, 1, 1);

    d->popupbox = new QCheckBox(this);
    d->popupbox->setText(i18n("Show a message in a popup"));
    d->popupbox->setIcon(KIcon("dialog-information"));
    connect(d->popupbox, SIGNAL(toggled(bool)), this, SLOT(_k_slotChanged()));
    d->layout->addWidget(d->popupbox, 2, 0);

    d->taskbarbox = new QCheckBox(this);
    d->taskbarbox->setText(i18n("Mark tasbar entry"));
    d->taskbarbox->setIcon(KIcon("services"));
    connect(d->taskbarbox, SIGNAL(toggled(bool)), this, SLOT(_k_slotChanged()));
    d->layout->addWidget(d->taskbarbox, 3, 0);
}

KNotificationConfigWidget::~KNotificationConfigWidget()
{
    delete d;
}

void KNotificationConfigWidget::save()
{
    // TODO:
    emit changed(false);
}

void KNotificationConfigWidget::configure(const QString &app, QWidget *parent)
{
    KDialog *dialog = new KDialog(parent);
    dialog->setCaption(i18n("Configure Notifications"));
    KNotificationConfigWidget *widget = new KNotificationConfigWidget(app, dialog);
    dialog->setMainWidget(widget);
    
    connect(dialog, SIGNAL(applyClicked()), widget, SLOT(save()));
    connect(dialog, SIGNAL(okClicked()), widget, SLOT(save()));
    connect(widget, SIGNAL(changed(bool)), dialog , SLOT(enableButtonApply(bool)));

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

#include "moc_knotificationconfigwidget.cpp"
