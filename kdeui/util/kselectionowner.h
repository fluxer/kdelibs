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

#ifndef KSELECTIONOWNER_H
#define KSELECTIONOWNER_H

#include <kdeui_export.h>

#include <QObject>
#include <QTimerEvent>

#include <X11/Xlib.h>
#include <fixx11h.h>

class KSelectionOwnerPrivate;

/*!
    Class to claim selection of X11 atom.
*/
class KDEUI_EXPORT KSelectionOwner : public QObject
{
    Q_OBJECT
public:
    KSelectionOwner(const char* atom, const int screen = -1, QObject *parent = nullptr);
    ~KSelectionOwner();

    Window ownerWindow() const;

public Q_SLOTS:
    bool claim(const bool force);
    void release();

Q_SIGNALS:
    void lostOwnership();

protected:
    //! @brief Reimplementation to support ownership check
    virtual void timerEvent(QTimerEvent *event);

private:
    Q_DISABLE_COPY(KSelectionOwner);
    KSelectionOwnerPrivate * const d;
};

#endif // KSELECTIONOWNER_H
