//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright 2007 Daniel Teske <teske@squorn.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkdialog.h"
#include "kbookmarkmanager.h"
#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include <QFormLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QHeaderView>
#include <klineedit.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kstandardguiitem.h>

// #########################
// KBookmarkDialogPrivate
class KBookmarkDialogPrivate
{
public:
    KBookmarkDialogPrivate(KBookmarkManager *mgr)
      : m_main(nullptr)
      , m_url(nullptr)
      , m_title(nullptr)
      , m_comment(nullptr)
      , m_titleLabel(nullptr)
      , m_urlLabel(nullptr)
      , m_commentLabel(nullptr)
      , m_folderTree(0)
      , m_mgr(mgr)
      , m_layout(false)
    {
    }

    KBookmarkDialog::BookmarkDialogMode m_mode;
    QWidget* m_main;
    KLineEdit* m_url;
    KLineEdit* m_title;
    KLineEdit* m_comment;
    QLabel* m_titleLabel;
    QLabel* m_urlLabel;
    QLabel* m_commentLabel;
    QTreeWidget* m_folderTree;
    KBookmarkManager* m_mgr;
    KBookmark m_bm;
    QList<QPair<QString, QString>> m_list;
    bool m_layout;
};


KBookmark KBookmarkDialog::editBookmark(const KBookmark &bm)
{
    if (!d->m_layout) {
        initLayoutPrivate();
    }

    setButtons(KDialog::Ok | KDialog::Cancel);
    setButtonGuiItem(KDialog::Ok, KGuiItem(i18nc("@action:button", "Update")));
    setCaption(i18nc("@title:window","Bookmark Properties"));
    d->m_url->setVisible(!bm.isGroup());
    d->m_urlLabel->setVisible(!bm.isGroup());
    d->m_bm = bm;
    d->m_title->setText(bm.fullText());
    d->m_url->setText(bm.url().url());
    d->m_comment->setVisible(true);
    d->m_commentLabel->setVisible(true);
    d->m_comment->setText(bm.description());
    d->m_folderTree->setVisible(false);

    d->m_mode = KBookmarkDialog::EditBookmark;
    aboutToShow(d->m_mode);

    if (exec() == QDialog::Accepted) {
        return d->m_bm;
    }
    return KBookmark();
}

KBookmark KBookmarkDialog::addBookmark(const QString & title, const KUrl & url, KBookmark parent)
{
    if (!d->m_layout)
        initLayoutPrivate();
    if (parent.isNull()) {
        parent = d->m_mgr->root();
    }

    setButtons(KDialog::User1 | KDialog::Ok | KDialog::Cancel);
    setButtonGuiItem(KDialog::Ok,  KGuiItem( i18nc("@action:button", "Add"), "bookmark-new"));
    setCaption( i18nc("@title:window","Add Bookmark") );
    setButtonGuiItem(KDialog::User1, KGuiItem(i18nc("@action:button", "&New Folder..." ), "folder-new"));
    d->m_url->setVisible(true);
    d->m_urlLabel->setVisible(true);
    d->m_title->setText(title);    
    d->m_url->setText(url.url());
    d->m_comment->setText(QString());
    d->m_comment->setVisible(true);
    d->m_commentLabel->setVisible(true);
    setParentBookmark(parent);
    d->m_folderTree->setVisible(true);

    d->m_mode = KBookmarkDialog::NewBookmark;
    aboutToShow(d->m_mode);

    if (exec() == QDialog::Accepted) {
        return d->m_bm;
    }
    return KBookmark();
}

