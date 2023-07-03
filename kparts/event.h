/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __kparts_event_h__
#define __kparts_event_h__

#include <QtGui/qevent.h>
#include <QWidget>

#include <kparts/kparts_export.h>


namespace KParts
{
class Part;

class EventPrivate;
/**
 * Base class for all KParts events.
 */
class KPARTS_EXPORT Event : public QEvent
{
public:
  Event( const char *eventName );
  ~Event();
  const char *eventName() const;

  static bool test( const QEvent *event );
  static bool test( const QEvent *event, const char *name );

private:
  EventPrivate * const d;
};

class GUIActivateEventPrivate;
/**
 * This event is sent to a Part when its GUI has been activated or deactivated.
 * GUIActivateEvent happens when the GUI is actually built, only for parts that
 * have GUI elements, and only if using KParts::MainWindow.
 * @see KParts::Part::guiActivateEvent()
 */
class KPARTS_EXPORT GUIActivateEvent : public Event
{
public:
  GUIActivateEvent( bool activated );
  ~GUIActivateEvent();

  bool activated() const;

  static bool test( const QEvent *event );

private:
  GUIActivateEventPrivate * const d;
};

} // namespace

#endif
