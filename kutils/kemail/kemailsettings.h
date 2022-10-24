/*-
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
 *
 */

#ifndef KEMAILSETTINGS_H
#define KEMAILSETTINGS_H

#include <QtCore/QString>

#include <kemail_export.h>

class KEMailSettingsPrivate;

/**
  * This is just a small class to facilitate accessing e-mail settings in 
  * a sane way, and allowing any program to manage multiple e-mail 
  * profiles effortlessly
  *
  * @see KEmail
  * @see KEmailDialog
  *
  * @author Alex Zepeda zipzippy@sonic.net
  **/
class KEMAIL_EXPORT KEMailSettings {
public:
    /**
     * The list of settings that I thought of when I wrote this 
     * class.
     *
     * @see getSetting()
     * @see setSetting()
     **/
    enum Setting {
        ClientProgram,
        ClientTerminal,
        RealName,
        EmailAddress,
        Organization,
        OutServer,
        OutServerSSL,
        OutServerLogin,
        OutServerPass
    };

    /**
     * Default constructor, just sets things up.
     **/
    KEMailSettings();

    /**
     * Default destructor, nothing to see here.
     **/
    ~KEMailSettings();

    /**
     * Get one of the predefined "basic" settings.
     * @param setting the setting to get
     * @return the value of the setting, or QString() if not set
     **/
    QString getSetting(const KEMailSettings::Setting setting) const;

    /**
     * Set one of the predefined "basic" settings.
     * @param setting the setting to set
     * @param value the new value of the setting, or QString() to unset
     **/
    void setSetting(const KEMailSettings::Setting setting, const QString &value);

private:
    KEMailSettingsPrivate* const d;
};

#endif // KEMAILSETTINGS_H
