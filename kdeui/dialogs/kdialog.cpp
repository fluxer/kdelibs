/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org)
 *                      by Holger Freyther <freyther@kde.org>
 *            2005-2009 by Olivier Goffart (ogoffart at kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdialog.h"
#include "kdialog_p.h"
#include <kdebug.h>
#include "kdialogqueue_p.h"

#include <config.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QtGui/qevent.h>
#include <QPointer>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <QWhatsThis>

#include <klocale.h>
#include <kpushbutton.h>
#include <kseparator.h>
#include <kstandardguiitem.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <netwm.h>
#endif

static bool sAllowEmbeddingInGraphicsView = false;

void KDialogPrivate::setupLayout()
{
    Q_Q(KDialog);
    if (!dirty) {
        QMetaObject::invokeMethod(q, "queuedLayoutUpdate", Qt::QueuedConnection);
        dirty = true;
    }
}

void KDialogPrivate::queuedLayoutUpdate()
{
    if (!dirty) {
        return;
    }

    dirty = false;

    Q_Q(KDialog);

    // Don't lose the focus widget when re-creating the layout.
    // Testcase: KOrganizer's "Select Categories" dialog
    QPointer<QWidget> focusWidget = mMainWidget ? mMainWidget->focusWidget() : 0;

    if (q->layout() && q->layout() != mTopLayout) {
        kWarning() << q->metaObject()->className() << "created with a layout; don't do that, KDialog takes care of it, use mainWidget or setMainWidget instead";
        delete q->layout();
    }

    delete mTopLayout;

    if (mButtonOrientation == Qt::Horizontal) {
        mTopLayout = new QVBoxLayout(q);
    } else {
        mTopLayout = new QHBoxLayout(q);
    }

    if (mUrlHelp) {
        mTopLayout->addWidget(mUrlHelp, 0, Qt::AlignRight);
    }

    if (mMainWidget) {
        mTopLayout->addWidget(mMainWidget, 10);
    }

    if (mDetailsWidget) {
        mTopLayout->addWidget(mDetailsWidget);
    }

    if (mActionSeparator) {
        mTopLayout->addWidget(mActionSeparator);
    }

    if (mButtonBox) {
        mButtonBox->setOrientation(mButtonOrientation);
        mTopLayout->addWidget(mButtonBox);
    }

    if (focusWidget) {
        focusWidget->setFocus();
    }
}

void KDialogPrivate::appendButton(KDialog::ButtonCode key, const KGuiItem &item)
{
    Q_Q(KDialog);

    QDialogButtonBox::ButtonRole role = QDialogButtonBox::InvalidRole;
    switch (key) {
        case KDialog::Help:
        case KDialog::Details:
            role = QDialogButtonBox::HelpRole;
            break;
        case KDialog::Default:
        case KDialog::Reset:
            role = QDialogButtonBox::ResetRole;
            break;
        case KDialog::Ok:
            role = QDialogButtonBox::AcceptRole;
            break;
        case KDialog::Apply:
            role = QDialogButtonBox::ApplyRole;
            break;
        case KDialog::Try:
        case KDialog::Yes:
            role = QDialogButtonBox::YesRole;
            break;
        case KDialog::Close:
        case KDialog::Cancel:
            role = QDialogButtonBox::RejectRole;
            break;
        case KDialog::No:
            role = QDialogButtonBox::NoRole;
            break;
        case KDialog::User1:
        case KDialog::User2:
        case KDialog::User3:
            role = QDialogButtonBox::ActionRole;
            break;
        default:
            role = QDialogButtonBox::InvalidRole;
            break;
    }

    if (role == QDialogButtonBox::InvalidRole) {
        return;
    }

    KPushButton *button = new KPushButton(item);
    mButtonBox->addButton(button, role);

    mButtonList.insert(key, button);
    mButtonSignalMapper.setMapping(button, key);

    QObject::connect(button, SIGNAL(clicked()), &mButtonSignalMapper, SLOT(map()));

    if (key == mDefaultButton) {
        // Now that it exists, set it as default
        q->setDefaultButton(mDefaultButton);
    }
}

void KDialogPrivate::init(KDialog *q)
{
    q_ptr = q;

    dirty = false;

    q->setButtons(KDialog::Ok | KDialog::Cancel);
    q->setDefaultButton(KDialog::Ok);

    q->connect(&mButtonSignalMapper, SIGNAL(mapped(int)), q, SLOT(slotButtonClicked(int)));

    q->setWindowTitle(KGlobal::caption()); // set appropriate initial window title for case it gets not set later
}

void KDialogPrivate::helpLinkClicked()
{
    q_ptr->slotButtonClicked(KDialog::Help);
}

KDialog::KDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, sAllowEmbeddingInGraphicsView ? flags : flags | Qt::BypassGraphicsProxyWidget ),
    d_ptr(new KDialogPrivate())
{
    d_ptr->init(this);
}