KBookmarkGroup KBookmarkDialog::addBookmarks(const QList<QPair<QString, QString>> &list, const QString &name, KBookmarkGroup parent)
{
    if (!d->m_layout) {
        initLayoutPrivate();
    }
    if (parent.isNull()) {
        parent = d->m_mgr->root();
    }

    d->m_list = list;

    setButtons(KDialog::User1 | KDialog::Ok | KDialog::Cancel);
    setButtonGuiItem(KDialog::Ok,  KGuiItem(i18nc("@action:button", "Add"), "bookmark-new"));
    setCaption(i18nc("@title:window","Add Bookmarks") );
    setButtonGuiItem(KDialog::User1, KGuiItem(i18nc("@action:button", "&New Folder..."), "folder-new"));
    d->m_url->setVisible(false);
    d->m_urlLabel->setVisible(false);
    d->m_title->setText(name);
    d->m_comment->setVisible(true);
    d->m_commentLabel->setVisible(true);
    d->m_comment->setText(QString());
    setParentBookmark(parent);
    d->m_folderTree->setVisible(true);

    d->m_mode = NewMultipleBookmarks;
    aboutToShow(d->m_mode);
    
    if (exec() == QDialog::Accepted) {
        return d->m_bm.toGroup();
    }
    return KBookmarkGroup();
}

KBookmarkGroup KBookmarkDialog::selectFolder(KBookmark parent)
{
    if (!d->m_layout) {
        initLayoutPrivate();
    }
    if (parent.isNull()) {
        parent = d->m_mgr->root();
    }

    setButtons(KDialog::User1 | KDialog::Ok | KDialog::Cancel);
    setButtonGuiItem(KDialog::Ok, KStandardGuiItem::ok());
    setButtonGuiItem(KDialog::User1, KGuiItem(i18nc("@action:button", "&New Folder..."), "folder-new"));
    setCaption( i18nc("@title:window","Select Folder"));
    d->m_url->setVisible(false);
    d->m_urlLabel->setVisible(false);
    d->m_title->setVisible(false);
    d->m_titleLabel->setVisible(false);
    d->m_comment->setVisible(false);
    d->m_commentLabel->setVisible(false);
    setParentBookmark(parent);
    d->m_folderTree->setVisible(true);

    d->m_mode = SelectFolder;
    aboutToShow(d->m_mode);

    if (exec() == QDialog::Accepted) {
        return d->m_bm.toGroup();
    }
    return KBookmarkGroup();
}

KBookmarkGroup KBookmarkDialog::createNewFolder(const QString &name, KBookmark parent)
{
    if (!d->m_layout) {
        initLayoutPrivate();
    }
    if (parent.isNull()) {
        parent = d->m_mgr->root();
    }

    setButtons(KDialog::Ok | KDialog::Cancel);
    setButtonGuiItem(KDialog::Ok, KStandardGuiItem::ok());
    setCaption(i18nc("@title:window","New Folder"));
    d->m_url->setVisible(false);
    d->m_urlLabel->setVisible(false);
    d->m_comment->setVisible(true);
    d->m_commentLabel->setVisible(true);
    d->m_comment->setText(QString());
    d->m_title->setText(name);
    setParentBookmark(parent);
    d->m_folderTree->setVisible(true);

    d->m_mode = NewFolder;
    aboutToShow(d->m_mode);

    if (exec() == QDialog::Accepted) {
        return d->m_bm.toGroup();
    }
    return KBookmarkGroup();
}

void KBookmarkDialog::setParentBookmark(const KBookmark &bm)
{
    QString address = bm.address();
    KBookmarkTreeItem * item = static_cast<KBookmarkTreeItem *>(d->m_folderTree->topLevelItem(0));
    while(true) {
        if (item->address() == bm.address()) {
            d->m_folderTree->setCurrentItem(item);
            return;
        }
        for(int i = 0; i < item->childCount(); ++i) {
            KBookmarkTreeItem * child = static_cast<KBookmarkTreeItem *>(item->child(i));
            if (KBookmark::commonParent(child->address(), address) == child->address()) {
                item = child;
                break;
            }
        }
    }
}

KBookmarkGroup KBookmarkDialog::parentBookmark()
{
    KBookmarkTreeItem *item = dynamic_cast<KBookmarkTreeItem *>(d->m_folderTree->currentItem());
    if (!item) {
        return d->m_mgr->root();
    }
    return d->m_mgr->findByAddress(item->address()).toGroup();
}

