/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "kspeech.h"

#include <QCoreApplication>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#if defined(HAVE_SPEECHD)
#  include <speech-dispatcher/libspeechd.h>
#endif

static KSpeechPrivate* s_kspeechprivate = nullptr;

// NOTE: the callback function is not called in the same thread as the object that opens the
// connection
class KSpeechPrivate : public QObject
{
    friend class KSpeech;
    Q_OBJECT
public:
    KSpeechPrivate(QObject *parent);
    ~KSpeechPrivate();

#if defined(HAVE_SPEECHD)
    void initSpeechD();
    static void speechCallback(size_t msg_id, size_t client_id, SPDNotificationType state);
#endif

Q_SIGNALS:
    void signalJobStateChanged(int jobNum, int state);

private:
#if defined(HAVE_SPEECHD)
    SPDConnection* m_speechd;
#endif
    QByteArray m_speechid;
};

KSpeechPrivate::KSpeechPrivate(QObject *parent)
    : QObject(parent)
#if defined(HAVE_SPEECHD)
    , m_speechd(nullptr)
#endif
{
    if (Q_UNLIKELY(s_kspeechprivate)) {
        kWarning() << "Multiple KSpeech instances are not supported";
    }

    s_kspeechprivate = this;
    m_speechid = QCoreApplication::applicationName().toLocal8Bit();
}

KSpeechPrivate::~KSpeechPrivate()
{
    s_kspeechprivate = nullptr;
}

#if defined(HAVE_SPEECHD)
void KSpeechPrivate::initSpeechD()
{
    if (!m_speechd) {
        m_speechd = spd_open(m_speechid.constData(), "main", NULL, SPD_MODE_THREADED);
        if (Q_UNLIKELY(!m_speechd)) {
            kWarning() << "Could not open speech-dispatcher connection";
            return;
        }

        m_speechd->callback_begin = KSpeechPrivate::speechCallback;
        m_speechd->callback_end = KSpeechPrivate::speechCallback;
        m_speechd->callback_cancel = KSpeechPrivate::speechCallback;

        spd_set_notification_on(m_speechd, SPD_BEGIN);
        spd_set_notification_on(m_speechd, SPD_END);
        spd_set_notification_on(m_speechd, SPD_CANCEL);

        KConfig kconfig("kspeechrc", KConfig::SimpleConfig);
        KConfigGroup kconfiggroup = kconfig.group(m_speechid);
        const int volume = kconfiggroup.readEntry("volume", 100);
        const int pitch = kconfiggroup.readEntry("pitch", 0);
        const QByteArray voice = kconfiggroup.readEntry("voice", QByteArray());
        // qDebug() << Q_FUNC_INFO << m_speechid << volume << pitch << voice;
        KSpeech* kspeech = qobject_cast<KSpeech*>(parent());
        Q_ASSERT(kspeech);
        kspeech->setVolume(volume);
        kspeech->setPitch(pitch);
        kspeech->setVoice(voice);
    }
}

void KSpeechPrivate::speechCallback(size_t msg_id, size_t client_id, SPDNotificationType state)
{
    Q_UNUSED(client_id);
    // qDebug() << Q_FUNC_INFO << msg_id << client_id << state;
    if (Q_UNLIKELY(!s_kspeechprivate)) {
        kWarning() << "Null kspeech private pointer";
        return;
    }

    const int jobNum = msg_id;
    switch (state) {
        case SPD_EVENT_BEGIN: {
            emit s_kspeechprivate->signalJobStateChanged(jobNum, KSpeech::JobStarted);
            break;
        }
        case SPD_EVENT_END: {
            emit s_kspeechprivate->signalJobStateChanged(jobNum, KSpeech::JobFinished);
            break;
        }
        case SPD_EVENT_CANCEL: {
            emit s_kspeechprivate->signalJobStateChanged(jobNum, KSpeech::JobCanceled);
            break;
        }
        default: {
            break;
        }
    }
}
#endif // HAVE_SPEECHD


KSpeech::KSpeech(QObject* parent)
    : QObject(parent),
    d(new KSpeechPrivate(this))
{
    connect(
        d, SIGNAL(signalJobStateChanged(int,int)),
        this, SLOT(_jobStateChanged(int,int))
    );

#if defined(HAVE_SPEECHD)
    d->initSpeechD();
#endif
    // qDebug() << Q_FUNC_INFO << volume() << pitch() << voices();
}

KSpeech::~KSpeech()
{
    delete d;
}

bool KSpeech::isSupported()
{
#if defined(HAVE_SPEECHD)
    return true;
#else
    return false;
#endif
}

