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

#ifndef KTIMEEDIT_H
#define KTIMEEDIT_H

#include <kdeui_export.h>

#include <QWidget>
#include <QTime>

class KTimeEditPrivate;

/*!
    Class to pick a time, suitable for timers.

    @since 4.23
    @warning the API is subject to change
*/
class KDEUI_EXPORT KTimeEdit: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QTime minimumTime READ minimumTime WRITE setMinimumTime)
    Q_PROPERTY(QTime maximumTime READ maximumTime WRITE setMaximumTime)
    Q_PROPERTY(QTime time READ time WRITE setTime NOTIFY timeChanged)
public:
    KTimeEdit(QWidget *parent = nullptr);
    ~KTimeEdit();

    QTime minimumTime() const;
    void setMinimumTime(const QTime &time);
    QTime maximumTime() const;
    void setMaximumTime(const QTime &time);

    QTime time() const;
    void setTime(const QTime &time);

Q_SIGNALS:
    void timeChanged(const QTime &time);

private:
    friend KTimeEditPrivate;
    Q_DISABLE_COPY(KTimeEdit);
    KTimeEditPrivate *d;

    Q_PRIVATE_SLOT(d, void slotValueChanged(int))
};

#endif //  KTIMEEDIT_H
