/*  This file is part of the KDE libraries
    Copyright (C) 2016 Ivailo Monev <xakepa10@gmail.com>

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

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#ifndef QT_KATIE
#include <QMouseEvent>
#endif

#include "kmediaplayer.h"
#include "kmediawidget.h"
#include "ui_kmediawidget.h"

KMediaWidget::KMediaWidget(QWidget *parent, KMediaOptions options)
    : QWidget(parent)
{
    d = new Ui_KMediaWidgetPrivate();
    d->setupUi(this);
    m_player = new KMediaPlayer(d->w_player);
    m_options = options;
    m_parent = parent;

    d->w_play->setEnabled(false);
    d->w_position->setEnabled(false);
    d->w_volume->setValue(m_player->volume());

    connect(d->w_play, SIGNAL(clicked()), this, SLOT(setPlay()));
    connect(d->w_position, SIGNAL(sliderMoved(int)), this, SLOT(setPosition(int)));
    connect(d->w_volume, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));

    connect(m_player, SIGNAL(paused(bool)), this, SLOT(_updatePlay(bool)));
    connect(m_player, SIGNAL(loaded()), this, SLOT(_updateLoaded()));
    connect(m_player, SIGNAL(finished()), this, SLOT(_updateFinished()));
    connect(m_player, SIGNAL(error(QString)), this, SLOT(_updateError(QString)));
    connect(m_player, SIGNAL(seekable(bool)), this, SLOT(_updateSeekable(bool)));
    connect(m_player, SIGNAL(position(double)), this, SLOT(_updatePosition(double)));

    if (options & DragDrop) {
        setAcceptDrops(true);
        m_player->setAcceptDrops(true);
    }

    bool extcontrols = (options & ExtendedControls);
    if (extcontrols) {
        _updateLoaded();

        m_menu = new QMenu(d->w_menu);
        m_menu->addAction(KIcon("document-open-remote"), i18n("O&pen URL"), this, SLOT(_menuOpenURL()));
        m_menu->addAction(KIcon("document-open"), i18n("&Open"), this, SLOT(_menuOpen()));
        m_menu->addSeparator();
        m_menu->addAction(KIcon("application-exit"), i18n("&Quit"), this, SLOT(_menuQuit()));
        connect(d->w_menu, SIGNAL(clicked()), this, SLOT(_showMenu()));
    }
    d->w_menu->setVisible(extcontrols);

    if (options & HiddenControls) {
        setMouseTracking(true);
    }
}

KMediaWidget::~KMediaWidget()
{
    delete m_player;
    delete d;
}

void KMediaWidget::open(QString path)
{
    // m_path should be updated from _updateLoaded() but that may be too late
    m_path = path;
    m_replay = false;

    d->w_play->setEnabled(true);
    d->w_position->setEnabled(true);

    m_player->load(path);

    d->w_position->setEnabled(m_player->isSeekable());

    if (m_options & HiddenControls) {
        startTimer(200);
        m_timer.start();
    }
}

KMediaPlayer* KMediaWidget::player()
{
    return m_player;
}

void KMediaWidget::setPlay(int value)
{
    // TODO: can we reliably store the position and restore it as well?
    if (m_replay && !m_path.isEmpty()) {
        open(m_path);
        return;
    }

    bool pause;
    if (value == -1) {
        pause = m_player->isPlaying();
    } else {
        pause = bool(value);
    }
    if (pause) {
        m_player->pause();
    } else {
        m_player->play();
    }
}

void KMediaWidget::setPosition(int value)
{
    m_player->seek(value);
}

void KMediaWidget::setVolume(int value)
{
    m_player->setVolume(value);
}

QSize KMediaWidget::sizeHint() const
{
    return d->w_player->sizeHint();
}

void KMediaWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_options & FullscreenVideo) {
        _fullscreen();
        event->ignore();
    }
}

void KMediaWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_options & HiddenControls) {
        _updateControls(true);
        m_timer.restart();
        event->ignore();
    }
}

void KMediaWidget::timerEvent(QTimerEvent *event)
{
    if (m_timer.elapsed() > 3000) {
        _updateControls(false);
    }
    Q_UNUSED(event);
}

void KMediaWidget::_showMenu()
{
    m_menu->exec(QCursor::pos());
}

void KMediaWidget::_fullscreen()
{
    /*
        Making a QWidget go fullscreen requires quite some magic for X11
        because showFullScreen() requires the parent of the widget to be a
        window (QMainWindow) thus the hack bellow. Asking the parent widget to
        go fullscreen is required to preserve the media controls visible and
        interactive. Note that setting the MPV property is just for consistency
        and possible clients quering it, it does nothing when MPV is embed (as
        of the time of writing this).
    */
    if (!m_parent && (parentWidget() == window()) && !m_parenthack) {
        kDebug() << i18n("using parent widget from parentWidget()");
        m_parent = parentWidget();
        m_parentsizehack = QSize(-1, -1);
        m_parenthack = NULL;
    } else if (!m_parent && parentWidget()) {
        kWarning() << i18n("creating a parent, detaching widget, starting voodoo dance..");
        m_parent = parentWidget();
        m_parentsizehack = m_parent->size();
        m_parenthack = new QMainWindow(m_parent);
    }
    if (m_player->isFullscreen()) {
        if (m_parenthack && m_parentsizehack.isValid() && m_parent) {
            kDebug() << i18n("restoring parent from hack widget");
            setParent(m_parent);
            resize(m_parentsizehack);
            show();
            delete m_parenthack;
            m_parenthack = NULL;
        } else if (m_parent) {
            kDebug() << i18n("restoring from parent widget");
            m_parent->showNormal();
        } else {
            kWarning() << i18n("cannot restore to non-fullscreen state");
        }
        m_player->setFullscreen(false);
    } else {
        if (m_parenthack && m_parentsizehack.isValid() && m_parent) {
            kDebug() << i18n("using parent hack widget");
            m_parenthack->setCentralWidget(this);
            m_parenthack->showFullScreen();
        } else if (m_parent) {
            kDebug() << i18n("using parent widget");
            m_parent->showFullScreen();
        } else {
            kWarning() << i18n("cannot set fullscreen state");
        }
        m_player->setFullscreen(true);
    }
}

