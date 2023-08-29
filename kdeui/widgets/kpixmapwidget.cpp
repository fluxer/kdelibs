/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kpixmapwidget.h"
#include "kmimetype.h"
#include "kimageio.h"
#include "kglobalsettings.h"
#include "kdebug.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QUrl>

static bool kCheckMimeData(const QMimeData *mimedata)
{
    if (!mimedata) {
        return false;
    } else if (mimedata->hasImage()) {
        return true;
    } else if (mimedata->hasUrls()) {
        const QList<QUrl> mimedataurls = mimedata->urls();
        foreach (const QUrl &mimedataurl, mimedataurls) {
            const KMimeType::Ptr mimetype = KMimeType::findByPath(mimedataurl.toLocalFile());
            if (mimetype && KImageIO::isSupported(mimetype->name())) {
                // atleast one supported image
                return true;
            }
        }
    }
    return false;
}

class KPixmapWidgetPrivate
{
public:
    KPixmapWidgetPrivate();

    QPixmap pixmap;
    Qt::Alignment alignment;
    bool dragenabled;
    QPoint dragstartpos;
};

KPixmapWidgetPrivate::KPixmapWidgetPrivate()
    : alignment(Qt::AlignHCenter | Qt::AlignVCenter),
    dragenabled(false)
{
}

KPixmapWidget::KPixmapWidget(QWidget *parent)
    : QWidget(parent),
    d(new KPixmapWidgetPrivate())
{
}

KPixmapWidget::~KPixmapWidget()
{
    delete d;
}

void KPixmapWidget::setPixmap(const QPixmap &pixmap)
{
    d->pixmap = pixmap;
    update();
}

QPixmap KPixmapWidget::pixmap() const
{
    return d->pixmap;
}

void KPixmapWidget::setAlignment(Qt::Alignment alignment)
{
    d->alignment = alignment;
    update();
}

Qt::Alignment KPixmapWidget::alignment() const
{
    return d->alignment;
}

bool KPixmapWidget::dragEnabled() const
{
    return d->dragenabled;
}

void KPixmapWidget::setDragEnabled(const bool enable)
{
    d->dragenabled = enable;
}

QSize KPixmapWidget::sizeHint() const
{
    return minimumSizeHint();
}

QSize KPixmapWidget::minimumSizeHint() const
{
    const QSize pixmapsize = d->pixmap.size();
    return pixmapsize.expandedTo(QWidget::minimumSize());
}

void KPixmapWidget::paintEvent(QPaintEvent *event)
{
    if (Q_LIKELY(!d->pixmap.isNull())) {
        QPainter painter(this);
        QStyle *style = QWidget::style();
        const int alignment = QStyle::visualAlignment(layoutDirection(), d->alignment);
        if (!isEnabled()) {
            QStyleOption styleoptions;
            styleoptions.initFrom(this);
            style->drawItemPixmap(
                &painter, contentsRect(), alignment,
                style->generatedIconPixmap(QIcon::Disabled, d->pixmap, &styleoptions)
            );
        } else {
            style->drawItemPixmap(
                &painter, contentsRect(), alignment, d->pixmap
            );
        }
    }
    QWidget::paintEvent(event);
}

void KPixmapWidget::mousePressEvent(QMouseEvent *event)
{
    if (d->dragenabled) {
        d->dragstartpos = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void KPixmapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (d->dragenabled &&
        event->buttons() & Qt::LeftButton &&
        (event->pos() - d->dragstartpos).manhattanLength() > KGlobalSettings::dndEventDelay())
    {
        QDrag* drag = new QDrag(this);
        QMimeData* mimedata = new QMimeData();
        mimedata->setImageData(d->pixmap.toImage());
        drag->setMimeData(mimedata);
        if (!d->pixmap.isNull()) {
            drag->setPixmap(d->pixmap.scaled(QSize(96, 96), Qt::KeepAspectRatio));
            // same as the one in KColorMimeData
            drag->setHotSpot(QPoint(-5,-7));
        }
        drag->start();
    }
    // don't propagate
    event->accept();
}

void KPixmapWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (kCheckMimeData(event->mimeData())) {
        event->acceptProposedAction();
    }
}

void KPixmapWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (kCheckMimeData(event->mimeData())) {
        event->acceptProposedAction();
    }
}

void KPixmapWidget::dropEvent(QDropEvent *event)
{
    const QMimeData* mimedata = event->mimeData();
    if (!mimedata) {
        return;
    } else if (mimedata->hasImage()) {
        // images are QPixmap-convertable
        setPixmap(qvariant_cast<QPixmap>(mimedata->imageData()));
    } else if (mimedata->hasUrls()) {
        const QList<QUrl> mimedataurls = mimedata->urls();
        foreach (const QUrl &mimedataurl, mimedataurls) {
            const QString mimedataurlpath = mimedataurl.toLocalFile();
            const KMimeType::Ptr mimetype = KMimeType::findByPath(mimedataurlpath);
            if (mimetype && KImageIO::isSupported(mimetype->name())) {
                const QPixmap mimedataurlpixmap = QPixmap(mimedataurlpath);
                if (!mimedataurlpixmap.isNull()) {
                    // the last pixmap wins
                    setPixmap(mimedataurlpixmap.scaled(QWidget::size(), Qt::KeepAspectRatio));
                }
            }
        }
    }
}

#include "moc_kpixmapwidget.cpp"
