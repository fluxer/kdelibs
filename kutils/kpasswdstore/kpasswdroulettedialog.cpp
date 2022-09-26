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

#include "kpasswdroulettedialog.h"
#include "krandom.h"
#include "klocale.h"
#include "kdebug.h"

#include "ui_kpasswdroulettedialog.h"

#include <QProcess>

class KPasswdRouletteDialogPrivate
{
public:
    KPasswdRouletteDialogPrivate();

    bool isvalid;
    QString validpasswd;
    QMap<QString, QString> passwdmap;
    Ui::KPasswdRouletteDialogUI ui;
};

KPasswdRouletteDialogPrivate::KPasswdRouletteDialogPrivate()
    : isvalid(false)
{
}

KPasswdRouletteDialog::KPasswdRouletteDialog(QWidget *parent)
    : KDialog(parent),
    d(new KPasswdRouletteDialogPrivate())
{
    KDialog::setButtons(KDialog::Ok | KDialog::Cancel);

    d->ui.setupUi(mainWidget());

    d->ui.ksqueezedtextlabel->setText(i18n("Enter password:"));
    d->ui.klineedit->setFocus();
}

KPasswdRouletteDialog::~KPasswdRouletteDialog()
{
    delete d;
}

bool KPasswdRouletteDialog::fortunate(const uint max)
{
    QMap<QString, QString> passwordsmap;
    while (passwordsmap.size() < max) {
        QProcess fortuneproc(this);
        fortuneproc.start("fortune");
        if (!fortuneproc.waitForStarted() || !fortuneproc.waitForFinished()) {
            kWarning() << "Fortune process failed";
            return false;
        }
        const QByteArray fortunedata = fortuneproc.readAllStandardOutput();
        QString question;
        QString answer;
        foreach (const QByteArray &fortuneline, fortunedata.split('\n')) {
            const QByteArray trimmedfortuneline = fortuneline.trimmed();
            if (trimmedfortuneline.startsWith("Q:")) {
                question = QString::fromLocal8Bit(trimmedfortuneline.constData(), trimmedfortuneline.size());
                question = question.mid(2).trimmed();
            } else if (trimmedfortuneline.startsWith("A:")) {
                answer = QString::fromLocal8Bit(trimmedfortuneline.constData(), trimmedfortuneline.size());
                answer = answer.mid(2).trimmed();
            }
            if (!question.isEmpty() && !answer.isEmpty()) {
                passwordsmap.insert(question, answer);
                break;
            }
        }
    }

    foreach (const QString &question, passwordsmap.keys()) {
        addPasswd(question, passwordsmap.value(question));
    }
    return true;
}

void KPasswdRouletteDialog::addPasswd(const QString &label, const QString &passwd)
{
    d->passwdmap.insert(label, passwd);

    const int randomrange = KRandom::randomMax(d->passwdmap.size());
    const QList<QString> labelslist = d->passwdmap.keys();
    const QString randomkey = labelslist.at(randomrange);
    d->validpasswd = d->passwdmap.value(randomkey);

    d->ui.ksqueezedtextlabel->setText(randomkey);
    d->ui.klineedit->clear();
}

bool KPasswdRouletteDialog::isValid() const
{
    return d->isvalid;
}

void KPasswdRouletteDialog::accept()
{
    d->isvalid = (d->ui.klineedit->text() == d->validpasswd);
    KDialog::accept();
}