KDialog::KDialog(KDialogPrivate &dd, QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, sAllowEmbeddingInGraphicsView ? flags : flags | Qt::BypassGraphicsProxyWidget),
    d_ptr(&dd)
{
    d_ptr->init(this);
}

KDialog::~KDialog()
{
    delete d_ptr;
}

void KDialog::setButtons(ButtonCodes buttonMask)
{
    Q_D(KDialog);
    if (d->mButtonBox) {
        d->mButtonList.clear();

        delete d->mButtonBox;
        d->mButtonBox = 0;
    }

    if (buttonMask & KDialog::Cancel) {
        buttonMask &= ~KDialog::Close;
    }
    if (buttonMask & KDialog::Apply) {
        buttonMask &= ~KDialog::Try;
    }
    if (buttonMask & KDialog::Details) {
        buttonMask &= ~KDialog::Default;
    }
    if (buttonMask == KDialog::None) {
        d->setupLayout();
        return; // When we want no button box
    }

    d->mEscapeButton = (buttonMask & KDialog::Cancel) ? KDialog::Cancel : KDialog::Close;
    d->mButtonBox = new QDialogButtonBox(this);

    if (buttonMask & KDialog::Help) {
        d->appendButton(KDialog::Help, KStandardGuiItem::help());
    }
    if (buttonMask & KDialog::Default) {
        d->appendButton(KDialog::Default, KStandardGuiItem::defaults());
    }
    if (buttonMask & KDialog::Reset) {
        d->appendButton(KDialog::Reset, KStandardGuiItem::reset());
    }
    if (buttonMask & KDialog::User3) {
        d->appendButton(KDialog::User3, KGuiItem());
    }
    if (buttonMask & KDialog::User2) {
        d->appendButton(KDialog::User2, KGuiItem());
    }
    if (buttonMask & KDialog::User1) {
        d->appendButton(KDialog::User1, KGuiItem());
    }
    if (buttonMask & KDialog::Ok) {
        d->appendButton(KDialog::Ok, KStandardGuiItem::ok());
    }
    if (buttonMask & KDialog::Apply) {
        d->appendButton(KDialog::Apply, KStandardGuiItem::apply());
    }
    if (buttonMask & KDialog::Try) {
        d->appendButton(KDialog::Try, KGuiItem(i18n("&Try")));
    }
    if (buttonMask & KDialog::Cancel) {
        d->appendButton(KDialog::Cancel, KStandardGuiItem::cancel());
    }
    if (buttonMask & KDialog::Close) {
        d->appendButton(KDialog::Close, KStandardGuiItem::close());
    }
    if (buttonMask & KDialog::Yes) {
        d->appendButton(KDialog::Yes, KStandardGuiItem::yes());
    }
    if (buttonMask & KDialog::No) {
        d->appendButton(KDialog::No, KStandardGuiItem::no());
    }
    if (buttonMask & KDialog::Details) {
        d->appendButton(KDialog::Details, KGuiItem(QString(), "help-about"));
        setDetailsWidgetVisible(false);
    }

    d->setupLayout();
}


void KDialog::setButtonsOrientation(Qt::Orientation orientation)
{
    Q_D(KDialog);
    if (d->mButtonOrientation != orientation) {
        d->mButtonOrientation = orientation;

        if (d->mActionSeparator) {
            d->mActionSeparator->setOrientation(d->mButtonOrientation);
        }

        if (d->mButtonOrientation == Qt::Vertical) {
            enableLinkedHelp(false); // 2000-06-18 Espen: No support for this yet.
        }
    }
}

void KDialog::setEscapeButton(ButtonCode id)
{
    d_func()->mEscapeButton = id;
}

