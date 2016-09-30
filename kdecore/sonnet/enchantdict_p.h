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
#ifndef QSPELL_ENCHANTDICT_H
#define QSPELL_ENCHANTDICT_H

#include <QStringList>
#include <enchant.h>

namespace Sonnet
{
    class QSpellEnchantClient;

    class QSpellEnchantDict
    {
    public:
        ~QSpellEnchantDict();
        bool isCorrect(const QString &word) const;

        QStringList suggest(const QString &word) const;

        bool checkAndSuggest(const QString& word,
                                     QStringList &suggestions) const;

        bool storeReplacement(const QString &bad,
                                    const QString &good);

        bool addToPersonal(const QString &word);
        bool addToSession(const QString &word);
        QString language() const;

    protected:
        friend class QSpellEnchantClient;
        QSpellEnchantDict(const QString &lang);
        QSpellEnchantDict(QSpellEnchantClient *client,
                        EnchantDict *dict,
                        const QString &language);

    private:
        EnchantDict   *m_dict;
        QSpellEnchantClient *m_client;
        QString m_language;
    };
}

#endif
