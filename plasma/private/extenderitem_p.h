/*
 * Copyright 2008 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef LIBS_PLASMA_EXTENDERITEM_P_H
#define LIBS_PLASMA_EXTENDERITEM_P_H

#include <QtCore/qpoint.h>
#include <QPoint>
#include <QRect>
#include <QString>

#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsView>
#include <QTimer>

namespace Plasma
{

class Applet;
class ExtenderGroup;
class ExtenderItem;
class Extender;
class Label;
class IconWidget;
class FrameSvg;
class ExtenderItemToolbox;

class ExtenderItemPrivate
{
    public:
        ExtenderItemPrivate(ExtenderItem *extenderItem, Extender *hostExtender);
        ~ExtenderItemPrivate();

        QRectF dragHandleRect();
        void toggleCollapse();
        void updateToolBox();
        Applet *hostApplet() const;
        void themeChanged();
        void sourceAppletRemoved();
        void resizeContent(const QSizeF &newSize);
        void actionDestroyed(QObject *o);
        void updateSizeHints();
        void setMovable(bool movable);

        ExtenderItem *q;

        QWeakPointer<QGraphicsWidget> widget;

        ExtenderItemToolbox *toolbox;
        QGraphicsLinearLayout *toolboxLayout;
        QGraphicsLinearLayout *layout;

        Extender *extender;
        Applet *sourceApplet;
        ExtenderGroup *group;

        KConfigGroup config;
        KSharedConfig::Ptr transientConfig;

        FrameSvg *background;

        IconWidget *collapseIcon;
        Label *titleLabel;

        QHash<QString, QAction*> actions;
        QList<QAction*> actionsInOrder;

        QString name;
        QString iconName;

        uint extenderItemId;
        QTimer *expirationTimer;

        bool dragStarted : 1;
        bool destroyActionVisibility : 1;
        bool collapsed : 1;
        bool transient : 1;

        static uint s_maxExtenderItemId;
};

}

#endif // LIB_PLASMA_EXTENDERITEM_P_H
