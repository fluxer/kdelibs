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

#include "textbrowser.h"

#include <QtGui/qgraphicssceneevent.h>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QTextBrowser>

#include <kmimetype.h>

#include "svg.h"
#include "theme.h"
#include "private/style_p.h"
#include "private/themedwidgetinterface_p.h"

namespace Plasma
{

class TextBrowserPrivate : public ThemedWidgetInterface<TextBrowser>
{
public:
    TextBrowserPrivate(TextBrowser *browser)
        : ThemedWidgetInterface<TextBrowser>(browser),
          savedMinimumHeight(0),
          savedMaximumHeight(QWIDGETSIZE_MAX),
          wasNotFixed(true)
    {
    }

    void setFixedHeight()
    {
        QTextBrowser *native = q->nativeWidget();
        if (native->document() &&
            q->sizePolicy().verticalPolicy() == QSizePolicy::Fixed &&
            native->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
            native->document()->setTextWidth(q->size().width());
            QSize s = native->document()->size().toSize();
            if (wasNotFixed) {
                savedMinimumHeight = q->minimumHeight();
                savedMaximumHeight = q->maximumHeight();
                wasNotFixed = false;
            }
            q->setMinimumHeight(s.height());
            q->setMaximumHeight(s.height());
        } else if (!wasNotFixed) {
            q->setMinimumHeight(savedMinimumHeight);
            q->setMaximumHeight(savedMaximumHeight);
            wasNotFixed = true;
        }
    }

    QTextBrowser *native;
    Plasma::Style::Ptr style;
    int savedMinimumHeight;
    int savedMaximumHeight;
    bool wasNotFixed;
};

TextBrowser::TextBrowser(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new TextBrowserPrivate(this))
{
    QTextBrowser *native = new QTextBrowser;
    native->setWindowFlags(native->windowFlags()|Qt::BypassGraphicsProxyWidget);
    connect(native, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
    connect(native, SIGNAL(textChanged()), this, SLOT(setFixedHeight()));
    native->setWindowIcon(QIcon());
    d->setWidget(native);
    d->native = native;
    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setFrameShape(QFrame::NoFrame);
    native->setTextBackgroundColor(Qt::transparent);
    native->viewport()->setAutoFillBackground(false);
    d->style = Plasma::Style::sharedStyle();
    native->verticalScrollBar()->setStyle(d->style.data());
    native->horizontalScrollBar()->setStyle(d->style.data());
    d->initTheming();
}

TextBrowser::~TextBrowser()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void TextBrowser::setText(const QString &text)
{
    static_cast<QTextBrowser*>(widget())->setText(text);
}

QString TextBrowser::text() const
{
    return static_cast<QTextBrowser*>(widget())->toHtml();
}

void TextBrowser::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    nativeWidget()->setHorizontalScrollBarPolicy(policy);
}

void TextBrowser::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    nativeWidget()->setVerticalScrollBarPolicy(policy);
}

void TextBrowser::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString TextBrowser::styleSheet()
{
    return widget()->styleSheet();
}

QTextBrowser *TextBrowser::nativeWidget() const
{
    return static_cast<QTextBrowser*>(widget());
}

void TextBrowser::append(const QString &text)
{
    return nativeWidget()->append(text);
}

void TextBrowser::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName)

    QTextBrowser *te = nativeWidget();
    te->clear();

    foreach (const QVariant &v, data) {
        if (v.canConvert(QVariant::String)) {
            te->append(v.toString() + '\n');
        }
    }
}

void TextBrowser::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu *popup = nativeWidget()->createStandardContextMenu(event->screenPos());
    if (popup) {
        popup->exec(event->screenPos());
        delete popup;
    }
}

void TextBrowser::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->setFixedHeight();
    QGraphicsProxyWidget::resizeEvent(event);
}

void TextBrowser::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (nativeWidget()->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff &&
        nativeWidget()->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
        event->ignore();
    } else {
        QGraphicsProxyWidget::wheelEvent(event);
    }
}

void TextBrowser::changeEvent(QEvent *event)
{
    d->changeEvent(event);
    QGraphicsProxyWidget::changeEvent(event);
}

} // namespace Plasma

#include "moc_textbrowser.cpp"

