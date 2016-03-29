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
#include <kmainwindow.h>
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
    connect(d->w_fullscreen, SIGNAL(clicked()), SLOT(setFullscreen()));

    connect(m_player, SIGNAL(paused(bool)), this, SLOT(_updatePlay(bool)));
    connect(m_player, SIGNAL(loaded()), this, SLOT(_updateLoaded()));
    connect(m_player, SIGNAL(seekable(bool)), this, SLOT(_updateSeekable(bool)));
    connect(m_player, SIGNAL(position(double)), this, SLOT(_updatePosition(double)));
    connect(m_player, SIGNAL(finished()), this, SLOT(_updateFinished()));
    connect(m_player, SIGNAL(error(QString)), this, SLOT(_updateError(QString)));

    if (options & DragDrop) {
        setAcceptDrops(true);
        m_player->setAcceptDrops(true);
    }

    if ((options & FullscreenVideo) == 0) {
        d->w_fullscreen->setVisible(false);
    }

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

void KMediaWidget::setFullscreen(int value)
{
    bool fullscreen;
    if (value == -1) {
        fullscreen = !m_player->isFullscreen();
    } else {
        fullscreen = bool(value);
    }
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

    if (fullscreen) {
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
    } else {
        if (m_parenthack && m_parentsizehack.isValid() && m_parent) {
            kDebug() << i18n("restoring parent from hack widget");
            setParent(m_parent);
            resize(m_parentsizehack);
            show();
            delete m_parenthack;
            m_parenthack = NULL;
            m_parent = NULL;
        } else if (m_parent) {
            kDebug() << i18n("restoring from parent widget");
            m_parent->showNormal();
        } else {
            kWarning() << i18n("cannot restore to non-fullscreen state");
        }
        m_player->setFullscreen(false);
    }
}

QSize KMediaWidget::sizeHint() const
{
    return d->w_player->sizeHint();
}

void KMediaWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_options & FullscreenVideo) {
        setFullscreen();
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
    event->ignore();
}

void KMediaWidget::_updateControls(bool visible)
{
    d->w_frame->setVisible(visible);
    emit controlsHidden(visible);
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

void KMediaWidget::_updateStatus(QString string)
{
    if (m_options & FullscreenVideo) {
        QWidget *windowwidget = window();
        KMainWindow *kmainwindow = qobject_cast<KMainWindow*>(windowwidget);
        if (kmainwindow) {
            kmainwindow->setCaption(string);
        } else if (windowwidget) {
            windowwidget->setWindowTitle(string);
        }
    }
}

void KMediaWidget::_updateFinished()
{
    m_replay = true;

    if (m_options & HiddenControls) {
        // show the controls until the next open
        m_timer.invalidate();
        _updateControls(true);
    }
    _updatePlay(true);
}

void KMediaWidget::_updateError(QString error)
{
    // since there are not many ways to indicate an error when
    // there are no extended controls use the play button to do so
    if (m_options & FullscreenVideo) {
        _updateStatus(error);
    } else {
        d->w_play->setIcon(KIcon("dialog-error"));
        d->w_play->setText(i18n("Error"));
    }

    d->w_position->setEnabled(false);
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