void KDialog::setDefaultButton(ButtonCode newDefaultButton)
{
    Q_D(KDialog);

    if (newDefaultButton == KDialog::None) {
        newDefaultButton = KDialog::NoDefault; // #148969
    }

    const KDialog::ButtonCode oldDefault = defaultButton();

    bool oldDefaultHadFocus = false;

    if (oldDefault != KDialog::NoDefault) {
        KPushButton *old = button(oldDefault);
        if (old) {
            oldDefaultHadFocus = (focusWidget() == old);
            old->setDefault(false);
        }
    }

    if (newDefaultButton != KDialog::NoDefault) {
        KPushButton *b = button(newDefaultButton);
        if (b) {
            b->setDefault(true);
            if (focusWidget() == 0 || oldDefaultHadFocus) {
                // No widget had focus yet, or the old default button had
                // -> ok to give focus to the new default button, so that it's
                // really default (Enter triggers it).
                // But we don't do this if the kdialog user gave focus to a
                // specific widget in the dialog.
                b->setFocus();
            }
        }
    }
    d->mDefaultButton = newDefaultButton;
    Q_ASSERT(defaultButton() == newDefaultButton);
}

KDialog::ButtonCode KDialog::defaultButton() const
{
    Q_D(const KDialog);
    QHashIterator<int, KPushButton*> it(d->mButtonList);
    while (it.hasNext()) {
        it.next();
        if (it.value()->isDefault()) {
            return static_cast<KDialog::ButtonCode>(it.key());
        }
    }

    return d->mDefaultButton;
}

void KDialog::setMainWidget(QWidget *widget)
{
    Q_D(KDialog);
    if (d->mMainWidget == widget) {
        return;
    }
    d->mMainWidget = widget;
    if (d->mMainWidget && d->mMainWidget->layout()) {
        // Avoid double-margin problem
        d->mMainWidget->layout()->setMargin(0);
    }
    d->setupLayout();
}

QWidget *KDialog::mainWidget()
{
    Q_D(KDialog);
    if (!d->mMainWidget) {
        setMainWidget(new QWidget(this));
    }
    return d->mMainWidget;
}

QSize KDialog::sizeHint() const
{
    Q_D(const KDialog);
    if (!d->mMinSize.isEmpty()) {
        return d->mMinSize.expandedTo( minimumSizeHint() ) + d->mIncSize;
    }
    if (d->dirty) {
        const_cast<KDialogPrivate*>(d)->queuedLayoutUpdate();
    }
    return QDialog::sizeHint() + d->mIncSize;
}

QSize KDialog::minimumSizeHint() const
{
    Q_D(const KDialog);
    if (d->dirty) {
        const_cast<KDialogPrivate*>(d)->queuedLayoutUpdate();
    }
    return QDialog::minimumSizeHint() + d->mIncSize;
}

//
// Grab QDialogs keypresses if non-modal.
//
void KDialog::keyPressEvent(QKeyEvent *event)
{
    Q_D(KDialog);
    if (event->modifiers() == 0) {
        if (event->key() == Qt::Key_F1) {
            KPushButton *button = this->button(KDialog::Help);
            if (button) {
                button->animateClick();
                event->accept();
                return;
            }
        }

        if (event->key() == Qt::Key_Escape) {
            KPushButton *button = this->button(d->mEscapeButton);
            if (button) {
                button->animateClick();
                event->accept();
                return;
            }
        }
    } else if (event->key() == Qt::Key_F1 && event->modifiers() == Qt::ShiftModifier) {
        QWhatsThis::enterWhatsThisMode();
        event->accept();
        return;
    } else if (event->modifiers() == Qt::ControlModifier &&
        (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter))
    {
        // accept the dialog when Ctrl-Return is pressed
        KPushButton *button = this->button(KDialog::Ok);
        if (button) {
            button->animateClick();
            event->accept();
            return;
        }
    }

    QDialog::keyPressEvent(event);
}

int KDialog::marginHint()
{
    return QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin);
}

int KDialog::spacingHint()
{
    return QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
}

int KDialog::groupSpacingHint()
{
    return QApplication::fontMetrics().lineSpacing();
}

