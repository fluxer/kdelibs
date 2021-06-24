/*  This file is part of the KDE libraries
    Copyright (C) 2016-2019 Ivailo Monev <xakepa10@gmail.com>

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

#include <kmimetype.h>
#include <kmediaplayer_export.h>

class KAbstractPlayerPrivate;

/*!
    Base class for @p KAudioPlayer and @p KMediaPlayer
    @since 4.19
    @todo implement path position saving/restoration?
*/
class KMEDIAPLAYER_EXPORT KAbstractPlayer
{
public:
    KAbstractPlayer() { };
    virtual ~KAbstractPlayer() { };

    //@{
    /**
        Low-level methods that you should most likely not use, if you do and the API has to change
        (e.g. for a backend switch) then you will be on your own! They are available in case the
        convenience methods bellow those are not enough for your use case, but it is better to let
        us know your requirements instead of using them. They may serve you as temporary solution,
        for testing purposes, etc. but beware there be dragons!

        @note Because QObject and QWidget have property system the methods are named option
        but in fact they set and get properties of the underlaying player, not options. There is
        a difference between them in MPV so make sure you are using the methods for properties if
        you rely on them.
    **/
    //! @brief A low-level player command sender
    virtual void command(const QVariant &params) const = 0;
    //! @brief A low-level player property getter
    virtual QVariant option(const QString &name) const = 0;
    //! @brief A low-level player property setter
    virtual void setOption(const QString &name, const QVariant &value) const = 0;
    //@}

    /*!
        @brief Sets the player ID to @p id and reloads the saved state
        @note The ID, which is @p QApplication::applicationName() by default, must be one of the
        strings in the "X-KDE-MediaPlayer" entry of the .desktop file
        @param id player ID that identifies feature
    */
    virtual void setPlayerID(const QString &id) = 0;
    /*!
        @brief Start playing from @p path
        @param path path to load, it can start with "file://", "dvd://", "http://" and other
        valid MPV protocols
        @warning Some protocols may not be supported if MPV itself was not build with support for
        such! That is choice of the vendors and you should be well aware of what yours is doing
        @link https://github.com/mpv-player/mpv/blob/master/DOCS/man/mpv.rst#protocols
    */
    void load(const QString &path);
    /*!
        @brief Start playing from @p data
        @param data raw data to play
        @warning Use only when you absolutely have to, when possible use @p load(QString)
        @overload
    */
    void load(const QByteArray &data);
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
        @param position position in seconds to seek to
        @see isSeekable
    */
    void seek(const float position);
    /*!
        @brief Some GUI controls cannot handle float thus the overload
        @overload
    */
    void seek(const int position);
    /*!
        @brief Send a stop command to the player
    */
    void stop();
    /*!
        @return Current loaded path, empty if nothing is loaded
    */
    QString path() const;
    /*!
        @return Current loaded path title
    */
    QString title() const;
    /*!
        @return Current play time in seconds, it may return 0 if not playing
    */
    float currentTime() const;
    /*!
        @return Remaining play time in seconds, it may return 0 if not playing
    */
    float remainingTime() const;
    /*!
        @return Total play time in seconds, it may return 0 if not playing
    */
    float totalTime() const;
    /*!
        @return Current volume level, usually in range of 0-100
        @see setVolume
    */
    float volume() const;
    /*!
        @return Whether the player is muted
        @see setMute
    */
    bool mute() const;
    /*!
        @return A list of valid protocols
        @see isProtocolSupported
    */
    QStringList protocols() const;
    /*!
        @brief Gets you the device used to output audio
        @return Audio output
    */
    QString audiooutput() const;
    /*!
        @return A list of available audio outputs
    */
    QStringList audiooutputs() const;
    /*!
        @return Whether the current state is playing (not paused), does not mean it's not buffering
    */
    bool isPlaying() const;
    /*!
        @return Whether the current state is playing (not paused) and buffering at the same time
    */
    bool isBuffering() const;
    /*!
        @return Whether seek can be performed, maybe partitially
    */
    bool isSeekable() const;
    /*!
        @return Whether the current video is taking all screen space
    */
    bool isFullscreen() const;
    /*!
        @param mime MIME type in the format \<category\>/\<format\>, e.g. "audio/aac" (without
        quotes)
        @return Whether the MIME type is supported
        @see isProtocolSupported, isPathSupported
    */
    virtual bool isMimeSupported(const QString &mime) const = 0;
    /*!
        @note You can obtain the scheme, which is the same as the meaning of protocol here, from a 
        KUrl / QUrl via url.scheme(). If you pass "http://" instead of just "http" the protocol will
        be considered valid too.
        @param protocol protocol type in the format \<protocol\>, e.g. "file" (without quotes)
        @return Whether the protocol is supported
        @see KUrl, isMimeSupported, isPathSupported
    */
    bool isProtocolSupported(const QString &protocol) const;
    /*!
        @note This will check MIME and protocol type, possibly some other things too. The MIME will
        be obtained via KMimeType which may be slow
        @param path file, directory or URL string, e.g. "file://home/joe/vid.mp4" (without quotes)
        @return Whether the path is supported
        @see isMimeSupported, isProtocolSupported
    */
    bool isPathSupported(const QString &path) const;
    /*!
        @param volume desired volume level
        @warning It does not do boundry check so you should be aware of the maximum volume value if
        you are going to set it to something above 100. While MPV itself allows for a value greater
        than 100 in recent versions it is discoraged for you to set it above 100
    */
    void setVolume(const float volume);
    /*!
        @brief Some GUI controls cannot handle float thus the overload
        @overload
    */
    void setVolume(const int volume);
    /*!
        @param mute mute state
    */
    void setMute(const bool mute);
    /*!
        @note If the output is not valid the player will fallback to automatic detection, you can
        obtain a list of valid outputs via @p audiooutputs()
        @param output audio output
        @see audiooutputs
    */
    void setAudioOutput(const QString &output);
    /*!
        @param fullscreen wheather it should take all screen space
        @warning This will most likely fail and the property will be set but MPV will do nothing
        because it is embeded, you will have to call @p QWidget::showFullscreen() on the parent
        widget!
    */
    void setFullscreen(const bool fullscreen);
};

