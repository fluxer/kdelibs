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

#include "kemail.h"
#include "klocale.h"
#include "kemailsettings.h"
#include "kdebug.h"

#include <string.h>
#include <curl/curl.h>

class KEMailPrivate
{
public:
    KEMailPrivate();
    ~KEMailPrivate();

    static QByteArray makeData(const QString &subject, const QString &message, const KUrl::List &attach);

    static size_t curlReadCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

    CURL* m_curl;
    struct curl_slist *m_curlrcpt;
    KUrl m_server;
    QString m_user;
    QString m_password;
    QString m_from;
    QStringList m_to;
    QByteArray m_data;
    QString m_errorstring;
};

KEMailPrivate::KEMailPrivate()
    : m_curl(nullptr),
    m_curlrcpt(nullptr)
{
}

KEMailPrivate::~KEMailPrivate()
{
    if (m_curlrcpt) {
        curl_slist_free_all(m_curlrcpt);
    }
    if (m_curl) {
        curl_easy_cleanup(m_curl);
    }
}

// for reference:
// https://www.rfc-editor.org/rfc/rfc5322
QByteArray KEMailPrivate::makeData(const QString &subject, const QString &message, const KUrl::List &attach)
{
    // TODO:
    Q_UNUSED(attach);

    QByteArray subjectbytes("Subject: ");
    subjectbytes.append(subject.toAscii());
    subjectbytes.append("\n\n");

    QByteArray result = subjectbytes;
    result.append(message.toAscii());
    result.append("\n");
    result.replace('\n', "\r\n");
    return result;
}

size_t KEMailPrivate::curlReadCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    KEMailPrivate* kemailprivate = static_cast<KEMailPrivate*>(userdata);
    Q_ASSERT(kemailprivate);
    Q_ASSERT(size == 1);
    const QByteArray kemailmessage = kemailprivate->m_data.left(qMin(int(nmemb), kemailprivate->m_data.size()));
    if (kemailmessage.size() <= 0) {
        kDebug() << "Message has been sent";
        return 0;
    }
    ::memcpy(ptr, kemailmessage.constData(), kemailmessage.size() * sizeof(char));
    kemailprivate->m_data = kemailprivate->m_data.mid(kemailmessage.size(), kemailprivate->m_data.size() - kemailmessage.size());
    return kemailmessage.size();
}

KEMail::KEMail(QObject *parent)
    : QObject(parent),
    d(new KEMailPrivate())
{
    KEMailSettings kemailsettings;
    setServer(KUrl(kemailsettings.getSetting(KEMailSettings::OutServer)));
    setFrom(kemailsettings.getSetting(KEMailSettings::EmailAddress));
    setUser(kemailsettings.getSetting(KEMailSettings::OutServerLogin));
    setPassword(kemailsettings.getSetting(KEMailSettings::OutServerPass));
}

KEMail::~KEMail()
{
    delete d;
}

KUrl KEMail::server() const
{
    return d->m_server;
}

bool KEMail::setServer(const KUrl &server)
{
    d->m_errorstring.clear();
    QString serverurl = server.url();
    if (serverurl.isEmpty()) {
        d->m_errorstring = i18n("Invalid server URL: %1", serverurl);
        return false;
    }
    if (!serverurl.startsWith("smtp://")) {
        serverurl.prepend(QLatin1String("smtp://"));
    }
    d->m_server = serverurl;
    return true;
}

QString KEMail::user() const
{
    return d->m_user;
}

bool KEMail::setUser(const QString &user)
{
    d->m_errorstring.clear();
    if (user.isEmpty()) {
        d->m_errorstring = i18n("Invalid user: %1", user);
        return false;
    }
    d->m_user = user;
    return true;
}

QString KEMail::password() const
{
    return d->m_password;
}

bool KEMail::setPassword(const QString &password)
{
    d->m_errorstring.clear();
    if (password.isEmpty()) {
        d->m_errorstring = i18n("Invalid password: %1", password);
        return false;
    }
    d->m_password = password;
    return true;
}

QString KEMail::from() const
{
    return d->m_from;
}

bool KEMail::setFrom(const QString &from)
{
    d->m_errorstring.clear();
    if (from.isEmpty()) {
        d->m_errorstring = i18n("Invalid sender: %1", from);
        return false;
    }
    d->m_from = from;
    return true;
}

QStringList KEMail::to() const
{
    return d->m_to;
}

bool KEMail::setTo(const QStringList &to)
{
    d->m_errorstring.clear();
    if (to.isEmpty()) {
        d->m_errorstring = i18n("Invalid receivers: %1", to.join(QLatin1String(", ")));
        return false;
    }
    d->m_to = to;
    return true;
}

bool KEMail::send(const QString &subject, const QString &message, const KUrl::List &attach)
{
    d->m_errorstring.clear();
    if (message.isEmpty()) {
        d->m_errorstring = i18n("Invalid message: %1", message);
        return false;
    }
    d->m_data = KEMailPrivate::makeData(subject, message, attach);
    d->m_curl = curl_easy_init();
    if (!d->m_curl) {
        d->m_errorstring = i18n("Could not create context");
        return false;
    }
    if (d->m_curlrcpt) {
        curl_slist_free_all(d->m_curlrcpt);
        d->m_curlrcpt = nullptr;
    }

    const QByteArray serverbytes = d->m_server.url().toAscii();
    const QByteArray userbytes = d->m_user.toAscii();
    const QByteArray passwordbytes = d->m_password.toAscii();
    const QByteArray frombytes = d->m_from.toAscii();
    curl_easy_setopt(d->m_curl, CURLOPT_URL, serverbytes.constData());
    curl_easy_setopt(d->m_curl, CURLOPT_USERNAME, userbytes.constData());
    curl_easy_setopt(d->m_curl, CURLOPT_PASSWORD, passwordbytes.constData());
    curl_easy_setopt(d->m_curl, CURLOPT_MAIL_FROM, frombytes.constData());
    foreach (const QString &to, d->m_to) {
        const QByteArray tobytes = to.toAscii();
        d->m_curlrcpt = curl_slist_append(d->m_curlrcpt, tobytes.constData());
    }
    curl_easy_setopt(d->m_curl, CURLOPT_MAIL_RCPT, d->m_curlrcpt);
    curl_easy_setopt(d->m_curl, CURLOPT_READFUNCTION, KEMailPrivate::curlReadCallback);
    curl_easy_setopt(d->m_curl, CURLOPT_READDATA, d);
    curl_easy_setopt(d->m_curl, CURLOPT_UPLOAD, 1L);
    // curl_easy_setopt(d->m_curl, CURLOPT_VERBOSE, 1L);
    // TODO: option for these and add setting to KEMailSettings
    curl_easy_setopt(d->m_curl, CURLOPT_LOGIN_OPTIONS, "AUTH=PLAIN");
    curl_easy_setopt(d->m_curl, CURLOPT_USE_SSL, (long)CURLUSESSL_TRY);

    bool result = true;
    const CURLcode curlresult = curl_easy_perform(d->m_curl);
    if (curlresult != CURLE_OK) {
        d->m_errorstring = curl_easy_strerror(curlresult);
        kWarning() << d->m_errorstring;
        result = false;
    }

    curl_slist_free_all(d->m_curlrcpt);
    d->m_curlrcpt = nullptr;
    curl_easy_cleanup(d->m_curl);
    d->m_curl = nullptr;
    return result;
}

QString KEMail::errorString() const
{
    return d->m_errorstring;
}
