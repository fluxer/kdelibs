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
#include "ui_kplaylistmanager.h"

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
    connect(m_player, SIGNAL(loaded()), this, SLOT(_updateMove()));
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
        _updateStatus(i18n("Awaiting input..."));

        _updateMove();
        connect(d->w_previous, SIGNAL(clicked()), this, SLOT(_movePrevious()));
        connect(d->w_next, SIGNAL(clicked()), this, SLOT(_moveNext()));

        m_menu = new QMenu(d->w_menu);
        m_menu->addAction(KIcon("view-list-text"), i18n("&Playlist"), this, SLOT(_menuManagePaths()));
        m_menu->addSeparator();
        m_menu->addAction(KIcon("document-open-remote"), i18n("O&pen URL"), this, SLOT(_menuOpenURL()));
        m_menu->addAction(KIcon("document-open"), i18n("&Open"), this, SLOT(_menuOpen()));
        m_menu->addSeparator();
        m_menu->addAction(KIcon("application-exit"), i18n("&Quit"), this, SLOT(_menuQuit()));
        connect(d->w_menu, SIGNAL(clicked()), this, SLOT(_showMenu()));

        m_pathmanager = new QDialog(this);
        d2 = new Ui_KPlaylistManagerPrivate();
        d2->setupUi(m_pathmanager);
        connect(d2->w_open, SIGNAL(clicked()), this, SLOT(_pathsOpen()));
        connect(d2->w_openurl, SIGNAL(clicked()), this, SLOT(_pathsOpenURL()));
        connect(d2->w_ok, SIGNAL(clicked()), this, SLOT(_pathsSave()));
        connect(d2->w_cancel, SIGNAL(clicked()), this, SLOT(_pathsReject()));
    }
    d->w_previous->setVisible(extcontrols);
    d->w_next->setVisible(extcontrols);
    d->w_menu->setVisible(extcontrols);
    d->w_status->setVisible(extcontrols);

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
    m_path = path;
    _updateStatus(i18n("Now playing: %1", path));

    d->w_play->setEnabled(true);
    d->w_position->setEnabled(true);

    m_player->load(path);

    d->w_position->setEnabled(m_player->isSeekable());

    m_failed = false;

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
    if (m_failed && !m_path.isEmpty()) {
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

void KMediaWidget::_movePrevious()
{
    QStringList playlist = m_player->paths();
    int position = playlist.indexOf(m_path);
    if (position > 0) {
        open(playlist.at(position-1));
    }
}

void KMediaWidget::_moveNext()
{
    QStringList playlist = m_player->paths();
    int position = playlist.indexOf(m_path);
    if (position+1 < playlist.count()) {
        open(playlist.at(position+1));
    }
}

void KMediaWidget::_fullscreen()
{
    /*
        Making a QWidget go fullscreen requires quite some magic for X11
        because showFullScreen() requires the parent of the widget to be
        a window (QMainWindow) thus some black magic bellow. asking the
        parent widget to go fullscreen is required to preserve the media
        controls visible and interactive. Note that setting the MPV
        property is just for consistency and possible clients quering it
        and nothing more as it does nothing when MPV is embed (as of the
        time of writing this)
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
        m_parenthack = new QMainWindow();
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
    if (m_visible != visible && !m_path.isEmpty()) {
        m_visible = visible;
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

void KMediaWidget::_updateMove()
{
    QStringList playlist = m_player->paths();
    d->w_previous->setEnabled(false);
    d->w_next->setEnabled(false);
    int position = playlist.indexOf(m_path);
    if (position > 0) {
        d->w_previous->setEnabled(true);
    }
    if (position+1 < playlist.count()) {
        d->w_next->setEnabled(true);
    }

    _updatePlay(!m_player->isPlaying());
    if (playlist.count() > 0) {
        d->w_play->setEnabled(true);
        m_path = playlist.first();
    }
}

void KMediaWidget::_updateStatus(QString error)
{
    if (m_options & ExtendedControls) {
        d->w_status->setText(error);
    }
}

void KMediaWidget::_updateFinished()
{
    _updateStatus(i18n("Was playing: %1", m_path));
    // BUG: it will move to start/end, what if there was a reason arg for finished()?
    // TODO: _moveNext()
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
        d->w_status->setText(error);
    } else {
        d->w_play->setIcon(KIcon("dialog-error"));
        d->w_play->setText(i18n("Error"));
    }

    m_failed = true;

    d->w_position->setEnabled(false);
}

void KMediaWidget::_menuManagePaths()
{
    d2->w_list->clear();
    QStringList playlist = m_player->paths();
    d2->w_list->insertStringList(playlist);
    m_pathmanager->show();
}

void KMediaWidget::_pathsOpen()
{
    QStringList paths = QFileDialog::getOpenFileNames(this, i18n("Select paths"));
    if (!paths.isEmpty()) {
        QStringList invalid, duplicate;
        foreach (const QString path, paths) {
            if (!m_player->isPathSupported(path)) {
                kDebug() << i18n("ignoring unsupported:\n%1", path);
                invalid.append(path);
                continue;
            } else if (d2->w_list->items().contains(path)) {
                duplicate.append(path);
                continue;
            }
            d2->w_list->insertItem(path);
        }
        if (!duplicate.isEmpty()) {
            QMessageBox::warning(this, i18n("Duplicate paths"),
                i18n("Some paths are duplicate:\n%1", duplicate.join("\n")));
        }
        if (!invalid.isEmpty()) {
            QMessageBox::warning(this, i18n("Invalid paths"),
                i18n("Some paths are invalid:\n%1", invalid.join("\n")));
        }
    }
}

void KMediaWidget::_pathsOpenURL()
{
    QString url = QInputDialog::getText(this, i18n("Input URL"),
        i18n("Supported protocols are: %1", m_player->protocols().join(",")));
    if (!url.isEmpty()) {
        if (!m_player->isPathSupported(url)) {
            kDebug() << i18n("ignoring unsupported:\n%1", url);
            QMessageBox::warning(this, i18n("Invalid URL"),
                i18n("Invalid URL:\n%1", url));
        } else if (d2->w_list->items().contains(url)) {
            QMessageBox::warning(this, i18n("Duplicate URL"),
                i18n("Duplicate URL:\n%1", url));
        } else {
            d2->w_list->insertItem(url);
        }
    }
}

void KMediaWidget::_pathsSave()
{
    m_player->clearPaths();
    QStringList playlist = d2->w_list->items();
    for (int i = 0; i < playlist.count(); ++i) {
        m_player->addPath(playlist.at(i));
    }
    _updateMove();
    if (!playlist.isEmpty()) {
        d->w_play->setEnabled(true);
    }
    m_path = playlist.first();
    m_pathmanager->hide();
}

void KMediaWidget::_pathsReject()
{
    m_pathmanager->hide();
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
    QStringList paths = QFileDialog::getOpenFileNames(this, i18n("Select paths"));
    if (!paths.isEmpty()) {
        bool isfirst = true;
        QStringList invalid;
        foreach (const QString path, paths) {
            if (!m_player->isPathSupported(path)) {
                kDebug() << i18n("ignoring unsupported:\n%1", path);
                invalid.append(path);
                continue;
            }
            if (isfirst) {
                open(path);
                isfirst = false;
            }
            m_player->addPath(path);
        }
        if (!invalid.isEmpty()) {
            QMessageBox::warning(this, i18n("Invalid paths"),
                i18n("Some paths are invalid:\n%1", invalid.join("\n")));
        }
    }
}

void KMediaWidget::_menuQuit()
{
    qApp->quit();
}

#include "moc_kmediawidget.cpp"