QString KDialog::makeStandardCaption(const QString &userCaption,
                                     QWidget* window,
                                     CaptionFlags flags)
{
    Q_UNUSED(window);
    QString caption = KGlobal::caption();
    QString captionString = userCaption.isEmpty() ? caption : userCaption;

    // If the document is modified, add '[modified]'.
    if (flags & KDialog::ModifiedCaption) {
        captionString += QString::fromUtf8(" [") + i18n("modified") + QString::fromUtf8("]");
    }

    if (!userCaption.isEmpty()) {
        // Add the application name if:
        // User asked for it, it's not a duplication  and the app name (caption()) is not empty
        if ( flags & AppNameCaption && !caption.isEmpty() && !userCaption.endsWith(caption)) {
           // TODO: check to see if this is a transient/secondary window before trying to add the app name
           //       on platforms that need this
          captionString += i18nc("Document/application separator in titlebar", " – ") + caption;
        }
    }

    return captionString;
}

void KDialog::setCaption(const QString &caption)
{
    setWindowTitle(makeStandardCaption(caption, this));
}

void KDialog::setCaption(const QString &caption, bool modified)
{
    CaptionFlags flags = HIGCompliantCaption;
    if (modified) {
        flags |= ModifiedCaption;
    }
    setWindowTitle(makeStandardCaption(caption, this, flags));
}

static QRect screenRect(QWidget *widget, int screen)
{
    QDesktopWidget *desktop = QApplication::desktop();
    KConfig gc("kdeglobals", KConfig::NoGlobals);
    KConfigGroup cg(&gc, "Windows");
    if (desktop->isVirtualDesktop() &&
        cg.readEntry("XineramaEnabled", true) &&
        cg.readEntry("XineramaPlacementEnabled", true))
    {

        if (screen < 0 || screen >= desktop->screenCount()) {
            if (screen == -1) {
                screen = desktop->primaryScreen();
            } else if (screen == -3) {
                screen = desktop->screenNumber(QCursor::pos());
            } else {
                screen = desktop->screenNumber(widget);
            }
        }

        return desktop->availableGeometry(screen);
    }
    return desktop->geometry();
}

void KDialog::centerOnScreen( QWidget *widget, int screen )
{
    if (!widget) {
        return;
    }

#ifdef Q_WS_X11
    if( !(widget->windowFlags() & Qt::X11BypassWindowManagerHint ) && widget->windowType() != Qt::Popup
        && NETRootInfo(QX11Info::display(), NET::Supported).isSupported(NET::WM2FullPlacement)) {
        return; // the WM can handle placement much better
    }
#endif

    const QRect rect = screenRect(widget, screen);
    widget->move(
        rect.center().x() - widget->width() / 2,
        rect.center().y() - widget->height() / 2
    );
}

bool KDialog::avoidArea(QWidget *widget, const QRect &area, int screen)
{
    if (!widget) {
        return false;
    }

    QRect fg = widget->frameGeometry();
    if (!fg.intersects(area)) {
        return true; // nothing to do.
    }

    const QRect scr = screenRect(widget, screen);
    QRect avoid(area); // let's add some margin
    avoid.translate(-5, -5);
    avoid.setRight(avoid.right() + 10);
    avoid.setBottom(avoid.bottom() + 10);

  if (qMax(fg.top(), avoid.top()) <= qMin(fg.bottom(), avoid.bottom())) {
    // We need to move the widget up or down
    int spaceAbove = qMax( 0, avoid.top() - scr.top() );
    int spaceBelow = qMax( 0, scr.bottom() - avoid.bottom() );
    if ( spaceAbove > spaceBelow ) // where's the biggest side?
      if ( fg.height() <= spaceAbove ) // big enough?
        fg.setY( avoid.top() - fg.height() );
      else
        return false;
    else
      if ( fg.height() <= spaceBelow ) // big enough?
        fg.setY( avoid.bottom() );
      else
        return false;
  }

    if (qMax(fg.left(), avoid.left()) <= qMin(fg.right(), avoid.right())) {
        // We need to move the widget left or right
        const int spaceLeft = qMax(0, avoid.left() - scr.left());
        const int spaceRight = qMax(0, scr.right() - avoid.right());
        if (spaceLeft > spaceRight) {
            // where's the biggest side?
            if (fg.width() <= spaceLeft) { // big enough?
                fg.setX(avoid.left() - fg.width());
            } else {
                return false;
            }
        } else {
            if (fg.width() <= spaceRight) { // big enough?
                fg.setX( avoid.right());
            } else {
                return false;
            }
        }
    }

    widget->move(fg.x(), fg.y());

    return true;
}

