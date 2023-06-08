/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "kspeller.h"
#include "kconfiggroup.h"
#include "kdebug.h"

#include <QLocale>

#include <enchant.h>

class KSpellerPrivate
{
public:
    KSpellerPrivate(KConfig *config);
    ~KSpellerPrivate();

    static void dictsCallback(const char* const lang_tag,
                              const char* const provider_name,
                              const char* const provider_desc,
                              const char* const provider_file,
                              void* user_data);

    KConfig* config;
    QString dictionary;
    QStringList dictionaries;
    QStringList personalwords;

    EnchantBroker* enchantbroker;
    EnchantDict* enchantdict;
};

KSpellerPrivate::KSpellerPrivate(KConfig *kconfig)
    : config(kconfig),
    enchantbroker(nullptr),
    enchantdict(nullptr)
{
    enchantbroker = enchant_broker_init();
    if (!enchantbroker) {
        kWarning() << "Could not create broker";
        return;
    }
    enchant_broker_list_dicts(
        enchantbroker,
        KSpellerPrivate::dictsCallback,
        this
    );
}

KSpellerPrivate::~KSpellerPrivate()
{
    if (enchantbroker) {
        if (enchantdict) {
            enchant_broker_free_dict(enchantbroker, enchantdict);
        }
        enchant_broker_free(enchantbroker);
    }
}

void KSpellerPrivate::dictsCallback(const char* const lang_tag,
                                  const char* const provider_name,
                                  const char* const provider_desc,
                                  const char* const provider_file,
                                  void* user_data)
{
    // qDebug() << Q_FUNC_INFO << lang_tag << provider_name << provider_desc << provider_file;
    Q_UNUSED(provider_name);
    Q_UNUSED(provider_desc);
    Q_UNUSED(provider_file);
    KSpellerPrivate* kspellprivate = static_cast<KSpellerPrivate*>(user_data);
    Q_ASSERT(kspellprivate);
    kspellprivate->dictionaries.append(QString::fromLatin1(lang_tag));
}

KSpeller::KSpeller(KConfig *config, QObject *parent)
    : QObject(parent),
    d(new KSpellerPrivate(config))
{
    if (!config) {
        setDictionary(KSpeller::defaultLanguage());
        kWarning() << "Null config passed";
        return;
    }
    KConfigGroup spellgroup = config->group("Spelling");
    setDictionary(spellgroup.readEntry("defaultLanguage", KSpeller::defaultLanguage()));
    const QStringList personalWords = spellgroup.readEntry("personalWords", QStringList());
    foreach (const QString &word, personalWords) {
        addToPersonal(word);
    }
}

KSpeller::~KSpeller()
{
    if (d->config) {
        KConfigGroup spellgroup = d->config->group("Spelling");
        spellgroup.writeEntry("personalWords", d->personalwords);
    }
    delete d;
}

QString KSpeller::dictionary() const
{
    return d->dictionary;
}

QStringList KSpeller::dictionaries() const
{
    return d->dictionaries;
}

bool KSpeller::setDictionary(const QString &dictionary)
{
    if (!d->enchantbroker || dictionary.isEmpty()) {
        return false;
    }
    if (d->enchantdict) {
        enchant_broker_free_dict(d->enchantbroker, d->enchantdict);
        d->enchantdict = nullptr;
    }
    const QByteArray dictionarybytes = dictionary.toUtf8();
    d->enchantdict = enchant_broker_request_dict(d->enchantbroker, dictionarybytes.constData());
    if (!d->enchantdict) {
        kWarning() << "Could not create dictionary" << enchant_broker_get_error(d->enchantbroker);
        return false;
    }
    d->dictionary = dictionary;
    return true;
}

bool KSpeller::check(const QString &word)
{
    if (!d->enchantdict) {
        return true;
    }
    const QByteArray wordbytes = word.toUtf8();
    const int enchantresult = enchant_dict_check(
        d->enchantdict,
        wordbytes.constData(), wordbytes.size()
    );
    if (enchantresult < 0) {
        kWarning() << "Could not check word" << enchant_dict_get_error(d->enchantdict);
        return true;
    }
    return (enchantresult == 0);
}

QStringList KSpeller::suggest(const QString &word)
{
    QStringList result;
    if (!d->enchantdict) {
        return result;
    }
    const QByteArray wordbytes = word.toUtf8();
    size_t enchantsuggestions = 0;
    char **enchantwords = enchant_dict_suggest(
        d->enchantdict,
        wordbytes.constData(), wordbytes.size(),
        &enchantsuggestions
    );
    for (size_t i = 0; i < enchantsuggestions; i++) {
        result.append(QString::fromUtf8(enchantwords[i]));
        ::free(enchantwords[i]);
    }
    return result;
}

bool KSpeller::addToPersonal(const QString &word)
{
    if (!d->enchantdict) {
        return false;
    }
    const QByteArray wordbytes = word.toUtf8();
    enchant_dict_add(
        d->enchantdict,
        wordbytes.constData(), wordbytes.size()
    );
    d->personalwords.append(word);
    return true;
}

bool KSpeller::removeFromPersonal(const QString &word)
{
    if (!d->enchantdict) {
        return false;
    }
    const QByteArray wordbytes = word.toUtf8();
    enchant_dict_remove(
        d->enchantdict,
        wordbytes.constData(), wordbytes.size()
    );
    d->personalwords.removeAll(word);
    return true;
}
bool KSpeller::addToSession(const QString &word)
{
    if (!d->enchantdict) {
        return false;
    }
    const QByteArray wordbytes = word.toUtf8();
    enchant_dict_add_to_session(
        d->enchantdict,
        wordbytes.constData(), wordbytes.size()
    );
    return true;
}

bool KSpeller::removeFromSession(const QString &word)
{
    if (!d->enchantdict) {
        return false;
    }
    const QByteArray wordbytes = word.toUtf8();
    enchant_dict_remove_from_session(
        d->enchantdict,
        wordbytes.constData(), wordbytes.size()
    );
    return true;
}

QString KSpeller::defaultLanguage()
{
    // NOTE: enchant_get_user_language() just returns the value of some environment variable used
    // to specify localizations language and C.UTF-8 is not valid dictionary for example
    QString result = QLocale::system().name();
    if (result == QLatin1String("C")) {
        result = QLatin1String("en");
    }
    // qDebug() << Q_FUNC_INFO << result;
    return result;
}

#include "moc_kspeller.cpp"
