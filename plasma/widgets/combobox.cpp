/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "combobox.h"

#include <QPainter>
#include <QGraphicsView>

#include <kcombobox.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kmimetype.h>

#include "applet.h"
#include "private/style_p.h"
#include "private/themedwidgetinterface_p.h"
#include "theme.h"

namespace Plasma
{

class ComboBoxPrivate : public ThemedWidgetInterface<ComboBox>
{
public:
    ComboBoxPrivate(ComboBox *comboBox)
         : ThemedWidgetInterface<ComboBox>(comboBox)
    {
        buttonColorForText = true;
    }

    Style::Ptr style;
};

ComboBox::ComboBox(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new ComboBoxPrivate(this))
{
    setZValue(900);

    setAcceptHoverEvents(true);

    d->style = Style::sharedStyle();

    setNativeWidget(new KComboBox());
    d->initTheming();
}

ComboBox::~ComboBox()
{
    delete d;
    Style::doneWithSharedStyle();
}

QString ComboBox::text() const
{
    return static_cast<KComboBox*>(widget())->currentText();
}

void ComboBox::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString ComboBox::styleSheet()
{
    return widget()->styleSheet();
}

void ComboBox::setNativeWidget(KComboBox *nativeWidget)
{
    if (widget()) {
        widget()->deleteLater();
    }

    connect(nativeWidget, SIGNAL(activated(QString)), this, SIGNAL(activated(QString)));
    connect(nativeWidget, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(currentIndexChanged(int)));
    connect(nativeWidget, SIGNAL(currentIndexChanged(QString)),
            this, SIGNAL(textChanged(QString)));

    d->setWidget(nativeWidget);
    nativeWidget->setWindowIcon(QIcon());

    nativeWidget->setAttribute(Qt::WA_NoSystemBackground);
    nativeWidget->setStyle(d->style.data());
}

KComboBox *ComboBox::nativeWidget() const
{
    return static_cast<KComboBox*>(widget());
}

void ComboBox::addItem(const QString &text)
{
    static_cast<KComboBox*>(widget())->addItem(text);
}

void ComboBox::clear()
{
    static_cast<KComboBox*>(widget())->clear();
}

void ComboBox::focusOutEvent(QFocusEvent *event)
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

    if (nativeWidget()->isEditable()) {
        QEvent closeEvent(QEvent::CloseSoftwareInputPanel);
        if (qApp) {
            if (QGraphicsView *view = qobject_cast<QGraphicsView*>(qApp->focusWidget())) {
                if (view->scene() && view->scene() == scene()) {
                    QApplication::sendEvent(view, &closeEvent);
                }
            }
        }
    }

    QGraphicsProxyWidget::focusOutEvent(event);
}

void ComboBox::changeEvent(QEvent *event)
{
    d->changeEvent(event);
    QGraphicsProxyWidget::changeEvent(event);
}

void ComboBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

int ComboBox::count() const
{
    return nativeWidget()->count();
}

int ComboBox::currentIndex() const
{
    return nativeWidget()->currentIndex();
}

void ComboBox::setCurrentIndex(int index)
{
    nativeWidget()->setCurrentIndex(index);
}

} // namespace Plasma

#include "moc_combobox.cpp"