void KDialog::showButtonSeparator(bool state)
{
    Q_D(KDialog);
    if ((d->mActionSeparator != 0) == state) {
        return;
    }
    if (state) {
        if (d->mActionSeparator) {
            return;
        }

        d->mActionSeparator = new KSeparator(this);
        d->mActionSeparator->setOrientation(d->mButtonOrientation);
    } else {
        delete d->mActionSeparator;
        d->mActionSeparator = 0;
    }

    d->setupLayout();
}

void KDialog::setInitialSize(const QSize &size)
{
    d_func()->mMinSize = size;
    adjustSize();
}

void KDialog::incrementInitialSize(const QSize &size)
{
    d_func()->mIncSize = size;
    adjustSize();
}

KPushButton* KDialog::button(ButtonCode id) const
{
    Q_D(const KDialog);
    return d->mButtonList.value(id, 0);
}

void KDialog::enableButton(ButtonCode id, bool state)
{
    KPushButton *button = this->button(id);
    if (button) {
        button->setEnabled(state);
    }
}

bool KDialog::isButtonEnabled(ButtonCode id) const
{
    KPushButton *button = this->button(id);
    if (button) {
        return button->isEnabled();
    }
    return false;
}

void KDialog::enableButtonOk(bool state)
{
    enableButton(KDialog::Ok, state);
}

void KDialog::enableButtonApply(bool state)
{
    enableButton(KDialog::Apply, state);
}

void KDialog::enableButtonCancel(bool state)
{
    enableButton(KDialog::Cancel, state);
}

void KDialog::showButton(ButtonCode id, bool state)
{
    KPushButton *button = this->button(id);
    if (button) {
        state ? button->show() : button->hide();
    }
}

void KDialog::setButtonGuiItem(ButtonCode id, const KGuiItem &item)
{
    KPushButton *button = this->button(id);
    if (!button) {
        return;
    }
    button->setGuiItem(item);
}

void KDialog::setButtonMenu(ButtonCode id, QMenu *menu, ButtonPopupMode popupmode)
{
    KPushButton *button = this->button(id);
    if (button) {
        if (popupmode == KDialog::InstantPopup) {
            button->setMenu(menu);
        } else {
            button->setDelayedMenu(menu);
        }
    }
}

void KDialog::setButtonText(ButtonCode id, const QString &text)
{
    Q_D(KDialog);
    if (!d->mSettingDetails && (id == KDialog::Details)) {
        d->mDetailsButtonText = text;
        setDetailsWidgetVisible(d->mDetailsVisible);
        return;
    }

    KPushButton *button = this->button(id);
    if (button) {
        button->setText(text);
    }
}

QString KDialog::buttonText(ButtonCode id) const
{
    KPushButton *button = this->button(id);
    if (button) {
        return button->text();
    }
    return QString();
}

void KDialog::setButtonIcon(ButtonCode id, const KIcon &icon)
{
    KPushButton *button = this->button(id);
    if (button) {
        button->setIcon(icon);
    }
}

KIcon KDialog::buttonIcon(ButtonCode id) const
{
    KPushButton *button = this->button(id);
    if (button) {
        return KIcon(button->icon());
    }
    return KIcon();
}

void KDialog::setButtonToolTip( ButtonCode id, const QString &text)
{
    KPushButton *button = this->button(id);
    if (button) {
        if (text.isEmpty()) {
            button->setToolTip(QString());
        } else {
            button->setToolTip(text);
        }
    }
}

QString KDialog::buttonToolTip(ButtonCode id) const
{
    KPushButton *button = this->button(id);
    if (button) {
        return button->toolTip();
    }
    return QString();
}

void KDialog::setButtonWhatsThis(ButtonCode id, const QString &text)
{
    KPushButton *button = this->button(id);
    if (button) {
        if (text.isEmpty()) {
            button->setWhatsThis(QString());
        } else {
            button->setWhatsThis(text);
        }
    }
}

QString KDialog::buttonWhatsThis(ButtonCode id) const
{
    KPushButton *button = this->button(id);
    if (button) {
        return button->whatsThis();
    }
    return QString();
}

