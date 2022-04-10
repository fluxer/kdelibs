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

#ifndef KPASSWDROULETTEDIALOG_H
#define KPASSWDROULETTEDIALOG_H

#include "kpasswdstore_export.h"

#include <kdialog.h>
#include <QWidget>

class KPasswdRouletteDialogPrivate;

/*!
    Class to ask for password from a choice of several passwords.

    @code
    KPasswdRouletteDialog kpasswdroulettedialog;
    kpasswdroulettedialog.addPasswd("Is foo in the bar?", "no");
    kpasswdroulettedialog.addPasswd("Can I haz a password?", "yes");
    if (kpasswdroulettedialog.exec() != KPasswdRouletteDialog::Accepted) {
        qDebug() << "password dialog not accepted";
    } else if (kpasswdroulettedialog.isValid()) {
        qDebug() << "password is valid";
    } else {
        qWarning() << "password is not valid";
    }
    @endcode

    @since 4.21
*/
class KPASSWDSTORE_EXPORT KPasswdRouletteDialog : public KDialog
{
    Q_OBJECT
public:
    /*!
        @brief Contructs object with @p parent
    */
    KPasswdRouletteDialog(QWidget *parent = nullptr);
    ~KPasswdRouletteDialog();

    /*!
        @brief Adds @p passwd as valid choice with the given @p label
    */
    void addPasswd(const QString &label, const QString &passwd);

    /*!
        @brief Returns @p true if valid password was entered, @p false
        otherwise
    */
    bool isValid() const;

public Q_SLOTS:
    virtual void accept();

private:
    Q_DISABLE_COPY(KPasswdRouletteDialog);
    KPasswdRouletteDialogPrivate *d;
};

#endif // KPASSWDROULETTEDIALOG_H
