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

#ifndef KMEDIAWIDGET_H
#define KMEDIAWIDGET_H

#include <QWidget>
#include <QMainWindow>
#include <QElapsedTimer>
#include <QMenu>
#include <kmediaplayer_export.h>
#include <kmediaplayer.h>

/*!
    The KMediaPlayer class provides an embedable widget that can be used to playback from various
    media sources including Hard-Drives (local and remote), Internet streams, CD, DVD, Blue-Ray,
    SMB, file-descriptor, raw data, you name it. Unlike KMediaPlayer it provides interactive media
    controls to play/pause, seek to position and set volume. It is ment to be a simple player but
    supports drag-n-drop actions, fullscreen support, extended media controls and can automatically
    hide the media controls optionally.

    For a simpler version of this class check out KMediaPlayer.

    @code
    KMediaWidget *player = new KMediaWidget(this);
    player->open("http://video.webmfiles.org/big-buck-bunny_trailer.webm");
    @endcode

    @note You should construct it with parent widget so that it can be layered on top of it.
    Otherwise when a video is played the widget will be floating. Ensuring that the widget has
    parent is a key to the fullscreen support as it will ask the parent to maximize itself when
    that needs to happen to ensure that the media controls are visible.
    @warning The API is not stable yet and it may break in the future!
    @see KMediaPlayer
    @todo keyboard shortcuts
*/

class Ui_KMediaWidgetPrivate;

class KMEDIAPLAYER_EXPORT KMediaWidget: public QWidget
{
    Q_OBJECT
public:
    enum KMediaOptions {
        //! @brief No options at all
        NoOptions = 0,
        /*!
            @long When URLs are dragged to the widget it will be opened
        */
        DragDrop = 1,
        /*!
            @long Provide fullscreen option, it is option because it will ask the parent to do it
        */
        FullscreenVideo = 2,
        /*!
            @long Currently only a menu button with some goodies
        */
        ExtendedControls = 4,
        /*!
            @long After a certain ammount of time the controls will hide themselfs allowing more
            screen space to be taken by the display widget
        */
        HiddenControls = 5,
        //! @brief All available options
        AllOptions = DragDrop | FullscreenVideo | ExtendedControls | HiddenControls,
        //! @brief Default options, currently none
        DefaultOptions = NoOptions
    };
    KMediaWidget(QWidget *parent, KMediaOptions options = DefaultOptions);
    ~KMediaWidget();

    /*!
        @return The player instance
    */
    KMediaPlayer* player();
    /*!
        @brief Open a path
        @long Aside from loading the path it will also setup the play/pause button state and enable
        the position seeking slider if the path supports seeking. It is recommended to use this
        method when a path must be loaded.
        @param path a path to load
        @see KMediaPlayer::load
    */
    void open(QString path);

    //! @brief Reimplementation to provide more accurate size hint
    virtual QSize sizeHint() const;

protected:
    //! @brief Reimplementation to support fullscreen
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    //! @brief Reimplementation to support hidden controls
    virtual void mouseMoveEvent(QMouseEvent *event);
    //! @brief Reimplementation to support hidden controls
    virtual void timerEvent(QTimerEvent *event);
    //! @brief Reimplementation to support Drag-n-Drop
    virtual void dragEnterEvent(QDragEnterEvent *event);
    //! @brief Reimplementation to support Drag-n-Drop
    virtual void dropEvent(QDropEvent *event);

public slots:
    /*!
        @brief Set the state to play (unpaused) or paused
        @param value A tristate value for the play state, if "-1" the state will be automatically
        decided for you. If "0" it will set the state to play (unpaused) and if "1" it will set it
        to pause (paused). Whenever called it updates the play/pause button state.
    */
    void setPlay(int value = -1);
    /*!
        @brief Set the position of the path currently loaded
        @param value A value in seconds
        @note The value is integer because the slider itself uses integer
        @see KMediaPlayer::currentTime, KMediaPlayer::remainingTime, KMediaPlayer::totalTime
    */
    void setPosition(int value);
    /*!
        @brief Set the volume of the path currently loaded
        @param value A value between 0-100 usually
        @note The value is integer because the dial itself uses integer
        @see KMediaPlayer::setVolume
    */
    void setVolume(int value);

private slots:
    void _showMenu();
    void _fullscreen();
    void _updateControls(bool visible);
    void _updatePlay(bool paused);
    void _updateSeekable(bool seekable);
    void _updatePosition(double seconds);
    void _updateLoaded();
    void _updateStatus(QString error);
    void _updateFinished();
    void _updateError(QString error);

    void _menuOpenURL();
    void _menuOpen();
    void _menuQuit();

private:
    KMediaPlayer *m_player;
    KMediaOptions m_options;
    QWidget *m_parent;
    QMainWindow *m_parenthack;
    QSize m_parentsizehack;
    QElapsedTimer m_timer;
    QString m_path;
    bool m_replay;
    QMenu *m_menu;
    Ui_KMediaWidgetPrivate *d;
};

#endif // KMEDIAWIDGET_H
