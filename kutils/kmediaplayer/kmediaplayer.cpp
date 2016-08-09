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
#include <QCoreApplication>
#include <QUrl>
#ifndef QT_KATIE
#include <QDragEnterEvent>
#endif
#include "kmediaplayer.h"

#ifdef HAVE_MPV
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

// QVariant cannot be constructed from WId type
typedef quintptr WIdType;

void KAbstractPlayer::load(QString path)
{
    command(QStringList() << "loadfile" << path);
}

void KAbstractPlayer::load(QByteArray data)
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

void KAbstractPlayer::seek(float position)
{
    command(QVariantList() << "seek" << position << "absolute");
}

void KAbstractPlayer::seek(int position)
{
    command(QVariantList() << "seek" << position << "absolute");
}

void KAbstractPlayer::stop()
{
    command(QVariantList() << "stop");
}

QString KAbstractPlayer::path()
{
    return property("path").toString();
}

QString KAbstractPlayer::title()
{
    return property("media-title").toString();
}

float KAbstractPlayer::currentTime()
{
    return property("time-pos").toFloat();
}

float KAbstractPlayer::remainingTime()
{
    return property("time-remaining").toFloat();
}

float KAbstractPlayer::totalTime()
{
    return property("duration").toFloat();
}

float KAbstractPlayer::volume()
{
    return property("volume").toFloat();
}

bool KAbstractPlayer::mute()
{
    return property("mute").toBool();
}

QStringList KAbstractPlayer::protocols()
{
    static QStringList s_protocols;
    if (s_protocols.isEmpty()) {
        s_protocols << property("protocol-list").toStringList();
    }
    return s_protocols;
}

QString KAbstractPlayer::audiooutput()
{
    return property("audio-device").toString();
}

