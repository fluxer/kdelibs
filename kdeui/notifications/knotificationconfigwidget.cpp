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
#include "kstandarddirs.h"
#include "klocale.h"
#include "kdebug.h"

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>

class KNotificationConfigWidgetPrivate
{
public:
    KNotificationConfigWidgetPrivate(KNotificationConfigWidget *q);

    void _k_slotItemChanged(QTreeWidgetItem *item, int column);

    KNotificationConfigWidget* parent;
    QVBoxLayout* layout;
    QTreeWidget* treewidget;
    QString enabledi18n;
    QString disabledi18n;
    QString popuptooltipi18n;
    QString soundtooltipi18n;
    QString taskbartooltipi18n;
    QMap<QString,QStringList> notificationchanges;
};

KNotificationConfigWidgetPrivate::KNotificationConfigWidgetPrivate(KNotificationConfigWidget *q)
    : parent(q),
    layout(nullptr),
    treewidget(nullptr)
{
    // translate once
    enabledi18n = i18n("Enabled");
    disabledi18n = i18n("Disabled");
    popuptooltipi18n = i18n("Show a message in a popup");
    soundtooltipi18n = i18n("Play a sound");
    taskbartooltipi18n = i18n("Mark taskbar entry");
}

void KNotificationConfigWidgetPrivate::_k_slotItemChanged(QTreeWidgetItem *item, int column)
{
    emit parent->changed(true);
    if (!item) {
        kWarning() << "null tree item";
        return;
    }
    if (column >= 1 && column <= 3) {
        item->setText(column, item->checkState(column) == Qt::Checked ? enabledi18n : disabledi18n);
    }
    const QString eventgroup = item->data(0, Qt::UserRole).toString();
    QStringList eventactions;
    const bool eventpopup = (item->checkState(1) == Qt::Checked);
    const bool eventsound = (item->checkState(2) == Qt::Checked);
    const bool eventtaskbar = (item->checkState(3) == Qt::Checked);
    if (eventpopup) {
        eventactions.append(QString::fromLatin1("Popup"));
    }
    if (eventsound) {
        eventactions.append(QString::fromLatin1("Sound"));
    }
    if (eventtaskbar) {
        eventactions.append(QString::fromLatin1("Taskbar"));
    }
    notificationchanges.insert(eventgroup, eventactions);
}


KNotificationConfigWidget::KNotificationConfigWidget(const QString &notification, QWidget *parent)
    : QWidget(parent),
    d(new KNotificationConfigWidgetPrivate(this))
{
    d->layout = new QVBoxLayout(this);
    setLayout(d->layout);

    d->treewidget = new QTreeWidget(this);
    d->treewidget->setColumnCount(4);
    QStringList treeheaders = QStringList()
        << i18n("Event")
        << i18n("Popup")
        << i18n("Sound")
        << i18n("Taskbar");
    d->treewidget->setHeaderLabels(treeheaders);
    d->treewidget->setRootIsDecorated(false);
    connect(
        d->treewidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
        this, SLOT(_k_slotItemChanged(QTreeWidgetItem*,int))
    );
    d->layout->addWidget(d->treewidget);

    setNotification(notification);
}

KNotificationConfigWidget::~KNotificationConfigWidget()
{
    delete d;
}

void KNotificationConfigWidget::save()
{
    KConfig notificationsconfig("knotificationrc", KConfig::NoGlobals);
    QMapIterator<QString,QStringList> iter(d->notificationchanges);
    while (iter.hasNext()) {
        iter.next();
        KConfigGroup eventgroupconfig(&notificationsconfig, iter.key());
        eventgroupconfig.writeEntry("Actions", iter.value());
    }
    notificationsconfig.sync();
    d->notificationchanges.clear();
    emit changed(false);
}

void KNotificationConfigWidget::setNotification(const QString &notification)
{
    d->treewidget->clear();

    const QString notifyconfig = KStandardDirs::locate("config", "notifications/" + notification+ ".notifyrc");
    if (notifyconfig.isEmpty()) {
        kWarning() << "invalid notification" << notification;
        return;
    }
    KConfig notificationconfig("knotificationrc", KConfig::NoGlobals);
    notificationconfig.addConfigSources(QStringList() << notifyconfig);
    KConfigGroup globalgroupconfig(&notificationconfig, notification);
    foreach (const QString &eventgroup, notificationconfig.groupList()) {
        if (!eventgroup.startsWith(notification + QLatin1Char('/'))) {
            continue;
        }
        KConfigGroup eventgroupconfig(&notificationconfig, eventgroup);
        QString eventtext = eventgroupconfig.readEntry("Name");
        if (eventtext.isEmpty()) {
            eventtext = globalgroupconfig.readEntry("Name");
        }
        QString eventicon = eventgroupconfig.readEntry("IconName");
        if (eventicon.isEmpty()) {
            eventicon = globalgroupconfig.readEntry("IconName");
        }
        QStringList eventactions = eventgroupconfig.readEntry("Actions", QStringList());
        if (eventactions.isEmpty()) {
            eventactions = globalgroupconfig.readEntry("Actions", QStringList());
        }
        const bool eventpopup = eventactions.contains(QString::fromLatin1("Popup"));
        const bool eventsound = eventactions.contains(QString::fromLatin1("Sound"));
        const bool eventtaskbar = eventactions.contains(QString::fromLatin1("Taskbar"));
        QTreeWidgetItem* eventitem = new QTreeWidgetItem();
        eventitem->setData(0, Qt::UserRole, eventgroup);
        eventitem->setIcon(0, KIcon(eventicon));
        eventitem->setText(0, eventtext);
        eventitem->setText(1, eventpopup ? d->enabledi18n : d->disabledi18n);
        eventitem->setCheckState(1, eventpopup ? Qt::Checked : Qt::Unchecked);
        eventitem->setToolTip(1, d->popuptooltipi18n);
        eventitem->setText(2, eventsound ? d->enabledi18n : d->disabledi18n);
        eventitem->setCheckState(2, eventsound ? Qt::Checked : Qt::Unchecked);
        eventitem->setToolTip(2, d->soundtooltipi18n);
        eventitem->setText(3, eventtaskbar ? d->enabledi18n : d->disabledi18n);
        eventitem->setCheckState(3, eventtaskbar ? Qt::Checked : Qt::Unchecked);
        eventitem->setToolTip(3, d->taskbartooltipi18n);
        d->treewidget->addTopLevelItem(eventitem);
    }
    d->treewidget->header()->setStretchLastSection(false);
    d->treewidget->header()->setResizeMode(0, QHeaderView::Stretch);
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