void KDialog::setButtonFocus(ButtonCode id)
{
    KPushButton *button = this->button(id);
    if (button) {
        button->setFocus();
    }
}

void KDialog::setDetailsWidget(QWidget *detailsWidget)
{
    Q_D(KDialog);
    if (d->mDetailsWidget == detailsWidget) {
        return;
    }
    delete d->mDetailsWidget;
    d->mDetailsWidget = detailsWidget;

    if (d->mDetailsWidget->parentWidget() != this) {
        d->mDetailsWidget->setParent(this);
    }

    d->mDetailsWidget->hide();
    d->setupLayout();

    if (!d->mSettingDetails) {
        setDetailsWidgetVisible(d->mDetailsVisible);
    }
}

bool KDialog::isDetailsWidgetVisible() const
{
    return d_func()->mDetailsVisible;
}

void KDialog::setDetailsWidgetVisible(bool visible)
{
    Q_D(KDialog);
    if (d->mDetailsButtonText.isEmpty()) {
        d->mDetailsButtonText = i18n("&Details");
    }

    d->mSettingDetails = true;
    d->mDetailsVisible = visible;
    if (d->mDetailsVisible) {
        emit aboutToShowDetails();
        setButtonText(KDialog::Details, d->mDetailsButtonText + " <<");
        if (d->mDetailsWidget) {
            if (layout()) {
                layout()->setEnabled(false);
            }

            d->mDetailsWidget->show();

            adjustSize();

            if (layout()) {
                layout()->activate();
                layout()->setEnabled(true);
            }
        }
    } else {
        setButtonText(KDialog::Details, d->mDetailsButtonText + " >>");
        if (d->mDetailsWidget) {
            d->mDetailsWidget->hide();
        }

        if (layout()) {
            layout()->activate();
            adjustSize();
        }
    }

    d->mSettingDetails = false;
}

void KDialog::delayedDestruct()
{
    if (isVisible()) {
        hide();
    }
    deleteLater();
}


void KDialog::slotButtonClicked(int button)
{
    Q_D(KDialog);
    emit buttonClicked(static_cast<KDialog::ButtonCode>(button));

    switch(button) {
        case KDialog::Ok: {
            emit okClicked();
            accept();
            break;
        }
        case KDialog::Apply: {
            emit applyClicked();
            break;
        }
        case KDialog::Try: {
            emit tryClicked();
            break;
        }
        case KDialog::User3: {
            emit user3Clicked();
            break;
        }
        case KDialog::User2: {
            emit user2Clicked();
            break;
        }
        case KDialog::User1: {
            emit user1Clicked();
            break;
        }
        case KDialog::Yes: {
            emit yesClicked();
            done(KDialog::Yes);
            break;
        }
        case KDialog::No: {
            emit noClicked();
            done(KDialog::No);
            break;
        }
        case KDialog::Cancel: {
            emit cancelClicked();
            reject();
            break;
        }
        case KDialog::Close: {
            emit closeClicked();
            done(KDialog::Close); // KDE5: call reject() instead; more QDialog-like.
            break;
        }
        case KDialog::Help: {
            emit helpClicked();
            if (!d->mAnchor.isEmpty() || !d->mHelpApp.isEmpty()) {
                KToolInvocation::invokeHelp( d->mAnchor, d->mHelpApp);
            }
            break;
        }
        case KDialog::Default: {
            emit defaultClicked();
            break;
        }
        case KDialog::Reset: {
            emit resetClicked();
            break;
        }
        case KDialog::Details: {
            setDetailsWidgetVisible(!d->mDetailsVisible);
            break;
        }
    }

    // If we're here from the closeEvent, and auto-delete is on, well, auto-delete now.
    if (d->mDeferredDelete) {
        d->mDeferredDelete = false;
        delayedDestruct();
    }
}

void KDialog::enableLinkedHelp(bool state)
{
    Q_D(KDialog);
    if ((d->mUrlHelp != 0) == state) {
        return;
    }
    if (state) {
        if (d->mUrlHelp) {
            return;
        }

        d->mUrlHelp = new KUrlLabel(this);
        d->mUrlHelp->setText(helpLinkText());
        d->mUrlHelp->setFloatEnabled(true);
        d->mUrlHelp->setUnderline(true);
        d->mUrlHelp->setMinimumHeight( fontMetrics().height() + marginHint());
        connect( d->mUrlHelp, SIGNAL(leftClickedUrl()), SLOT(helpLinkClicked()));

        d->mUrlHelp->show();
    } else {
        delete d->mUrlHelp;
        d->mUrlHelp = 0;
    }

    d->setupLayout();
}


