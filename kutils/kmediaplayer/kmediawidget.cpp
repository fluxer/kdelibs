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

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QTimeLine>

#include "kdebug.h"
#include "klocale.h"
#include "kicon.h"
#include "kmainwindow.h"
#include "kstatusbar.h"
#include "kmediaplayer.h"
#include "kmediawidget.h"
#include "ui_kmediawidget.h"

class KMediaWidgetPrivate
{
public:
    KMediaWidgetPrivate();

    KMediaPlayer *m_player;
    bool m_dragdrop;
    bool m_fullscreen;
    bool m_hiddencontrols;
    bool m_smoothvolume;
    QWidget *m_parent;
    QMainWindow *m_parenthack;
    QSize m_parentsizehack;
    QTimeLine m_volumeline;
    int m_timerid;
    QString m_path;
    bool m_replay;
    bool m_visible;
    Ui_KMediaWidgetPrivate *m_ui;
};

KMediaWidgetPrivate::KMediaWidgetPrivate()
    : m_dragdrop(false), m_fullscreen(false), m_hiddencontrols(false), m_smoothvolume(false),
    m_parent(nullptr), m_parenthack(nullptr),
    m_timerid(0),
    m_replay(false),
    m_visible(false)
{
}

KMediaWidget::KMediaWidget(QWidget *parent, KMediaOptions options)
    : QWidget(parent), d(new KMediaWidgetPrivate())
{
    d->m_ui = new Ui_KMediaWidgetPrivate();
    d->m_ui->setupUi(this);
    d->m_player = new KMediaPlayer(d->m_ui->w_player);
    d->m_dragdrop = (options & DragDrop) != options;
    d->m_fullscreen = (options & FullscreenVideo) != options;
    d->m_hiddencontrols = (options & HiddenControls) != options;
    d->m_smoothvolume = (options & SmoothVolume) != options;
    d->m_parent = parent;

    d->m_ui->w_play->setIcon(KIcon("media-playback-start"));
    d->m_ui->w_play->setText(i18n("Play"));
    d->m_ui->w_play->setEnabled(false);
    d->m_ui->w_position->setEnabled(false);
    d->m_ui->w_volume->setValue(d->m_player->volume());
    d->m_ui->w_fullscreen->setIcon(KIcon("view-fullscreen"));

    connect(&d->m_volumeline, SIGNAL(frameChanged(int)), this, SLOT(_updateVolume(int)));
    d->m_volumeline.setFrameRange(0, d->m_ui->w_volume->value());
    d->m_volumeline.setDuration(3000);
    d->m_volumeline.setDirection(QTimeLine::Forward);

    connect(d->m_ui->w_play, SIGNAL(clicked()), this, SLOT(setPlay()));
    // connect(d->m_ui->w_position, SIGNAL(sliderMoved(int)), this, SLOT(setPosition(int)));
    connect(d->m_ui->w_position, SIGNAL(sliderReleased()), this, SLOT(_setPosition()));
    connect(d->m_ui->w_volume, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
    connect(d->m_ui->w_fullscreen, SIGNAL(clicked()), SLOT(setFullscreen()));

    connect(d->m_player, SIGNAL(paused(bool)), this, SLOT(_updatePlay(bool)));
    connect(d->m_player, SIGNAL(loaded()), this, SLOT(_updateLoaded()));
    connect(d->m_player, SIGNAL(seekable(bool)), this, SLOT(_updateSeekable(bool)));
    connect(d->m_player, SIGNAL(position(double)), this, SLOT(_updatePosition(double)));
    connect(d->m_player, SIGNAL(finished()), this, SLOT(_updateFinished()));
    connect(d->m_player, SIGNAL(error(QString)), this, SLOT(_updateError(QString)));

    if (d->m_dragdrop) {
        setAcceptDrops(true);
        d->m_player->setAcceptDrops(true);
    }

    if (!d->m_fullscreen) {
        d->m_ui->w_fullscreen->setVisible(false);
    }

    if (d->m_hiddencontrols) {
        d->m_visible = true;
        setMouseTracking(false);
    }
}

KMediaWidget::~KMediaWidget()
{
    if (d->m_timerid >= 0) {
        killTimer(d->m_timerid);
    }
    if (d->m_volumeline.state() == QTimeLine::Running) {
        d->m_volumeline.stop();
        setVolume(d->m_volumeline.endFrame());
    }
    d->m_player->stop();
    d->m_player->deleteLater();
    delete d->m_ui;
    delete d;
}

void KMediaWidget::open(const QString &path)
{
    // m_path should be updated from _updateLoaded() but that may be too late
    d->m_path = path;
    d->m_replay = false;

    d->m_ui->w_play->setEnabled(true);
    d->m_ui->w_position->setEnabled(true);

    if (d->m_smoothvolume) {
        if (d->m_volumeline.state() == QTimeLine::Running) {
            d->m_volumeline.stop();
            setVolume(d->m_volumeline.endFrame());
        }
        d->m_volumeline.setFrameRange(0, d->m_ui->w_volume->value());
        setVolume(0);
    }
    d->m_player->load(path);
    if (d->m_smoothvolume) {
        d->m_volumeline.start();
    }

    d->m_ui->w_position->setEnabled(d->m_player->isSeekable());
}

KMediaPlayer* KMediaWidget::player() const
{
    return d->m_player;
}

void KMediaWidget::setPlay(const int value)
{
    // TODO: can the position be stored and restored reliably as well?
    if (d->m_replay && !d->m_path.isEmpty()) {
        KMediaWidget::open(d->m_path);
        return;
    }

    bool pause;
    if (value == -1) {
        pause = d->m_player->isPlaying();
    } else {
        pause = bool(value);
    }
    if (pause) {
        d->m_player->pause();
    } else {
        d->m_player->play();
    }
}

void KMediaWidget::setPosition(const int value)
{
    d->m_player->seek(value);
}

void KMediaWidget::setVolume(const int value)
{
    d->m_player->setVolume(value);
}

void KMediaWidget::setFullscreen(const int value)
{
    bool fullscreen;
    if (value == -1) {
        fullscreen = !d->m_player->isFullscreen();
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
    if (!d->m_parent && (parentWidget() == window()) && !d->m_parenthack) {
        kDebug() << i18n("using parent widget from parentWidget()");
        d->m_parent = parentWidget();
        d->m_parentsizehack = QSize(-1, -1);
        d->m_parenthack = NULL;
    } else if (!d->m_parent && parentWidget()) {
        kWarning() << i18n("creating a parent, detaching widget, starting voodoo dance..");
        d->m_parent = parentWidget();
        d->m_parentsizehack = d->m_parent->size();
        d->m_parenthack = new QMainWindow(d->m_parent);
    }

    if (fullscreen) {
        if (d->m_parenthack && d->m_parentsizehack.isValid() && d->m_parent) {
            kDebug() << i18n("using parent hack widget");
            d->m_parenthack->setCentralWidget(this);
            d->m_parenthack->showFullScreen();
        } else if (d->m_parent) {
            kDebug() << i18n("using parent widget");
            d->m_parent->showFullScreen();
        } else {
            kWarning() << i18n("cannot set fullscreen state");
        }
        d->m_player->setFullscreen(true);
    } else {
        if (d->m_parenthack && d->m_parentsizehack.isValid() && d->m_parent) {
            kDebug() << i18n("restoring parent from hack widget");
            setParent(d->m_parent);
            resize(d->m_parentsizehack);
            show();
            delete d->m_parenthack;
            d->m_parenthack = NULL;
            d->m_parent = NULL;
        } else if (d->m_parent) {
            kDebug() << i18n("restoring from parent widget");
            d->m_parent->showNormal();
        } else {
            kWarning() << i18n("cannot restore to non-fullscreen state");
        }
        d->m_player->setFullscreen(false);
    }
}

void KMediaWidget::resetControlsTimer()
{
    if (d->m_timerid >= 0) {
        killTimer(d->m_timerid);
        d->m_timerid = 0;
    }
    // do not hide the controls if path is not loaded
    if (!d->m_player->path().isEmpty()) {
        d->m_timerid = startTimer(3000);
    }
}

QSize KMediaWidget::sizeHint() const
{
    return d->m_ui->w_player->sizeHint();
}

QSize KMediaWidget::minimumSizeHint() const
{
    if (d->m_fullscreen) {
        return QSize(300, 233);
    }
    return QSize(180, 140);
}

void KMediaWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (d->m_fullscreen) {
        setFullscreen();
    }
    event->ignore();
}

void KMediaWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (d->m_hiddencontrols) {
        resetControlsTimer();
        _updateControls(true);
    }
    event->ignore();
}

void KMediaWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d->m_timerid
        && !d->m_ui->w_play->isDown()
        && !d->m_ui->w_position->isSliderDown()
        && !d->m_ui->w_volume->isSliderDown()
        && !d->m_ui->w_fullscreen->isDown()) {
        _updateControls(false);
        event->accept();
    } else {
        event->ignore();
    }
}

void KMediaWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void KMediaWidget::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    QStringList invalid;
    foreach (const QUrl url, urls) {
        QString urlstring = url.toString();
        if (!d->m_player->isPathSupported(urlstring)) {
            kDebug() << i18n("ignoring unsupported:\n%1", urlstring);
            invalid.append(urlstring);
            continue;
        }
        open(urlstring);
    }
    if (!invalid.isEmpty()) {
        QMessageBox::warning(this, i18n("Invalid paths"),
            i18n("Some paths are invalid:\n%1", invalid.join("\n")));
    } else {
        event->acceptProposedAction();
    }
}

void KMediaWidget::_updateControls(const bool visible)
{
    if (d->m_hiddencontrols && visible != d->m_visible) {
        d->m_ui->w_frame->setVisible(visible);
        emit controlsHidden(visible);
        d->m_visible = visible;
    }
}

void KMediaWidget::_updatePlay(const bool paused)
{
    if (paused) {
        d->m_ui->w_play->setIcon(KIcon("media-playback-start"));
        d->m_ui->w_play->setText(i18n("Play"));
    } else {
        d->m_ui->w_play->setIcon(KIcon("media-playback-pause"));
        d->m_ui->w_play->setText(i18n("Pause"));
    }
}