void KSpeech::setSpeechID(const QString &id)
{
    removeAllJobs();
    d->m_speechid = id.toUtf8();
#if defined(HAVE_SPEECHD)
    d->initSpeechD();
#endif
}

int KSpeech::volume() const
{
#if defined(HAVE_SPEECHD)
    if (Q_UNLIKELY(!d->m_speechd)) {
        kDebug() << "Null speech-dispatcher pointer";
        return 0;
    }

    return spd_get_volume(d->m_speechd);
#else
    return 0;
#endif
}

bool KSpeech::setVolume(const int volume)
{
#if defined(HAVE_SPEECHD)
    if (Q_UNLIKELY(!d->m_speechd)) {
        kDebug() << "Null speech-dispatcher pointer";
        return false;
    }

    // TODO: error check
    (void)spd_set_volume(d->m_speechd, volume);
    return true;
#else
    return false;
#endif
}

int KSpeech::pitch() const
{
#if defined(HAVE_SPEECHD)
    if (Q_UNLIKELY(!d->m_speechd)) {
        kDebug() << "Null speech-dispatcher pointer";
        return 0;
    }

    return spd_get_voice_pitch(d->m_speechd);
#else
    return 0;
#endif
}

bool KSpeech::setPitch(const int pitch)
{
#if defined(HAVE_SPEECHD)
    if (Q_UNLIKELY(!d->m_speechd)) {
        kDebug() << "Null speech-dispatcher pointer";
        return false;
    }

    // TODO: error check
    (void)spd_set_voice_pitch(d->m_speechd, pitch);
    return true;
#else
    return false;
#endif
}

QList<QByteArray> KSpeech::voices() const
{
    QList<QByteArray> result;
#if defined(HAVE_SPEECHD)
    if (Q_UNLIKELY(!d->m_speechd)) {
        kDebug() << "Null speech-dispatcher pointer";
        return result;
    }

    SPDVoice** speechdvoices = spd_list_synthesis_voices(d->m_speechd);
    if (!speechdvoices) {
        kWarning() << "Null speech-dispatcher voices pointer";
        return result;
    }
    int i = 0;
    while (speechdvoices[i]) {
        result.append(speechdvoices[i]->name);
        i++;
    }
    free_spd_voices(speechdvoices);
#endif
    return result;
}

bool KSpeech::setVoice(const QByteArray &voice)
{
#if defined(HAVE_SPEECHD)
    if (Q_UNLIKELY(!d->m_speechd)) {
        kDebug() << "Null speech-dispatcher pointer";
        return false;
    }

    if (voice.isEmpty()) {
        kWarning() << "Empty voice" << voice;
        return false;
    } else if (!KSpeech::voices().contains(voice)) {
        // NOTE: if voice is invalid speech-dispatcher will not dispatch anything
        kWarning() << "Invalid voice" << voice;
        return false;
    }

    // TODO: error check
    (void)spd_set_synthesis_voice(d->m_speechd, voice.constData());
    return true;
#else
    return false;
#endif
}

int KSpeech::say(const QString &text)
{
#if defined(HAVE_SPEECHD)
    if (Q_UNLIKELY(!d->m_speechd)) {
        kDebug() << "Null speech-dispatcher pointer";
        return 0;
    }

    const QByteArray textbytes = text.toUtf8();
    const int jobNum = spd_say(d->m_speechd, SPD_TEXT, textbytes.constData());
    if (jobNum < 0) {
        kWarning() << "Speech-dispatcher say failed";
        return 0;
    }

    // qDebug() << Q_FUNC_INFO << jobNum << spd_get_client_id(d->m_speechd);
    return jobNum;
#else
    kWarning() << "KSpeech is a stub";
    return 0;
#endif // HAVE_SPEECHD
}

void KSpeech::removeAllJobs()
{
#if defined(HAVE_SPEECHD)
    if (Q_UNLIKELY(!d->m_speechd)) {
        kDebug() << "Null speech-dispatcher pointer";
        return;
    }

    // TODO: error check
    (void)spd_stop(d->m_speechd);
    spd_close(d->m_speechd);
    d->m_speechd = nullptr;
#endif
}

void KSpeech::removeJob(int jobNum)
{
#if defined(HAVE_SPEECHD)
    if (Q_UNLIKELY(!d->m_speechd)) {
        kWarning() << "Null speech-dispatcher pointer";
        return;
    }

    // TODO: error check
    (void)spd_stop_uid(d->m_speechd, jobNum);
#endif
}

void KSpeech::_jobStateChanged(int jobNum, int state)
{
    emit jobStateChanged(jobNum, state);
}
#include "kspeech.moc"