/*!
    The @p KAudioPlayer class provides an object that can be used to playback from various media
    sources including Hard-Drives, Internet streams, CD, DVD, Blue-Ray, file-descriptor, raw data,
    you name it.

    It supports per-application state too, this includes audio output device, volume and mute state
    currently. That feature requires a special entry in the application .desktop file -
    "X-KDE-MediaPlayer=<playerid>[,<playerid>,...]" - which indicates that it uses the class and
    makes it appear in the K Control Module (KCM) for multimedia. If the player is not used in
    application but in @p KPart or plugin which may create multiple instances for different
    purposes you may want to set its ID via @p setPlayerID().

    For an extended version of this class check out @p KMediaPlayer and @p KMediaWidget.

    @since 4.19
    @see KMediaPlayer, KMediaWidget
*/
class KMEDIAPLAYER_EXPORT KAudioPlayer: public QObject, public KAbstractPlayer
{
    Q_OBJECT
public:
    KAudioPlayer(QObject *parent = 0);
    ~KAudioPlayer();

    void command(const QVariant &command) const;
    QVariant option(const QString &name) const;
    void setOption(const QString &name, const QVariant& value) const;

    void setPlayerID(const QString &id);
    bool isMimeSupported(const QString &mime) const;

Q_SIGNALS:
    //! @brief Signals that a path was loaded
    void loaded();
    //! @brief Signals that the playing state was paused/unpaused when buffering data
    void buffering(const bool buffering);
    //! @brief Signals that the playing state was paused/unpaused
    void paused(const bool paused);
    //! @brief Signals that the playing state can advance at position, maybe partitially
    void seekable(const bool seekable);
    //! @brief Signals that the playing state was advanced at position in seconds
    void position(const double seconds);
    /*!
        @brief Signals that the playing state was finished
        @note It is not guaranteed that the playing was successfull, for an example if a stream
        was interrupted and the player cannot continue it may emit the signal
    */
    void finished();
    /*!
        @brief Signals that playback was finished with error
        @note You still have to connect to the finished signal
    */
    void error(const QString error);

private Q_SLOTS:
    void _processHandleEvents();

private:
    KAbstractPlayerPrivate *d;
};


/*!
    The @p KMediaPlayer class provides an object that can be used to playback from various media
    sources including Hard-Drives, Internet streams, CD, DVD, Blue-Ray, file-descriptor, raw data,
    you name it.

    It supports per-application state too, this includes audio output device, volume and mute state
    currently. That feature requires a special entry in the application .desktop file -
    "X-KDE-MediaPlayer=<playerid>[,<playerid>,...]" - which indicates that it uses the class and
    makes it appear in the K Control Module (KCM) for multimedia. If the player is not used in
    application but in @p KPart or plugin which may create multiple instances for different
    purposes you may want to set its ID via @p setPlayerID().

    For an extended version of this class check out @p KMediaWidget.

    @note You should construct it with parent widget, preferably a QMainWindow, so that it can be
    layered on top of it. Otherwise when a video is played the widget will be floating.
    @since 4.19
    @see KMediaWidget
*/
class KMEDIAPLAYER_EXPORT KMediaPlayer: public QWidget, public KAbstractPlayer
{
    Q_OBJECT
public:
    KMediaPlayer(QWidget *parent = 0);
    ~KMediaPlayer();

    void command(const QVariant &command) const;
    QVariant option(const QString &name) const;
    void setOption(const QString &name, const QVariant &value) const;

    void setPlayerID(const QString &id);
    bool isMimeSupported(const QString &mime) const;

Q_SIGNALS:
    //! @brief Signals that a path was loaded
    void loaded();
    //! @brief Signals that the playing state was paused/unpaused when buffering data
    void buffering(const bool buffering);
    //! @brief Signals that the playing state was paused/unpaused
    void paused(const bool paused);
    //! @brief Signals that the playing state can advance at position, maybe partitially
    void seekable(const bool seekable);
    //! @brief Signals that the playing state was advanced at position in seconds
    void position(const double seconds);
    /*!
        @brief Signals that the playing state was finished
        @note It is not guaranteed that the playing was successfull, for an example if a stream
        was interrupted and the player cannot continue it may emit the signal
    */
    void finished();
    /*!
        @brief Signals that playback was finished with error
        @note You still have to connect to the finished signal
    */
    void error(const QString error);

private Q_SLOTS:
    void _processHandleEvents();

private:
    KAbstractPlayerPrivate *d;
};

#endif // KMEDIAPLAYER_H
