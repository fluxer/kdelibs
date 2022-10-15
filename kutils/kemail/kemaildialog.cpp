/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "kemaildialog.h"
#include "klocale.h"
#include "kmessagebox.h"
#include "kdebug.h"

#include "ui_kemaildialog.h"

class KEMailDialogPrivate
{
public:
    KEMailDialogPrivate();
    ~KEMailDialogPrivate();

    KEMail* m_kemail;
    Ui::KEMailDialogUI ui;
};

KEMailDialogPrivate::KEMailDialogPrivate()
    : m_kemail(nullptr)
{
    m_kemail = new KEMail();
}

KEMailDialogPrivate::~KEMailDialogPrivate()
{
    delete m_kemail;
}

KEMailDialog::KEMailDialog(QWidget *parent, Qt::WindowFlags flags)
    : KDialog(parent, flags),
    d(new KEMailDialogPrivate())
{
    d->ui.setupUi(mainWidget());
    d->ui.fromlineedit->setText(d->m_kemail->from());
    d->ui.userlineedit->setText(d->m_kemail->user());
    d->ui.passlineedit->setText(d->m_kemail->password());
    // TODO: connect(d->ui.settingslabel)
}

KEMailDialog::~KEMailDialog()
{
    delete d;
}

KEMail* KEMailDialog::kemail() const
{
    return d->m_kemail;
}

void KEMailDialog::slotButtonClicked(int button)
{
    if (button == KDialog::Ok) {
        d->m_kemail->setFrom(d->ui.fromlineedit->text());
        d->m_kemail->setUser(d->ui.userlineedit->text());
        d->m_kemail->setPassword(d->ui.passlineedit->text());
        d->m_kemail->setTo(d->ui.recipientslistwidget->items());
        if (!d->m_kemail->server().isValid()) {
            KMessageBox::error(this, i18n("No server specified"));
            return;
        } else if (d->m_kemail->from().isEmpty()) {
            KMessageBox::error(this, i18n("No sender specified"));
            return;
        } else if (d->m_kemail->to().isEmpty()) {
            KMessageBox::error(this, i18n("No recipients specified"));
            return;
        } else if (d->ui.sibjectlineedit->text().isEmpty()) {
            KMessageBox::error(this, i18n("No subject specified"));
            return;
        } else if (d->ui.messagetextedit->textOrHtml().isEmpty()) {
            KMessageBox::error(this, i18n("No message specified"));
            return;
        }
        // TODO: do it in thread
        const bool result = d->m_kemail->send(
            d->ui.sibjectlineedit->text(),
            d->ui.messagetextedit->textOrHtml(),
            d->ui.attachlistwidget->items()
        );
        if (!result) {
            KMessageBox::error(this, d->m_kemail->errorString());
            return;
        }
    }
    KDialog::slotButtonClicked(button);
}
