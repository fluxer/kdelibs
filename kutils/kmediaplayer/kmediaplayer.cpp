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
#include "kmediaplayer.h"
#include <QApplication>

#if defined(HAVE_MPV)
#include <mpv/client.h>
#include <mpv/qthelper.hpp>
#else
static bool s_fullscreen = false;
#endif // HAVE_MPV

/*
    Since sigals/slots cannot be virtual nor multiple QObject inheritance works (KAbstractPlayer
    cannot inherit from QObject if it is to be used in a class that inherits QWidget), these
    pre-processor definitions are used to share the code as much as possible making modification
    easier.
*/
#define COMMON_STATE_SAVE \
    if (d->m_handle && d->m_settings && d->m_settings->isWritable()) { \
        d->m_settings->beginGroup(d->m_appname); \
        d->m_settings->setValue("audiooutput", audiooutput()); \
        d->m_settings->setValue("volume", int(volume())); \
        d->m_settings->setValue("mute", mute()); \
        d->m_settings->endGroup(); \
        d->m_settings->sync(); \
    } else { \
        kWarning() << i18n("Could not save state"); \
    }

#define COMMMON_COMMAND_SENDER \
    kDebug() << i18n("sending command") << command; \
    if (d->m_handle) { \
        const QVariant result = mpv::qt::command(d->m_handle, command); \
        if (mpv::qt::is_error(result)) { \
            kWarning() << mpv_error_string(mpv::qt::get_error(result)); \
        } \
    }

#define COMMON_OPTION_GETTER \
    kDebug() << i18n("getting option") << name; \
    if (d->m_handle) { \
        const QVariant result = mpv::qt::get_property(d->m_handle, name); \
        if (mpv::qt::is_error(result)) { \
            kWarning() << mpv_error_string(mpv::qt::get_error(result)); \
            return QVariant(); \
        } \
        return result; \
    }

#define COMMON_OPTION_SETTER \
    kDebug() << i18n("setting option") << name << value; \
    if (d->m_handle) { \
        const QVariant result = mpv::qt::set_property(d->m_handle, name, value); \
        if (mpv::qt::is_error(result)) { \
            kWarning() << mpv_error_string(mpv::qt::get_error(result)); \
        } \
    }

#define COMMMON_EVENT_HANDLER \
    while (!d->m_stopprocessing) { \
        mpv_event *event = mpv_wait_event(d->m_handle, 0); \
        if (event->event_id == MPV_EVENT_NONE) { \
            break; \
        } \
        switch (event->event_id) { \
            case MPV_EVENT_FILE_LOADED: { \
                kDebug() << i18n("playback loaded"); \
                emit loaded(); \
                break; \
            } \
            case MPV_EVENT_PAUSE: { \
                kDebug() << i18n("playback paused"); \
                emit paused(true); \
                break; \
            } \
            case MPV_EVENT_UNPAUSE: { \
                kDebug() << i18n("playback unpaused"); \
                emit paused(false); \
                break; \
            } \
            case MPV_EVENT_END_FILE: { \
                mpv_event_end_file *prop = static_cast<mpv_event_end_file *>(event->data); \
                if (prop->reason == MPV_END_FILE_REASON_ERROR) { \
                    QString mpverror = QString::fromLatin1(mpv_error_string(prop->error)); \
                    kWarning() << i18n("playback finished with error") << mpverror; \
                    emit finished(); \
                    emit error(mpverror); \
                } else if (prop->reason == MPV_END_FILE_REASON_EOF \
                    || prop->reason == MPV_END_FILE_REASON_STOP \
                    || prop->reason == MPV_END_FILE_REASON_QUIT) { \
                    if (option("path").isNull()) { \
                        kDebug() << i18n("playback finished"); \
                        emit finished(); \
                    } \
                } \
                break; \
            } \
            case MPV_EVENT_PROPERTY_CHANGE: { \
                mpv_event_property *prop = static_cast<mpv_event_property *>(event->data); \
                kDebug() << i18n("property changed") << QString::fromLatin1(prop->name); \
                if (strcmp(prop->name, "time-pos") == 0) { \
                    double value = 0; \
                    if (prop->format == MPV_FORMAT_DOUBLE) { \
                        value = *(double *)prop->data; \
                    } else { \
                        Q_ASSERT_X(false, "KMediaPlayer", "the time-pos format has changed"); \
                    } \
                    emit position(value); \
                } else if (strcmp(prop->name, "seekable") == 0) { \
                    bool value = false; \
                    if (prop->format == MPV_FORMAT_FLAG) { \
                        value = *(bool *)prop->data; \
                    } else { \
                        Q_ASSERT_X(false, "KMediaPlayer", "the seekable format has changed"); \
                    } \
                    emit seekable(value); \
                } else if (strcmp(prop->name, "partially-seekable") == 0) { \
                    if (option("seekable").toBool() == false) { \
                        bool value = false; \
                        if (prop->format == MPV_FORMAT_FLAG) { \
                            value = *(bool *)prop->data; \
                        } else { \
                            Q_ASSERT_X(false, "KMediaPlayer", "the partially-seekable format has changed"); \
                        } \
                        emit seekable(value); \
                    } \
                } else if (strcmp(prop->name, "paused-for-cache") == 0) { \
                    bool value = false; \
                    if (prop->format == MPV_FORMAT_FLAG) { \
                        value = *(bool *)prop->data; \
                    } else { \
                        Q_ASSERT_X(false, "KMediaPlayer", "the paused-for-cache format has changed"); \
                    } \
                    emit buffering(value); \
                } \
                break; \
            } \
            case MPV_EVENT_LOG_MESSAGE: { \
                mpv_event_log_message *msg = static_cast<mpv_event_log_message *>(event->data); \
                kDebug() << msg->prefix << msg->text; \
                break; \
            } \
            case MPV_EVENT_QUEUE_OVERFLOW: { \
                kWarning() << i18n("event queue overflow"); \
                break; \
            } \
            default: { \
                /* ignore uninteresting or unknown events */ \
            } \
        } \
    }

