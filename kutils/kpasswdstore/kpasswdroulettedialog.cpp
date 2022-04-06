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
#include "krandomsequence.h"
#include "klocale.h"
#include "kdebug.h"

#include "ui_kpasswdroulettedialog.h"

class KPasswdRouletteDialogPrivate
{
public:
    KPasswdRouletteDialogPrivate();

    bool isvalid;
    QString validpasswd;
    QMap<QString, QString> passwdmap;
    KRandomSequence krandomsequence;
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

void KPasswdRouletteDialog::addPasswd(const QString &label, const QString &passwd)
{
    d->passwdmap.insert(label, passwd);

    const int randomrange = d->krandomsequence.getLong(d->passwdmap.size());
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
