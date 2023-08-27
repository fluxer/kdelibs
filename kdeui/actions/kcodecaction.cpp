/*
    kcodecaction.cpp

    Copyright (c) 2003 Jason Keirstead   <jason@keirstead.org>
    Copyrigth (c) 2006 Michel Hermier    <michel.hermier@gmail.com>
    Copyright (c) 2007 Nick Shaforostoff <shafff@ukr.net>

    ********************************************************************
    *                                                                  *
    * This library is free software; you can redistribute it and/or    *
    * modify it under the terms of the GNU Lesser General Public       *
    * License as published by the Free Software Foundation; either     *
    * version 2 of the License, or (at your option) any later version. *
    *                                                                  *
    * This library is distributed in the hope that it will be useful,  *
    * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
    * GNU Lesser General Public License for more details.              *
    *                                                                  *
    * You should have received a copy of the GNU Lesser General Public *
    * License along with this library; if not, write to the            *
    * Free Software Foundation, Inc., 51 Franklin Street,              *
    * Fifth Floor, Boston, MA  02110-1301  USA                         *
    *                                                                  *
    ********************************************************************
*/

#include "kcodecaction.h"

#include <kcharsets.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

#include <QMenu>
#include <QVariant>
#include <QtCore/QTextCodec>

// Acording to http://www.iana.org/assignments/ianacharset-mib
// the default/unknown mib value is 2.
#define MIB_DEFAULT 2

class KCodecAction::Private
{
public:
    Private(KCodecAction *parent)
        : q(parent),
          currentSubAction(0)
    {
    }

    void init();

    void _k_subActionTriggered(QAction*);

    KCodecAction *q;
    QAction *currentSubAction;
};

KCodecAction::KCodecAction(QObject *parent)
    : KSelectAction(parent)
    , d(new Private(this))
{
    d->init();
}

KCodecAction::KCodecAction(const QString &text, QObject *parent)
    : KSelectAction(text, parent)
    , d(new Private(this))
{
    d->init();
}

KCodecAction::KCodecAction(const KIcon &icon, const QString &text, QObject *parent)
    : KSelectAction(icon, text, parent)
    , d(new Private(this))
{
    d->init();
}

KCodecAction::~KCodecAction()
{
    delete d;
}

void KCodecAction::Private::init()
{
    q->setToolBarMode(MenuMode);

    foreach(const QStringList &encodingsForScript, KGlobal::charsets()->encodingsByScript()) {
        KSelectAction* tmp = new KSelectAction(encodingsForScript.at(0), q);
        for (int i = 1; i<encodingsForScript.size(); ++i) {
            tmp->addAction(encodingsForScript.at(i));
        }
        q->connect(tmp,SIGNAL(triggered(QAction*)),q,SLOT(_k_subActionTriggered(QAction*)));
        tmp->setCheckable(true);
        q->addAction(tmp);
    }
    q->setCurrentItem(0);
}

int KCodecAction::mibForName(const QString &codecName, bool *ok) const
{
    // FIXME logic is good but code is ugly

    bool success = false;
    int mib = MIB_DEFAULT;
    KCharsets *charsets = KGlobal::charsets();

    QTextCodec *codec = charsets->codecForName(codecName, success);
    if (!success) {
        // Maybe we got a description name instead
        codec = charsets->codecForName(charsets->encodingForName(codecName), success);
    }

    if (codec) {
        mib = codec->mibEnum();
    }

    if (ok) {
        *ok = success;
    }

    if (success) {
        return mib;
    }

    kWarning() << "Invalid codec name: "  << codecName;
    return MIB_DEFAULT;
}

QTextCodec *KCodecAction::codecForMib(int mib) const
{
    if (mib == MIB_DEFAULT) {
        return QTextCodec::codecForLocale();
    }
    return QTextCodec::codecForMib(mib);
}

void KCodecAction::Private::_k_subActionTriggered(QAction *action)
{
    if (currentSubAction == action) {
        return;
    }
    currentSubAction = action;
    bool ok = false;
    int mib = q->mibForName(action->text(), &ok);
    if (ok) {
        emit q->triggered(action->text());
        emit q->triggered(q->codecForMib(mib));
    }
}

QTextCodec *KCodecAction::currentCodec() const
{
    return codecForMib(currentCodecMib());
}

bool KCodecAction::setCurrentCodec( QTextCodec *codec )
{
    if (!codec) {
        return false;
    }

    for (int i = 0; i <actions().size(); ++i) {
        if (actions().at(i)->menu()) {
            for (int j = 0; j < actions().at(i)->menu()->actions().size(); ++j) {
                if (codec == KGlobal::charsets()->codecForName(actions().at(i)->menu()->actions().at(j)->text())) {
                    d->currentSubAction = actions().at(i)->menu()->actions().at(j);
                    d->currentSubAction->trigger();
                    return true;
                }
            }
        }
    }
    return false;

}

QString KCodecAction::currentCodecName() const
{
    return d->currentSubAction->text();
}

bool KCodecAction::setCurrentCodec(const QString &codecName)
{
    return setCurrentCodec(KGlobal::charsets()->codecForName(codecName));
}

int KCodecAction::currentCodecMib() const
{
    return mibForName(currentCodecName());
}

bool KCodecAction::setCurrentCodec(int mib)
{
    return setCurrentCodec(codecForMib(mib));
}

#include "moc_kcodecaction.cpp"
