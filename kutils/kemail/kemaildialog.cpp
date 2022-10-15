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

#include <QThread>
#include <QProcess>

class KEMailDialogPrivate : public QThread
{
    Q_OBJECT
public:
    KEMailDialogPrivate();
    ~KEMailDialogPrivate();

    KEMail* m_kemail;
    Ui::KEMailDialogUI ui;

    void sendMail(const QString &subject, const QString &message, const QStringList &attach);

Q_SIGNALS:
    void sent();
    void error(const QString &errorstring);

protected:
    void run() final;

private:
    QString m_subject;
    QString m_message;
    QStringList m_attach;
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

void KEMailDialogPrivate::sendMail(const QString &subject, const QString &message, const QStringList &attach)
{
    m_subject = subject;
    m_message = message;
    m_attach = attach;
    start();
}

void KEMailDialogPrivate::run()
{
    const bool result = m_kemail->send(m_subject, m_message, m_attach);
    if (result) {
        emit sent();
    } else {
        emit error(m_kemail->errorString());
    }
}


KEMailDialog::KEMailDialog(QWidget *parent, Qt::WindowFlags flags)
    : KDialog(parent, flags),
    d(new KEMailDialogPrivate())
{
    d->ui.setupUi(mainWidget());
    d->ui.fromlineedit->setText(d->m_kemail->from());
    d->ui.userlineedit->setText(d->m_kemail->user());
    d->ui.passlineedit->setText(d->m_kemail->password());
    connect(d->ui.settingslabel, SIGNAL(leftClickedUrl()), this, SLOT(_slotSettings()));

    connect(d, SIGNAL(sent()), this, SLOT(_slotSent()));
    connect(d, SIGNAL(error(QString)), this, SLOT(_slotError(QString)));
}

KEMailDialog::~KEMailDialog()
{
    delete d;
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
        d->sendMail(
            d->ui.sibjectlineedit->text(),
            d->ui.messagetextedit->textOrHtml(),
            d->ui.attachlistwidget->items()
        );
        return;
    }
    KDialog::slotButtonClicked(button);
}

void KEMailDialog::_slotSettings()
{
    QProcess::startDetached(
        QString::fromLatin1("kcmshell4"),
        QStringList() << QString::fromLatin1("useraccount")
    );
}

void KEMailDialog::_slotSent()
{
    KDialog::slotButtonClicked(KDialog::Ok);
}

void KEMailDialog::_slotError(const QString &errorstring)
{
    KMessageBox::error(this, errorstring);
}

#include "kemaildialog.moc"