void KMediaWidget::_updateSeekable(const bool seekable)
{
    d->m_ui->w_position->setEnabled(seekable);
    d->m_ui->w_position->setMaximum(d->m_player->totalTime());
}

void KMediaWidget::_setPosition()
{
    d->m_player->seek(d->m_ui->w_position->value());
}

void KMediaWidget::_updatePosition(const double seconds)
{
    // do not update the slider while it's dragged by the user
    if (!d->m_ui->w_position->isSliderDown()) {
        d->m_ui->w_position->setValue(seconds);
    }
}

void KMediaWidget::_updateLoaded()
{
    d->m_path = d->m_player->path();
    const QString title = d->m_player->title();
    if (!title.isEmpty()) {
        _updateStatus(title);
    }
    _updatePlay(!d->m_player->isPlaying());

    if (d->m_hiddencontrols) {
        setMouseTracking(true);
        resetControlsTimer();
        _updateControls(true);
    }
}

void KMediaWidget::_updateStatus(const QString string)
{
    if (d->m_fullscreen) {
        QWidget *windowwidget = window();
        KMainWindow *kmainwindow = qobject_cast<KMainWindow*>(windowwidget);
        if (kmainwindow) {
            kmainwindow->setCaption(string);
            KStatusBar *statusbar = kmainwindow->statusBar();
            if (statusbar) {
                if (d->m_player->isPlaying()) {
                    statusbar->showMessage(i18n("Now playing: %1", string));
                } else {
                    statusbar->showMessage(string);
                }
            }
        } else if (windowwidget) {
            windowwidget->setWindowTitle(string);
        }
    }
}

void KMediaWidget::_updateFinished()
{
    d->m_replay = true;

    if (d->m_hiddencontrols) {
        // show the controls until the next open
        if (d->m_timerid >= 0) {
            killTimer(d->m_timerid);
            d->m_timerid = 0;
        }
        setMouseTracking(false);
        _updateControls(true);
    }
    _updatePlay(true);
}

void KMediaWidget::_updateVolume(const int volume)
{
    if (volume == d->m_volumeline.endFrame()) {
        d->m_volumeline.stop();
    }
    setVolume(volume);
}

void KMediaWidget::_updateError(const QString error)
{
    if (d->m_fullscreen) {
        _updateStatus(error);
    } else {
        // since there are not many ways to indicate an error when
        // there are no extended controls use the play button to do so
        d->m_ui->w_play->setIcon(KIcon("dialog-error"));
        d->m_ui->w_play->setText(i18n("Error"));
    }

    d->m_ui->w_position->setEnabled(false);
}


#include "moc_kmediawidget.cpp"
