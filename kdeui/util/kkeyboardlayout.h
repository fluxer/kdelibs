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

#ifndef KKEYBOARDLAYOUT_H
#define KKEYBOARDLAYOUT_H

#include <kdeui_export.h>

#include <QObject>
#include <QList>
#include <qwindowdefs.h>

class KKeyboardLayoutPrivate;

struct KKeyboardType
{
    QByteArray rule;
    QByteArray model;
    QByteArray layout;
    QByteArray variant;
    QByteArray option;

    bool operator==(const KKeyboardType &other) const;
};

/*!
    Class to query and change the keyboard layout.

    @since 4.24
    @warning the API is subject to change
*/
class KDEUI_EXPORT KKeyboardLayout : public QObject
{
    Q_OBJECT
public:
    KKeyboardLayout(QObject *parent = nullptr);
    ~KKeyboardLayout();

    QList<KKeyboardType> layouts() const;
    bool setLayouts(const QList<KKeyboardType> &layouts);

    static KKeyboardType defaultLayout();

    static QList<QByteArray> modelNames();
    static QList<QByteArray> layoutNames();
    static QList<QByteArray> variantNames(const QByteArray &layout);
    static QList<QByteArray> optionNames();
    static QString modelDescription(const QByteArray &model);
    static QString layoutDescription(const QByteArray &layout);
    static QString variantDescription(const QByteArray &layout, const QByteArray &variant);
    static QString optionDescription(const QByteArray &option);

Q_SIGNALS:
    void layoutChanged();

private:
    friend KKeyboardLayoutPrivate;
    Q_DISABLE_COPY(KKeyboardLayout);
    KKeyboardLayoutPrivate * const d;

    Q_PRIVATE_SLOT(d, void _k_checkLayouts())
};

#endif // KKEYBOARDLAYOUT_H
