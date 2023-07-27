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
 
#ifndef KSWITCHLANGUAGEDIALOG_H
#define KSWITCHLANGUAGEDIALOG_H

#include "kdialog.h"
#include "keditlistwidget.h"

#include <QVBoxLayout>
#include <QLabel>

/**
 * @short Standard "switch application language" dialog box.
 *
 * This class provides "switch application language" dialog box that is used
 * in KHelpMenu
 *
 * @internal
 */
class KSwitchLanguageDialog : public KDialog
{
    Q_OBJECT
public:
    KSwitchLanguageDialog(QWidget *parent = nullptr);
    ~KSwitchLanguageDialog();
    
private Q_SLOTS:
    void slotOk();
    void slotDefault();

private:
    QWidget* m_dialogwidget;
    QVBoxLayout* m_dialoglayout;
    QLabel* m_languagelabel;
    KEditListWidget* m_languageedit;
};

#endif // KSWITCHLANGUAGEDIALOG_H
