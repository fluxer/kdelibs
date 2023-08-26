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
#include <QComboBox>
#include <QFileInfo>

Q_DECLARE_METATYPE(QTreeWidgetItem*);

class KNotificationConfigDialog : public KDialog
{
    Q_OBJECT
public:
    KNotificationConfigDialog(const QString &notification, QWidget *parent = nullptr);
    ~KNotificationConfigDialog();
};

KNotificationConfigDialog::KNotificationConfigDialog(const QString &notification, QWidget *parent)
    : KDialog(parent, 0)
{
    setCaption(i18n("Configure Notifications"));
    KNotificationConfigWidget *widget = new KNotificationConfigWidget(notification, this);
    setMainWidget(widget);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(this, SIGNAL(applyClicked()), widget, SLOT(save()));
    connect(this, SIGNAL(okClicked()), widget, SLOT(save()));
    connect(widget, SIGNAL(changed(bool)), this , SLOT(enableButtonApply(bool)));

    setInitialSize(QSize(620, 310));
    KConfigGroup kconfiggroup(KGlobal::config(), "KNotificationConfigDialog");
    restoreDialogSize(kconfiggroup);
}

KNotificationConfigDialog::~KNotificationConfigDialog()
{
    KConfigGroup kconfiggroup(KGlobal::config(), "KNotificationConfigDialog");
    saveDialogSize(kconfiggroup);
    KGlobal::config()->sync();
}


struct KNotificationChanges
{
    QStringList eventactions;
    QString eventsound;
};


class KNotificationConfigWidgetPrivate
{
public:
    KNotificationConfigWidgetPrivate(KNotificationConfigWidget *q);

    void _k_slotItemChanged(QTreeWidgetItem *item, int column);
    void _k_slotSoundChanged(int index);

    KNotificationConfigWidget* parent;
    QVBoxLayout* layout;
    QTreeWidget* treewidget;
    QString enabledi18n;
    QString disabledi18n;
    QString popuptooltipi18n;
    QString soundtooltipi18n;
    QString taskbartooltipi18n;
    QMap<QString,KNotificationChanges> notificationchanges;
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
    if (column == 1 || column == 3) {
        item->setText(column, item->checkState(column) == Qt::Checked ? enabledi18n : disabledi18n);
    }
    const QString eventgroup = item->data(0, Qt::UserRole).toString();
    QStringList eventactions;
    const bool eventactionpopup = (item->checkState(1) == Qt::Checked);
    const QComboBox* eventbox = qobject_cast<QComboBox*>(item->treeWidget()->itemWidget(item, 2));
    Q_ASSERT(eventbox != nullptr);
    const bool eventactionsound = (eventbox->currentIndex() != 0);
    const bool eventactiontaskbar = (item->checkState(3) == Qt::Checked);
    if (eventactionpopup) {
        eventactions.append(QString::fromLatin1("Popup"));
    }
    if (eventactionsound) {
        eventactions.append(QString::fromLatin1("Sound"));
    }
    if (eventactiontaskbar) {
        eventactions.append(QString::fromLatin1("Taskbar"));
    }
    KNotificationChanges eventchanges;
    eventchanges.eventactions = eventactions;
    eventchanges.eventsound = eventbox->itemData(eventbox->currentIndex()).toString();
    notificationchanges.insert(eventgroup, eventchanges);
}

void KNotificationConfigWidgetPrivate::_k_slotSoundChanged(int index)
{
    QComboBox* eventbox = qobject_cast<QComboBox*>(parent->sender());
    if (index != 0) {
        // update the current sound data
        eventbox->setItemData(0, eventbox->itemText(index));
    }
    // and trigger item changes
    QTreeWidgetItem* eventitem = qvariant_cast<QTreeWidgetItem*>(eventbox->property("_k_eventitem"));
    _k_slotItemChanged(eventitem, 2);
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
    d->treewidget->header()->setStretchLastSection(false);
    d->treewidget->header()->setResizeMode(0, QHeaderView::Stretch);
    d->treewidget->header()->setResizeMode(2, QHeaderView::Stretch);
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
    QMapIterator<QString,KNotificationChanges> iter(d->notificationchanges);
    while (iter.hasNext()) {
        iter.next();
        KConfigGroup eventgroupconfig(&notificationsconfig, iter.key());
        const KNotificationChanges eventchanges = iter.value();
        eventgroupconfig.writeEntry("Actions", eventchanges.eventactions);
        eventgroupconfig.writeEntry("Sound", eventchanges.eventsound);
    }
    notificationsconfig.sync();
    d->notificationchanges.clear();
    emit changed(false);
}

