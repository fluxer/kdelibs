/*
 * Copyright (c) 2000 Alex Zepeda <zipzippy@sonic.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "kemailsettings.h"
#include "kemail.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kpasswdstore.h"
#include "klocale.h"
#include "kdebug.h"

class KEMailSettingsPrivate
{
public:
    KEMailSettingsPrivate();
    ~KEMailSettingsPrivate();

    KConfig *m_config;
    KPasswdStore *m_store;
};

KEMailSettingsPrivate::KEMailSettingsPrivate()
    : m_config(nullptr),
    m_store(nullptr)
{
    m_config = new KConfig("emaildefaults");
    m_store = new KPasswdStore();
    m_store->setStoreID(QString::fromLatin1("KEMailSettings"));
}

KEMailSettingsPrivate::~KEMailSettingsPrivate()
{
    delete m_store;
    delete m_config;
}

QString KEMailSettings::getSetting(KEMailSettings::Setting setting) const
{
    KConfigGroup cg(d->m_config, QString::fromLatin1("General"));
    switch (setting) {
        case ClientProgram: {
            return cg.readEntry("EmailClient");
        }
        case ClientTerminal: {
            return cg.readEntry("TerminalClient", QVariant(false)).toString();
        }
        case RealName: {
            return cg.readEntry("FullName");
        }
        case EmailAddress: {
            return cg.readEntry("EmailAddress");
        }
        case Organization: {
            return cg.readEntry("Organization");
        }
        case OutServer: {
            return cg.readEntry("OutgoingServer");
        }
        case OutServerSSL: {
            // NOTE: same default as KEMail
            return cg.readEntry("OutgoingServerSSL", QString::fromLatin1("try"));
        }
        case OutServerLogin: {
            return d->m_store->getPasswd(KPasswdStore::makeKey("OutgoingUserName"));
        }
        case OutServerPass: {
            return d->m_store->getPasswd(KPasswdStore::makeKey("OutgoingPassword"));
        }
        case OutServerOAuth: {
            return d->m_store->getPasswd(KPasswdStore::makeKey("OutgoingOAuth"));
        }
    };
    return QString();
}
void KEMailSettings::setSetting(KEMailSettings::Setting setting, const QString &value)
{
    KConfigGroup cg(d->m_config, QString::fromLatin1("General"));
    switch (setting) {
        case ClientProgram: {
            cg.writePathEntry("EmailClient", value);
            break;
        }
        case ClientTerminal: {
            cg.writeEntry("TerminalClient", (value == QLatin1String("true")));
            break;
        }
        case RealName: {
            cg.writeEntry("FullName", value);
            break;
        }
        case EmailAddress: {
            cg.writeEntry("EmailAddress", value);
            break;
        }
        case Organization: {
            cg.writeEntry("Organization", value);
            break;
        }
        case OutServer: {
            cg.writeEntry("OutgoingServer", value);
            break;
        }
        case OutServerSSL: {
            cg.writeEntry("OutgoingServerSSL", value);
            break;
        }
        case OutServerLogin: {
            d->m_store->storePasswd(KPasswdStore::makeKey("OutgoingUserName"), value);
            break;
        }
        case OutServerPass: {
            d->m_store->storePasswd(KPasswdStore::makeKey("OutgoingPassword"), value);
            break;
        }
        case OutServerOAuth: {
            d->m_store->storePasswd(KPasswdStore::makeKey("OutgoingOAuth"), value);
            break;
        }
    };
    cg.sync();
}

KEMailSettings::KEMailSettings()
    : d(new KEMailSettingsPrivate())
{
}

KEMailSettings::~KEMailSettings()
{
    delete d;
}