QStringList KAbstractPlayer::audiooutputs()
{
    const QVariantList value = property("audio-device-list").toList();
    QStringList stringlist;
    foreach (QVariant variant, value) {
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

bool KAbstractPlayer::isPlaying()
{
    return !property("pause").toBool() && !property("path").isNull();
}

bool KAbstractPlayer::isBuffering()
{
    return property("paused-for-cache").toBool();
}

bool KAbstractPlayer::isSeekable()
{
    return property("seekable").toBool() || property("partially-seekable").toBool();
}

bool KAbstractPlayer::isFullscreen()
{
#ifdef HAVE_MPV
    return property("fullscreen").toBool();
#else
    return s_fullscreen;
#endif // HAVE_MPV
}

bool KAbstractPlayer::isProtocolSupported(QString protocol)
{
    foreach(QString proto, protocols()) {
        if (protocol.startsWith(proto)) {
            return true;
        }
    }
    return false;
}

bool KAbstractPlayer::isPathSupported(QString path)
{
    KMimeType::Ptr mime = KMimeType::findByPath(path);
    if (mime && isMimeSupported(mime->name())) {
        return true;
    }
    return isProtocolSupported(path);
}

void KAbstractPlayer::setVolume(float volume)
{
    setProperty("volume", volume);
}

void KAbstractPlayer::setVolume(int volume)
{
    setProperty("volume", volume);
}

void KAbstractPlayer::setMute(bool mute)
{
    setProperty("mute", mute);
}

void KAbstractPlayer::setAudioOutput(QString output)
{
    setProperty("audio-device", output);
}

void KAbstractPlayer::setFullscreen(bool fullscreen)
{
#ifdef HAVE_MPV
    setProperty("fullscreen", fullscreen);
#else
    s_fullscreen = fullscreen;
#endif // HAVE_MPV
}

#ifdef HAVE_MPV
/*
    Since exposing mpv_handle is not desirable and sigals/slots cannot be virtual nor multiple
    QObject inheritance works here are some pre-processor definitions used to share the code as
    much as possible making modifications easier
*/
#define COMMON_CONSTRUCTOR \
    kDebug() << i18n("initializing player"); \
    m_stopprocessing = false; \
    setlocale(LC_NUMERIC, "C"); \
    m_handle = mpv_create(); \
    if (m_handle) { \
        int rc = mpv_initialize(m_handle); \
        if (rc < 0) { \
            kWarning() << mpv_error_string(rc); \
        } else {\
            mpv_observe_property(m_handle, 0, "time-pos", MPV_FORMAT_DOUBLE); \
            mpv_observe_property(m_handle, 0, "loadfile", MPV_FORMAT_NONE); \
            mpv_observe_property(m_handle, 0, "paused-for-cache", MPV_FORMAT_FLAG); \
            mpv_observe_property(m_handle, 0, "seekable", MPV_FORMAT_FLAG); \
            mpv_observe_property(m_handle, 0, "partially-seekable", MPV_FORMAT_FLAG); \
            mpv_request_log_messages(m_handle, "info"); \
        } \
    } else { \
        kWarning() << i18n("context creation failed"); \
    }

#define COMMON_DESTRUCTOR \
    kDebug() << i18n("destroying player"); \
    m_stopprocessing = true; \
    mpv_terminate_destroy(m_handle); \
    if (m_settings) { \
        delete m_settings; \
    }

#define COMMMON_COMMAND_SENDER \
    kDebug() << i18n("sending command") << command; \
    if (m_handle) { \
        QVariant error = mpv::qt::command_variant(m_handle, command); \
        if (!error.isNull()) { \
            kWarning() << error; \
        } \
    }

#define COMMON_PROPERTY_SETTER \
    kDebug() << i18n("setting property") << name << value; \
    if (m_handle) { \
        mpv::qt::set_property_variant(m_handle, name, value); \
    }

#define COMMON_PROPERTY_GETTER \
    kDebug() << i18n("getting property") << name; \
    if (m_handle) { \
        return mpv::qt::get_property_variant(m_handle, name); \
    } \
    return QVariant();

#define COMMON_OPTION_SETTER \
    kDebug() << i18n("setting option") << name << value; \
    if (m_handle) { \
        mpv::qt::set_option_variant(m_handle, name, value); \
    }

#define COMMMON_EVENT_HANDLER \
    while (!m_stopprocessing) { \
        mpv_event *event = mpv_wait_event(m_handle, 0); \
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
                    } \
                    emit position(value); \
                } else if (strcmp(prop->name, "seekable") == 0) { \
                    bool value = false; \
                    if (prop->format == MPV_FORMAT_FLAG) { \
                        value = *(bool *)prop->data; \
                    } \
                    emit seekable(value); \
                } else if (strcmp(prop->name, "partially-seekable") == 0) { \
                    if (property("seekable").toBool() == false) { \
                        bool value = false; \
                        if (prop->format == MPV_FORMAT_FLAG) { \
                            value = *(bool *)prop->data; \
                        } \
                        emit seekable(value); \
                    } \
                } else if (strcmp(prop->name, "paused-for-cache") == 0) { \
                    bool value = false; \
                    if (prop->format == MPV_FORMAT_FLAG) { \
                        value = *(bool *)prop->data; \
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
    : QObject(parent)
{
    COMMON_CONSTRUCTOR

    m_appname = QCoreApplication::applicationName();
    m_settings = new QSettings("KMediaPlayer", "kmediaplayer");
    if (m_handle) {
        mpv_set_wakeup_callback(m_handle, wakeup_audio, this);
        // TODO: newer releases use vid, video is compat!
        // NOTE: the change is pre-2014
        setProperty("video", "no");

        QString globalaudio = m_settings->value("global/audiooutput", "auto").toString();
        int globalvolume = m_settings->value("global/volume", 90).toInt();
        bool globalmute = m_settings->value("global/mute", false).toBool();
        setAudioOutput(m_settings->value(m_appname + "/audiooutput", globalaudio).toString());
        setVolume(m_settings->value(m_appname + "/volume", globalvolume).toInt());
        setMute(m_settings->value(m_appname + "/mute", globalmute).toBool());
    }
}

KAudioPlayer::~KAudioPlayer()
{
    if (m_handle && m_settings && m_settings->isWritable()) {
        m_settings->beginGroup(m_appname);
        m_settings->setValue("audiooutput", audiooutput());
        m_settings->setValue("volume", volume());
        m_settings->setValue("mute", mute());
        m_settings->endGroup();
        m_settings->sync();
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
    : QWidget(parent)
{
    COMMON_CONSTRUCTOR

    m_appname = QCoreApplication::applicationName();
    m_settings = new QSettings("KMediaPlayer", "kmediaplayer");
    if (m_handle) {
        mpv_set_wakeup_callback(m_handle, wakeup_media, this);
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

        QString globalaudio = m_settings->value("global/audiooutput", "auto").toString();
        int globalvolume = m_settings->value("global/volume", 90).toInt();
        bool globalmute = m_settings->value("global/mute", false).toBool();
        setAudioOutput(m_settings->value(m_appname + "/audiooutput", globalaudio).toString());
        setVolume(m_settings->value(m_appname + "/volume", globalvolume).toInt());
        setMute(m_settings->value(m_appname + "/mute", globalmute).toBool());
    }
}

KMediaPlayer::~KMediaPlayer()
{
    if (m_handle && m_settings && m_settings->isWritable()) {
        m_settings->beginGroup(m_appname);
        m_settings->setValue("audiooutput", audiooutput());
        m_settings->setValue("volume", volume());
        m_settings->setValue("mute", mute());
        m_settings->endGroup();
        m_settings->sync();
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