void KBookmarkDialog::slotButtonClicked(int button)
{
    if (button == KDialog::Ok) {
        if(d->m_mode == KBookmarkDialog::NewFolder) {
            KBookmarkGroup parent = parentBookmark();
            if (d->m_title->text().isEmpty()) {
                d->m_title->setText("New Folder");
            }
            d->m_bm = parent.createNewFolder(d->m_title->text());
            d->m_bm.setDescription(d->m_comment->text());
            save(d->m_mode, d->m_bm);
            d->m_mgr->emitChanged(parent);
        } else if(d->m_mode == KBookmarkDialog::NewBookmark) {
            KBookmarkGroup parent = parentBookmark();
            if (d->m_title->text().isEmpty()) {
                d->m_title->setText("New Bookmark");
            }
            d->m_bm = parent.addBookmark(d->m_title->text(), KUrl(d->m_url->text()));
            d->m_bm.setDescription(d->m_comment->text());
            save(d->m_mode, d->m_bm);
            d->m_mgr->emitChanged(parent);
        } else if(d->m_mode == KBookmarkDialog::NewMultipleBookmarks) {
            KBookmarkGroup parent = parentBookmark();
            if (d->m_title->text().isEmpty()) {
                d->m_title->setText("New Folder");
            }
            d->m_bm = parent.createNewFolder(d->m_title->text());
            d->m_bm.setDescription(d->m_comment->text());
            QList< QPair<QString, QString> >::iterator  it, end;
            end = d->m_list.end();
            for(it = d->m_list.begin(); it!= d->m_list.end(); ++it) {
                d->m_bm.toGroup().addBookmark( (*it).first, KUrl((*it).second));
            }
            save(d->m_mode, d->m_bm);
            d->m_mgr->emitChanged(parent);
        } else if(d->m_mode == KBookmarkDialog::EditBookmark) {
            d->m_bm.setFullText(d->m_title->text());
            d->m_bm.setUrl(KUrl(d->m_url->text()));
            d->m_bm.setDescription(d->m_comment->text());
            save(d->m_mode, d->m_bm);
            d->m_mgr->emitChanged(d->m_bm.parentGroup());
        } else if(d->m_mode == KBookmarkDialog::SelectFolder) {
            d->m_bm = parentBookmark();
            save(d->m_mode, d->m_bm);
        }
    }
    KDialog::slotButtonClicked(button);
}

void KBookmarkDialog::save(BookmarkDialogMode mode, const KBookmark &bk)
{
    Q_UNUSED(mode);
    Q_UNUSED(bk);
}

void KBookmarkDialog::aboutToShow(BookmarkDialogMode mode)
{
    Q_UNUSED(mode);
}

void KBookmarkDialog::initLayout()
{
    QBoxLayout *vbox = new QVBoxLayout(d->m_main);
    vbox->setMargin(0);
    QFormLayout * form = new QFormLayout();
    vbox->addLayout(form);

    form->addRow(d->m_titleLabel, d->m_title);
    form->addRow(d->m_urlLabel, d->m_url);
    form->addRow(d->m_commentLabel, d->m_comment);

    vbox->addWidget(d->m_folderTree);
}