void KNotificationConfigWidget::setNotification(const QString &notification)
{
    d->treewidget->clear();

    const QString notifyconfig = KStandardDirs::locate("config", "notifications/" + notification + ".notifyrc");
    if (notifyconfig.isEmpty()) {
        kWarning() << "invalid notification" << notification;
        return;
    }

    QStringList sounds = KGlobal::dirs()->findAllResources("sound", "*", KStandardDirs::Recursive);
    sounds.sort();
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
        QString eventsound = eventgroupconfig.readEntry("Sound");
        if (eventsound.isEmpty()) {
            eventsound = globalgroupconfig.readEntry("Sound");
        }
        QStringList eventactions = eventgroupconfig.readEntry("Actions", QStringList());
        if (eventactions.isEmpty()) {
            eventactions = globalgroupconfig.readEntry("Actions", QStringList());
        }
        const bool eventactionpopup = eventactions.contains(QString::fromLatin1("Popup"));
        const bool eventactionsound = eventactions.contains(QString::fromLatin1("Sound"));
        const bool eventactiontaskbar = eventactions.contains(QString::fromLatin1("Taskbar"));
        QTreeWidgetItem* eventitem = new QTreeWidgetItem();
        eventitem->setData(0, Qt::UserRole, eventgroup);
        eventitem->setIcon(0, KIcon(eventicon));
        eventitem->setText(0, eventtext);
        eventitem->setText(1, eventactionpopup ? d->enabledi18n : d->disabledi18n);
        eventitem->setCheckState(1, eventactionpopup ? Qt::Checked : Qt::Unchecked);
        eventitem->setToolTip(1, d->popuptooltipi18n);
        eventitem->setToolTip(2, d->soundtooltipi18n);
        eventitem->setText(3, eventactiontaskbar ? d->enabledi18n : d->disabledi18n);
        eventitem->setCheckState(3, eventactiontaskbar ? Qt::Checked : Qt::Unchecked);
        eventitem->setToolTip(3, d->taskbartooltipi18n);
        d->treewidget->addTopLevelItem(eventitem);
        QComboBox* eventbox = new QComboBox(d->treewidget);
        eventbox->setProperty("_k_eventitem", QVariant::fromValue(eventitem));
        foreach (const QString &sound, sounds) {
            const QString soundfilename = QFileInfo(sound).fileName();
            eventbox->addItem(soundfilename, soundfilename);
        }
        if (eventactionsound) {
            // lookup has to be done before inserting the "Disabled" item with the current sound
            // data
            const int eventsoundindex = eventbox->findData(eventsound);
            if (eventsoundindex >= 0) {
                eventbox->setCurrentIndex(eventsoundindex);
            } else {
                kWarning() << "event sound not found" << eventsound;
            }
        }
        eventbox->insertItem(0, d->disabledi18n, eventsound);
        if (!eventactionsound) {
            eventbox->setCurrentIndex(0);
        }
        connect(eventbox, SIGNAL(currentIndexChanged(int)), this, SLOT(_k_slotSoundChanged(int)));
        d->treewidget->setItemWidget(eventitem, 2, eventbox);
    }
}

void KNotificationConfigWidget::configure(const QString &notification, QWidget *parent)
{
    KNotificationConfigDialog *dialog = new KNotificationConfigDialog(notification, parent);
    dialog->show();
}

#include "moc_knotificationconfigwidget.cpp"
#include "knotificationconfigwidget.moc"
