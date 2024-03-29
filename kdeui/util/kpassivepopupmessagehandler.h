/*  This file is part of the KDE libraries
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KPASSIVEPOPUPMESSAGEHANDLER_H
#define KPASSIVEPOPUPMESSAGEHANDLER_H

#include <kdeui_export.h>
#include <kmessage.h>

#include <QObject>
#include <QWidget>

/**
 * @brief This is a convenience KMessageHandler that uses KPassivePopup.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class KDEUI_EXPORT KPassivePopupMessageHandler : public QObject, public KMessageHandler
{
    Q_OBJECT
public:
    /**
     * @brief Create a new KPassivePopupMessageHandler
     * @param parent Parent widget to use for the KPassivePopup.
     */
    explicit KPassivePopupMessageHandler(QWidget *parent = 0);

    /**
     * @copydoc KMessageHandler::message
     */
    virtual void message(KMessage::MessageType messageType, const QString &text, const QString &caption);
};

#endif // KPASSIVEPOPUPMESSAGEHANDLER_H
