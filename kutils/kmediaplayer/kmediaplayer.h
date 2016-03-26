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

#ifndef KMEDIAPLAYER_H
#define KMEDIAPLAYER_H

#include <QWidget>
#include <QEvent>
#include <QSettings>
#include <kmimetype.h>
#include <kmediaplayer_export.h>

#ifdef MAKE_KMEDIAPLAYER_LIB
typedef struct mpv_handle mpv_handle;
#endif

/*!
    Base class for KAudioPlayer and KMediaPlayer
    @since 4.19
*/
class KMEDIAPLAYER_EXPORT KAbstractPlayer
{
public:
    KAbstractPlayer() { };
    virtual ~KAbstractPlayer() { };

    //@{
    /**
        Low-level methods that you should most likely not use, if you do and the API has to change
        (e.g. from MPV to a fork of MPV, who knows) then you will be on your own! It is available
        in case the convenience methods bellow those are not enough for your use case, but it is
        better to let us know your requirements instead of using them. They may serve you as
        temporary solution, for testing purposes, etc. but beware there be dragons!
    **/
    //! @brief A low-level player command sender
    virtual void command(const QVariant& params) const = 0;
    //! @brief A low-level player property setter
    virtual void setProperty(const QString& name, const QVariant& value) const = 0;
    //! @brief A low-level player property getter
    virtual QVariant property(const QString& name) const = 0;
    //! @brief A low-level player option setter
    virtual void setOption(const QString& name, const QVariant& value) const = 0;
    //@}

    /*!
        @brief Start playing from a path
        @long This a virtual method that should be reimplemented to delay initialization of the
        player to delay it until something is actually about to be played
        @param path a path to load, it can start with "file://", "dvd://", "http://" and other
        valid MPV protocols
        @warning Some protocols may not be supported if MPV itself was not build with support for
        such! That is choice of the vendors and you should be well aware of what yours is doing
        @link https://github.com/mpv-player/mpv/blob/master/DOCS/man/mpv.rst#protocols
    */
    virtual void load(QString path) = 0;
   /*!
        @brief Send a play command to the player, it may do nothing if a path was not loaded first
    */
    void play();
    /*!
        @brief Send a pause command to the player, it may do nothing if a path was not loaded first
    */
    void pause();
    /*!
        @brief Send a seek command to the player
        @param position Position in seconds to seek to
    */
    void seek(float position);
    /*!
        @brief Some GUI controls cannot handle float thus the overload
        @overload
    */
    void seek(int position);
    /*!
        @brief Send a stop command to the player
    */
    void stop();
    /*!
        @brief current or last set path to be played
    */
    QString path();
    /*!
        @brief current loaded path title
    */
    QString title();
    /*!
        @brief Gets you the current play time, the time should be threated as seconds
        @return current play time, it may return 0 if not playing
    */
    float currentTime();
    /*!
        @brief Gets you the remaining play time, the time should be threated as seconds
        @return remaining play time, it may return 0 if not playing
    */
    float remainingTime();
    /*!
        @brief Gets you the total play time, the time should be threated as seconds
        @return total play time, it may return 0 if not playing
    */
    float totalTime();
    /*!
        @return currently set volume level, usually from 0-100
        @see setVolume
    */
    float volume();
    /*!
        @return Whether the player is muted
    */
    bool mute();
    /*!
        @return A list of valid protocols
        @see isProtocolSupported
    */
    QStringList protocols();
    /*!
        @return Whether the current state is playing (not paused), does not mean it's not buffering
    */
    bool isPlaying();
    /*!
        @return Whether the current state is playing (not paused) and buffering at the same time
    */
    bool isBuffering();
    /*!
        @return Whether seek can be performed, maybe partitially
    */
    bool isSeekable();
    /*!
        @return Whether the current video is taking all screen space
    */
    bool isFullscreen();
    /*!
        @param mime MIME type in the format \<category\>/\<format\>, e.g. "audio/aac" (without
        quotes)
        @return Whether the MIME type is supported
    */
    virtual bool isMimeSupported(QString mime) const = 0;
    /*!
        @note You can obtain the scheme, which is the same as the meaning of protocol here, from a 
        KUrl/QUrl via url.scheme(). If you pass "http://" instead of just "http" the protocol will
        be considered valid too.
        @param protocol protocol type in the format \<protocol\>, e.g. "file" (without quotes)
        @return Whether the protocol is supported
        @see KUrl
        @todo The check is incomplete and there is no protocols property (yet?). it implements
        checks only for those listed in the MPV shipped .desktop file and should be improved
    */
    static bool isProtocolSupported(QString protocol);
    /*!
        @note This will check MIME and protocol type, possibly some other things too. The MIME will
        be obtained via KMimeType which may be slow
        @param path file, directory or URL string, e.g. "file://home/joe/vid.mp4" (without quotes)
        @return Whether the path is supported
        @see isMimeSupported, isProtocolSupported
    */
    bool isPathSupported(QString path) const;
    /*!
        @param volume desired volume level
        @warning It does not do boundry check so you should be aware of the maximum volume value if
        you are going to set it to something above 100. While MPV itself allows for a value greater
        than 100 in recent versions it is discoraged for you to set it above 100
    */
    void setVolume(float volume);
    /*!
        @brief Some GUI controls cannot handle float thus the overload
        @overload
    */
    void setVolume(int volume);
    /*!
        @param mute mute state
    */
    void setMute(bool mute);
    /*!
        @param fullscreen wheather it should take all screen space
        @warning This will most likely fail and the property will be set but MPV will do nothing
        because it is embeded, you will have to call QWidget::showFullscreen() on the parent wiget!
    */
    void setFullscreen(bool fullscreen);
};

