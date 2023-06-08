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

#ifndef KSPELLDICTIONARYCOMBOBOX_H
#define KSPELLDICTIONARYCOMBOBOX_H

#include "kdeui_export.h"
#include "kcombobox.h"

/*!
    Widget to show and choose from available spelling dictionaries.

    @since 4.23
*/
class KDEUI_EXPORT KSpellDictionaryComboBox : public KComboBox
{
    Q_OBJECT
public:
    KSpellDictionaryComboBox(QWidget *parent = nullptr);

    QString currentDictionary() const;
    void setCurrentByDictionary(const QString &dictionary);

Q_SIGNALS:
    void dictionaryChanged(const QString &dictionary);
    void dictionaryNameChanged(const QString &dictionaryName);

private Q_SLOTS:
    void _dictionaryChanged(const int index);
};

#endif // KSPELLDICTIONARYCOMBOBOX_H 
