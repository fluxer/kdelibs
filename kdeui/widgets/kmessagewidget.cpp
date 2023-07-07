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

#include <kaction.h>
#include <kcolorscheme.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kstandardaction.h>

#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QToolButton>
#include <QStyle>

// NOTE: KateAnimation relies on this being 500ms, see:
// kde-workspace/kate/part/view/kateanimation.cpp
static const int s_kmessageanimationduration = 500;

//---------------------------------------------------------------------
// KMessageWidgetPrivate
//---------------------------------------------------------------------
class KMessageWidgetPrivate
{
public:
    void init(KMessageWidget *q_ptr);

    KMessageWidget* q;
    QLabel* iconLabel;
    QLabel* textLabel;
    QToolButton* closeButton;
    QIcon icon;

    KMessageWidget::MessageType messageType;
    QList<QToolButton*> buttons;
    QPropertyAnimation* animation;

    void updateLayout();
};

void KMessageWidgetPrivate::init(KMessageWidget *q_ptr)
{
    q = q_ptr;

    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    iconLabel = new QLabel(q);
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    iconLabel->hide();

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

    animation = nullptr;

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
        layout->addWidget(iconLabel, 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
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
        layout->addWidget(iconLabel);
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
            "margin: %5px;"
            "color: %6;"
            "}"
            )
        .arg(bg0.name())
        .arg(bg1.name())
        .arg(bg2.name())
        .arg(border.name())
        // DefaultFrameWidth returns the size of the external margin + border width. We know our border is 1px, so we subtract this from the frame normal QStyle FrameWidth to get our margin
        .arg(style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, this) -1)
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
    if (d->animation) {
        delete d->animation;
        d->animation = nullptr;
    }

    if (!(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)) {
        show();
        return;
    }

    if (isVisible()) {
        return;
    }

    d->animation = new QPropertyAnimation(this, "windowOpacity", this);
    d->animation->setStartValue(0.0);
    d->animation->setEndValue(1.0);
    d->animation->setEasingCurve(QEasingCurve::InOutQuad);
    d->animation->setDuration(s_kmessageanimationduration);
    d->animation->start();
    QFrame::show();
}

void KMessageWidget::animatedHide()
{
    if (d->animation) {
        delete d->animation;
        d->animation = nullptr;
    }

    if (!(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)) {
        hide();
        return;
    }

    if (!isVisible()) {
        return;
    }

    d->animation = new QPropertyAnimation(this, "windowOpacity", this);
    d->animation->setStartValue(1.0);
    d->animation->setEndValue(0.0);
    d->animation->setEasingCurve(QEasingCurve::InOutQuad);
    d->animation->setDuration(s_kmessageanimationduration);
    d->animation->start();
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
        d->iconLabel->hide();
    } else {
        const int size = KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
        d->iconLabel->setPixmap(d->icon.pixmap(size));
        d->iconLabel->show();
    }
}

#include "moc_kmessagewidget.cpp"
