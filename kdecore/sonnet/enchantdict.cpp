// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * Copyright 2006  Zack Rusin <zack@kde.org>
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
#include "enchantdict_p.h"
#include "enchantclient_p.h"

#include <QtCore/QTextCodec>
#include <QtCore/QDebug>

namespace Sonnet
{

QSpellEnchantDict::QSpellEnchantDict(QSpellEnchantClient *client, 
                                     EnchantDict *dict,
                                     const QString &language)
    : m_dict(dict),
      m_client(client)
{
}

QSpellEnchantDict::~QSpellEnchantDict()
{
    //Enchant caches dictionaries, so it will always return the same one.
    // therefore we do not want to delete the EnchantDict here but in the
    // client when it knows that nothing is using it anymore
    m_client->removeDictRef(m_dict);
}

bool QSpellEnchantDict::isCorrect(const QString &word) const
{
    QByteArray data = word.toUtf8();
    return !enchant_dict_check(m_dict, data.constData(), data.length());
}

QStringList QSpellEnchantDict::suggest(const QString &word) const
{
    /* Needed for Unicode conversion */
    QTextCodec *codec = QTextCodec::codecForName("utf8");

    QByteArray data = word.toUtf8();
    size_t number = 0;
    char **suggestions =
        enchant_dict_suggest(m_dict, data.constData(), data.length(),
                             &number);

    QStringList qsug;
    for (size_t i = 0; i < number; ++i) {
        qsug.append(codec->toUnicode(suggestions[i]));
    }

    if (suggestions && number)
        enchant_dict_free_string_list(m_dict, suggestions);
    return qsug;
}

bool QSpellEnchantDict::checkAndSuggest(const QString &word,
                                    QStringList &suggestions) const
{
    bool c = isCorrect(word);
    if (!c)
        suggestions = suggest(word);
    return c;
}

bool QSpellEnchantDict::storeReplacement(const QString &bad,
                                  const QString &good)
{
    QByteArray baddata = bad.toUtf8();
    QByteArray gooddata = good.toUtf8();
    enchant_dict_store_replacement(m_dict,
                                   baddata.constData(), baddata.length(),
                                   gooddata.constData(), gooddata.length());
    return true;
}

bool QSpellEnchantDict::addToPersonal(const QString &word)
{
    enchant_dict_add(m_dict, word.toUtf8(), word.toUtf8().length());
    return true;
}

bool QSpellEnchantDict::addToSession(const QString &word)
{
    enchant_dict_add_to_session(m_dict, word.toUtf8(),
                                word.toUtf8().length());
    return true;
}

QString QSpellEnchantDict::language() const
{
    return m_language;
}

}
