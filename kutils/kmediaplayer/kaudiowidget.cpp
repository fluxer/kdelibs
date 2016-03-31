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
#include <kfilemetainfo.h>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#ifndef QT_KATIE
#include <QMouseEvent>
#endif

#include "kmediaplayer.h"
#include "kaudiowidget.h"
#include "ui_kaudiowidget.h"

KAudioWidget::KAudioWidget(QWidget *parent, KAudioOptions options)
    : QWidget(parent)
{
    d = new Ui_KAudioWidgetPrivate();
    d->setupUi(this);
    m_player = new KAudioPlayer(this);
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
    connect(m_player, SIGNAL(seekable(bool)), this, SLOT(_updateSeekable(bool)));
    connect(m_player, SIGNAL(position(double)), this, SLOT(_updatePosition(double)));
    connect(m_player, SIGNAL(finished()), this, SLOT(_updateFinished()));
    connect(m_player, SIGNAL(error(QString)), this, SLOT(_updateError(QString)));

    if (options & DragDrop) {
        setAcceptDrops(true);
        d->w_table->setAcceptDrops(true);
    }

    startTimer(500);
}

KAudioWidget::~KAudioWidget()
{
    delete m_player;
    delete d;
}

void KAudioWidget::open(QString path)
{
    // m_path should be updated from _updateLoaded() but that may be too late
    m_path = path;
    m_replay = false;

    d->w_play->setEnabled(true);
    d->w_position->setEnabled(true);

    m_player->load(path);

    d->w_position->setEnabled(m_player->isSeekable());
}

KAudioPlayer* KAudioWidget::player()
{
    return m_player;
}

void KAudioWidget::setPlay(int value)
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

void KAudioWidget::setPosition(int value)
{
    m_player->seek(value);
}

void KAudioWidget::setVolume(int value)
{
    m_player->setVolume(value);
}

void KAudioWidget::setPlaylist(QStringList value)
{
    // TODO:
    Q_UNUSED(value);
}

QSize KAudioWidget::sizeHint() const
{
    return d->w_table->sizeHint();
}

QSize KAudioWidget::minimumSizeHint() const
{
    // TODO: revisit once UI is finalized
    return d->w_table->minimumSizeHint();
}

void KAudioWidget::timerEvent(QTimerEvent *event)
{
    if (m_player->isPlaying()) {
        // TODO: this could use some love
        d->w_time->setText(QString("%1/%2").arg(m_player->currentTime()).arg(m_player->totalTime()));
    }
    event->ignore();
}

void KAudioWidget::_updatePlay(bool paused)
{
    if (paused) {
        d->w_play->setIcon(KIcon("media-playback-start"));
    } else {
        d->w_play->setIcon(KIcon("media-playback-pause"));
    }
}

void KAudioWidget::_updateSeekable(bool seekable)
{
    d->w_position->setEnabled(seekable);
    d->w_position->setMaximum(m_player->totalTime());
}

void KAudioWidget::_updatePosition(double seconds)
{
    d->w_position->setValue(seconds);
}

void KAudioWidget::_updateLoaded()
{
    m_path = m_player->path();
    QString title = m_player->title();
    if (!title.isEmpty()) {
        _updateStatus(title);
    }
    _updatePlay(!m_player->isPlaying());
}

void KAudioWidget::_updateStatus(QString string)
{
    // TODO: set window title?
    d->w_status->setText(string);
}

void KAudioWidget::_updateFinished()
{
    m_replay = true;

    _updatePlay(true);
}

void KAudioWidget::_updateError(QString error)
{
    _updateStatus(error);

    d->w_play->setIcon(KIcon("dialog-error"));

    d->w_position->setEnabled(false);
}

void KAudioWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void KAudioWidget::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    QStringList invalid;
    bool isfirst = true;
    int rowcount = d->w_table->rowCount();
    foreach (const QUrl url, urls) {
        QString urlstring = url.toString();
        if (!m_player->isPathSupported(urlstring)) {
            kDebug() << i18n("ignoring unsupported:\n%1", urlstring);
            invalid.append(urlstring);
            continue;
        }
        if (isfirst) {
            open(urlstring);
            isfirst = false;
        }
        d->w_table->setRowCount(rowcount+1);
        // NOTE: using KUrl bellow as urlstring may contain file:// and the metainfo will be
        // considered invalid in such case
        KFileMetaInfo* metainfo = new KFileMetaInfo(KUrl(urlstring));
        if (metainfo->isValid()) {
            KFileMetaInfoItem titleinfo = metainfo->item("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title");
            d->w_table->setItem(rowcount, 0, new QTableWidgetItem(titleinfo.value().toString()));
            KFileMetaInfoItem artistinfo = metainfo->item("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator");
            d->w_table->setItem(rowcount, 1, new QTableWidgetItem(artistinfo.value().toString()));
            KFileMetaInfoItem albuminfo = metainfo->item("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum");
            d->w_table->setItem(rowcount, 2, new QTableWidgetItem(albuminfo.value().toString()));
            KFileMetaInfoItem lenghtinfo = metainfo->item("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration");
            d->w_table->setItem(rowcount, 3, new QTableWidgetItem(lenghtinfo.value().toString()));
            qDebug() << lenghtinfo.value().toString();
        } else {
            kWarning() << i18n("invalid metainfo");
            QTableWidgetItem *pathnameitem = new QTableWidgetItem(urlstring);
            d->w_table->setItem(rowcount, 0, pathnameitem);
        }
        rowcount += 1;
    }
    if (!invalid.isEmpty()) {
        QMessageBox::warning(this, i18n("Invalid paths"),
            i18n("Some paths are invalid:\n%1", invalid.join("\n")));
    }
    event->acceptProposedAction();
}

#include "moc_kaudiowidget.cpp"
