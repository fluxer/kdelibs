/*
   This file is part of the KDE project
   Copyright (C) 2009 Shaun Reich <shaun.reich@kdemail.net>
   Copyright © 2006-2007, 2008 Fredrik Höglund <fredrik@kde.org>


   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kfileitemdelegate.h"

#include <QApplication>
#include <QStyle>
#include <QtCore/qabstractitemmodel.h>
#include <QPainter>
#include <QCache>
#include <QImage>
#include <QPainterPath>
#include <QTextLayout>
#include <QListView>
#include <QPaintEngine>
#include <qmath.h>

#include <kglobal.h>
#include <klocale.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kdirmodel.h>
#include <kfileitem.h>
#include <kcolorscheme.h>
#include <kglobalsettings.h>
#include <ktextedit.h>
#include <kstringhandler.h>

struct Margin
{
    int left, right, top, bottom;
};


class KFileItemDelegate::Private
{
    public:
        enum MarginType { ItemMargin = 0, TextMargin, IconMargin, NMargins };

        Private();
        ~Private() {}

        QSize decorationSizeHint(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const;
        QSize displaySizeHint(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const;
        QString replaceNewlines(const QString &string) const;
        inline KFileItem fileItem(const QModelIndex &index) const;
        QString elidedText(QTextLayout &layout, const QStyleOptionViewItemV4 &option, const QSize &maxSize) const;
        QSize layoutText(QTextLayout &layout, const QStyleOptionViewItemV4 &option,
                         const QString &text, const QSize &constraints) const;
        QSize layoutText(QTextLayout &layout, const QString &text, int maxWidth) const;
        inline void setLayoutOptions(QTextLayout &layout, const QStyleOptionViewItemV4 &options) const;
        inline bool verticalLayout(const QStyleOptionViewItemV4 &option) const;
        inline QBrush brush(const QVariant &value, const QStyleOptionViewItemV4 &option) const;
        QBrush foregroundBrush(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const;
        inline void setActiveMargins(Qt::Orientation layout);
        void setVerticalMargin(MarginType type, int left, int right, int top, int bottom);
        void setHorizontalMargin(MarginType type, int left, int right, int top, int bottom);
        inline void setVerticalMargin(MarginType type, int hor, int ver);
        inline void setHorizontalMargin(MarginType type, int hor, int ver);
        inline QRect addMargin(const QRect &rect, MarginType type) const;
        inline QRect subtractMargin(const QRect &rect, MarginType type) const;
        inline QSize addMargin(const QSize &size, MarginType type) const;
        inline QSize subtractMargin(const QSize &size, MarginType type) const;
        QString itemSize(const QModelIndex &index, const KFileItem &item) const;
        QString information(const QStyleOptionViewItemV4 &option, const QModelIndex &index, const KFileItem &item) const;
        bool isListView(const QStyleOptionViewItemV4 &option) const;
        QString display(const QModelIndex &index) const;
        QIcon decoration(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const;
        QPoint iconPosition(const QStyleOptionViewItemV4 &option) const;
        QRect labelRectangle(const QStyleOptionViewItemV4 &option) const;
        void layoutTextItems(const QStyleOptionViewItemV4 &option, const QModelIndex &index,
                             QTextLayout *labelLayout, QTextLayout *infoLayout, QRect *textBoundingRect) const;
        void drawTextItems(QPainter *painter, const QTextLayout &labelLayout, const QTextLayout &infoLayout,
                           const QRect &textBoundingRect) const;
        QPixmap applyHoverEffect(const QPixmap &icon) const;
        void initStyleOption(QStyleOptionViewItemV4 *option, const QModelIndex &index) const;
        void drawFocusRect(QPainter *painter, const QStyleOptionViewItemV4 &option, const QRect &rect) const;

    public:
        KFileItemDelegate::InformationList informationList;
        QColor shadowColor;
        QPointF shadowOffset;
        qreal shadowBlur;
        QSize maximumSize;
        bool showToolTipWhenElided;
        QTextOption::WrapMode wrapMode;

    private:
        Margin verticalMargin[NMargins];
        Margin horizontalMargin[NMargins];
        Margin *activeMargins;
};


KFileItemDelegate::Private::Private()
     : shadowColor(Qt::transparent), shadowOffset(1, 1), shadowBlur(2), maximumSize(0, 0),
       showToolTipWhenElided(true), wrapMode( QTextOption::WrapAtWordBoundaryOrAnywhere ),
       activeMargins(0)
{
}


void KFileItemDelegate::Private::setActiveMargins(Qt::Orientation layout)
{
    activeMargins = (layout == Qt::Horizontal ?
            horizontalMargin : verticalMargin);
}


void KFileItemDelegate::Private::setVerticalMargin(MarginType type, int left, int top, int right, int bottom)
{
    verticalMargin[type].left   = left;
    verticalMargin[type].right  = right;
    verticalMargin[type].top    = top;
    verticalMargin[type].bottom = bottom;
}


void KFileItemDelegate::Private::setHorizontalMargin(MarginType type, int left, int top, int right, int bottom)
{
    horizontalMargin[type].left   = left;
    horizontalMargin[type].right  = right;
    horizontalMargin[type].top    = top;
    horizontalMargin[type].bottom = bottom;
}


void KFileItemDelegate::Private::setVerticalMargin(MarginType type, int horizontal, int vertical)
{
    setVerticalMargin(type, horizontal, vertical, horizontal, vertical);
}


void KFileItemDelegate::Private::setHorizontalMargin(MarginType type, int horizontal, int vertical)
{
    setHorizontalMargin(type, horizontal, vertical, horizontal, vertical);
}


QRect KFileItemDelegate::Private::addMargin(const QRect &rect, MarginType type) const
{
    Q_ASSERT(activeMargins != 0);
    const Margin &m = activeMargins[type];
    return rect.adjusted(-m.left, -m.top, m.right, m.bottom);
}


QRect KFileItemDelegate::Private::subtractMargin(const QRect &rect, MarginType type) const
{
    Q_ASSERT(activeMargins != 0);
    const Margin &m = activeMargins[type];
    return rect.adjusted(m.left, m.top, -m.right, -m.bottom);
}


QSize KFileItemDelegate::Private::addMargin(const QSize &size, MarginType type) const
{
    Q_ASSERT(activeMargins != 0);
    const Margin &m = activeMargins[type];
    return QSize(size.width() + m.left + m.right, size.height() + m.top + m.bottom);
}


QSize KFileItemDelegate::Private::subtractMargin(const QSize &size, MarginType type) const
{
    Q_ASSERT(activeMargins != 0);
    const Margin &m = activeMargins[type];
    return QSize(size.width() - m.left - m.right, size.height() - m.top - m.bottom);
}


// Returns the size of a file, or the number of items in a directory, as a QString
QString KFileItemDelegate::Private::itemSize(const QModelIndex &index, const KFileItem &item) const
{
    // Return a formatted string containing the file size, if the item is a file
    if (item.isFile())
        return KGlobal::locale()->formatByteSize(item.size());

    // Return the number of items in the directory
    const QVariant value = index.data(KDirModel::ChildCountRole);
    const int count = value.type() == QVariant::Int ? value.toInt() : KDirModel::ChildCountUnknown;

    if (count == KDirModel::ChildCountUnknown) {
        // was: i18nc("Items in a folder", "? items");
        // but this just looks useless in a remote directory listing,
        // better not show anything.
        return QString();
    }

    return i18ncp("Items in a folder", "1 item", "%1 items", count);
}


// Returns the additional information string, if one should be shown, or an empty string otherwise
QString KFileItemDelegate::Private::information(const QStyleOptionViewItemV4 &option, const QModelIndex &index,
                                                const KFileItem &item) const
{
    QString string;

    if (informationList.isEmpty() || item.isNull() || !isListView(option))
        return string;

    foreach (KFileItemDelegate::Information info, informationList)
    {
        if (info == KFileItemDelegate::NoInformation)
            continue;

        if (!string.isEmpty())
            string += QChar::LineSeparator;

        switch (info)
        {
            case KFileItemDelegate::Size:
                string += itemSize(index, item);
                break;

            case KFileItemDelegate::Permissions:
                string += item.permissionsString();
                break;

            case KFileItemDelegate::OctalPermissions:
                string += QString('0') + QString::number(item.permissions(), 8);
                break;

            case KFileItemDelegate::Owner:
                string += item.user();
                break;

            case KFileItemDelegate::OwnerAndGroup:
                string += item.user() + ':' + item.group();
                break;

            case KFileItemDelegate::CreationTime:
                string += item.timeString(KFileItem::CreationTime);
                break;

            case KFileItemDelegate::ModificationTime:
                string += item.timeString(KFileItem::ModificationTime);
                break;

            case KFileItemDelegate::AccessTime:
                string += item.timeString(KFileItem::AccessTime);
                break;

            case KFileItemDelegate::MimeType:
                string += item.isMimeTypeKnown() ? item.mimetype() : i18nc("@info mimetype","Unknown");
                break;

            case KFileItemDelegate::FriendlyMimeType:
                string += item.isMimeTypeKnown() ? item.mimeComment() : i18nc("@info mimetype","Unknown");
                break;

            case KFileItemDelegate::LinkDest:
                string += item.linkDest();
                break;

            case KFileItemDelegate::LocalPathOrUrl:
                if(!item.localPath().isEmpty())
                    string += item.localPath();
                else
                    string += item.url().pathOrUrl();
                break;

            case KFileItemDelegate::Comment:
                string += item.comment();
                break;

            default:
                break;
        } // switch (info)
    } // foreach (info, list)

    return string;
}


// Returns the KFileItem for the index
KFileItem KFileItemDelegate::Private::fileItem(const QModelIndex &index) const
{
    const QVariant value = index.data(KDirModel::FileItemRole);
    return qvariant_cast<KFileItem>(value);
}


// Replaces any newline characters in the provided string, with QChar::LineSeparator
QString KFileItemDelegate::Private::replaceNewlines(const QString &text) const
{
    QString string = text;
    const QChar newline = QLatin1Char('\n');

    for (int i = 0; i < string.length(); i++)
        if (string[i] == newline)
            string[i] = QChar::LineSeparator;

    return string;
}


// Lays the text out in a rectangle no larger than constraints, eliding it as necessary
QSize KFileItemDelegate::Private::layoutText(QTextLayout &layout, const QStyleOptionViewItemV4 &option,
                                             const QString &text, const QSize &constraints) const
{
    const QSize size = layoutText(layout, text, constraints.width());

    if (size.width() > constraints.width() || size.height() > constraints.height())
    {
        const QString elided = elidedText(layout, option, constraints);
        return layoutText(layout, elided, constraints.width());
    }

    return size;
}


// Lays the text out in a rectangle no wider than maxWidth
QSize KFileItemDelegate::Private::layoutText(QTextLayout &layout, const QString &text, int maxWidth) const
{
    QFontMetrics metrics(layout.font());
    int leading     = metrics.leading();
    int height      = 0;
    qreal widthUsed = 0;
    QTextLine line;

    layout.setText(text);

    layout.beginLayout();
    while ((line = layout.createLine()).isValid())
    {
        line.setLineWidth(maxWidth);
        height += leading;
        line.setPosition(QPoint(0, height));
        height += int(line.height());
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
    }
    layout.endLayout();

    return QSize(qCeil(widthUsed), height);
}


// Elides the text in the layout, by iterating over each line in the layout, eliding
// or word breaking the line if it's wider than the max width, and finally adding an
// ellipses at the end of the last line, if there are more lines than will fit within
// the vertical size constraints.
QString KFileItemDelegate::Private::elidedText(QTextLayout &layout, const QStyleOptionViewItemV4 &option,
                                               const QSize &size) const
{
    const QString text = layout.text();
    int maxWidth       = size.width();
    int maxHeight      = size.height();
    qreal height       = 0;
    bool wrapText      = (option.features & QStyleOptionViewItemV2::WrapText);

    // If the string contains a single line of text that shouldn't be word wrapped
    if (!wrapText && text.indexOf(QChar::LineSeparator) == -1)
        return option.fontMetrics.elidedText(text, option.textElideMode, maxWidth);

    // Elide each line that has already been laid out in the layout.
    QString elided;
    elided.reserve(text.length());

    for (int i = 0; i < layout.lineCount(); i++)
    {
        QTextLine line = layout.lineAt(i);
        int start  = line.textStart();
        int length = line.textLength();

        height += option.fontMetrics.leading();
        if (height + line.height() + option.fontMetrics.lineSpacing() > maxHeight)
        {
            // Unfortunately, if the line ends because of a line separator, elidedText() will be too
            // clever and keep adding lines until it finds one that's too wide.
            if (line.naturalTextWidth() < maxWidth && text[start + length - 1] == QChar::LineSeparator)
                elided += text.mid(start, length - 1);
            else
                elided += option.fontMetrics.elidedText(text.mid(start), option.textElideMode, maxWidth);
            break;
        }
        else if (line.naturalTextWidth() > maxWidth)
        {
            elided += option.fontMetrics.elidedText(text.mid(start, length), option.textElideMode, maxWidth);
            if (!elided.endsWith(QChar::LineSeparator))
                elided += QChar::LineSeparator;
        }
        else
            elided += text.mid(start, length);

        height += line.height();
    }

    return elided;
}


void KFileItemDelegate::Private::setLayoutOptions(QTextLayout &layout, const QStyleOptionViewItemV4 &option) const
{
    QTextOption textoption;
    textoption.setTextDirection(option.direction);
    textoption.setAlignment(QStyle::visualAlignment(option.direction, option.displayAlignment));
    textoption.setWrapMode((option.features & QStyleOptionViewItemV2::WrapText) ? wrapMode : QTextOption::NoWrap);

    layout.setFont(option.font);
    layout.setTextOption(textoption);
}


QSize KFileItemDelegate::Private::displaySizeHint(const QStyleOptionViewItemV4 &option,
                                                  const QModelIndex &index) const
{
    QString label = option.text;
    int maxWidth = 0;
    if (maximumSize.isEmpty()) {
        maxWidth = verticalLayout(option) && (option.features & QStyleOptionViewItemV2::WrapText)
                   ? option.decorationSize.width() + 10 : 32757;
    }
    else {
        const Margin &itemMargin = activeMargins[ItemMargin];
        const Margin &textMargin = activeMargins[TextMargin];
        maxWidth = maximumSize.width() -
                   (itemMargin.left + itemMargin.right) -
                   (textMargin.left + textMargin.right);
    }

    KFileItem item = fileItem(index);

    // To compute the nominal size for the label + info, we'll just append
    // the information string to the label
    const QString info = information(option, index, item);
    if (!info.isEmpty())
        label += QString(QChar::LineSeparator) + info;

    QTextLayout layout;
    setLayoutOptions(layout, option);

    QSize size = layoutText(layout, label, maxWidth);
    if (!info.isEmpty())
    {
        // As soon as additional information is shown, it might be necessary that
        // the label and/or the additional information must get elided. To prevent
        // an expensive eliding in the scope of displaySizeHint, the maximum
        // width is reserved instead.
        size.setWidth(maxWidth);
    }

    return addMargin(size, TextMargin);
}


QSize KFileItemDelegate::Private::decorationSizeHint(const QStyleOptionViewItemV4 &option,
                                                     const QModelIndex &index) const
{
    Q_UNUSED(index)

    QSize iconSize = option.icon.actualSize(option.decorationSize);
    if (!verticalLayout(option))
        iconSize.rwidth() = option.decorationSize.width();
    else if (iconSize.width() < option.decorationSize.width())
        iconSize.rwidth() = qMin(iconSize.width() + 10, option.decorationSize.width());
    if (iconSize.height() < option.decorationSize.height())
        iconSize.rheight() = option.decorationSize.height();

    return addMargin(iconSize, IconMargin);
}


bool KFileItemDelegate::Private::verticalLayout(const QStyleOptionViewItemV4 &option) const
{
    return (option.decorationPosition == QStyleOptionViewItem::Top ||
            option.decorationPosition == QStyleOptionViewItem::Bottom);
}


// Converts a QVariant of type Brush or Color to a QBrush
QBrush KFileItemDelegate::Private::brush(const QVariant &value, const QStyleOptionViewItemV4 &option) const
{
    if (value.userType() == qMetaTypeId<KStatefulBrush>())
        return qvariant_cast<KStatefulBrush>(value).brush(option.palette);
    switch (value.type())
    {
        case QVariant::Color:
            return QBrush(qvariant_cast<QColor>(value));

        case QVariant::Brush:
            return qvariant_cast<QBrush>(value);

        default:
            return QBrush(Qt::NoBrush);
    }
}


QBrush KFileItemDelegate::Private::foregroundBrush(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const
{
    QPalette::ColorGroup cg = QPalette::Active;
    if (!(option.state & QStyle::State_Enabled)) {
        cg = QPalette::Disabled;
    } else if (!(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    // Always use the highlight color for selected items
    if (option.state & QStyle::State_Selected)
        return option.palette.brush(cg, QPalette::HighlightedText);

    // If the model provides its own foreground color/brush for this item
    const QVariant value = index.data(Qt::ForegroundRole);
    if (value.isValid())
        return brush(value, option);

    return option.palette.brush(cg, QPalette::Text);
}


bool KFileItemDelegate::Private::isListView(const QStyleOptionViewItemV4 &option) const
{
    if (qobject_cast<const QListView*>(option.widget) || verticalLayout(option))
        return true;

    return false;
}


QPixmap KFileItemDelegate::Private::applyHoverEffect(const QPixmap &icon) const
{
    KIconEffect *effect = KIconLoader::global()->iconEffect();

    // Note that in KIconLoader terminology, active = hover.
    // ### We're assuming that the icon group is desktop/filemanager, since this
    //     is KFileItemDelegate.
    if (effect->hasEffect(KIconLoader::Desktop, KIconLoader::ActiveState))
        return effect->apply(icon, KIconLoader::Desktop, KIconLoader::ActiveState);

    return icon;
}

void KFileItemDelegate::Private::layoutTextItems(const QStyleOptionViewItemV4 &option, const QModelIndex &index,
                                                 QTextLayout *labelLayout, QTextLayout *infoLayout,
                                                 QRect *textBoundingRect) const
{
    KFileItem item       = fileItem(index);
    const QString info   = information(option, index, item);
    bool showInformation = false;

    setLayoutOptions(*labelLayout, option);

    const QRect textArea = labelRectangle(option);
    QRect textRect       = subtractMargin(textArea, Private::TextMargin);

    // Sizes and constraints for the different text parts
    QSize maxLabelSize = textRect.size();
    QSize maxInfoSize  = textRect.size();
    QSize labelSize;
    QSize infoSize;

    // If we have additional info text, and there's space for at least two lines of text,
    // adjust the max label size to make room for at least one line of the info text
    if (!info.isEmpty() && textRect.height() >= option.fontMetrics.lineSpacing() * 2)
    {
        infoLayout->setFont(labelLayout->font());
        infoLayout->setTextOption(labelLayout->textOption());

        maxLabelSize.rheight() -= option.fontMetrics.lineSpacing();
        showInformation = true;
    }

    // Lay out the label text, and adjust the max info size based on the label size
    labelSize = layoutText(*labelLayout, option, option.text, maxLabelSize);
    maxInfoSize.rheight() -= labelSize.height();

    // Lay out the info text
    if (showInformation)
        infoSize = layoutText(*infoLayout, option, info, maxInfoSize);
    else
        infoSize = QSize(0, 0);

    // Compute the bounding rect of the text
    const QSize size(qMax(labelSize.width(), infoSize.width()), labelSize.height() + infoSize.height());
    *textBoundingRect = QStyle::alignedRect(option.direction, option.displayAlignment, size, textRect);

    // Compute the positions where we should draw the layouts
    labelLayout->setPosition(QPointF(textRect.x(), textBoundingRect->y()));
    infoLayout->setPosition(QPointF(textRect.x(), textBoundingRect->y() + labelSize.height()));
}


void KFileItemDelegate::Private::drawTextItems(QPainter *painter, const QTextLayout &labelLayout,
                                               const QTextLayout &infoLayout, const QRect &boundingRect) const
{
    if (shadowColor.alpha() > 0)
    {
        QPixmap pixmap(boundingRect.size());
        pixmap.fill(Qt::transparent);

        QPainter p(&pixmap);
        p.translate(-boundingRect.topLeft());
        p.setPen(painter->pen());
        labelLayout.draw(&p, QPoint());

        if (!infoLayout.text().isEmpty())
        {
            QColor color = p.pen().color();
            color.setAlphaF(0.6);

            p.setPen(color);
            infoLayout.draw(&p, QPoint());
        }
        p.end();

        int padding = qCeil(shadowBlur);
        int blurFactor = qRound(shadowBlur);

        QImage image(boundingRect.size() + QSize(padding * 2, padding * 2), QImage::Format_ARGB32_Premultiplied);
        image.fill(0);
        p.begin(&image);
        p.drawImage(padding, padding, pixmap.toImage());
        p.end();

        KIconEffect::shadowBlur(image, blurFactor, shadowColor);

        painter->drawImage(boundingRect.topLeft() - QPoint(padding, padding) + shadowOffset.toPoint(), image);
        painter->drawPixmap(boundingRect.topLeft(), pixmap);
        return;
    }

    labelLayout.draw(painter, QPoint());

    if (!infoLayout.text().isEmpty())
    {
        // TODO - for apps not doing funny things with the color palette,
        // KColorScheme::InactiveText would be a much more correct choice. We
        // should provide an API to specify what color to use for information.
        QColor color = painter->pen().color();
        color.setAlphaF(0.6);

        painter->setPen(color);
        infoLayout.draw(painter, QPoint());
    }
}


void KFileItemDelegate::Private::initStyleOption(QStyleOptionViewItemV4 *option,
                                                 const QModelIndex &index) const
{
    const KFileItem item = fileItem(index);
    bool updateFontMetrics = false;

    // Try to get the font from the model
    QVariant value = index.data(Qt::FontRole);
    if (value.isValid()) {
        option->font = qvariant_cast<QFont>(value).resolve(option->font);
        updateFontMetrics = true;
    }

    // Use an italic font for symlinks
    if (!item.isNull() && item.isLink()) {
        option->font.setItalic(true);
        updateFontMetrics = true;
    }

    if (updateFontMetrics)
        option->fontMetrics = QFontMetrics(option->font);

    // Try to get the alignment for the item from the model
    value = index.data(Qt::TextAlignmentRole);
    if (value.isValid())
        option->displayAlignment = Qt::Alignment(value.toInt());

    value = index.data(Qt::BackgroundRole);
    if (value.isValid())
        option->backgroundBrush = brush(value, *option);

    option->text = display(index);
    if (!option->text.isEmpty())
        option->features |= QStyleOptionViewItemV2::HasDisplay;

    option->icon = decoration(*option, index);
    if (!option->icon.isNull())
        option->features |= QStyleOptionViewItemV2::HasDecoration;

    // ### Make sure this value is always true for now
    option->showDecorationSelected = true;
}

// ---------------------------------------------------------------------------


KFileItemDelegate::KFileItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent), d(new Private())
{
    int focusHMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    int focusVMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin);

    // Margins for horizontal mode (list views, tree views, table views)
    const int textMargin = focusHMargin * 4;
    if (QApplication::isRightToLeft())
        d->setHorizontalMargin(Private::TextMargin, textMargin, focusVMargin, focusHMargin, focusVMargin);
    else
        d->setHorizontalMargin(Private::TextMargin, focusHMargin, focusVMargin, textMargin, focusVMargin);

    d->setHorizontalMargin(Private::IconMargin, focusHMargin, focusVMargin);
    d->setHorizontalMargin(Private::ItemMargin, 0, 0);

    // Margins for vertical mode (icon views)
    d->setVerticalMargin(Private::TextMargin, 6, 2);
    d->setVerticalMargin(Private::IconMargin, focusHMargin, focusVMargin);
    d->setVerticalMargin(Private::ItemMargin, 0, 0);

    setShowInformation(NoInformation);
}


KFileItemDelegate::~KFileItemDelegate()
{
    delete d;
}


QSize KFileItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // If the model wants to provide its own size hint for the item
    const QVariant value = index.data(Qt::SizeHintRole);
    if (value.isValid())
        return qvariant_cast<QSize>(value);

    QStyleOptionViewItemV4 opt(option);
    d->initStyleOption(&opt, index);
    d->setActiveMargins(d->verticalLayout(opt) ? Qt::Vertical : Qt::Horizontal);

    const QSize displaySize    = d->displaySizeHint(opt, index);
    const QSize decorationSize = d->decorationSizeHint(opt, index);

    QSize size;

    if (d->verticalLayout(opt))
    {
        size.rwidth()  = qMax(displaySize.width(), decorationSize.width());
        size.rheight() = decorationSize.height() + displaySize.height() + 1;
    }
    else
    {
        size.rwidth()  = decorationSize.width() + displaySize.width() + 1;
        size.rheight() = qMax(decorationSize.height(), displaySize.height());
    }

    size = d->addMargin(size, Private::ItemMargin);
    if (!d->maximumSize.isEmpty())
    {
        size = size.boundedTo(d->maximumSize);
    }

    return size;
}


QString KFileItemDelegate::Private::display(const QModelIndex &index) const
{
    const QVariant value = index.data(Qt::DisplayRole);

    switch (value.type())
    {
        case QVariant::String:
        {
            if (index.column() == KDirModel::Size)
                return itemSize(index, fileItem(index));
            else {
                const QString text = replaceNewlines(value.toString());
                return KStringHandler::preProcessWrap(text);
            }
        }

        case QVariant::Double:
            return KGlobal::locale()->formatNumber(value.toDouble());

        case QVariant::Int:
        case QVariant::UInt:
            return KGlobal::locale()->formatLong(value.toInt());

        default:
            return QString();
    }
}


void KFileItemDelegate::setShowInformation(const InformationList &list)
{
    d->informationList = list;
}


void KFileItemDelegate::setShowInformation(Information value)
{
    if (value != NoInformation)
        d->informationList = InformationList() << value;
    else
        d->informationList = InformationList();
}


KFileItemDelegate::InformationList KFileItemDelegate::showInformation() const
{
    return d->informationList;
}


void KFileItemDelegate::setShadowColor(const QColor &color)
{
    d->shadowColor = color;
}


QColor KFileItemDelegate::shadowColor() const
{
    return d->shadowColor;
}


void KFileItemDelegate::setShadowOffset(const QPointF &offset)
{
    d->shadowOffset = offset;
}


QPointF KFileItemDelegate::shadowOffset() const
{
    return d->shadowOffset;
}


void KFileItemDelegate::setShadowBlur(qreal factor)
{
    d->shadowBlur = factor;
}


qreal KFileItemDelegate::shadowBlur() const
{
    return d->shadowBlur;
}


void KFileItemDelegate::setMaximumSize(const QSize &size)
{
    d->maximumSize = size;
}


QSize KFileItemDelegate::maximumSize() const
{
    return d->maximumSize;
}


void KFileItemDelegate::setShowToolTipWhenElided(bool showToolTip)
{
    d->showToolTipWhenElided = showToolTip;
}


bool KFileItemDelegate::showToolTipWhenElided() const
{
    return d->showToolTipWhenElided;
}


void KFileItemDelegate::setWrapMode(QTextOption::WrapMode wrapMode)
{
    d->wrapMode = wrapMode;
}


QTextOption::WrapMode KFileItemDelegate::wrapMode() const
{
    return d->wrapMode;
}

QRect KFileItemDelegate::iconRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt(option);
    d->initStyleOption(&opt, index);
    return QRect(d->iconPosition(opt), opt.icon.actualSize(opt.decorationSize));
}


QIcon KFileItemDelegate::Private::decoration(const QStyleOptionViewItemV4 &option, const QModelIndex &index) const
{
    const QVariant value = index.data(Qt::DecorationRole);
    QIcon icon;

    switch (value.type())
    {
    case QVariant::Icon:
        icon = qvariant_cast<QIcon>(value);
        break;

    case QVariant::Pixmap:
        icon.addPixmap(qvariant_cast<QPixmap>(value));
        break;

    case QVariant::Color: {
        QPixmap pixmap(option.decorationSize);
        pixmap.fill(qvariant_cast<QColor>(value));
        icon.addPixmap(pixmap);
        break;
    }

    default:
        break;
    }

    return icon;
}


QRect KFileItemDelegate::Private::labelRectangle(const QStyleOptionViewItemV4 &option) const
{
    if (option.icon.isNull())
        return subtractMargin(option.rect, Private::ItemMargin);

    const QSize decoSize = addMargin(option.decorationSize, Private::IconMargin);
    const QRect itemRect = subtractMargin(option.rect, Private::ItemMargin);
    QRect textArea(QPoint(0, 0), itemRect.size());

    switch (option.decorationPosition)
    {
        case QStyleOptionViewItem::Top:
            textArea.setTop(decoSize.height() + 1);
            break;

        case QStyleOptionViewItem::Bottom:
            textArea.setBottom(itemRect.height() - decoSize.height() - 1);
            break;

        case QStyleOptionViewItem::Left:
            textArea.setLeft(decoSize.width() + 1);
            break;

        case QStyleOptionViewItem::Right:
            textArea.setRight(itemRect.width() - decoSize.width() - 1);
            break;
    }

    textArea.translate(itemRect.topLeft());
    return QStyle::visualRect(option.direction, option.rect, textArea);
}


QPoint KFileItemDelegate::Private::iconPosition(const QStyleOptionViewItemV4 &option) const
{
    const QRect itemRect = subtractMargin(option.rect, Private::ItemMargin);
    Qt::Alignment alignment;

    // Convert decorationPosition to the alignment the decoration will have in option.rect
    switch (option.decorationPosition)
    {
        case QStyleOptionViewItem::Top:
            alignment = Qt::AlignHCenter | Qt::AlignTop;
            break;

        case QStyleOptionViewItem::Bottom:
            alignment = Qt::AlignHCenter | Qt::AlignBottom;
            break;

        case QStyleOptionViewItem::Left:
            alignment = Qt::AlignVCenter | Qt::AlignLeft;
            break;

        case QStyleOptionViewItem::Right:
            alignment = Qt::AlignVCenter | Qt::AlignRight;
            break;
    }

    // Compute the nominal decoration rectangle
    const QSize size = addMargin(option.decorationSize, Private::IconMargin);
    const QRect rect = QStyle::alignedRect(option.direction, alignment, size, itemRect);

    // Position the icon in the center of the rectangle
    QRect iconRect = QRect(QPoint(), option.icon.actualSize(option.decorationSize));
    iconRect.moveCenter(rect.center());

    return iconRect.topLeft();
}


void KFileItemDelegate::Private::drawFocusRect(QPainter *painter, const QStyleOptionViewItemV4 &option,
                                               const QRect &rect) const
{
    if (!(option.state & QStyle::State_HasFocus))
        return;

    QStyleOptionFocusRect opt;
    opt.direction       = option.direction;
    opt.fontMetrics     = option.fontMetrics;
    opt.palette         = option.palette;
    opt.rect            = rect;
    opt.state           = option.state | QStyle::State_KeyboardFocusChange | QStyle::State_Item;
    opt.backgroundColor = option.palette.color(option.state & QStyle::State_Selected ?
                                               QPalette::Highlight : QPalette::Base);

    // Apparently some widget styles expect this hint to not be set
    painter->setRenderHint(QPainter::Antialiasing, false);

    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, painter, option.widget);

    painter->setRenderHint(QPainter::Antialiasing);
}


void KFileItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    QStyleOptionViewItemV4 opt(option);
    d->initStyleOption(&opt, index);
    d->setActiveMargins(d->verticalLayout(opt) ? Qt::Vertical : Qt::Horizontal);

    if (!(option.state & QStyle::State_Enabled))
    {
        opt.palette.setCurrentColorGroup(QPalette::Disabled);
    }

    // Unset the mouse over bit if we're not drawing the first column
    if (index.column() > 0)
        opt.state &= ~QStyle::State_MouseOver;
    else
        opt.viewItemPosition = QStyleOptionViewItemV4::OnlyOne;

    qreal progress = ((opt.state & QStyle::State_MouseOver) &&
    index.column() == KDirModel::Name) ? 1.0 : 0.0;
    const QPoint iconPos   = d->iconPosition(opt);
    QIcon::Mode iconMode   = option.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
    QIcon::State iconState = option.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
    QPixmap icon           = opt.icon.pixmap(opt.decorationSize, iconMode, iconState);

    // Compute the metrics, and lay out the text items
    // ========================================================================
    const QPen pen         = QPen(d->foregroundBrush(opt, index), 0);

    //### Apply the selection effect to the icon when the item is selected and
    //     showDecorationSelected is false.

    QTextLayout labelLayout, infoLayout;
    QRect textBoundingRect;

    d->layoutTextItems(opt, index, &labelLayout, &infoLayout, &textBoundingRect);

    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

    int focusHMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
    int focusVMargin = style->pixelMetric(QStyle::PM_FocusFrameVMargin);
    QRect focusRect = textBoundingRect.adjusted(-focusHMargin, -focusVMargin,
                                                +focusHMargin, +focusVMargin);

    // Render the item directly if we're not using a cached rendering
    // ========================================================================
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);

    if (progress > 0 && !(opt.state & QStyle::State_MouseOver))
    {
        opt.state |= QStyle::State_MouseOver;
        icon = d->applyHoverEffect(icon);
    }

    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
    painter->drawPixmap(iconPos, icon);

    d->drawTextItems(painter, labelLayout, infoLayout, textBoundingRect);
    d->drawFocusRect(painter, opt, focusRect);

    painter->restore();
}


QWidget *KFileItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt(option);
    d->initStyleOption(&opt, index);

    KTextEdit *edit = new KTextEdit(parent);
    edit->setAcceptRichText(false);
    edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setAlignment(opt.displayAlignment);
    edit->setEnabled(false); //Disable the text-edit to mark it as un-initialized
    return edit;
}


bool KFileItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(model)
    Q_UNUSED(option)
    Q_UNUSED(index)

    return false;
}


void KFileItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    KTextEdit *textedit = qobject_cast<KTextEdit*>(editor);
    Q_ASSERT(textedit != 0);

    //Do not update existing text that the user may already have edited.
    //The models will call setEditorData(..) whenever the icon has changed,
    //and this makes the editing work correctly despite that.
    if(textedit->isEnabled()) {
        return;
    }
    textedit->setEnabled(true); //Enable the text-edit to mark it as initialized

    const QVariant value = index.data(Qt::EditRole);
    const QString text = value.toString();
    textedit->insertPlainText(text);
    textedit->selectAll();

    const QString extension = KMimeType::extractKnownExtension(text);
    if (!extension.isEmpty()) {
        // The filename contains an extension. Assure that only the filename
        // gets selected.
        const int selectionLength = text.length() - extension.length() - 1;
        QTextCursor cursor = textedit->textCursor();
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, selectionLength);
        textedit->setTextCursor(cursor);
    }
}


void KFileItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    KTextEdit *textedit = qobject_cast<KTextEdit*>(editor);
    Q_ASSERT(textedit != 0);

    model->setData(index, textedit->toPlainText(), Qt::EditRole);
}


void KFileItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt(option);
    d->initStyleOption(&opt, index);
    d->setActiveMargins(d->verticalLayout(opt) ? Qt::Vertical : Qt::Horizontal);

    QRect r = d->labelRectangle(opt);

    // Use the full available width for the editor when maximumSize is set
    if (!d->maximumSize.isEmpty()) {
        if (d->verticalLayout(option)) {
            int diff = qMax(r.width(), d->maximumSize.width()) - r.width();
            if (diff > 1) {
                r.adjust(-(diff / 2), 0, diff / 2, 0);
            }
        }
        else {
            int diff = qMax(r.width(), d->maximumSize.width() - opt.decorationSize.width()) - r.width();
            if (diff > 0) {
                if (opt.decorationPosition == QStyleOptionViewItem::Left) {
                    r.adjust(0, 0, diff, 0);
                }
                else {
                    r.adjust(-diff, 0, 0, 0);
                }
            }
        }
    }

    KTextEdit *textedit = qobject_cast<KTextEdit*>(editor);
    Q_ASSERT(textedit != 0);
    const int frame = textedit->frameWidth();
    r.adjust(-frame, -frame, frame, frame);

    editor->setGeometry(r);
}


bool KFileItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option,
                                  const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(view)

    // if the tooltip information the model keeps is different from the display information,
    // show it always
    const QVariant toolTip = index.data(Qt::ToolTipRole);

    if (!toolTip.isValid()) {
        return false;
    }

    if (index.data() != toolTip) {
        return QAbstractItemDelegate::helpEvent(event, view, option, index);
    }

    if (!d->showToolTipWhenElided) {
        return false;
    }

    // in the case the tooltip information is the same as the display information,
    // show it only in the case the display information is elided
    QStyleOptionViewItemV4 opt(option);
    d->initStyleOption(&opt, index);
    d->setActiveMargins(d->verticalLayout(opt) ? Qt::Vertical : Qt::Horizontal);

    QTextLayout labelLayout;
    QTextLayout infoLayout;
    QRect textBoundingRect;
    d->layoutTextItems(opt, index, &labelLayout, &infoLayout, &textBoundingRect);
    const QString elidedText = d->elidedText(labelLayout, opt, textBoundingRect.size());

    if (elidedText != d->display(index)) {
        return QAbstractItemDelegate::helpEvent(event, view, option, index);
    }

    return false;
}

QRegion KFileItemDelegate::shape(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QStyleOptionViewItemV4 opt(option);
    d->initStyleOption(&opt, index);
    d->setActiveMargins(d->verticalLayout(opt) ? Qt::Vertical : Qt::Horizontal);

    QTextLayout labelLayout;
    QTextLayout infoLayout;
    QRect textBoundingRect;
    d->layoutTextItems(opt, index, &labelLayout, &infoLayout, &textBoundingRect);

    const QPoint pos = d->iconPosition(opt);
    QRect iconRect = QRect(pos, opt.icon.actualSize(opt.decorationSize));

    // Extend the icon rect so it touches the text rect
    switch (opt.decorationPosition)
    {
    case QStyleOptionViewItem::Top:
        if (iconRect.width() < textBoundingRect.width())
            iconRect.setBottom(textBoundingRect.top());
        else
            textBoundingRect.setTop(iconRect.bottom());
        break;
    case QStyleOptionViewItem::Bottom:
        if (iconRect.width() < textBoundingRect.width())
            iconRect.setTop(textBoundingRect.bottom());
        else
            textBoundingRect.setBottom(iconRect.top());
        break;
    case QStyleOptionViewItem::Left:
        iconRect.setRight(textBoundingRect.left());
        break;
    case QStyleOptionViewItem::Right:
        iconRect.setLeft(textBoundingRect.right());
        break;
    }

    QRegion region;
    region += iconRect;
    region += textBoundingRect;
    return region;
}

bool KFileItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    KTextEdit *editor = qobject_cast<KTextEdit*>(object);
    if (!editor)
        return false;

    switch (event->type())
    {
    case QEvent::KeyPress:
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key())
        {
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            emit commitData(editor);
            emit closeEditor(editor, NoHint);
            return true;

        case Qt::Key_Enter:
        case Qt::Key_Return: {
            const QString text = editor->toPlainText();
            if (text.isEmpty() || (text == QLatin1String(".")) || (text == QLatin1String("..")))
                return true; // So a newline doesn't get inserted

            emit commitData(editor);
            emit closeEditor(editor, SubmitModelCache);
            return true;
        }

        case Qt::Key_Escape:
            emit closeEditor(editor, RevertModelCache);
            return true;

        default:
            return false;
        } // switch (keyEvent->key())
    } // case QEvent::KeyPress

    case QEvent::FocusOut:
    {
        const QWidget *w = QApplication::activePopupWidget();
        if (!w || w->parent() != editor)
        {
            emit commitData(editor);
            emit closeEditor(editor, NoHint);
            return true;
        }
        else
            return false;
    }

    default:
        return false;
    } // switch (event->type())
}



#include "moc_kfileitemdelegate.cpp"


// kate: space-indent on; indent-width 4; replace-tabs on;
