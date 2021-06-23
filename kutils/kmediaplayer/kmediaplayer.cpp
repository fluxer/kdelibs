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

#include "kdebug.h"
#include "klocale.h"
#include "ksettings.h"
#include "kmediaplayer.h"

#include <QApplication>

#if defined(HAVE_VLC)
#include <vlc/libvlc.h>
#include <vlc/libvlc_renderer_discoverer.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_media_player.h>
#include <vlc/libvlc_events.h>
#else
static bool s_fullscreen = false;
#endif // HAVE_VLC

/*
    Since sigals/slots cannot be virtual nor multiple QObject inheritance works (KAbstractPlayer
    cannot inherit from QObject if it is to be used in a class that inherits QWidget), these
    pre-processor definitions are used to share the code as much as possible making modification
    easier.
*/
#define COMMON_STATE_LOAD \
    d->m_settings->sync(); \
    const QString globalaudio = d->m_settings->value("global/audiooutput", "auto").toString(); \
    const int globalvolume = d->m_settings->value("global/volume", 90).toInt(); \
    const bool globalmute = d->m_settings->value("global/mute", false).toBool(); \
    setAudioOutput(d->m_settings->value(d->m_playerid + "/audiooutput", globalaudio).toString()); \
    setVolume(d->m_settings->value(d->m_playerid + "/volume", globalvolume).toInt()); \
    setMute(d->m_settings->value(d->m_playerid + "/mute", globalmute).toBool());

#define COMMON_STATE_SAVE \
    if (d->vlcMediaPlayer && d->m_settings && d->m_settings->isWritable()) { \
        d->m_settings->setValue(d->m_playerid + "/audiooutput", audiooutput()); \
        d->m_settings->setValue(d->m_playerid + "/volume", int(volume())); \
        d->m_settings->setValue(d->m_playerid + "/mute", mute()); \
        d->m_settings->sync(); \
    } else { \
        kWarning() << i18n("Could not save state"); \
    }


// TODO: path is always empty
// TODO: time-remaining, protocol-list, audio-device-list(QStringList) -> libvlc_audio_output_list_get()
// TODO: libvlc_media_get_duration() for duration?
#define COMMON_OPTION_GETTER \
    kDebug() << i18n("getting option") << name; \
    if (!d->vlcMediaPlayer) { \
        return QVariant(); \
    } else if (name == QLatin1String("volume")) { \
        return QVariant(libvlc_audio_get_volume(d->vlcMediaPlayer)); \
    } else if (name == QLatin1String("duration")) { \
        return QVariant(qint64(libvlc_media_player_get_length(d->vlcMediaPlayer))); \
    } else if (name == QLatin1String("time-pos")) { \
        return QVariant(qint64(libvlc_media_player_get_time(d->vlcMediaPlayer))); \
    } else if (name == QLatin1String("media-title")) { \
        return QVariant(libvlc_media_player_get_title(d->vlcMediaPlayer)); \
    } else if (name == QLatin1String("pause")) { \
        return QVariant(bool(!libvlc_media_player_is_playing(d->vlcMediaPlayer))); \
    } else if (name == QLatin1String("seekable")) { \
        return QVariant(bool(libvlc_media_player_is_seekable(d->vlcMediaPlayer))); \
    } else if (name == QLatin1String("fullscreen")) { \
        return QVariant(bool(libvlc_get_fullscreen(d->vlcMediaPlayer))); \
    } else if (name == QLatin1String("audio-device")) { \
        return QVariant(libvlc_audio_output_device_get(d->vlcMediaPlayer)); \
    } else if (name == QLatin1String("mute")) { \
        return QVariant(bool(libvlc_audio_get_mute(d->vlcMediaPlayer))); \
    } else if (name == QLatin1String("buffering")) { \
        libvlc_media_t* media = libvlc_media_player_get_media(d->vlcMediaPlayer); \
        if (!media) { \
            return QVariant(); \
        } \
        return QVariant(bool(libvlc_media_get_state(media) == libvlc_Buffering)); \
    } else if (name == QLatin1String("path")) { \
        libvlc_media_t* media = libvlc_media_player_get_media(d->vlcMediaPlayer); \
        if (!media) { \
            return QVariant(); \
        } \
        QByteArray url = libvlc_media_get_meta(media, libvlc_meta_URL); \
        if (url.isEmpty() && libvlc_media_player_is_playing(d->vlcMediaPlayer)) { \
            qDebug() << "falling back"; \
            url = "Unknown"; \
        } \
        return QVariant(url); \
    } else { \
        kWarning() << "unimplemented option" << name; \
    }

