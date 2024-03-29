/* This file is part of the KDE libraries
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2007 Michaël Larouche <larouche@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB. If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ktitlewidget.h"
#include "kicon.h"
#include "kiconloader.h"
#include "kpixmapwidget.h"

#include <QtCore/QTimer>
#include <QtGui/qevent.h>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QTextDocument>

class KTitleWidget::Private
{
public:
    Private(KTitleWidget* parent)
        : q(parent),
          autoHideTimeout(0),
          messageType(InfoMessage)
    {
    }

    void updateTextWidget() const
    {
        QFont f = textLabel->font();
        f.setBold(true);
        textLabel->setFont(f);
    }

    void updateCommentWidget() const
    {
        // FIXME: need the usability color styles to implement different
        // yet palette appropriate colours for the different use cases!
        // also .. should an icon be included here, perhaps using the pixmapWidget?
        switch (messageType) {
            case InfoMessage:
            case WarningMessage:
            case ErrorMessage: {
                QPalette p = commentLabel->palette();
                const QColor c = q->palette().color(QPalette::HighlightedText);
                p.setColor(QPalette::Foreground, c);
                p.setColor(QPalette::Background, c);
                commentLabel->setPalette(p);
                break;
            }
            case PlainMessage:
            default: {
                QPalette p = commentLabel->palette();
                QColor c = q->palette().color(QPalette::Foreground);
                p.setColor(QPalette::Foreground, c);
                c = q->palette().color(QPalette::Background);
                p.setColor(QPalette::Background, c);
                commentLabel->setPalette(p);
                break;
            }
        }
    }

    KTitleWidget* q;
    QGridLayout *headerLayout;
    KPixmapWidget *pixmapWidget;
    QLabel *textLabel;
    QLabel *commentLabel;
    int autoHideTimeout;
    MessageType messageType;

    /**
     * @brief Get the icon name from the icon type
     * @param type icon type from the enum
     * @return named icon as QString
     */
    QString iconTypeToIconName(KTitleWidget::MessageType type);

    void _k_timeoutFinished()
    {
        q->setVisible(false);
    }
};

QString KTitleWidget::Private::iconTypeToIconName(KTitleWidget::MessageType type)
{
    switch (type) {
        case KTitleWidget::InfoMessage:
            return QLatin1String("dialog-information");
        case KTitleWidget::ErrorMessage:
            return QLatin1String("dialog-error");
        case KTitleWidget::WarningMessage:
            return QLatin1String("dialog-warning");
        case KTitleWidget::PlainMessage:
            break;
    }
    return QString();
}

KTitleWidget::KTitleWidget(QWidget *parent)
  : QWidget(parent),
    d(new Private(this))
{
    QFrame *titleFrame = new QFrame(this);
    titleFrame->setAutoFillBackground(true);
    titleFrame->setFrameShape(QFrame::StyledPanel);
    titleFrame->setFrameShadow(QFrame::Plain);
    titleFrame->setBackgroundRole(QPalette::Base);

    // default image / text part start
    d->headerLayout = new QGridLayout(titleFrame);
    d->headerLayout->setColumnStretch(0, 1);
    d->headerLayout->setMargin(6);

    d->textLabel = new QLabel(titleFrame);
    d->textLabel->setVisible(false);
    d->textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);

    d->pixmapWidget = new KPixmapWidget(titleFrame);
    d->pixmapWidget->setVisible(false);

    d->headerLayout->addWidget(d->textLabel, 0, 0);
    d->headerLayout->addWidget(d->pixmapWidget, 0, 1, 1, 2);

    d->commentLabel = new QLabel(titleFrame);
    d->commentLabel->setVisible(false);
    d->commentLabel->setOpenExternalLinks(true);
    d->commentLabel->setWordWrap(true);
    d->commentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    d->headerLayout->addWidget(d->commentLabel, 1, 0);

    // default image / text part end

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleFrame);
    mainLayout->setMargin(0);
    setLayout(mainLayout);
}

KTitleWidget::~KTitleWidget()
{
    delete d;
}