void KDialog::setHelp(const QString &anchor, const QString &appname)
{
    Q_D(KDialog);
    d->mAnchor  = anchor;
    d->mHelpApp = appname;
}


void KDialog::setHelpLinkText(const QString &text)
{
    Q_D(KDialog);
    d->mHelpLinkText = text;
    if (d->mUrlHelp) {
        d->mUrlHelp->setText(helpLinkText());
    }
}

QString KDialog::helpLinkText() const
{
    Q_D(const KDialog);
    return (d->mHelpLinkText.isEmpty() ? i18n("Get help...") : d->mHelpLinkText);
}

void KDialog::hideEvent(QHideEvent *event)
{
    emit hidden();
    if (!event->spontaneous()) {
        emit finished();
    }
}

void KDialog::closeEvent(QCloseEvent *event)
{
    Q_D(KDialog);
    KPushButton *button = this->button(d->mEscapeButton);
    if (button && !isHidden()) {
        button->animateClick();

        if (testAttribute(Qt::WA_DeleteOnClose)) {
            // Don't let QWidget::close do a deferred delete just yet, wait for the click first
            d->mDeferredDelete = true;
            setAttribute(Qt::WA_DeleteOnClose, false);
        }
    } else {
        QDialog::closeEvent(event);
    }
}

void KDialog::restoreDialogSize(const KConfigGroup &cfg)
{
    int width, height;
    int scnum = QApplication::desktop()->screenNumber(parentWidget());
    QRect desk = QApplication::desktop()->screenGeometry(scnum);

    width = sizeHint().width();
    height = sizeHint().height();

    width = cfg.readEntry(QString::fromLatin1("Width %1").arg(desk.width()), width);
    height = cfg.readEntry(QString::fromLatin1("Height %1").arg(desk.height()), height);

    resize(width, height);
}

void KDialog::saveDialogSize(KConfigGroup &config, KConfigGroup::WriteConfigFlags options) const
{
    int scnum = QApplication::desktop()->screenNumber(parentWidget());
    QRect desk = QApplication::desktop()->screenGeometry(scnum);

    const QSize sizeToSave = size();

    config.writeEntry(QString::fromLatin1("Width %1").arg(desk.width()), sizeToSave.width(), options);
    config.writeEntry(QString::fromLatin1("Height %1").arg(desk.height()), sizeToSave.height(), options);
}

void KDialog::setAllowEmbeddingInGraphicsView(bool allowEmbedding)
{
    sAllowEmbeddingInGraphicsView = allowEmbedding;
}


class KDialogQueue::Private
{
public:
    Private(KDialogQueue *q): q(q), busy(false) {}

    void slotShowQueuedDialog();

    KDialogQueue *q;
    QList< QPointer<QDialog> > queue;
    bool busy;
};

KDialogQueue* KDialogQueue::self()
{
    K_GLOBAL_STATIC(KDialogQueue, _self)
    return _self;
}

KDialogQueue::KDialogQueue()
    : d(new Private(this))
{
}

KDialogQueue::~KDialogQueue()
{
    delete d;
}

// static
void KDialogQueue::queueDialog(QDialog *dialog)
{
    KDialogQueue *_this = self();
    _this->d->queue.append(dialog);

    QTimer::singleShot(0, _this, SLOT(slotShowQueuedDialog()));
}

void KDialogQueue::Private::slotShowQueuedDialog()
{
    if (busy) {
        return;
    }

    QDialog *dialog = nullptr;
    do {
        if (queue.isEmpty()) {
            return;
        }
        dialog = queue.first();
        queue.pop_front();
    } while(!dialog);

    busy = true;
    dialog->exec();
    busy = false;
    delete dialog;

    if (!queue.isEmpty()) {
        QTimer::singleShot(20, q, SLOT(slotShowQueuedDialog()));
    }
}

#include "moc_kdialog.cpp"
#include "moc_kdialogqueue_p.cpp"