// QVariant cannot be constructed from WId type
typedef quintptr WIdType;

// the video decoder may run into its own thread, make sure that does not cause trouble
#if defined(HAVE_MPV) && defined(Q_WS_X11)
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

#if defined(HAVE_MPV)
    mpv_handle *m_handle;
#endif
    QString m_appname;
    QSettings *m_settings;
    // the handle pointer is not NULL-ed once mpv_terminate_destroy() has been
    // called, doing it manually is a race because _processHandleEvents() is
    // called asynchronous
    bool m_stopprocessing;
};

KAbstractPlayerPrivate::KAbstractPlayerPrivate()
    : m_appname(QApplication::applicationName()),
    m_settings(new QSettings("KMediaPlayer", "kmediaplayer"))
{
    kDebug() << i18n("initializing player");
    m_stopprocessing = false;
#if defined(HAVE_MPV)
    setlocale(LC_NUMERIC, "C");
    m_handle = mpv_create();
    if (m_handle) {
        const int rc = mpv_initialize(m_handle);
        if (rc < 0) {
            kWarning() << mpv_error_string(rc);
        } else {
            mpv_observe_property(m_handle, 0, "time-pos", MPV_FORMAT_DOUBLE);
            mpv_observe_property(m_handle, 0, "loadfile", MPV_FORMAT_NONE);
            mpv_observe_property(m_handle, 0, "paused-for-cache", MPV_FORMAT_FLAG);
            mpv_observe_property(m_handle, 0, "seekable", MPV_FORMAT_FLAG);
            mpv_observe_property(m_handle, 0, "partially-seekable", MPV_FORMAT_FLAG);
            mpv_request_log_messages(m_handle, "info");
        }
    } else {
        kWarning() << i18n("context creation failed");
    }
#endif
}

KAbstractPlayerPrivate::~KAbstractPlayerPrivate()
{
    kDebug() << i18n("destroying player");
    m_stopprocessing = true;
#if defined(HAVE_MPV)
    mpv_terminate_destroy(m_handle);
#endif
    if (m_settings) {
        m_settings->deleteLater();
    }
}

void KAbstractPlayer::load(const QString &path)
{
    command(QVariantList() << "loadfile" << path);
}

void KAbstractPlayer::load(const QByteArray &data)
{
    // SECURITY: this is dangerous but some applications/libraries (like KHTML) require it
    command(QVariantList() << "loadfile" << QString("memory://%1").arg(data.data()));
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
    command(QVariantList() << "seek" << position << "absolute");
}

void KAbstractPlayer::seek(const int position)
{
    command(QVariantList() << "seek" << position << "absolute");
}

