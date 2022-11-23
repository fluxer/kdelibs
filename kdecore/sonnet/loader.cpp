// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "loader_p.h"
#include "settings_p.h"
#include "enchantclient_p.h"

#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>

namespace Sonnet
{

class Loader::Private
{
public:
    Settings *settings;

    // <language, Clients with that language >
    QMap<QString, QList<QSpellEnchantClient*> > languageClients;
    QStringList clients;

    QStringList languagesNameCache;
};

K_GLOBAL_STATIC(Loader, s_loader)

Loader *Loader::openLoader()
{
    if (s_loader.isDestroyed()) {
        return 0;
    }

    return s_loader;
}

Loader::Loader()
    :d(new Private)
{
    d->settings = new Settings(this);
    KConfig config(QString::fromLatin1("sonnetrc"));
    d->settings->restore(&config);

    QSpellEnchantClient *client = new QSpellEnchantClient(this);
    d->clients.append(client->name());

    foreach (const QString &itr, client->languages()) {
        const QList<QSpellEnchantClient*> langclient = d->languageClients[itr];
        if (!langclient.isEmpty() &&
            client->reliability() < langclient.first()->reliability()) {
            d->languageClients[itr].append(client);
        } else {
            d->languageClients[itr].prepend(client);
        }
    }
}

Loader::~Loader()
{
    //kDebug()<<"Removing loader : "<< this;
    delete d->settings; d->settings = 0;
    delete d;
}

QSpellEnchantDict *Loader::createSpeller(const QString& language,
                                         const QString& clientName) const
{
    QString plang   = language;

    if (plang.isEmpty()) {
        plang = d->settings->defaultLanguage();
    }

    const QList<QSpellEnchantClient*> lClients = d->languageClients[plang];

    if (lClients.isEmpty()) {
        kError() << "No language dictionaries for the language : " << plang;
        return 0;
    }

    QListIterator<QSpellEnchantClient*> itr(lClients);
    while (itr.hasNext()) {
        QSpellEnchantClient* item = itr.next();
        if (!clientName.isEmpty()) {
            if (clientName == item->name()) {
                return item->createSpeller(plang);
            }
        } else {
            //the first one is the one with the highest
            //reliability
            return item->createSpeller(plang);
        }
    }

    return 0;
}

QStringList Loader::clients() const
{
    return d->clients;
}

QStringList Loader::languages() const
{
    return d->languageClients.keys();
}

QString Loader::languageNameForCode(const QString &langCode) const
{
    QString currentDictionary = langCode,   // e.g. en_GB-ize-wo_accents
        lISOName,            // language ISO name
        cISOName,            // country ISO name
        variantName,         // dictionary variant name e.g. w_accents
        localizedLang,       // localized language
        localizedCountry;    // localized country
    QByteArray variantEnglish; // dictionary variant in English

    struct variantListType
    {
        const char* variantShortName;
        const char* variantEnglishName;
    };

    const variantListType variantList[] = {
        { "40", I18N_NOOP2("dictionary variant", "40") }, // what does 40 mean?
        { "60", I18N_NOOP2("dictionary variant", "60") }, // what does 60 mean?
        { "80", I18N_NOOP2("dictionary variant", "80") }, // what does 80 mean?
        { "ise", I18N_NOOP2("dictionary variant", "-ise suffixes") },
        { "ize", I18N_NOOP2("dictionary variant", "-ize suffixes") },
        { "ise-w_accents", I18N_NOOP2("dictionary variant", "-ise suffixes and with accents") },
        { "ise-wo_accents", I18N_NOOP2("dictionary variant", "-ise suffixes and without accents") },
        { "ize-w_accents", I18N_NOOP2("dictionary variant", "-ize suffixes and with accents") },
        { "ize-wo_accents", I18N_NOOP2("dictionary variant", "-ize suffixes and without accents") },
        { "lrg", I18N_NOOP2("dictionary variant", "large") },
        { "med", I18N_NOOP2("dictionary variant", "medium") },
        { "sml", I18N_NOOP2("dictionary variant", "small") },
        { "variant_0", I18N_NOOP2("dictionary variant", "variant 0") },
        { "variant_1", I18N_NOOP2("dictionary variant", "variant 1") },
        { "variant_2", I18N_NOOP2("dictionary variant", "variant 2") },
        { "wo_accents", I18N_NOOP2("dictionary variant", "without accents") },
        { "w_accents", I18N_NOOP2("dictionary variant", "with accents") },
        { "ye", I18N_NOOP2("dictionary variant", "with ye") },
        { "yeyo", I18N_NOOP2("dictionary variant", "with yeyo") },
        { "yo", I18N_NOOP2("dictionary variant", "with yo") },
        { "extended", I18N_NOOP2("dictionary variant", "extended") },
        { 0, 0 }
    };

    const int minusPos = currentDictionary.indexOf(QLatin1Char('-'));
    const int underscorePos = currentDictionary.indexOf(QLatin1Char('_'));
    if (underscorePos != -1 && underscorePos <= 3) {
        cISOName = currentDictionary.mid(underscorePos + 1, 2);
        lISOName = currentDictionary.left(underscorePos);
        if ( minusPos != -1 )
            variantName = currentDictionary.right(
                                     currentDictionary.length() - minusPos - 1);
    }  else {
        if ( minusPos != -1 ) {
            variantName = currentDictionary.right(
                                     currentDictionary.length() - minusPos - 1);
            lISOName = currentDictionary.left(minusPos);
        }
        else
            lISOName = currentDictionary;
    }
    localizedLang = KGlobal::locale()->languageCodeToName(lISOName);
    if (localizedLang.isEmpty())
        localizedLang = lISOName;
    if (!cISOName.isEmpty()) {
        if (!KGlobal::locale()->countryCodeToName(cISOName).isEmpty())
            localizedCountry = KGlobal::locale()->countryCodeToName(cISOName);
        else
            localizedCountry = cISOName;
    }
    if (!variantName.isEmpty()) {
        int variantCount = 0;
        while (variantList[variantCount].variantShortName != 0)
            if (QLatin1String(variantList[variantCount].variantShortName) == variantName)
                break;
            else
                variantCount++;
        if (variantList[variantCount].variantShortName != 0)
            variantEnglish = variantList[variantCount].variantEnglishName;
        else
            variantEnglish = variantName.toLatin1();
    }
    if (!cISOName.isEmpty() && !variantName.isEmpty())
        return i18nc(
                    "dictionary name. %1-language, %2-country and %3 variant name",
                    "%1 (%2) [%3]", localizedLang, localizedCountry,
                    i18nc( "dictionary variant", variantEnglish));
    else if (!cISOName.isEmpty())
        return i18nc(
                        "dictionary name. %1-language and %2-country name",
                        "%1 (%2)", localizedLang, localizedCountry);
    else if (!variantName.isEmpty())
        return i18nc(
                            "dictionary name. %1-language and %2-variant name",
                            "%1 [%2]", localizedLang,
                            i18nc("dictionary variant", variantEnglish));
    else
        return localizedLang;
}

QStringList Loader::languageNames() const
{
    /* For whatever reason languages() might change. So,
     * to be in sync with it let's do the following check.
     */
    if (d->languagesNameCache.count() == languages().count() )
        return d->languagesNameCache;

    QStringList allLocalizedDictionaries;
    const QStringList allDictionaries = languages();

    for (QStringList::ConstIterator it = allDictionaries.begin();
         it != allDictionaries.end(); ++it) {
        allLocalizedDictionaries.append(languageNameForCode(*it));
    }
    // cache the list
    d->languagesNameCache = allLocalizedDictionaries;
    return allLocalizedDictionaries;
}

Settings* Loader::settings() const
{
    return d->settings;
}

void Loader::changed()
{
    emit configurationChanged();
}

}

#include "moc_loader_p.cpp"