#define COMMON_OPTION_SETTER \
    kDebug() << i18n("setting option") << name << value; \
    if (!d->vlcMediaPlayer) { \
        return; \
    } else if (name == QLatin1String("loadfile")) { \
        QByteArray utf8path = value.toByteArray(); \
        if (utf8path.startsWith("/")) { \
            utf8path.prepend("file://"); \
        } \
        qDebug() << "loadfile" << utf8path; \
        libvlc_media_t *vlcMedia = libvlc_media_new_location(d->vlcInstance, utf8path.constData()); \
        if (!vlcMedia) { \
            libvlc_media_player_stop(d->vlcMediaPlayer); \
            kWarning() << "Cannot create media" << utf8path; \
            return; \
        } \
        libvlc_media_player_set_media(d->vlcMediaPlayer, vlcMedia); \
        libvlc_media_release(vlcMedia); \
        if (libvlc_media_player_play(d->vlcMediaPlayer) != 0) { \
            kWarning() << "Cannot play media" << utf8path; \
        } \
    } else if (name == QLatin1String("pause")) { \
        const bool pause = value.toBool(); \
        qDebug() << "pausing" << pause; \
        libvlc_media_player_set_pause(d->vlcMediaPlayer, pause); \
    } else if (name == QLatin1String("seek")) { \
        const float seek = value.toFloat(); \
        qDebug() << "seeking" << seek; \
        libvlc_media_player_set_time(d->vlcMediaPlayer, seek); \
    } else if (name == QLatin1String("volume")) { \
        const float volume = value.toFloat(); \
        qDebug() << "volume" << volume; \
        libvlc_audio_set_volume(d->vlcMediaPlayer, volume); \
    } else if (name == QLatin1String("fullscreen")) { \
        const bool fullscreen = value.toBool(); \
        qDebug() << "fullscreen" << fullscreen; \
        libvlc_set_fullscreen(d->vlcMediaPlayer, fullscreen); \
    } else if (name == QLatin1String("stop")) { \
        qDebug() << "stop"; \
        libvlc_media_player_stop(d->vlcMediaPlayer); \
    }