/*!
    The KAudioPlayer class provides an object that can be used to playback from various media
    sources including Hard-Drives (local and remote), Internet streams, CD, DVD, Blue-Ray, SMB,
    file-descriptor, raw data, you name it. It supports per-application state too, this
    includes volume and mute state currently.

    For an extended version of this class check out KMediaPlayer and KMediaWidget.

    @warning The API is not stable yet and it may break in the future!
    @since 4.19
    @see KMediaPlayer, KMediaWidget
*/
class KMEDIAPLAYER_EXPORT KAudioPlayer: public QObject, public KAbstractPlayer
{
    Q_OBJECT
public:
    KAudioPlayer(QObject *parent = 0);
    ~KAudioPlayer();

    void command(const QVariant& params) const;
    void setProperty(const QString& name, const QVariant& value) const;
    QVariant property(const QString& name) const;
    void setOption(const QString& name, const QVariant& value) const;

    void load(QString path);
    bool isMimeSupported(QString mime) const;

signals:
    //! @brief Signals that a path was loaded
    void loaded();
    //! @brief Signals that the playing state was paused/unpaused when buffering data
    void buffering(bool buffering);
    /*!
        @brief Signals that the playing state was paused/unpaused
        @note You will still have to connect to the finished signal to update play/pause buttons
        (if any) when the path is done playing
    */
    void paused(bool paused);
    //! @brief Signals that the playing state can advance at position, maybe partitially
    void seekable(bool seekable);
    //! @brief Signals that the playing state was advanced at position in seconds
    void position(double seconds);
    /*!
        @brief Signals that the playing state was finished
        @warning It is not guaranteed that the playing was successfull, for an example if a stream
        was interrupted and the player cannot continue it may emit the signal
    */
    void finished();
    /*!
        @brief Signals that playback was finished with error
        @note You still have to connect to the finished signal
    */
    void error(QString error);

private slots:
    void _processHandleEvents();

private:
#ifdef MAKE_KMEDIAPLAYER_LIB
    mpv_handle *m_handle;
#endif // MAKE_KMEDIAPLAYER_LIB
    bool m_initialized;
    QSettings *m_settings;
};


/*!
    The KMediaPlayer class provides an embedable widget that can be used to playback from various
    media sources including Hard-Drives (local and remote), Internet streams, CD, DVD, Blue-Ray,
    SMB, file-descriptor, raw data, you name it. It supports per-application state too, this
    includes volume, mute and fullscreen state currently.

    For an extended version of this class check out KMediaWidget.

    @note Constructing it with parent widget will layer in top of it.
    @warning The API is not stable yet and it may break in the future!
    @since 4.19
    @see KMediaWidget
*/
class KMEDIAPLAYER_EXPORT KMediaPlayer: public QWidget, public KAbstractPlayer
{
    Q_OBJECT
public:
    KMediaPlayer(QWidget *parent = 0);
    ~KMediaPlayer();

    void command(const QVariant& params) const;
    void setProperty(const QString& name, const QVariant& value) const;
    QVariant property(const QString& name) const;
    void setOption(const QString& name, const QVariant& value) const;

    void load(QString path);
    bool isMimeSupported(QString mime) const;

signals:
    //! @brief Signals that a path was loaded
    void loaded();
    //! @brief Signals that the playing state was paused/unpaused when buffering data
    void buffering(bool buffering);
    /*!
        @brief Signals that the playing state was paused/unpaused
        @note You will still have to connect to the finished signal to update play/pause buttons
        (if any) when the path is done playing
    */
    void paused(bool paused);
    //! @brief Signals that the playing state can advance at position, maybe partitially
    void seekable(bool seekable);
    //! @brief Signals that the playing state was advanced at position in seconds
    void position(double seconds);
    /*!
        @brief Signals that the playing state was finished
        @warning It is not guaranteed that the playing was successfull, for an example if a stream
        was interrupted and the player cannot continue it may emit the signal
    */
    void finished();
    /*!
        @brief Signals that playback was finished with error
        @note You still have to connect to the finished signal
    */
    void error(QString error);

private slots:
    void _processHandleEvents();

private:
#ifdef MAKE_KMEDIAPLAYER_LIB
    mpv_handle *m_handle;
#endif // MAKE_KMEDIAPLAYER_LIB
    bool m_initialized;
    QSettings *m_settings;
};

#endif // KMEDIAPLAYER_H
