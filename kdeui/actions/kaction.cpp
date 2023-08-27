/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
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

#include "kaction.h"
#include "kaction_p.h"
#include "kglobalaccel_p.h"
#include "klocale.h"
#include "kmessagebox.h"
#include "kdebug.h"
#include "kicon.h"

#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/qevent.h>
#include <QtGui/QToolBar>

//---------------------------------------------------------------------
// KActionPrivate
//---------------------------------------------------------------------

void KActionPrivate::init(KAction *q_ptr)
{
    q = q_ptr;
    globalShortcutEnabled = false;
    neverSetGlobalShortcut = true;
    QObject::connect(q, SIGNAL(triggered(bool)), q, SLOT(slotTriggered()));
    q->setProperty("isShortcutConfigurable", true);
}

void KActionPrivate::setActiveGlobalShortcutNoEnable(const KShortcut &cut)
{
    globalShortcut = cut;
    emit q->globalShortcutChanged(cut.primary());
}

void KActionPrivate::slotTriggered()
{
    emit q->triggered(QApplication::mouseButtons(), QApplication::keyboardModifiers());
}

bool KAction::event(QEvent *event)
{
    if (event->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent*>(event);
        if(se->isAmbiguous()) {
            KMessageBox::information(
                NULL,  // No widget to be seen around here
                i18n(
                    "The key sequence '%1' is ambiguous. Use 'Configure Shortcuts'\n"
                    "from the 'Settings' menu to solve the ambiguity.\n"
                    "No action will be triggered.", se->key().toString(QKeySequence::NativeText)
                ),
                i18n("Ambiguous shortcut detected")
            );
            return true;
        }
    }
    return QAction::event(event);
}


//---------------------------------------------------------------------
// KAction
//---------------------------------------------------------------------
KAction::KAction(QObject *parent)
    : QWidgetAction(parent),
    d(new KActionPrivate())
{
    d->init(this);
}

KAction::KAction(const QString &text, QObject *parent)
    : QWidgetAction(parent),
    d(new KActionPrivate())
{
    d->init(this);
    setText(text);
}

KAction::KAction(const KIcon &icon, const QString &text, QObject *parent)
    : QWidgetAction(parent),
    d(new KActionPrivate)
{
    d->init(this);
    setIcon(icon);
    setText(text);
}

KAction::~KAction()
{
    if (d->globalShortcutEnabled) {
        // - remove the action from KGlobalAccel
        d->globalShortcutEnabled = false;
        KGlobalAccel::self()->d->remove(this, KGlobalAccelPrivate::SetInactive);
    }
    delete d;
}

bool KAction::isShortcutConfigurable() const
{
    return property("isShortcutConfigurable").toBool();
}

void KAction::setShortcutConfigurable(bool b)
{
    setProperty("isShortcutConfigurable", b);
}

KShortcut KAction::shortcut(ShortcutTypes type) const
{
    Q_ASSERT(type);
    if (type == DefaultShortcut) {
        return KShortcut(
            property("defaultPrimaryShortcut").value<QKeySequence>(),
            property("defaultAlternateShortcut").value<QKeySequence>()
        );
    }
    const QList cuts = shortcuts();
    return KShortcut(cuts.value(0), cuts.value(1));
}

void KAction::setShortcut(const KShortcut &shortcut, ShortcutTypes type)
{
    Q_ASSERT(type);
    if (type & KAction::DefaultShortcut) {
        setProperty("defaultPrimaryShortcut", shortcut.primary());
        setProperty("defaultAlternateShortcut", shortcut.alternate());
    }
    if (type & ActiveShortcut) {
        QAction::setShortcuts(shortcut);
    }
}

void KAction::setShortcut(const QKeySequence &keySeq, ShortcutTypes type)
{
    Q_ASSERT(type);
    if (type & KAction::DefaultShortcut) {
        setProperty("defaultPrimaryShortcut", keySeq);
    }
    if (type & KAction::ActiveShortcut) {
        QAction::setShortcut(keySeq);
    }
}

void KAction::setShortcuts(const QList<QKeySequence> &shortcuts, ShortcutTypes type)
{
    setShortcut(KShortcut(shortcuts), type);
}

const KShortcut& KAction::globalShortcut(ShortcutTypes type) const
{
    Q_ASSERT(type);
    if (type == KAction::DefaultShortcut) {
        return d->defaultGlobalShortcut;
    }
    return d->globalShortcut;
}

void KAction::setGlobalShortcut(const KShortcut &shortcut, ShortcutTypes type,
                                GlobalShortcutLoading load)
{
    Q_ASSERT(type);
    bool changed = false;

    if (!d->globalShortcutEnabled) {
        changed = true;
        if (objectName().isEmpty() || objectName().startsWith(QLatin1String("unnamed-"))) {
            kWarning(283) << "Attempt to set global shortcut for action without objectName()."
                             " Read the setGlobalShortcut() documentation.";
            return;
        }
        d->globalShortcutEnabled = true;
        KGlobalAccel::self()->d->doRegister(this);
    }

    if ((type & KAction::DefaultShortcut) && d->defaultGlobalShortcut != shortcut) {
        d->defaultGlobalShortcut = shortcut;
        changed = true;
    }

    if ((type & KAction::ActiveShortcut) && d->globalShortcut != shortcut) {
        d->globalShortcut = shortcut;
        changed = true;
    }

    //We want to have updateGlobalShortcuts called on a new action in any case so that
    //it will be registered properly. In the case of the first setShortcut() call getting an
    //empty shortcut parameter this would not happen...
    if (changed || d->neverSetGlobalShortcut) {
        KGlobalAccel::self()->d->updateGlobalShortcut(this, type | load);
        d->neverSetGlobalShortcut = false;
    }
}


bool KAction::isGlobalShortcutEnabled() const
{
    return d->globalShortcutEnabled;
}

void KAction::forgetGlobalShortcut()
{
    d->globalShortcut = KShortcut();
    d->defaultGlobalShortcut = KShortcut();
    if (d->globalShortcutEnabled) {
        d->globalShortcutEnabled = false;
        d->neverSetGlobalShortcut = true; //it's a fresh start :)
        KGlobalAccel::self()->d->remove(this, KGlobalAccelPrivate::UnRegister);
    }
}

void KAction::setHelpText(const QString &text)
{
    setStatusTip(text);
    setToolTip(text);
    if (whatsThis().isEmpty()) {
        setWhatsThis(text);
    }
}

#include "moc_kaction.cpp"
