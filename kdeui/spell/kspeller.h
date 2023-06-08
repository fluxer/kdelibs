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

#ifndef KSPELLER_H
#define KSPELLER_H

#include "kdeui_export.h"
#include "kconfig.h"

#include <QObject>
#include <QStringList>

class KSpellerPrivate;

/*!
    Class to check spelling and query for corrections.

    @since 4.23
*/
class KDEUI_EXPORT KSpeller : public QObject
{
    Q_OBJECT
public:
    KSpeller(KConfig *config, QObject *parent = nullptr);
    ~KSpeller();

    QStringList dictionaries() const;
    QString dictionary() const;
    bool setDictionary(const QString &dictionary);

    bool check(const QString &word);
    QStringList suggest(const QString &word);

    bool addToPersonal(const QString &word);
    bool removeFromPersonal(const QString &word);
    bool addToSession(const QString &word);
    bool removeFromSession(const QString &word);

    static QString defaultLanguage();

private:
    Q_DISABLE_COPY(KSpeller);
    KSpellerPrivate *d;
};

#endif // KSPELLER_H 
