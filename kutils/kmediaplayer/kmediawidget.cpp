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
#include <QTimer>
#include <QTimeLine>

#include "kdebug.h"
#include "klocale.h"
#include "kicon.h"
#include "kmessagebox.h"
#include "kmainwindow.h"
#include "kstatusbar.h"
#include "kmediaplayer.h"
#include "kmediawidget.h"
#include "ui_kmediawidget.h"

class KMediaWidgetPrivate
{
public:
    KMediaWidgetPrivate(KMediaWidget* q);

    void updatePlayText(const QString &string);
    void updateStatus(const QString &string);

    KMediaWidget* m_q;
    KMediaPlayer *m_player;
    bool m_dragdrop;
    bool m_smoothvolume;
    QTimeLine m_volumeline;
    QString m_path;
    bool m_replay;
    QString m_playtext;
    Ui_KMediaWidgetUI *m_ui;
};

KMediaWidgetPrivate::KMediaWidgetPrivate(KMediaWidget* q)
    : m_q(q),
    m_player(nullptr),
    m_dragdrop(false), m_smoothvolume(false),
    m_replay(false),
    m_ui(nullptr)
{
}

void KMediaWidgetPrivate::updatePlayText(const QString &text)
{
    m_playtext = text;
    const QSize qsize = m_q->size();
    const QSize qminimumsize = m_q->minimumSizeHint();
    if (qsize.width() > qminimumsize.width()) {
        m_ui->w_play->setText(text);
    } else {
        m_ui->w_play->setText(QString());
    }
}

void KMediaWidgetPrivate::updateStatus(const QString &string)
{
    QWidget *windowwidget = m_q->window();
    KMainWindow *kmainwindow = qobject_cast<KMainWindow*>(windowwidget);
    if (kmainwindow) {
        kmainwindow->setCaption(string);
        KStatusBar *statusbar = kmainwindow->statusBar();
        if (statusbar) {
            if (m_player->isPlaying()) {
                statusbar->showMessage(i18n("Now playing: %1", string));
            } else {
                statusbar->showMessage(string);
            }
        }
    }
}

KMediaWidget::KMediaWidget(QWidget *parent, KMediaOptions options)
    : QWidget(parent),
    d(new KMediaWidgetPrivate(this))
{
    // TODO: show a buffering indicator somewhere (KPixmapSequence("process-working", 22))

    d->m_ui = new Ui_KMediaWidgetUI();
    d->m_ui->setupUi(this);
    d->m_player = new KMediaPlayer(d->m_ui->w_player);
    d->m_dragdrop = (options & DragDrop) != options;
    d->m_smoothvolume = (options & SmoothVolume) != options;

    d->m_ui->w_play->setIcon(KIcon("media-playback-start"));
    d->updatePlayText(i18n("Play"));
    d->m_ui->w_play->setEnabled(false);
    d->m_ui->w_position->setEnabled(false);
    d->m_ui->w_volume->setValue(d->m_player->volume());

    connect(&d->m_volumeline, SIGNAL(frameChanged(int)), this, SLOT(_updateVolume(int)));
    d->m_volumeline.setFrameRange(0, d->m_ui->w_volume->value());
    d->m_volumeline.setDuration(3000);
    d->m_volumeline.setDirection(QTimeLine::Forward);

    connect(d->m_ui->w_play, SIGNAL(clicked()), this, SLOT(setPlay()));
    d->m_ui->w_position->setTracking(false);
    connect(d->m_ui->w_position, SIGNAL(valueChanged(int)), this, SLOT(setPosition(int)));
    connect(d->m_ui->w_volume, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));

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
}

KMediaWidget::~KMediaWidget()
{
    if (d->m_volumeline.state() == QTimeLine::Running) {
        d->m_volumeline.stop();
        setVolume(d->m_volumeline.endFrame());
    }
    d->m_player->stop();
    /*
        Deleting the player has to be done before the UI is deleted because the player is embeded
        into UI widget and MPV accessing the window ID of d->m_ui->w_player may cause fatal X11 I/O
    */
    delete d->m_player;
    delete d->m_ui;
    delete d;
}

