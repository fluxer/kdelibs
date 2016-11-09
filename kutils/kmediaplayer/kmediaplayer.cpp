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

#include "kdebug.h"
#include "klocale.h"
#include "kmediaplayer.h"
#include <QCoreApplication>
#include <QUrl>
#include <QDragEnterEvent>

#if defined(HAVE_MPV)
#include <mpv/client.h>
#include <mpv/qthelper.hpp>
#else
static bool s_fullscreen = false;
#endif // HAVE_MPV

// the video decoder may run into its own thread, make sure that does not cause trouble
#if defined(HAVE_MPV) && defined(Q_WS_X11)
#include <QApplication>
static int kmp_x11_init_threads() {
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
    return 1;
};
Q_CONSTRUCTOR_FUNCTION(kmp_x11_init_threads)
#endif

class KAbstractPlayerPrivate
{
public:
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

// QVariant cannot be constructed from WId type
typedef quintptr WIdType;

void KAbstractPlayer::load(const QString path)
{
    command(QStringList() << "loadfile" << path);
}

void KAbstractPlayer::load(const QByteArray data)
{
    command(QStringList() << "loadfile" << QString("memory://%1").arg(data.data()));
}

void KAbstractPlayer::play()
{
    setProperty("pause", false);
}

void KAbstractPlayer::pause()
{
    setProperty("pause", true);
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
    return property("path").toString();
}

QString KAbstractPlayer::title() const
{
    return property("media-title").toString();
}

float KAbstractPlayer::currentTime() const
{
    return property("time-pos").toFloat();
}

float KAbstractPlayer::remainingTime() const
{
    return property("time-remaining").toFloat();
}

float KAbstractPlayer::totalTime() const
{
    return property("duration").toFloat();
}

float KAbstractPlayer::volume() const
{
    return property("volume").toFloat();
}

bool KAbstractPlayer::mute() const
{
    return property("mute").toBool();
}

QStringList KAbstractPlayer::protocols() const
{
    static QStringList s_protocols;
    if (s_protocols.isEmpty()) {
        s_protocols << property("protocol-list").toStringList();
    }
    return s_protocols;
}

QString KAbstractPlayer::audiooutput() const
{
    return property("audio-device").toString();
}

QStringList KAbstractPlayer::audiooutputs() const
{
    const QVariantList value = property("audio-device-list").toList();
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
    return !property("pause").toBool() && !property("path").isNull();
}

bool KAbstractPlayer::isBuffering() const
{
    return property("paused-for-cache").toBool();
}

bool KAbstractPlayer::isSeekable() const
{
    return property("seekable").toBool() || property("partially-seekable").toBool();
}

bool KAbstractPlayer::isFullscreen() const
{
#if defined(HAVE_MPV)
    return property("fullscreen").toBool();
#else
    return s_fullscreen;
#endif // HAVE_MPV
}

bool KAbstractPlayer::isProtocolSupported(const QString protocol) const
{
    foreach(const QString proto, protocols()) {
        if (protocol.startsWith(proto)) {
            return true;
        }
    }
    return false;
}

bool KAbstractPlayer::isPathSupported(const QString path) const
{
    const KMimeType::Ptr mime = KMimeType::findByPath(path);
    if (mime && isMimeSupported(mime->name())) {
        return true;
    }
    return isProtocolSupported(path);
}

void KAbstractPlayer::setVolume(const float volume)
{
    setProperty("volume", volume);
}

void KAbstractPlayer::setVolume(const int volume)
{
    setProperty("volume", volume);
}

void KAbstractPlayer::setMute(const bool mute)
{
    setProperty("mute", mute);
}

void KAbstractPlayer::setAudioOutput(const QString output)
{
    setProperty("audio-device", output);
}

void KAbstractPlayer::setFullscreen(const bool fullscreen)
{
#if defined(HAVE_MPV)
    setProperty("fullscreen", fullscreen);
#else
    s_fullscreen = fullscreen;
#endif // HAVE_MPV
}

#if defined(HAVE_MPV)
/*
    Since sigals/slots cannot be virtual nor multiple QObject inheritance works (KAbstractPlayer
    cannot inherit from QObject if it is to be used in a class that inherits QWidget) here are
    some pre-processor definitions used to share the code as much as possible making modification
    easier.
*/
#define COMMON_CONSTRUCTOR \
    kDebug() << i18n("initializing player"); \
    d->m_stopprocessing = false; \
    setlocale(LC_NUMERIC, "C"); \
    d->m_handle = mpv_create(); \
    if (d->m_handle) { \
        const int rc = mpv_initialize(d->m_handle); \
        if (rc < 0) { \
            kWarning() << mpv_error_string(rc); \
        } else {\
            mpv_observe_property(d->m_handle, 0, "time-pos", MPV_FORMAT_DOUBLE); \
            mpv_observe_property(d->m_handle, 0, "loadfile", MPV_FORMAT_NONE); \
            mpv_observe_property(d->m_handle, 0, "paused-for-cache", MPV_FORMAT_FLAG); \
            mpv_observe_property(d->m_handle, 0, "seekable", MPV_FORMAT_FLAG); \
            mpv_observe_property(d->m_handle, 0, "partially-seekable", MPV_FORMAT_FLAG); \
            mpv_request_log_messages(d->m_handle, "info"); \
        } \
    } else { \
        kWarning() << i18n("context creation failed"); \
    }

#define COMMON_DESTRUCTOR \
    kDebug() << i18n("destroying player"); \
    d->m_stopprocessing = true; \
    mpv_terminate_destroy(d->m_handle); \
    if (d->m_settings) { \
        d->m_settings->deleteLater(); \
    } \
    delete d;

#define COMMMON_COMMAND_SENDER \
    kDebug() << i18n("sending command") << command; \
    if (d->m_handle) { \
        const QVariant error = mpv::qt::command_variant(d->m_handle, command); \
        if (!error.isNull()) { \
            kWarning() << error; \
        } \
    }

#define COMMON_PROPERTY_SETTER \
    kDebug() << i18n("setting property") << name << value; \
    if (d->m_handle) { \
        mpv::qt::set_property_variant(d->m_handle, name, value); \
    }

#define COMMON_PROPERTY_GETTER \
    kDebug() << i18n("getting property") << name; \
    if (d->m_handle) { \
        return mpv::qt::get_property_variant(d->m_handle, name); \
    } \
    return QVariant();

#define COMMON_OPTION_SETTER \
    kDebug() << i18n("setting option") << name << value; \
    if (d->m_handle) { \
        mpv::qt::set_option_variant(d->m_handle, name, value); \
    }

#define COMMMON_EVENT_HANDLER \
    while (!d->m_stopprocessing) { \
        mpv_event *event = mpv_wait_event(d->m_handle, 0); \
        if (event->event_id == MPV_EVENT_NONE) { \
            break; \
        } \
        switch (event->event_id) { \
            case MPV_EVENT_FILE_LOADED: { \
                emit loaded(); \
                break; \
            } \
            case MPV_EVENT_PAUSE: { \
                emit paused(true); \
                break; \
            } \
            case MPV_EVENT_UNPAUSE: { \
                emit paused(false); \
                break; \
            } \
            case MPV_EVENT_END_FILE: { \
                mpv_event_end_file *prop = (mpv_event_end_file *)event->data; \
                if (prop->reason == MPV_END_FILE_REASON_ERROR) { \
                    emit finished(); \
                    emit error(QString(mpv_error_string(prop->error))); \
                } else if (prop->reason == MPV_END_FILE_REASON_EOF \
                    || prop->reason == MPV_END_FILE_REASON_STOP \
                    || prop->reason == MPV_END_FILE_REASON_QUIT) { \
                    if (property("path").isNull()) { \
                        emit finished(); \
                    } \
                } \
                break; \
            } \
            case MPV_EVENT_PROPERTY_CHANGE: { \
                mpv_event_property *prop = (mpv_event_property *)event->data; \
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
                    if (property("seekable").toBool() == false) { \
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
                mpv_event_log_message *msg = (mpv_event_log_message *)event->data; \
                kDebug() << msg->text; \
                break; \
            } \
            case MPV_EVENT_QUEUE_OVERFLOW: { \
                kWarning() << i18n("event queue overflow"); \
                break; \
            } \
            default: { \
                /* Ignore uninteresting or unknown events. */ \
            } \
        } \
    }

static void wakeup_audio(void *ctx)
{
    KAudioPlayer *pctx = static_cast<KAudioPlayer*>(ctx);
    QMetaObject::invokeMethod(pctx, "_processHandleEvents", Qt::QueuedConnection);
}

KAudioPlayer::KAudioPlayer(QObject *parent)
    : QObject(parent), d(new KAbstractPlayerPrivate)
{
    COMMON_CONSTRUCTOR

    d->m_appname = QCoreApplication::applicationName();
    d->m_settings = new QSettings("KMediaPlayer", "kmediaplayer");
    if (d->m_handle) {
        mpv_set_wakeup_callback(d->m_handle, wakeup_audio, this);
        // TODO: newer releases use vid, video is compat!
        // NOTE: the change is pre-2014
        setProperty("video", "no");

        const QString globalaudio = d->m_settings->value("global/audiooutput", "auto").toString();
        const int globalvolume = d->m_settings->value("global/volume", 90).toInt();
        const bool globalmute = d->m_settings->value("global/mute", false).toBool();
        setAudioOutput(d->m_settings->value(d->m_appname + "/audiooutput", globalaudio).toString());
        setVolume(d->m_settings->value(d->m_appname + "/volume", globalvolume).toInt());
        setMute(d->m_settings->value(d->m_appname + "/mute", globalmute).toBool());
    }
}

KAudioPlayer::~KAudioPlayer()
{
    if (d->m_handle && d->m_settings && d->m_settings->isWritable()) {
        d->m_settings->beginGroup(d->m_appname);
        d->m_settings->setValue("audiooutput", audiooutput());
        d->m_settings->setValue("volume", volume());
        d->m_settings->setValue("mute", mute());
        d->m_settings->endGroup();
        d->m_settings->sync();
    } else {
        kWarning() << i18n("Could not save state");
    }

    COMMON_DESTRUCTOR
}

void KAudioPlayer::command(const QVariant& command) const
{
    COMMMON_COMMAND_SENDER
}

void KAudioPlayer::setProperty(const QString& name, const QVariant& value) const
{
    COMMON_PROPERTY_SETTER
}

QVariant KAudioPlayer::property(const QString& name) const
{
    COMMON_PROPERTY_GETTER
}

void KAudioPlayer::setOption(const QString& name, const QVariant& value) const
{
    COMMON_OPTION_SETTER
}

void KAudioPlayer::_processHandleEvents()
{
    COMMMON_EVENT_HANDLER
}

bool KAudioPlayer::isMimeSupported(const QString mime) const
{
    return mime.startsWith("audio/") || mime == QLatin1String("application/octet-stream");
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
    COMMON_CONSTRUCTOR

    d->m_appname = QCoreApplication::applicationName();
    d->m_settings = new QSettings("KMediaPlayer", "kmediaplayer");
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
}

KMediaPlayer::~KMediaPlayer()
{
    if (d->m_handle && d->m_settings && d->m_settings->isWritable()) {
        d->m_settings->beginGroup(d->m_appname);
        d->m_settings->setValue("audiooutput", audiooutput());
        d->m_settings->setValue("volume", volume());
        d->m_settings->setValue("mute", mute());
        d->m_settings->endGroup();
        d->m_settings->sync();
    } else {
        kWarning() << i18n("Could not save state");
    }

    COMMON_DESTRUCTOR
}

void KMediaPlayer::command(const QVariant& command) const
{
    COMMMON_COMMAND_SENDER
}

void KMediaPlayer::setProperty(const QString& name, const QVariant& value) const
{
    COMMON_PROPERTY_SETTER
}

QVariant KMediaPlayer::property(const QString& name) const
{
    COMMON_PROPERTY_GETTER
}

void KMediaPlayer::setOption(const QString& name, const QVariant& value) const
{
    COMMON_OPTION_SETTER
}

void KMediaPlayer::_processHandleEvents()
{
    COMMMON_EVENT_HANDLER
}

bool KMediaPlayer::isMimeSupported(const QString mime) const
{
    return mime.startsWith("audio/") || mime.startsWith("video/")
        || mime == QLatin1String("application/octet-stream");
}

#else // HAVE_MPV
/////
KAudioPlayer::KAudioPlayer(QObject *parent)
    : QObject(parent)
{
    kWarning() << i18n("KAudioPlayer is a stub");
}

KAudioPlayer::~KAudioPlayer()
{
}

void KAudioPlayer::command(const QVariant& command) const
{
    Q_UNUSED(command);
}

void KAudioPlayer::setProperty(const QString& name, const QVariant& value) const
{
    Q_UNUSED(name);
    Q_UNUSED(value);
}

QVariant KAudioPlayer::property(const QString& name) const
{
    Q_UNUSED(name);
    return QVariant();
}

void KAudioPlayer::setOption(const QString& name, const QVariant& value) const
{
    Q_UNUSED(name);
    Q_UNUSED(value);
}

void KAudioPlayer::_processHandleEvents()
{
}

bool KAudioPlayer::isMimeSupported(const QString mime) const
{
    Q_UNUSED(mime);
    return false;
}

/////
KMediaPlayer::KMediaPlayer(QWidget *parent)
    : QWidget(parent)
{
    kWarning() << i18n("KMediaPlayer is a stub");
}

KMediaPlayer::~KMediaPlayer()
{
}

void KMediaPlayer::command(const QVariant& command) const
{
    Q_UNUSED(command);
}

void KMediaPlayer::setProperty(const QString& name, const QVariant& value) const
{
    Q_UNUSED(name);
    Q_UNUSED(value);
}

QVariant KMediaPlayer::property(const QString& name) const
{
    Q_UNUSED(name);
    return QVariant();
}

void KMediaPlayer::setOption(const QString& name, const QVariant& value) const
{
}

void KMediaPlayer::_processHandleEvents()
{
}

bool KMediaPlayer::isMimeSupported(const QString mime) const
{
    Q_UNUSED(mime);
    return false;
}

#endif // HAVE_MPV

#include "moc_kmediaplayer.cpp"
