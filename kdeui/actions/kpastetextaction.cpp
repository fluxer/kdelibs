/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#include "kpastetextaction.h"

#include <QtGui/QClipboard>
#include <QtDBus/QtDBus>

#include <kapplication.h>
#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>

#include "kmenu.h"

class KPasteTextActionPrivate
{
public:
    KPasteTextActionPrivate(KPasteTextAction *parent)
        : q(parent),
        m_popup(nullptr)
    {
    }

    ~KPasteTextActionPrivate()
    {
        delete m_popup;
    }

    void _k_menuAboutToShow();
    void _k_slotTriggered(QAction*);

    void init();

    KPasteTextAction *q;
    KMenu *m_popup;
};

KPasteTextAction::KPasteTextAction(QObject *parent)
    : KAction(parent),
    d(new KPasteTextActionPrivate(this))
{
    d->init();
}

KPasteTextAction::KPasteTextAction(const QString &text, QObject *parent)
    : KAction(parent),
    d(new KPasteTextActionPrivate(this))
{
    d->init();
    setText(text);
}

KPasteTextAction::KPasteTextAction(const KIcon &icon, const QString &text, QObject *parent)
    : KAction(icon, text, parent),
    d(new KPasteTextActionPrivate(this))
{
    d->init();
}

void KPasteTextActionPrivate::init()
{
    m_popup = new KMenu();
    q->connect(m_popup, SIGNAL(aboutToShow()), q, SLOT(_k_menuAboutToShow()));
    q->connect(m_popup, SIGNAL(triggered(QAction*)), q, SLOT(_k_slotTriggered(QAction*)));
}

KPasteTextAction::~KPasteTextAction()
{
    delete d;
}

void KPasteTextActionPrivate::_k_menuAboutToShow()
{
    m_popup->clear();
    QStringList list;
    QDBusInterface klipper("org.kde.klipper", "/klipper", "org.kde.klipper.klipper");
    if (klipper.isValid()) {
        QDBusReply<QStringList> reply = klipper.call("getClipboardHistoryMenu");
        if (reply.isValid()) {
            list = reply;
        }
    }
    QString clipboardText = qApp->clipboard()->text(QClipboard::Clipboard);
    if (list.isEmpty()) {
        list << clipboardText;
    }
    bool found = false;
    const QFontMetrics fm = m_popup->fontMetrics();
    foreach (const QString &string, list) {
        QString text = fm.elidedText(string.simplified(), Qt::ElideMiddle, fm.maxWidth() * 20);
        text.replace('&', "&&");
        QAction* action = m_popup->addAction(text);
        if (!found && string == clipboardText) {
            action->setChecked(true);
            found = true;
        }
    }
}

void KPasteTextActionPrivate::_k_slotTriggered(QAction* action)
{
    QDBusInterface klipper("org.kde.klipper", "/klipper", "org.kde.klipper.klipper");
    if (klipper.isValid()) {
        QDBusReply<QString> reply = klipper.call("getClipboardHistoryItem", m_popup->actions().indexOf(action));
        if (!reply.isValid()) {
            return;
        }
        QString clipboardText = reply;
        reply = klipper.call("setClipboardContents", clipboardText);
        if (reply.isValid()) {
            kDebug(129) << "Clipboard: " << qApp->clipboard()->text(QClipboard::Clipboard);
        }
    }
}

#include "moc_kpastetextaction.cpp"