bool KTitleWidget::eventFilter(QObject *object, QEvent *event)
{
    // Hide message label on click
    if (d->autoHideTimeout > 0 &&
        event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent && mouseEvent->button() == Qt::LeftButton) {
            setVisible(false);
            return true;
        }
    }

    return QWidget::eventFilter(object, event);
}

void KTitleWidget::setWidget(QWidget *widget)
{
    d->headerLayout->addWidget(widget, 2, 0, 1, 2);
}

QString KTitleWidget::text() const
{
    return d->textLabel->text();
}

QString KTitleWidget::comment() const
{
    return d->commentLabel->text();
}

QPixmap KTitleWidget::pixmap() const
{
    return d->pixmapWidget->pixmap();
}

void KTitleWidget::setBuddy(QWidget *buddy)
{
    d->textLabel->setBuddy(buddy);
}

void KTitleWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    if (e->type() == QEvent::PaletteChange || e->type() == QEvent::FontChange) {
        d->updateTextWidget();
        d->updateCommentWidget();
    }
}

void KTitleWidget::setText(const QString &text, Qt::Alignment alignment)
{
    d->textLabel->setVisible(!text.isNull());

    if (!Qt::mightBeRichText(text)) {
        d->updateTextWidget();
    }

    d->textLabel->setText(text);
    d->textLabel->setAlignment(alignment);
    show();
}

void KTitleWidget::setText(const QString &text, MessageType type)
{
    setPixmap(type);
    setText(text);
}

void KTitleWidget::setComment(const QString &comment, MessageType type)
{
    d->commentLabel->setVisible(!comment.isNull());

    //TODO: should we override the current icon with the corresponding MessageType icon?
    d->messageType = type;
    d->updateCommentWidget();
    d->commentLabel->setText(comment);
    show();
}

void KTitleWidget::setPixmap(const QPixmap &pixmap, ImageAlignment alignment)
{
    d->pixmapWidget->setVisible(!pixmap.isNull());

    d->headerLayout->removeWidget(d->textLabel);
    d->headerLayout->removeWidget(d->commentLabel);
    d->headerLayout->removeWidget(d->pixmapWidget);

    if (alignment == ImageLeft) {
        // swap the text and image labels around
        d->headerLayout->addWidget(d->pixmapWidget, 0, 0, 2, 1);
        d->headerLayout->addWidget(d->textLabel, 0, 1);
        d->headerLayout->addWidget(d->commentLabel, 1, 1);
        d->headerLayout->setColumnStretch(0, 0);
        d->headerLayout->setColumnStretch(1, 1);
    } else {
        d->headerLayout->addWidget(d->textLabel, 0, 0);
        d->headerLayout->addWidget(d->commentLabel, 1, 0);
        d->headerLayout->addWidget(d->pixmapWidget, 0, 1, 2, 1);
        d->headerLayout->setColumnStretch(1, 0);
        d->headerLayout->setColumnStretch(0, 1);
    }

    d->pixmapWidget->setPixmap(pixmap);
}


void KTitleWidget::setPixmap(const QString &icon, ImageAlignment alignment)
{
    setPixmap(KIcon(icon), alignment);
}

void KTitleWidget::setPixmap(const QIcon& icon, ImageAlignment alignment)
{
    setPixmap(icon.pixmap(IconSize(KIconLoader::Dialog)), alignment);
}

void KTitleWidget::setPixmap(MessageType type, ImageAlignment alignment)
{
    setPixmap(KIcon(d->iconTypeToIconName(type)), alignment);
}

int KTitleWidget::autoHideTimeout() const
{
    return d->autoHideTimeout;
}

void KTitleWidget::setAutoHideTimeout(int msecs)
{
    d->autoHideTimeout = msecs;

    if (msecs > 0) {
        installEventFilter(this);
    } else {
        removeEventFilter(this);
    }
}

void KTitleWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    if (d->autoHideTimeout > 0) {
        QTimer::singleShot(d->autoHideTimeout, this, SLOT(_k_timeoutFinished()));
    }
}

#include "moc_ktitlewidget.cpp"
