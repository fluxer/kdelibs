/* This file is part of the KDE libraries
 *
 * Copyright (c) 2011 Aurélien Gâteau <agateau@kde.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "kmessagewidget.h"
#include "kaction.h"
#include "kcolorscheme.h"
#include "kicon.h"
#include "kiconloader.h"
#include "kstandardaction.h"
#include "kpixmapwidget.h"
#include "kdebug.h"

#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QStyle>

//---------------------------------------------------------------------
// KMessageWidgetPrivate
//---------------------------------------------------------------------
class KMessageWidgetPrivate
{
public:
    void init(KMessageWidget *q_ptr);

    KMessageWidget* q;
    KPixmapWidget* iconWidget;
    QLabel* textLabel;
    QToolButton* closeButton;
    QIcon icon;
    KMessageWidget::MessageType messageType;
    QList<QToolButton*> buttons;

    void updateLayout();
};

void KMessageWidgetPrivate::init(KMessageWidget *q_ptr)
{
    q = q_ptr;

    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    iconWidget = new KPixmapWidget(q);
    iconWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    iconWidget->hide();

    textLabel = new QLabel(q);
    textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    QObject::connect(textLabel, SIGNAL(linkActivated(QString)), q, SIGNAL(linkActivated(QString)));
    QObject::connect(textLabel, SIGNAL(linkHovered(QString)), q, SIGNAL(linkHovered(QString)));

    KAction* closeAction = KStandardAction::close(q, SLOT(animatedHide()), q);

    // The default shortcut assigned by KStandardAction is Ctrl+W,
    // which might conflict with application-specific shortcuts.
    closeAction->setShortcut(QKeySequence());

    closeButton = new QToolButton(q);
    closeButton->setAutoRaise(true);
    closeButton->setDefaultAction(closeAction);

    q->setMessageType(KMessageWidget::Information);
}

void KMessageWidgetPrivate::updateLayout()
{
    if (q->layout()) {
        delete q->layout();
    }
    qDeleteAll(buttons);
    buttons.clear();

    Q_FOREACH(QAction* action, q->actions()) {
        QToolButton* button = new QToolButton(q);
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        buttons.append(button);
    }

    // AutoRaise reduces visual clutter, but we don't want to turn it on if
    // there are other buttons, otherwise the close button will look different
    // from the others.
    closeButton->setAutoRaise(buttons.isEmpty());

    if (textLabel->wordWrap()) {
        QGridLayout* layout = new QGridLayout(q);
        // Set alignment to make sure icon does not move down if text wraps
        layout->addWidget(iconWidget, 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
        layout->addWidget(textLabel, 0, 1);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        Q_FOREACH(QToolButton* button, buttons) {
            // For some reason, calling show() is necessary if wordwrap is true,
            // otherwise the buttons do not show up. It is not needed if
            // wordwrap is false.
            button->show();
            buttonLayout->addWidget(button);
        }
        buttonLayout->addWidget(closeButton);
        buttonLayout->addStretch();
        layout->addItem(buttonLayout, 1, 0, 1, 2);
    } else {
        QHBoxLayout* layout = new QHBoxLayout(q);
        layout->addWidget(iconWidget);
        layout->addWidget(textLabel);

        Q_FOREACH(QToolButton* button, buttons) {
            layout->addWidget(button);
        }

        layout->addWidget(closeButton);
    };

    q->updateGeometry();
}


//---------------------------------------------------------------------
// KMessageWidget
//---------------------------------------------------------------------
KMessageWidget::KMessageWidget(QWidget *parent)
    : QFrame(parent),
    d(new KMessageWidgetPrivate())
{
    d->init(this);
}

KMessageWidget::KMessageWidget(const QString &text, QWidget *parent)
    : QFrame(parent),
    d(new KMessageWidgetPrivate())
{
    d->init(this);
    setText(text);
}

KMessageWidget::~KMessageWidget()
{
    delete d;
}

QString KMessageWidget::text() const
{
    return d->textLabel->text();
}

void KMessageWidget::setText(const QString& text)
{
    d->textLabel->setText(text);
    updateGeometry();
}

KMessageWidget::MessageType KMessageWidget::messageType() const
{
    return d->messageType;
}

static void getColorsFromColorScheme(KColorScheme::BackgroundRole bgRole, QColor* bg, QColor* fg)
{
    KColorScheme scheme(QPalette::Active, KColorScheme::Window);
    *bg = scheme.background(bgRole).color();
    *fg = scheme.foreground().color();
}

void KMessageWidget::setMessageType(KMessageWidget::MessageType type)
{
    d->messageType = type;
    QColor bg0, bg1, bg2, border, fg;
    switch (type) {
    case Positive:
        getColorsFromColorScheme(KColorScheme::PositiveBackground, &bg1, &fg);
        break;
    case Information:
        // There is no "information" background role in KColorScheme, use the
        // colors of highlighted items instead
        bg1 = palette().highlight().color();
        fg = palette().highlightedText().color();
        break;
    case Warning:
        getColorsFromColorScheme(KColorScheme::NeutralBackground, &bg1, &fg);
        break;
    case Error:
        getColorsFromColorScheme(KColorScheme::NegativeBackground, &bg1, &fg);
        break;
    }

    // Colors
    bg0 = bg1.lighter(110);
    bg2 = bg1.darker(110);
    border = KColorScheme::shade(bg1, KColorScheme::DarkShade);

    setStyleSheet(
        QString("QLabel {"
            "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
            "    stop: 0 %1,"
            "    stop: 0.1 %2,"
            "    stop: 1.0 %3);"
            "border-radius: 5px;"
            "border: 1px solid %4;"
            "color: %5;"
            "}"
            )
        .arg(bg0.name())
        .arg(bg1.name())
        .arg(bg2.name())
        .arg(border.name())
        .arg(fg.name())
    );
}

QSize KMessageWidget::sizeHint() const
{
    ensurePolished();
    return QFrame::sizeHint();
}

QSize KMessageWidget::minimumSizeHint() const
{
    ensurePolished();
    return QFrame::minimumSizeHint();
}

bool KMessageWidget::event(QEvent* event)
{
    if (event->type() == QEvent::Polish && !layout()) {
        d->updateLayout();
    }
    return QFrame::event(event);
}

int KMessageWidget::heightForWidth(int width) const
{
    ensurePolished();
    return QFrame::heightForWidth(width);
}

bool KMessageWidget::wordWrap() const
{
    return d->textLabel->wordWrap();
}

void KMessageWidget::setWordWrap(bool wordWrap)
{
    d->textLabel->setWordWrap(wordWrap);
    d->updateLayout();
}

bool KMessageWidget::isCloseButtonVisible() const
{
    return d->closeButton->isVisible();
}

void KMessageWidget::setCloseButtonVisible(bool show)
{
    d->closeButton->setVisible(show);
    updateGeometry();
}

void KMessageWidget::addAction(QAction* action)
{
    QFrame::addAction(action);
    d->updateLayout();
}

void KMessageWidget::removeAction(QAction* action)
{
    QFrame::removeAction(action);
    d->updateLayout();
}

void KMessageWidget::animatedShow()
{
    if (isVisible()) {
        return;
    }

    // yep, no animation. changing the geometry for 500ms looks exactly the same as showing the
    // widget without doing so
    QFrame::show();
}

void KMessageWidget::animatedHide()
{
    if (!isVisible()) {
        return;
    }

    QFrame::hide();
}

QIcon KMessageWidget::icon() const
{
    return d->icon;
}

void KMessageWidget::setIcon(const QIcon& icon)
{
    d->icon = icon;
    if (d->icon.isNull()) {
        d->iconWidget->hide();
    } else {
        const int size = KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
        d->iconWidget->setPixmap(d->icon.pixmap(size));
        d->iconWidget->show();
    }
}

#include "moc_kmessagewidget.cpp"