void KBookmarkDialog::initLayoutPrivate()
{
    d->m_main = new QWidget(this);
    setMainWidget(d->m_main);
    connect(this, SIGNAL(user1Clicked()), this, SLOT(newFolderButton()));

    d->m_title = new KLineEdit(d->m_main);
    d->m_title->setMinimumWidth(300);
    d->m_titleLabel = new QLabel(i18nc("@label:textbox", "Name:" ), d->m_main);
    d->m_titleLabel->setBuddy(d->m_title);

    d->m_url = new KLineEdit( d->m_main);
    d->m_url->setMinimumWidth(300);
    d->m_urlLabel = new QLabel(i18nc("@label:textbox", "Location:"), d->m_main);
    d->m_urlLabel->setBuddy(d->m_url);

    d->m_comment = new KLineEdit(d->m_main);
    d->m_comment->setMinimumWidth(300);
    d->m_commentLabel = new QLabel( i18nc("@label:textbox", "Comment:"), d->m_main);
    d->m_commentLabel->setBuddy(d->m_comment);

    d->m_folderTree = new QTreeWidget(d->m_main);
    d->m_folderTree->setColumnCount(1);
    d->m_folderTree->header()->hide();
    d->m_folderTree->setSortingEnabled(false);
    d->m_folderTree->setSelectionMode( QTreeWidget::SingleSelection);
    d->m_folderTree->setSelectionBehavior( QTreeWidget::SelectRows);
    d->m_folderTree->setMinimumSize(60, 100);
    QTreeWidgetItem *root = new KBookmarkTreeItem(d->m_folderTree);    
    fillGroup(root, d->m_mgr->root());

    initLayout();
    d->m_layout = true;
}

KBookmarkDialog::KBookmarkDialog(KBookmarkManager *mgr, QWidget *parent)
  : KDialog(parent),
    d(new KBookmarkDialogPrivate(mgr))
{
}

KBookmarkDialog::~KBookmarkDialog()
{
    delete d;
}

void KBookmarkDialog::newFolderButton()
{

    QString caption = parentBookmark().fullText().isEmpty() ?
                      i18nc("@title:window","Create New Bookmark Folder") :
                      i18nc("@title:window","Create New Bookmark Folder in %1" ,
                        parentBookmark().text());
    bool ok = false;
    QString text = KInputDialog::getText(caption, i18nc("@label:textbox", "New folder:"), QString(), &ok);
    if (!ok) {
        return;
    }

    KBookmarkGroup group = parentBookmark().createNewFolder(text);
    if (!group.isNull()) {
        KBookmarkGroup parentGroup = group.parentGroup();
        d->m_mgr->emitChanged(parentGroup);
        d->m_folderTree->clear();
        QTreeWidgetItem *root = new KBookmarkTreeItem(d->m_folderTree);
        fillGroup(root, d->m_mgr->root(), group);
    }
}

void KBookmarkDialog::fillGroup(QTreeWidgetItem *parentItem, const KBookmarkGroup &group)
{
    fillGroup(parentItem, group, KBookmarkGroup());
}

void KBookmarkDialog::fillGroup(QTreeWidgetItem *parentItem, const KBookmarkGroup &group, const KBookmarkGroup &selectGroup)
{
    for (KBookmark bk = group.first() ; !bk.isNull() ; bk = group.next(bk)) {
        if (bk.isGroup()) {
            const KBookmarkGroup bkGroup = bk.toGroup();
            QTreeWidgetItem* item = new KBookmarkTreeItem(parentItem, d->m_folderTree, bkGroup);
            if (selectGroup == bkGroup) {
                d->m_folderTree->setCurrentItem(item);
            }
            fillGroup(item, bkGroup, selectGroup);
        }
    }
}

/********************************************************************/

KBookmarkTreeItem::KBookmarkTreeItem(QTreeWidget *tree)
    : QTreeWidgetItem(tree), m_address("")
{
    setText(0, i18nc("name of the container of all browser bookmarks","Bookmarks"));
    setIcon(0, SmallIcon("bookmarks"));
    tree->expandItem(this);
    tree->setCurrentItem(this);
    tree->setItemSelected(this, true);
}

KBookmarkTreeItem::KBookmarkTreeItem(QTreeWidgetItem * parent, QTreeWidget * tree, const KBookmarkGroup &bk)
    : QTreeWidgetItem(parent)
{
    setIcon(0, SmallIcon(bk.icon()));
    setText(0, bk.fullText() );
    tree->expandItem(this);
    m_address = bk.address();
}

QString KBookmarkTreeItem::address()
{
    return m_address;
}
