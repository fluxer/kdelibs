/* This file is part of the KDE libraries

   Copyright (c) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
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

#ifndef KCURSOR_P_H
#define KCURSOR_P_H

#include <QtCore/qhash.h>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtGui/QCursor>

#include <QWidget>

/**
 * I don't want the eventFilter to be in KCursor, so we have another class
 * for that stuff
 * @internal
 * @author John Firebaugh <jfirebaugh@kde.org>
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
*/
class KCursorPrivateAutoHideEventFilter : public QObject
{
    Q_OBJECT

public:
    KCursorPrivateAutoHideEventFilter(QWidget*widget);
    ~KCursorPrivateAutoHideEventFilter();

    virtual bool eventFilter(QObject *o, QEvent *e);

    void resetWidget();

private Q_SLOTS:
    void hideCursor();
    void unhideCursor();

private:
    QWidget* mouseWidget() const;

    QTimer m_autoHideTimer;
    QWidget* m_widget;
    bool m_wasMouseTracking;
    bool m_isCursorHidden;
    bool m_isOwnCursor;
    QCursor m_oldCursor;
};

/**
 * @internal
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @author John Firebaugh <jfirebaugh@kde.org>
*/
class KCursorPrivate : public QObject
{
    friend class KCursor; // to shut up the compiler
    Q_OBJECT

public:
    static KCursorPrivate *self();

    void setAutoHideCursor(QWidget *w, bool enable, bool customEventFilter);
    virtual bool eventFilter(QObject *o, QEvent *e);

    int hideCursorDelay;

private Q_SLOTS:
    void slotViewportDestroyed(QObject *o);
    void slotWidgetDestroyed(QObject *o);

private:
    KCursorPrivate();
    ~KCursorPrivate();

    bool enabled;
    static KCursorPrivate *s_self;

    QHash<QObject*,KCursorPrivateAutoHideEventFilter*> m_eventFilters;
};



#endif // KCURSOR_PRIVATE_H
