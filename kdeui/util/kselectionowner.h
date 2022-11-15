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
    /*!
        @brief Constructs object for @p atomname and @p screen with @p parent in preparation to
        claim selection.
    */
    KSelectionOwner(const char* const atomname, const int screen = -1, QObject *parent = nullptr);
    ~KSelectionOwner();

    /*!
        @brief Returns the X11 window used to claim the selection, valid only if selection is
        claimed by this object and until it is either released or lost
    */
    Window ownerWindow() const;

    /*!
        @brief Returns the current X11 window that claims the selection, may not be the same as
        the one returned by @p ownerWindow()
    */
    Window currentOwnerWindow() const;

public Q_SLOTS:
    /*!
        @brief Attempts to claim the selection, if @p force is true and the selection is owned by
        someone else the owner selection will be cleared and possibly killed.
    */
    bool claim(const bool force);
    /*!
        @brief Releases the selection.
    */
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
