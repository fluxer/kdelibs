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

// for reference:
// https://en.wikipedia.org/wiki/SMTP_Authentication

static QString enumToOutServerSSL(const KEMail::KEMailSSLType value)
{
    switch (value) {
        case KEMail::KEMailSSLType::SSLNo: {
            return QString::fromLatin1("no");
        }
        case KEMail::KEMailSSLType::SSLTry: {
            return QString::fromLatin1("try");
        }
        case KEMail::KEMailSSLType::SSLYes: {
            return QString::fromLatin1("yes");
        }
    }
    return QString::fromLatin1("try");
}

static KEMail::KEMailSSLType outServerSSLToEnum(const QString &value)
{
    if (value == QLatin1String("yes")) {
        return KEMail::KEMailSSLType::SSLYes;
    } else if (value == QLatin1String("no")) {
        return KEMail::KEMailSSLType::SSLNo;
    }
    return KEMail::KEMailSSLType::SSLTry;
}

class KEMailPrivate
{
public:
    KEMailPrivate();
    ~KEMailPrivate();

    static QByteArray makeData(const QString &from, const QStringList &to,
                               const QString &subject, const QString &message,
                               const KUrl::List &attach);

    static size_t curlReadCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

    CURL* m_curl;
    struct curl_slist *m_curlrcpt;
    KUrl m_server;
    KEMail::KEMailSSLType m_ssl;
    QString m_user;
    QString m_password;
    QString m_from;
    QByteArray m_data;
    QString m_errorstring;
};

KEMailPrivate::KEMailPrivate()
    : m_curl(nullptr),
    m_curlrcpt(nullptr),
    m_ssl(KEMail::SSLTry)
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
QByteArray KEMailPrivate::makeData(const QString &from, const QStringList &to,
                                   const QString &subject, const QString &message,
                                   const KUrl::List &attach)
{
    QByteArray subjectbytes("Subject: ");
    subjectbytes.append(subject.toAscii());

    QByteArray frombytes("From: ");
    frombytes.append(from.toAscii());

    QByteArray toandccbytes("To: ");
    toandccbytes.append(to.at(0).toAscii());

    if (to.size() > 1) {
        toandccbytes.append("\nCc: ");
        for (int i = 1; i < to.size(); i++) {
            if (i != 1) {
                toandccbytes.append(',');
            }
            toandccbytes.append(to.at(i).toAscii());
        }
    }

    if (!attach.isEmpty()) {
        // TODO:
        kWarning() << "Attachments not implemented";
    }

    QByteArray result = subjectbytes;
    result.append("\n");
    result.append(frombytes);
    result.append("\n");
    result.append(toandccbytes);
    result.append("\n\n");
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
    setSSL(outServerSSLToEnum(kemailsettings.getSetting(KEMailSettings::OutServerSSL)));
    setFrom(kemailsettings.getSetting(KEMailSettings::EmailAddress));
}

