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

#ifndef KAUDIOWIDGET_H
#define KAUDIOWIDGET_H

#include <QWidget>
#include <kmediaplayer_export.h>
#include <kmediaplayer.h>

/*!
    The @p KAudioWidget class provides an embedable widget that can be used to playback from
    various media sources including Hard-Drives (local and remote), Internet streams, CD, DVD,
    Blue-Ray, file-descriptor, raw data, you name it. Unlike @p KAudioPlayer it provides
    interactive media controls to play/pause, seek to position and set volume. It is ment to be a
    simple player but supports drag-n-drop optionally.

    For a simpler version of this class check out @p KMediaPlayer.

    @code
    KAudioWidget *player = new KAudioWidget(this);
    player->open("http://video.webmfiles.org/big-buck-bunny_trailer.webm");
    @endcode

    @warning The API is not stable yet and it may break in the future!
    @see KMediaPlayer
    @todo keyboard shortcuts
*/

class Ui_KAudioWidgetPrivate;

class KMEDIAPLAYER_EXPORT KAudioWidget: public QWidget
{
    Q_OBJECT
public:
    enum KAudioOption {
        //! @brief No options at all
        NoOptions = 0,
        //! @brief When URL is dragged to the widget it will be opened
        DragDrop = 1,
        //! @brief All available options
        AllOptions = DragDrop,
        //! @brief Default options
        DefaultOptions = NoOptions
    };
    Q_DECLARE_FLAGS(KAudioOptions, KAudioOption);

    KAudioWidget(QWidget *parent, KAudioOptions options = DefaultOptions);
    ~KAudioWidget();

    /*!
        @return The player instance
    */
    KAudioPlayer* player();
    /*!
        @brief Open a path
        @long Aside from loading @p path it will also setup the play/pause button state and enable
        the position seeking slider if the path supports seeking. It is recommended to use this
        method when a path must be loaded.
        @param path a path to load
        @see KAudioPlayer::load
    */
    void open(QString path);

    //! @brief Reimplementation to provide more accurate size hint
    virtual QSize sizeHint() const;
        //! @brief Reimplementation to provide reasonable minimum size
    virtual QSize minimumSizeHint() const;

protected:
    //! @brief Reimplementation to support play time indicator
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
        @see KAudioPlayer::play, KAudioPlayer::pause
    */
    void setPlay(int value = -1);
    /*!
        @brief Set the position of the path currently loaded
        @param value A value in seconds
        @note The value is integer because the slider itself uses integer
        @see KAudioPlayer::currentTime, KAudioPlayer::remainingTime, KAudioPlayer::totalTime
    */
    void setPosition(int value);
    /*!
        @brief Set the volume of the path currently loaded
        @param value A value between 0-100 usually
        @note The value is integer because the dial itself uses integer
        @see KAudioPlayer::setVolume
    */
    void setVolume(int value);
    /*!
        @brief Set the playlist
        @param value A list of paths
        @see KAudioPlayer::load
    */
    void setPlaylist(QStringList value);

private slots:
    void _updatePlay(bool paused);
    void _updateSeekable(bool seekable);
    void _updatePosition(double seconds);
    void _updateLoaded();
    void _updateStatus(QString string);
    void _updateFinished();
    void _updateError(QString error);

private:
    KAudioPlayer *m_player;
    KAudioOptions m_options;
    QWidget *m_parent;
    QString m_path;
    bool m_replay;
    Ui_KAudioWidgetPrivate *d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KAudioWidget::KAudioOptions);

#endif // KAUDIOWIDGET_H
 
