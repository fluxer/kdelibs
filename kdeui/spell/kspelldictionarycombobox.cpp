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

#include "kspelldictionarycombobox.h"
#include "kspeller.h"
#include "kglobal.h"
#include "klocale.h"
#include "kdebug.h"

KSpellDictionaryComboBox::KSpellDictionaryComboBox(QWidget *parent)
    : KComboBox(parent)
{
    const QStringList dictionaries = KSpeller(KGlobal::config().data()).dictionaries();
    foreach (const QString &dictionary, dictionaries) {
        const int underscoreindex = dictionary.indexOf(QLatin1Char('_'));
        if (underscoreindex > 0) {
            const QString language = dictionary.mid(0, underscoreindex);
            const QString country = dictionary.mid(underscoreindex + 1, dictionary.size() - underscoreindex - 1);
            // qDebug() << Q_FUNC_INFO << dictionary << language << country;
            const QString dictionarytext = QString::fromLatin1("%1 (%2)")
                         .arg(KGlobal::locale()->languageCodeToName(language))
                         .arg(KGlobal::locale()->countryCodeToName(country));
            KComboBox::addItem(dictionarytext, dictionary);
        } else {
            KComboBox::addItem(KGlobal::locale()->languageCodeToName(dictionary), dictionary);
        }
    }
    connect(
        this, SIGNAL(currentIndexChanged(int)),
        this, SLOT(_dictionaryChanged(int))
    );
    connect(
        this, SIGNAL(currentIndexChanged(QString)),
        this, SIGNAL(dictionaryNameChanged(QString))
    );
}

QString KSpellDictionaryComboBox::currentDictionary() const
{
    return KComboBox::itemData(KComboBox::currentIndex()).toString();
}

void KSpellDictionaryComboBox::setCurrentByDictionary(const QString &dictionary)
{
    const int dictionaryindex = KComboBox::findData(dictionary);
    if (dictionaryindex >= 0) {
        KComboBox::setCurrentIndex(dictionaryindex);
    }
}

void KSpellDictionaryComboBox::_dictionaryChanged(const int index)
{
    emit dictionaryChanged(KComboBox::itemData(index).toString());
}

#include "moc_kspelldictionarycombobox.cpp"
