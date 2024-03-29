/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *                 2007 Alexis Ménard <darktears31@gmail.com>
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

#ifndef PLASMA_ANIMATOR_H
#define PLASMA_ANIMATOR_H

#include <QImage>
#include <QObject>
#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QTimeLine>

#include <plasma/plasma_export.h>

namespace Plasma
{

class Animation;

/**
 * @class Animator plasma/animator.h <Plasma/Animator>
 *
 * @short A system for applying effects to Plasma elements
 */
class PLASMA_EXPORT Animator : public QObject
{
    Q_OBJECT
    Q_ENUMS(Animation)

public:
    enum Animation {
        FadeAnimation = 0,         /* Can be used for both fade in and out */
        PulseAnimation,            /* Pulse animated object (opacity/geometry/scale) */
        SlideAnimation,            /* Move the position of animated object */
        GeometryAnimation,         /* Geometry animation */
        ZoomAnimation,             /* Zoom animation */
        PixmapTransitionAnimation, /* Transition between two pixmaps */
        LastAnimation = 1024
    };

    /**
     * Factory to build new animation objects. To control their behavior,
     * check \ref AbstractAnimation properties.
     * @since 4.4
     **/
    static Plasma::Animation *create(Animator::Animation type, QObject *parent = 0);

private:
    Animator();
};

} // namespace Plasma

#endif

