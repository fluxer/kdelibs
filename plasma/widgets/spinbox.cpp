/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Davide Bettio <davide.bettio@kdemail.net>
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

#include "spinbox.h"

#include "applet.h"
#include "private/style_p.h"
#include "private/themedwidgetinterface_p.h"
#include "theme.h"

#include <QGraphicsView>
#include <kmimetype.h>
#include <knuminput.h>

namespace Plasma
{

class SpinBoxPrivate : public ThemedWidgetInterface<SpinBox>
{
public:
    SpinBoxPrivate(SpinBox *spinBox)
        : ThemedWidgetInterface<SpinBox>(spinBox)
    {
        buttonColorForText = true;
    }

    Plasma::Style::Ptr style;
};

SpinBox::SpinBox(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new SpinBoxPrivate(this))
{
    KIntSpinBox *native = new KIntSpinBox;

    connect(native, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));
    connect(native, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));

    d->setWidget(native);
    native->setWindowIcon(QIcon());
    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setAutoFillBackground(false);

    d->style = Plasma::Style::sharedStyle();
    native->setStyle(d->style.data());
    d->initTheming();
}

SpinBox::~SpinBox()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void SpinBox::setMaximum(int max)
{
    static_cast<KIntSpinBox*>(widget())->setMaximum(max);
}

int SpinBox::maximum() const
{
    return static_cast<KIntSpinBox*>(widget())->maximum();
}

void SpinBox::setMinimum(int min)
{
    static_cast<KIntSpinBox*>(widget())->setMinimum(min);
}

int SpinBox::minimum() const
{
    return static_cast<KIntSpinBox*>(widget())->minimum();
}

void SpinBox::setRange(int min, int max)
{
    static_cast<KIntSpinBox*>(widget())->setRange(min, max);
}

void SpinBox::setValue(int value)
{
    static_cast<KIntSpinBox*>(widget())->setValue(value);
}

int SpinBox::value() const
{
    return static_cast<KIntSpinBox*>(widget())->value();
}

void SpinBox::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString SpinBox::styleSheet()
{
    return widget()->styleSheet();
}

KIntSpinBox *SpinBox::nativeWidget() const
{
    return static_cast<KIntSpinBox*>(widget());
}

void SpinBox::changeEvent(QEvent *event)
{
    d->changeEvent(event);
    QGraphicsProxyWidget::changeEvent(event);
}

void SpinBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget *widget = parentWidget();
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);

    while (!applet && widget) {
        widget = widget->parentWidget();
        applet = qobject_cast<Plasma::Applet *>(widget);
    }

    if (applet) {
        applet->setStatus(Plasma::AcceptingInputStatus);
    }
    QGraphicsProxyWidget::mousePressEvent(event);
}

void SpinBox::focusOutEvent(QFocusEvent *event)
{
    QGraphicsWidget *widget = parentWidget();
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);

    while (!applet && widget) {
        widget = widget->parentWidget();
        applet = qobject_cast<Plasma::Applet *>(widget);
    }

    if (applet) {
        applet->setStatus(Plasma::UnknownStatus);
    }

    QEvent closeEvent(QEvent::CloseSoftwareInputPanel);
    if (qApp) {
        if (QGraphicsView *view = qobject_cast<QGraphicsView*>(qApp->focusWidget())) {
            if (view->scene() && view->scene() == scene()) {
                QApplication::sendEvent(view, &closeEvent);
            }
        }
    }

    QGraphicsProxyWidget::focusOutEvent(event);
}

} // namespace Plasma

#include "moc_spinbox.cpp"

