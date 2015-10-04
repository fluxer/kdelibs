/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "focusindicator_p.h"

#include <QtGui/qgraphicssceneevent.h>
#include <QPainter>
#include <QStringBuilder>
#include <QtGui/qstyleoption.h>

#include <plasma/theme.h>
#include <plasma/framesvg.h>

namespace Plasma
{

FocusIndicator::FocusIndicator(QGraphicsWidget *parent, const QString &widget)
    : QGraphicsWidget(parent),
      m_parent(parent),
      m_background(new Plasma::FrameSvg(this)),
      m_isUnderMouse(false)
{
    m_background->setImagePath(widget);
    init(parent);
}

FocusIndicator::FocusIndicator(QGraphicsWidget *parent, FrameSvg *svg)
    : QGraphicsWidget(parent),
      m_parent(parent),
      m_background(svg),
      m_isUnderMouse(false)
{
    init(parent);
}

void FocusIndicator::init(QGraphicsWidget *parent)
{
    setVisible(!Theme::defaultTheme()->useNativeWidgetStyle());
    setFlag(QGraphicsItem::ItemStacksBehindParent);
    setAcceptsHoverEvents(true);

    m_background->setCacheAllRenderedFrames(true);

    m_testPrefix = "hover";
    if (m_background->hasElementPrefix("shadow") ||
        m_background->hasElement("shadow")) {
        m_prefix = "shadow";
    }

    parent->installEventFilter(this);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(syncGeometry()));
}

FocusIndicator::~FocusIndicator()
{
    m_parent->removeEventFilter(this);
}

void FocusIndicator::setCustomGeometry(const QRectF &geometry)
{
    if (m_customGeometry == geometry) {
        return;
    }

    m_customGeometry = geometry;
    syncGeometry();
}

void FocusIndicator::setCustomPrefix(const QString &prefix)
{
    QString was = m_prefix;
    if (!m_prefix.isEmpty() && !m_customPrefix.isEmpty()) {
        m_prefix.remove(m_customPrefix);
    }

    m_customPrefix = prefix;

    if (!m_prefix.isEmpty()) {
        m_prefix.prepend(m_customPrefix);
    }

    m_testPrefix = m_customPrefix % "hover";
    if (m_prefix.isEmpty()) {
        m_prefix = m_customPrefix % "shadow";
    }

    if (m_prefix == was) {
        return;
    }

    syncGeometry();
    resizeEvent(0);
}

bool FocusIndicator::eventFilter(QObject *watched, QEvent *event)
{
    if (Theme::defaultTheme()->useNativeWidgetStyle() ||
        static_cast<QGraphicsWidget *>(watched) != m_parent || !m_parent ) {
        return false;
    }

    if (event->type() == QEvent::GraphicsSceneHoverEnter) {
        m_isUnderMouse = true;
    } else if (event->type() == QEvent::GraphicsSceneHoverLeave) {
        m_isUnderMouse = false;
    }

    switch (event->type()) {
        case QEvent::GraphicsSceneHoverEnter:
            if (!m_parent->hasFocus()) {
                m_prefix = m_customPrefix % "hover";
                syncGeometry();
                if (m_background->hasElementPrefix(m_testPrefix)) {
                    m_background->setElementPrefix(m_customPrefix % "shadow");
                    m_background->setElementPrefix(m_customPrefix % "hover");
                }
            }
            break;

        case QEvent::GraphicsSceneHoverLeave:
            if (!m_parent->hasFocus()) {
                m_prefix = m_customPrefix % "shadow";
                syncGeometry();

                if (m_background->hasElementPrefix(m_testPrefix)) {
                    m_background->setElementPrefix(m_customPrefix % "hover");
                    m_background->setElementPrefix(m_customPrefix % "shadow");
                }
            }
            break;

        case QEvent::GraphicsSceneResize:
            syncGeometry();
        break;

        case QEvent::FocusIn:
            m_prefix = m_customPrefix % "focus";
            syncGeometry();

            if (m_background->hasElementPrefix(m_customPrefix % "focus")) {
                //m_background->setElementPrefix(m_customPrefix % "shadow");
                m_background->setElementPrefix(m_customPrefix % "focus");
            }

            break;

        case QEvent::FocusOut:
            if (!m_isUnderMouse) {
                m_prefix = m_customPrefix % "shadow";
                syncGeometry();

                if (m_background->hasElementPrefix(m_customPrefix % "focus")) {
                    m_background->setElementPrefix("focus");
                    m_background->setElementPrefix("shadow");
                }
            }
            break;

        default:
            break;
    };

    return false;
}

void FocusIndicator::resizeEvent(QGraphicsSceneResizeEvent *)
{
    if (m_background->hasElementPrefix(m_customPrefix % "shadow")) {
        m_background->setElementPrefix(m_customPrefix % "shadow");
        m_background->resizeFrame(size());
    }

    if (m_background->hasElementPrefix(m_customPrefix % "hover")) { 
        m_background->setElementPrefix(m_customPrefix % "hover");
        m_background->resizeFrame(size());
    }

    if (m_background->hasElementPrefix(m_customPrefix % "focus")) { 
        m_background->setElementPrefix(m_customPrefix % "focus");
        m_background->resizeFrame(size());
    }

    if (m_background->hasElementPrefix(m_testPrefix)) {
        m_background->setElementPrefix(m_prefix);
    }
}

void FocusIndicator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
#pragma remove? it is a stub now...
}

void FocusIndicator::syncGeometry()
{
    if (Theme::defaultTheme()->useNativeWidgetStyle()) {
        hide();
        return;
    } else if (!isVisible()) {
        show();
    }

    QRectF geom;
    if (!m_customGeometry.isEmpty()) {
        geom = m_customGeometry;
    } else {
        geom = m_parent->boundingRect();
    }

    if (m_background->hasElementPrefix(m_testPrefix)) {
        //always take borders from hover to make it stable
        m_background->setElementPrefix(m_testPrefix);
        qreal left, top, right, bottom;
        m_background->getMargins(left, top, right, bottom);
        m_background->setElementPrefix(m_prefix);
        setGeometry(QRectF(geom.topLeft() + QPointF(-left, -top), geom.size() + QSize(left+right, top+bottom)));
    } else if (m_background->hasElement(m_testPrefix)) {
        QRectF elementRect = m_background->elementRect(m_testPrefix);
        elementRect.moveCenter(geom.center());
        setGeometry(elementRect);
    }
}

void FocusIndicator::setFrameSvg(FrameSvg *frameSvg)
{
    if (m_background != frameSvg) {
        m_background = frameSvg;
    }
}

FrameSvg *FocusIndicator::frameSvg() const
{
    return m_background;
}

}

#include "moc_focusindicator_p.cpp"

