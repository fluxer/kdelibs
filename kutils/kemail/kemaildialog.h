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

#ifndef KEMAILDIALOG_H
#define KEMAILDIALOG_H

#include "kemail_export.h"

#include <QShowEvent>
#include <kdialog.h>
#include <kemail.h>

class KEMailDialogPrivate;

/*!
    Class to send e-mail via convenience dialog.

    Example:
    \code
    KEMailDialog kemaildialog;
    kemaildialog.show();
    qDebug() << kemaildialog.exec();
    \endcode

    @since 4.22
    @warning the API is subject to change

    @see KEMail
    @see KEMailSettings
*/
class KEMAIL_EXPORT KEMailDialog : public KDialog
{
    Q_OBJECT
public:
    /*!
        @brief Contructs object with @p parent and @p flags
    */
    KEMailDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
    ~KEMailDialog();

    QString from() const;
    bool setFrom(const QString &from);
    QStringList to() const;
    bool setTo(const QStringList &to);
    QString subject() const;
    bool setSubject(const QString &subject);
    QString message() const;
    bool setMessage(const QString &message);
    QStringList attach() const;
    bool setAttach(const QStringList &attach);

    // QWidget reimplementation
protected:
    virtual void showEvent(QShowEvent *event);

    // KDialog reimplementation
protected Q_SLOTS:
    virtual void slotButtonClicked(int button);

private Q_SLOTS:
    void _slotSettings();
    void _slotSent();
    void _slotError(const QString &errorstring);
    void _slotFinished();

private:
    Q_DISABLE_COPY(KEMailDialog);
    KEMailDialogPrivate *d;
};

#endif // KEMAILDIALOG_H
