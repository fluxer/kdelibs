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

#ifndef KSPELLBACKGROUNDCHECKER_H
#define KSPELLBACKGROUNDCHECKER_H

#include "kdeui_export.h"
#include "kconfig.h"

#include <QObject>

class KSpellBackgroundCheckerPrivate;

/*!
    Class to check spelling in the background.

    @since 4.23
*/
class KDEUI_EXPORT KSpellBackgroundChecker : public QObject
{
    Q_OBJECT
public:
    KSpellBackgroundChecker(KConfig *config, QObject *parent = nullptr);
    ~KSpellBackgroundChecker();

    void setText(const QString &text);
    QString text() const;

public Q_SLOTS:
    void start();
    void stop();
    void changeLanguage(const QString &lang);

Q_SIGNALS:
    void misspelling(const QString &word, int start);
    void done();

private:
    Q_DISABLE_COPY(KSpellBackgroundChecker);
    KSpellBackgroundCheckerPrivate *d;
};

#endif // KSPELLBACKGROUNDCHECKER_H 
