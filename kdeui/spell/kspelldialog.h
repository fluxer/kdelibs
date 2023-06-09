/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KSPELLDIALOG_H
#define KSPELLDIALOG_H

#include "kdeui_export.h"
#include "kdialog.h"

class KSpellDialogPrivate;

/*!
    Dialog to check spelling and apply correction interactively.

    @since 4.23
*/
class KDEUI_EXPORT KSpellDialog : public KDialog
{
    Q_OBJECT
public:
    KSpellDialog(KConfig *config, QWidget *parent = nullptr);
    ~KSpellDialog();

    void showSpellCheckCompletionMessage(bool b = true);
    void setSpellCheckContinuedAfterReplacement(bool b);

    QString buffer() const;

public Q_SLOTS:
    void setBuffer(const QString &buffer);
    void changeLanguage(const QString &lang);

Q_SIGNALS:
    void misspelling(const QString &word, int start);
    void replace(const QString &oldWord, int start, const QString &newWord);
    void languageChanged(const QString &language);

private Q_SLOTS:
    void _correct();
    void _next();
    void _done();
    void _suggest(const QString &word);

private:
    Q_DISABLE_COPY(KSpellDialog);
    KSpellDialogPrivate *d;
};

#endif // KSPELLDIALOG_H 