// TODO: not working properly?
#if 0
    } else if (name == QLatin1String("mute")) {
        const bool mute = value.toBool();
        qDebug() << "mute" << mute;
        libvlc_audio_set_mute(d->vlcMediaPlayer, mute);
    } else if (name == QLatin1String("audio-device")) {
        QByteArray utf8output = value.toByteArray();
        qDebug() << "audio-device" << utf8output;
        libvlc_audio_output_set(d->vlcMediaPlayer, utf8output.constData());
#endif

#define COMMON_EVENT_HANDLER \
    switch (event->type) { \
        case libvlc_MediaPlayerOpening: { \
            qDebug() << "loaded"; \
            emit loaded(); \
            break; \
        } \
        case libvlc_MediaPlayerEncounteredError: { \
            kWarning() << "Media player encountered error"; \
            emit error(QString(libvlc_errmsg())); \
            break; \
        } \
        case libvlc_MediaPlayerEndReached: { \
            qDebug() << "finished"; \
            emit finished(); \
            break; \
        } \
        case libvlc_MediaPlayerBuffering: { \
            qDebug() << "buffering" << event->u.media_player_buffering.new_cache; \
            emit buffering(event->u.media_player_buffering.new_cache != 100.0); \
            break; \
        } \
        case libvlc_MediaPlayerSeekableChanged: { \
            qDebug() << "seekable" << event->u.media_player_seekable_changed.new_seekable; \
            emit seekable(event->u.media_player_seekable_changed.new_seekable); \
            break; \
        } \
        case libvlc_MediaPlayerPaused: { \
            qDebug() << "paused" << event->u.media_state_changed.new_state; \
            int state = event->u.media_state_changed.new_state; \
            if (state == libvlc_Playing) { \
                emit paused(false); \
            } else if (state == libvlc_Paused) { \
                emit paused(true); \
            } \
            break; \
        } \
        case libvlc_MediaPlayerTimeChanged: { \
            kDebug() << "time changed" << event->u.media_player_length_changed.new_length; \
            emit position(event->u.media_player_length_changed.new_length); \
            break; \
        } \
        default: { \
            kWarning() << "unknown event" << event->type; \
            break; \
        } \
    }

// the video decoder may run into its own thread, make sure that does not cause trouble
#if defined(HAVE_VLC) && defined(Q_WS_X11)
static int kmp_x11_init_threads() {
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
    return 1;
};
Q_CONSTRUCTOR_FUNCTION(kmp_x11_init_threads)
#endif


class KAbstractPlayerPrivate
{
public:
    KAbstractPlayerPrivate();
    ~KAbstractPlayerPrivate();

#if defined(HAVE_VLC)
    libvlc_instance_t *vlcInstance;
    libvlc_media_player_t *vlcMediaPlayer;
#endif
    QString m_playerid;
    KSettings *m_settings;
};

KAbstractPlayerPrivate::KAbstractPlayerPrivate()
    : vlcInstance(Q_NULLPTR),
    vlcMediaPlayer(Q_NULLPTR),
    m_playerid(QApplication::applicationName()),
    m_settings(new KSettings("kmediaplayer", KSettings::FullConfig))
{
    kDebug() << i18n("initializing player");
#if defined(HAVE_VLC)
    static const char *arguments[] = { "--no-video-title-show" };
    vlcInstance = libvlc_new(sizeof(arguments) / sizeof(arguments[0]), arguments);
    if (!vlcInstance) {
        kWarning() << i18n("instance creation failed") << libvlc_errmsg();
        return;
    }
    vlcMediaPlayer = libvlc_media_player_new(vlcInstance);
    if (!vlcMediaPlayer) {
        kWarning() << i18n("media player creation failed") << libvlc_errmsg();
        return;
    }
#endif
}

KAbstractPlayerPrivate::~KAbstractPlayerPrivate()
{
    kDebug() << i18n("destroying player");
#if defined(HAVE_VLC)
    if (vlcMediaPlayer) {
        libvlc_media_player_release(vlcMediaPlayer);
    }

    if (vlcInstance) {
        libvlc_release(vlcInstance);
    }
#endif
    if (m_settings) {
        m_settings->deleteLater();
    }
}

void KAbstractPlayer::load(const QString &path)
{
    setOption("loadfile", path);
}

void KAbstractPlayer::load(const QByteArray &data)
{
    // SECURITY: this is dangerous but some applications/libraries (like KHTML) require it
    setOption("loadfile", QString("memory://%1").arg(data.data()));
}

void KAbstractPlayer::play()
{
    setOption("pause", false);
}

void KAbstractPlayer::pause()
{
    setOption("pause", true);
}

void KAbstractPlayer::seek(const float position)
{
    setOption("seek", position);
}

void KAbstractPlayer::seek(const int position)
{
    setOption("seek", position);
}

void KAbstractPlayer::stop()
{
    setOption("stop", QVariant());
}

QString KAbstractPlayer::path() const
{
    return option("path").toString();
}

QString KAbstractPlayer::title() const
{
    return option("media-title").toString();
}

float KAbstractPlayer::currentTime() const
{
    return option("time-pos").toFloat();
}

float KAbstractPlayer::remainingTime() const
{
    return option("time-remaining").toFloat();
}

float KAbstractPlayer::totalTime() const
{
    return option("duration").toFloat();
}

float KAbstractPlayer::volume() const
{
    return option("volume").toFloat();
}

bool KAbstractPlayer::mute() const
{
    return option("mute").toBool();
}

QStringList KAbstractPlayer::protocols() const
{
    static QStringList s_protocols;
    if (s_protocols.isEmpty()) {
        s_protocols = option("protocol-list").toStringList();
        s_protocols.removeDuplicates();
        qSort(s_protocols);
    }
    return s_protocols;
}

QString KAbstractPlayer::audiooutput() const
{
    return option("audio-device").toString();
}

QStringList KAbstractPlayer::audiooutputs() const
{
    const QVariantList value = option("audio-device-list").toList();
    QStringList stringlist;
    foreach (const QVariant &variant, value) {
        QMapIterator<QString,QVariant> iter(variant.toMap());
        while (iter.hasNext()) {
            iter.next();
            if (iter.key() == "name") {
                stringlist.append(iter.value().toString());
            }
        }
    }
    return stringlist;
}

bool KAbstractPlayer::isPlaying() const
{
    return !option("pause").toBool() && !option("path").isNull();
}

bool KAbstractPlayer::isBuffering() const
{
    return option("bufferring").toBool();
}

bool KAbstractPlayer::isSeekable() const
{
    return option("seekable").toBool();
}

bool KAbstractPlayer::isFullscreen() const
{
#if defined(HAVE_VLC)
    return option("fullscreen").toBool();
#else
    return s_fullscreen;
#endif // HAVE_VLC
}

bool KAbstractPlayer::isProtocolSupported(const QString &protocol) const
{
    foreach(const QString &proto, protocols()) {
        if (protocol.startsWith(proto)) {
            return true;
        }
    }
    return false;
}

bool KAbstractPlayer::isPathSupported(const QString &path) const
{
    const KMimeType::Ptr mime = KMimeType::findByPath(path);
    if (mime && isMimeSupported(mime->name())) {
        return true;
    }
    return isProtocolSupported(path);
}

void KAbstractPlayer::setVolume(const float volume)
{
    setOption("volume", volume);
}

void KAbstractPlayer::setVolume(const int volume)
{
    setOption("volume", volume);
}

void KAbstractPlayer::setMute(const bool mute)
{
    setOption("mute", mute);
}

void KAbstractPlayer::setAudioOutput(const QString &output)
{
    setOption("audio-device", output);
}

void KAbstractPlayer::setFullscreen(const bool fullscreen)
{
#if defined(HAVE_VLC)
    setOption("fullscreen", fullscreen);
#else
    s_fullscreen = fullscreen;
#endif // HAVE_VLC
}

static void wakeup_audio(const libvlc_event_t *event, void *instance)
{
    KAudioPlayer *pctx = static_cast<KAudioPlayer*>(instance);
    pctx->_processHandleEvents(event);
}

KAudioPlayer::KAudioPlayer(QObject *parent)
    : QObject(parent), d(new KAbstractPlayerPrivate())
{
#if defined(HAVE_VLC)
    if (d->vlcMediaPlayer) {
        libvlc_event_manager_t *eventManager = libvlc_media_player_event_manager(d->vlcMediaPlayer);
        libvlc_event_e eventTypes[] = {
            libvlc_MediaPlayerOpening,
            libvlc_MediaPlayerEncounteredError,
            libvlc_MediaPlayerEndReached,
            libvlc_MediaPlayerBuffering,
            libvlc_MediaPlayerSeekableChanged,
            libvlc_MediaPlayerPaused,
            libvlc_MediaPlayerTimeChanged
        };

        for (uint i = 0; i < (sizeof(eventTypes) / sizeof(eventTypes[0])); ++i) {
            if (libvlc_event_attach(eventManager, eventTypes[i], wakeup_audio, this) != 0) {
                kWarning() << "Cannot attach event handler" << eventTypes[i];
                break;
            }
        }

        // TODO: disable video

        COMMON_STATE_LOAD
    }
#else
    kWarning() << i18n("KAudioPlayer is a stub");
#endif
}

KAudioPlayer::~KAudioPlayer()
{
#if defined(HAVE_VLC)
    COMMON_STATE_SAVE
#endif

    delete d;
}

QVariant KAudioPlayer::option(const QString &name) const
{
#if defined(HAVE_VLC)
    COMMON_OPTION_GETTER
#else
    Q_UNUSED(name);
#endif
    return QVariant();
}

void KAudioPlayer::setOption(const QString &name, const QVariant &value) const
{
#if defined(HAVE_VLC)
    COMMON_OPTION_SETTER
#else
    Q_UNUSED(name);
    Q_UNUSED(value);
#endif
}

void KAudioPlayer::_processHandleEvents(const libvlc_event_t *event)
{
#if defined(HAVE_VLC)
    COMMON_EVENT_HANDLER
#endif
}

void KAudioPlayer::setPlayerID(const QString &id)
{
    d->m_playerid = id;
#if defined(HAVE_VLC)
    COMMON_STATE_LOAD
#endif
}

bool KAudioPlayer::isMimeSupported(const QString &mime) const
{
#if defined(HAVE_VLC)
    return mime.startsWith("audio/") || mime == QLatin1String("application/octet-stream");
#else
    Q_UNUSED(mime);
    return false;
#endif
}

/////
static void wakeup_media(const libvlc_event_t *event, void *instance)
{
    KMediaPlayer *pctx = static_cast<KMediaPlayer*>(instance);
    pctx->_processHandleEvents(event);
}

KMediaPlayer::KMediaPlayer(QWidget *parent)
    : QWidget(parent), d(new KAbstractPlayerPrivate)
{
#if defined(HAVE_VLC)
    if (d->vlcMediaPlayer) {
        libvlc_event_manager_t *eventManager = libvlc_media_player_event_manager(d->vlcMediaPlayer);
        libvlc_event_e eventTypes[] = {
            libvlc_MediaPlayerOpening,
            libvlc_MediaPlayerEncounteredError,
            libvlc_MediaPlayerEndReached,
            libvlc_MediaPlayerBuffering,
            libvlc_MediaPlayerSeekableChanged,
            libvlc_MediaPlayerPaused,
            libvlc_MediaPlayerTimeChanged
        };

        for (uint i = 0; i < (sizeof(eventTypes) / sizeof(eventTypes[0])); ++i) {
            if (libvlc_event_attach(eventManager, eventTypes[i], wakeup_media, this) != 0) {
                kWarning() << "Cannot attach event handler" << eventTypes[i];
                break;
            }
        }

        WId wid;
        if (parent) {
            wid = parent->winId();
        } else {
            wid = winId();
        }
        libvlc_media_player_set_xwindow(d->vlcMediaPlayer, wid);

        COMMON_STATE_LOAD
    }
#else
    kWarning() << i18n("KMediaPlayer is a stub");
#endif
}

KMediaPlayer::~KMediaPlayer()
{
#if defined(HAVE_VLC)
    COMMON_STATE_SAVE
#endif

    delete d;
}

QVariant KMediaPlayer::option(const QString &name) const
{
#if defined(HAVE_VLC)
    COMMON_OPTION_GETTER
#else
    Q_UNUSED(name);
#endif
    return QVariant();
}

void KMediaPlayer::setOption(const QString &name, const QVariant &value) const
{
#if defined(HAVE_VLC)
    COMMON_OPTION_SETTER
#else
    Q_UNUSED(name);
    Q_UNUSED(value);
#endif
}

void KMediaPlayer::_processHandleEvents(const libvlc_event_t *event)
{
#if defined(HAVE_VLC)
    COMMON_EVENT_HANDLER
#endif
}

void KMediaPlayer::setPlayerID(const QString &id)
{
    d->m_playerid = id;
#if defined(HAVE_VLC)
    COMMON_STATE_LOAD
#endif
}

bool KMediaPlayer::isMimeSupported(const QString &mime) const
{
#if defined(HAVE_VLC)
    return mime.startsWith("audio/") || mime.startsWith("video/")
        || mime == QLatin1String("application/octet-stream");
#else
    Q_UNUSED(mime);
    return false;
#endif
}

#include "moc_kmediaplayer.cpp"