void KMediaWidget::open(const QString &path)
{
    // m_path should be updated from _updateLoaded() but that may be too late
    d->m_path = path;
    d->m_replay = false;

    if (d->m_smoothvolume) {
        if (d->m_volumeline.state() == QTimeLine::Running) {
            d->m_volumeline.stop();
            setVolume(d->m_volumeline.endFrame());
        }
        d->m_volumeline.setFrameRange(0, d->m_ui->w_volume->value());
        setVolume(0);
    }
    d->m_ui->w_position->setSliderPosition(0); // fake seek to start
    d->m_player->load(path);
    if (d->m_smoothvolume) {
        d->m_volumeline.start();
    }
}

KMediaPlayer* KMediaWidget::player() const
{
    return d->m_player;
}

void KMediaWidget::setPlay(const int value)
{
    if (d->m_replay && !d->m_path.isEmpty()) {
        KMediaWidget::open(d->m_path);
        return;
    }

    bool play;
    if (value == -1) {
        play = !d->m_player->isPlaying();
    } else {
        play = bool(value);
    }
    _updatePlay(!play); // the state is known, don't have to wait for paused() signal
    if (play) {
        d->m_player->play();
    } else {
        d->m_player->pause();
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

QSize KMediaWidget::sizeHint() const
{
    return d->m_ui->w_player->sizeHint();
}

QSize KMediaWidget::minimumSizeHint() const
{
    return QSize(180, 140);
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
    foreach (const QUrl &url, urls) {
        const QString urlstring = url.toString();
        if (!d->m_player->isPathSupported(urlstring)) {
            kDebug() << i18n("ignoring unsupported:\n%1", urlstring);
            invalid.append(urlstring);
            continue;
        }
        open(urlstring);
    }
    if (!invalid.isEmpty()) {
        KMessageBox::error(
            this,
            i18n("Some paths are invalid:\n%1", invalid.join("\n")),
            i18n("Invalid paths")
        );
    } else {
        event->acceptProposedAction();
    }
}

void KMediaWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    d->updatePlayText(d->m_playtext);
}

void KMediaWidget::_updatePlay(const bool paused)
{
    if (paused) {
        d->m_ui->w_play->setIcon(KIcon("media-playback-start"));
        d->updatePlayText(i18n("Play"));
    } else {
        d->m_ui->w_play->setIcon(KIcon("media-playback-pause"));
        d->updatePlayText(i18n("Pause"));
    }
}

void KMediaWidget::_updateSeekable(const bool seekable)
{
    d->m_ui->w_position->setEnabled(seekable);
    d->m_ui->w_position->setMaximum(d->m_player->totalTime());
}

void KMediaWidget::_updatePosition(const double seconds)
{
    // do not update the slider while it's dragged by the user
    if (!d->m_ui->w_position->isSliderDown()) {
        d->m_ui->w_position->setSliderPosition(qRound(seconds));
    }
}

void KMediaWidget::_updateLoaded()
{
    d->m_path = d->m_player->path();
    const QString title = d->m_player->title();
    if (!title.isEmpty()) {
        d->updateStatus(title);
    }
    d->m_ui->w_play->setEnabled(true);
    _updatePlay(!d->m_player->isPlaying());
    _updateSeekable(d->m_player->isSeekable());
}

void KMediaWidget::_updateFinished()
{
    d->m_replay = true;
    _updatePlay(true);
    d->m_ui->w_position->setSliderPosition(0);
}

void KMediaWidget::_updateVolume(const int volume)
{
    if (volume == d->m_volumeline.endFrame()) {
        d->m_volumeline.stop();
    }
    setVolume(volume);
}

void KMediaWidget::_updateError(const QString &error)
{
    d->updateStatus(error);
    // since there are not many ways to indicate an error use the play button to do so aswell in
    // case the status bar is hidden
    d->m_ui->w_play->setIcon(KIcon("dialog-error"));
    d->updatePlayText(i18n("Error"));

    d->m_ui->w_position->setEnabled(false);
}


#include "moc_kmediawidget.cpp"