void KAbstractPlayer::stop()
{
    command(QVariantList() << "stop");
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
    foreach (const QVariant variant, value) {
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
    return option("paused-for-cache").toBool();
}

bool KAbstractPlayer::isSeekable() const
{
    return option("seekable").toBool() || option("partially-seekable").toBool();
}

bool KAbstractPlayer::isFullscreen() const
{
#if defined(HAVE_MPV)
    return option("fullscreen").toBool();
#else
    return s_fullscreen;
#endif // HAVE_MPV
}

bool KAbstractPlayer::isProtocolSupported(const QString &protocol) const
{
    foreach(const QString proto, protocols()) {
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
#if defined(HAVE_MPV)
    setOption("fullscreen", fullscreen);
#else
    s_fullscreen = fullscreen;
#endif // HAVE_MPV
}

static void wakeup_audio(void *ctx)
{
    KAudioPlayer *pctx = static_cast<KAudioPlayer*>(ctx);
    QMetaObject::invokeMethod(pctx, "_processHandleEvents", Qt::QueuedConnection);
}

KAudioPlayer::KAudioPlayer(QObject *parent)
    : QObject(parent), d(new KAbstractPlayerPrivate)
{
#if defined(HAVE_MPV)
    if (d->m_handle) {
        mpv_set_wakeup_callback(d->m_handle, wakeup_audio, this);

        // newer releases use vid, video is compat! the change is pre-2014 but yeah..
        setOption("vid", "no");
        setOption("video", "no");

        const QString globalaudio = d->m_settings->value("global/audiooutput", "auto").toString();
        const int globalvolume = d->m_settings->value("global/volume", 90).toInt();
        const bool globalmute = d->m_settings->value("global/mute", false).toBool();
        setAudioOutput(d->m_settings->value(d->m_appname + "/audiooutput", globalaudio).toString());
        setVolume(d->m_settings->value(d->m_appname + "/volume", globalvolume).toInt());
        setMute(d->m_settings->value(d->m_appname + "/mute", globalmute).toBool());
    }
#else
    kWarning() << i18n("KAudioPlayer is a stub");
#endif
}

KAudioPlayer::~KAudioPlayer()
{
#if defined(HAVE_MPV)
    COMMON_STATE_SAVE
#endif

    delete d;
}

void KAudioPlayer::command(const QVariant &command) const
{
#if defined(HAVE_MPV)
    COMMMON_COMMAND_SENDER
#else
    Q_UNUSED(command);
#endif
}

QVariant KAudioPlayer::option(const QString &name) const
{
#if defined(HAVE_MPV)
    COMMON_OPTION_GETTER
#else
    Q_UNUSED(name);
#endif
    return QVariant();
}

void KAudioPlayer::setOption(const QString &name, const QVariant &value) const
{
#if defined(HAVE_MPV)
    COMMON_OPTION_SETTER
#else
    Q_UNUSED(name);
    Q_UNUSED(value);
#endif
}

void KAudioPlayer::_processHandleEvents()
{
#if defined(HAVE_MPV)
    COMMMON_EVENT_HANDLER
#endif
}

bool KAudioPlayer::isMimeSupported(const QString &mime) const
{
#if defined(HAVE_MPV)
    return mime.startsWith("audio/") || mime == QLatin1String("application/octet-stream");
#else
    Q_UNUSED(mime);
    return false;
#endif
}

/////
static void wakeup_media(void *ctx)
{
    KMediaPlayer *pctx = static_cast<KMediaPlayer*>(ctx);
    QMetaObject::invokeMethod(pctx, "_processHandleEvents", Qt::QueuedConnection);
}

KMediaPlayer::KMediaPlayer(QWidget *parent)
    : QWidget(parent), d(new KAbstractPlayerPrivate)
{
#if defined(HAVE_MPV)
    if (d->m_handle) {
        mpv_set_wakeup_callback(d->m_handle, wakeup_media, this);

        QVariant wid;
        if (parent) {
            wid = QVariant::fromValue(static_cast<WIdType>(parent->winId()));
        } else {
            wid = QVariant::fromValue(static_cast<WIdType>(winId()));
        }
        if (wid.isValid()) {
            setOption("wid", wid);
        } else {
            kWarning() << i18n("Could not get widget ID");
        }

        const QString globalaudio = d->m_settings->value("global/audiooutput", "auto").toString();
        const int globalvolume = d->m_settings->value("global/volume", 90).toInt();
        const bool globalmute = d->m_settings->value("global/mute", false).toBool();
        setAudioOutput(d->m_settings->value(d->m_appname + "/audiooutput", globalaudio).toString());
        setVolume(d->m_settings->value(d->m_appname + "/volume", globalvolume).toInt());
        setMute(d->m_settings->value(d->m_appname + "/mute", globalmute).toBool());
    }
#else
    kWarning() << i18n("KMediaPlayer is a stub");
#endif
}

KMediaPlayer::~KMediaPlayer()
{
#if defined(HAVE_MPV)
    COMMON_STATE_SAVE
#endif

    delete d;
}

void KMediaPlayer::command(const QVariant &command) const
{
#if defined(HAVE_MPV)
    COMMMON_COMMAND_SENDER
#else
    Q_UNUSED(command);
#endif
}

QVariant KMediaPlayer::option(const QString &name) const
{
#if defined(HAVE_MPV)
    COMMON_OPTION_GETTER
#else
    Q_UNUSED(name);
#endif
    return QVariant();
}

void KMediaPlayer::setOption(const QString &name, const QVariant &value) const
{
#if defined(HAVE_MPV)
    COMMON_OPTION_SETTER
#else
    Q_UNUSED(name);
    Q_UNUSED(value);
#endif
}

void KMediaPlayer::_processHandleEvents()
{
#if defined(HAVE_MPV)
    COMMMON_EVENT_HANDLER
#endif
}

bool KMediaPlayer::isMimeSupported(const QString &mime) const
{
#if defined(HAVE_MPV)
    return mime.startsWith("audio/") || mime.startsWith("video/")
        || mime == QLatin1String("application/octet-stream");
#else
    Q_UNUSED(mime);
    return false;
#endif
}

#include "moc_kmediaplayer.cpp"