KEMail::~KEMail()
{
    KEMailSettings kemailsettings;
    kemailsettings.setSetting(KEMailSettings::OutServer, server().url());
    kemailsettings.setSetting(KEMailSettings::OutServerSSL, enumToOutServerSSL(ssl()));
    kemailsettings.setSetting(KEMailSettings::EmailAddress, from());

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

KEMail::KEMailSSLType KEMail::ssl() const
{
    return d->m_ssl;
}

bool KEMail::setSSL(const KEMailSSLType ssl)
{
    if (ssl < KEMailSSLType::SSLNo || ssl > KEMailSSLType::SSLYes) {
        d->m_errorstring = i18n("Invalid SSL type: %1", int(ssl));
        return false;
    }
    d->m_ssl = ssl;
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

bool KEMail::send(const QStringList &to, const QString &subject, const QString &message, const KUrl::List &attach)
{
    d->m_errorstring.clear();
    if (to.isEmpty()) {
        d->m_errorstring = i18n("Invalid receivers: %1", to.join(QLatin1String(", ")));
        return false;
    } else if (subject.isEmpty()) {
        d->m_errorstring = i18n("Invalid subject: %1", subject);
        return false;
    } else if (message.isEmpty()) {
        d->m_errorstring = i18n("Invalid message: %1", message);
        return false;
    }

    if (d->m_curlrcpt) {
        curl_slist_free_all(d->m_curlrcpt);
        d->m_curlrcpt = nullptr;
    }

    d->m_data = KEMailPrivate::makeData(d->m_from, to, subject, message, attach);
    d->m_curl = curl_easy_init();
    if (!d->m_curl) {
        d->m_errorstring = i18n("Could not create context");
        return false;
    }

    const QByteArray serverbytes = d->m_server.url().toAscii();
    const QByteArray userbytes = d->m_user.toAscii();
    const QByteArray passwordbytes = d->m_password.toAscii();
    const QByteArray frombytes = d->m_from.toAscii();

    CURLcode curlresult = curl_easy_setopt(d->m_curl, CURLOPT_URL, serverbytes.constData());
    if (curlresult != CURLE_OK) {
        d->m_errorstring = curl_easy_strerror(curlresult);
        kWarning() << d->m_errorstring;
        curl_easy_cleanup(d->m_curl);
        d->m_curl = nullptr;
        return false;
    }

    switch (d->m_ssl) {
        case KEMailSSLType::SSLNo: {
            curlresult = curl_easy_setopt(d->m_curl, CURLOPT_USE_SSL, (long)CURLUSESSL_NONE);
            break;
        }
        case KEMailSSLType::SSLTry: {
            curlresult = curl_easy_setopt(d->m_curl, CURLOPT_USE_SSL, (long)CURLUSESSL_TRY);
            break;
        }
        case KEMailSSLType::SSLYes: {
            curlresult = curl_easy_setopt(d->m_curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
            break;
        }
    }
    if (curlresult != CURLE_OK) {
        d->m_errorstring = curl_easy_strerror(curlresult);
        kWarning() << d->m_errorstring;
        curl_easy_cleanup(d->m_curl);
        d->m_curl = nullptr;
        return false;
    }

    curlresult = curl_easy_setopt(d->m_curl, CURLOPT_USERNAME, userbytes.constData());
    if (curlresult != CURLE_OK) {
        d->m_errorstring = curl_easy_strerror(curlresult);
        kWarning() << d->m_errorstring;
        curl_easy_cleanup(d->m_curl);
        d->m_curl = nullptr;
        return false;
    }

    curlresult = curl_easy_setopt(d->m_curl, CURLOPT_PASSWORD, passwordbytes.constData());
    if (curlresult != CURLE_OK) {
        d->m_errorstring = curl_easy_strerror(curlresult);
        kWarning() << d->m_errorstring;
        curl_easy_cleanup(d->m_curl);
        d->m_curl = nullptr;
        return false;
    }

    // TODO: XOAUTH2 option and add setting to KEMailSettings
    // (void)curl_easy_setopt(d->m_curl, CURLOPT_XOAUTH2_BEARER, "");
    curlresult = curl_easy_setopt(d->m_curl, CURLOPT_LOGIN_OPTIONS, "AUTH=PLAIN");
    if (curlresult != CURLE_OK) {
        d->m_errorstring = curl_easy_strerror(curlresult);
        kWarning() << d->m_errorstring;
        curl_easy_cleanup(d->m_curl);
        d->m_curl = nullptr;
        return false;
    }

    curlresult = curl_easy_setopt(d->m_curl, CURLOPT_MAIL_FROM, frombytes.constData());
    if (curlresult != CURLE_OK) {
        d->m_errorstring = curl_easy_strerror(curlresult);
        kWarning() << d->m_errorstring;
        curl_easy_cleanup(d->m_curl);
        d->m_curl = nullptr;
        return false;
    }

    foreach (const QString &it, to) {
        const QByteArray tobytes = it.toAscii();
        d->m_curlrcpt = curl_slist_append(d->m_curlrcpt, tobytes.constData());
    }
    curlresult = curl_easy_setopt(d->m_curl, CURLOPT_MAIL_RCPT, d->m_curlrcpt);
    if (curlresult != CURLE_OK) {
        d->m_errorstring = curl_easy_strerror(curlresult);
        kWarning() << d->m_errorstring;
        curl_slist_free_all(d->m_curlrcpt);
        d->m_curlrcpt = nullptr;
        curl_easy_cleanup(d->m_curl);
        d->m_curl = nullptr;
        return false;
    }

    (void)curl_easy_setopt(d->m_curl, CURLOPT_READFUNCTION, KEMailPrivate::curlReadCallback);
    (void)curl_easy_setopt(d->m_curl, CURLOPT_READDATA, d);
    (void)curl_easy_setopt(d->m_curl, CURLOPT_UPLOAD, 1L);
    // curl_easy_setopt(d->m_curl, CURLOPT_VERBOSE, 1L);

    bool result = true;
    curlresult = curl_easy_perform(d->m_curl);
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