void KMediaWidget::_updateControls(bool visible)
{
    // avoid hiding the controls until something has been played
    if (m_player->path().isEmpty()) {
        d->w_frame->setVisible(true);
    } else {
        d->w_frame->setVisible(visible);
    }
}

void KMediaWidget::_updatePlay(bool paused)
{
    if (paused) {
        d->w_play->setIcon(KIcon("media-playback-start"));
        d->w_play->setText(i18n("Play"));
    } else {
        d->w_play->setIcon(KIcon("media-playback-pause"));
        d->w_play->setText(i18n("Pause"));
    }
}

void KMediaWidget::_updateSeekable(bool seekable)
{
    d->w_position->setEnabled(seekable);
    d->w_position->setMaximum(m_player->totalTime());
}

void KMediaWidget::_updatePosition(double seconds)
{
    d->w_position->setValue(seconds);
}

void KMediaWidget::_updateLoaded()
{
    m_path = m_player->path();
    QString title = m_player->title();
    if (!title.isEmpty()) {
        _updateStatus(title);
    }
    _updatePlay(!m_player->isPlaying());
}

void KMediaWidget::_updateStatus(QString error)
{
    if (m_options & ExtendedControls) {
        QWidget *windowwidget = window();
        if (windowwidget) {
            windowwidget->setWindowTitle(error);
        }
    }
}

void KMediaWidget::_updateFinished()
{
    m_replay = true;

    _updatePlay(true);
}

void KMediaWidget::_updateError(QString error)
{
    if (m_options & HiddenControls) {
        // show the controls until the next open
        m_timer.invalidate();
        _updateControls(true);
    }
    // since there are not many ways to indicate an error when
    // there are no extended controls use the play button to do so
    if (m_options & ExtendedControls) {
        _updateStatus(error);
    } else {
        d->w_play->setIcon(KIcon("dialog-error"));
        d->w_play->setText(i18n("Error"));
    }

    m_replay = true;

    d->w_position->setEnabled(false);
}

void KMediaWidget::_menuOpenURL()
{
    QString url = QInputDialog::getText(this, i18n("Input URL"),
        i18n("Supported protocols are: %1", m_player->protocols().join(",")));
    if (!url.isEmpty()) {
        if (!m_player->isPathSupported(url)) {
            kDebug() << i18n("ignoring unsupported:\n%1", url);
            QMessageBox::warning(this, i18n("Invalid URL"),
                i18n("Invalid URL:\n%1", url));
        } else {
            open(url);
        }
    }
}

void KMediaWidget::_menuOpen()
{
    QString path = QFileDialog::getOpenFileName(this, i18n("Select paths"));
    if (!path.isEmpty()) {
        if (!m_player->isPathSupported(path)) {
            kDebug() << i18n("ignoring unsupported:\n%1", path);
            QMessageBox::warning(this, i18n("Invalid path"),
                i18n("The path is invalid:\n%1", path));
        } else {
            open(path);
        }
    }
}

void KMediaWidget::_menuQuit()
{
    qApp->quit();
}

void KMediaWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void KMediaWidget::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    QStringList invalid;
    foreach (const QUrl url, urls) {
        QString urlstring = url.toString();
        if (!m_player->isPathSupported(urlstring)) {
            kDebug() << i18n("ignoring unsupported:\n%1", urlstring);
            invalid.append(urlstring);
            continue;
        }
        open(urlstring);
    }
    if (!invalid.isEmpty()) {
        QMessageBox::warning(this, i18n("Invalid paths"),
            i18n("Some paths are invalid:\n%1", invalid.join("\n")));
    }
    event->acceptProposedAction();
}

#include "moc_kmediawidget.cpp"
