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
#include "kstandarddirs.h"
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

    KEMail* kemail;
    Ui::KEMailDialogUI ui;

    void sendMail(const QStringList &to, const QString &subject, const QString &message, const QStringList &attach);

Q_SIGNALS:
    void sent();
    void error(const QString &errorstring);

protected:
    void run() final;

private:
    QStringList m_to;
    QString m_subject;
    QString m_message;
    QStringList m_attach;
};

KEMailDialogPrivate::KEMailDialogPrivate()
    : kemail(nullptr)
{
    kemail = new KEMail(this);
}

KEMailDialogPrivate::~KEMailDialogPrivate()
{
    delete kemail;
}

void KEMailDialogPrivate::sendMail(const QStringList &to, const QString &subject, const QString &message, const QStringList &attach)
{
    m_to = to;
    m_subject = subject;
    m_message = message;
    m_attach = attach;
    start();
}

void KEMailDialogPrivate::run()
{
    const bool result = kemail->send(m_to, m_subject, m_message, m_attach);
    if (result) {
        emit sent();
    } else {
        emit error(kemail->errorString());
    }
}


KEMailDialog::KEMailDialog(QWidget *parent, Qt::WindowFlags flags)
    : KDialog(parent, flags),
    d(new KEMailDialogPrivate())
{
    d->ui.setupUi(mainWidget());
    d->ui.fromlineedit->setText(d->kemail->from());
    d->ui.userlineedit->setText(d->kemail->user());
    d->ui.passlineedit->setText(d->kemail->password());
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
        d->kemail->setFrom(d->ui.fromlineedit->text());
        d->kemail->setUser(d->ui.userlineedit->text());
        d->kemail->setPassword(d->ui.passlineedit->text());
        if (!d->kemail->server().isValid()) {
            KMessageBox::error(this, i18n("No server specified"));
            return;
        } else if (d->kemail->from().isEmpty()) {
            KMessageBox::error(this, i18n("No sender specified"));
            return;
        } else if (d->ui.recipientslistwidget->items().isEmpty()) {
            KMessageBox::error(this, i18n("No recipients specified"));
            return;
        } else if (d->ui.subjectlineedit->text().isEmpty()) {
            KMessageBox::error(this, i18n("No subject specified"));
            return;
        } else if (d->ui.messagetextedit->textOrHtml().isEmpty()) {
            KMessageBox::error(this, i18n("No message specified"));
            return;
        }
        KDialog::enableButtonOk(false);
        d->sendMail(
            d->ui.recipientslistwidget->items(),
            d->ui.subjectlineedit->text(),
            d->ui.messagetextedit->textOrHtml(),
            d->ui.attachlistwidget->items()
        );
        return;
    } else if (button == KDialog::Cancel) {
        if (d->isRunning() &&
            KMessageBox::questionYesNo(this, i18n("Mail is being send, are you sure?")) == KMessageBox::Yes) {
            d->terminate();
        }
    }
    KDialog::slotButtonClicked(button);
}

QString KEMailDialog::from() const
{
    return d->kemail->from();
}

bool KEMailDialog::setFrom(const QString &from)
{
    return d->kemail->setFrom(from);
}

QStringList KEMailDialog::to() const
{
    return d->ui.recipientslistwidget->items();
}

bool KEMailDialog::setTo(const QStringList &to)
{
    if (to.isEmpty()) {
        return false;
    }
    d->ui.recipientslistwidget->setItems(to);
    return true;
}

QString KEMailDialog::subject() const
{
    return d->ui.subjectlineedit->text();
}

bool KEMailDialog::setSubject(const QString &subject)
{
    if (subject.isEmpty()) {
        return false;
    }
    d->ui.subjectlineedit->setText(subject);
    return true;
}

QString KEMailDialog::message() const
{
    return d->ui.messagetextedit->textOrHtml();
}

bool KEMailDialog::setMessage(const QString &message)
{
    if (message.isEmpty()) {
        return false;
    }
    d->ui.messagetextedit->setText(message);
    return true;
}

QStringList KEMailDialog::attach() const
{
    return d->ui.attachlistwidget->items();
}

bool KEMailDialog::setAttach(const QStringList &attach)
{
    if (attach.isEmpty()) {
        return false;
    }
    d->ui.attachlistwidget->setItems(attach);
    return true;
}

void KEMailDialog::_slotSettings()
{
    const QString kcmshell4exe = KStandardDirs::findExe(QString::fromLatin1("kcmshell4"));
    if (kcmshell4exe.isEmpty()) {
        KMessageBox::error(this, i18n("kcmshell4 not found"));
        return;
    }
    QProcess::startDetached(
        kcmshell4exe,
        QStringList() << QString::fromLatin1("useraccount")
    );
}

void KEMailDialog::_slotSent()
{
    KMessageBox::information(this, i18n("Mail sent"), QString(), QString::fromLatin1("mailsent"));
    KDialog::enableButtonOk(true);
    KDialog::slotButtonClicked(KDialog::Ok);
}

void KEMailDialog::_slotError(const QString &errorstring)
{
    KDialog::enableButtonOk(true);
    KMessageBox::error(this, errorstring);
}

#include "kemaildialog.moc"
