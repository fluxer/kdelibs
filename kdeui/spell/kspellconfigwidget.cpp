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

#include "kspellconfigwidget.h"
#include "kspeller.h"
#include "kspelldictionarycombobox.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "klocale.h"
#include "keditlistwidget.h"
#include "klineedit.h"
#include "kdebug.h"

#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>

class KSpellConfigWidgetPrivate
{
public:
    KSpellConfigWidgetPrivate(KConfig *config);

    KConfig* config;
    QCheckBox* enablebox;
    KSpellDictionaryComboBox* dictionarybox;
    KEditListWidget* wordslistedit;
};

KSpellConfigWidgetPrivate::KSpellConfigWidgetPrivate(KConfig *kconfig)
    : config(kconfig),
    enablebox(nullptr),
    dictionarybox(nullptr),
    wordslistedit(nullptr)
{
}

KSpellConfigWidget::KSpellConfigWidget(KConfig *config, QWidget *parent)
    : QWidget(parent),
    d(new KSpellConfigWidgetPrivate(config))
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox* dictionarygroup = new QGroupBox(this);
    dictionarygroup->setTitle(i18n("General"));
    QGridLayout* dictionarylayout = new QGridLayout(dictionarygroup);
    d->enablebox = new QCheckBox(this);
    d->enablebox->setText(i18n("Automatic spell checking enabled by default"));
    dictionarylayout->addWidget(d->enablebox, 0, 0, 1, 2);
    QLabel* dictionarylabel = new QLabel(i18n("Default language:"), d->dictionarybox);
    dictionarylayout->addWidget(dictionarylabel, 1, 0);
    d->dictionarybox = new KSpellDictionaryComboBox(d->dictionarybox);
    d->dictionarybox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    dictionarylayout->addWidget(d->dictionarybox, 1, 1);

    QGroupBox* wordgroup = new QGroupBox(this);
    wordgroup->setTitle(i18n("Ignored Words"));
    QGridLayout* wordlayout = new QGridLayout(wordgroup);
    d->wordslistedit = new KEditListWidget(wordgroup);
    d->wordslistedit->lineEdit()->setPlaceholderText(i18n("Type word here to add it to the personal ignore list"));
    wordlayout->addWidget(d->wordslistedit, 0, 0);

    layout->addWidget(dictionarygroup);
    layout->addWidget(wordgroup);

    connect(
        d->enablebox, SIGNAL(stateChanged(int)),
        this, SIGNAL(configChanged())
    );
    connect(
        d->dictionarybox, SIGNAL(currentIndexChanged(int)),
        this, SIGNAL(configChanged())
    );
    connect(
        d->wordslistedit, SIGNAL(changed()),
        this, SIGNAL(configChanged())
    );

    if (!config) {
        kWarning() << "Null config passed";
        return;
    }
    KConfigGroup spellgroup = config->group("Spelling");
    d->enablebox->setChecked(spellgroup.readEntry("checkerEnabledByDefault", false));
    d->dictionarybox->setCurrentByDictionary(spellgroup.readEntry("defaultLanguage", KSpeller::defaultLanguage()));
    d->wordslistedit->setItems(spellgroup.readEntry("personalWords", QStringList()));
}

KSpellConfigWidget::~KSpellConfigWidget()
{
    delete d;
}

void KSpellConfigWidget::save()
{
    if (!d->config) {
        return;
    }
    KConfigGroup spellgroup = d->config->group("Spelling");
    spellgroup.writeEntry("checkerEnabledByDefault", d->enablebox->isChecked());
    spellgroup.writeEntry("defaultLanguage", d->dictionarybox->currentDictionary());
    spellgroup.writeEntry("personalWords", d->wordslistedit->items());
}

void KSpellConfigWidget::slotDefault()
{
    if (!d->config) {
        return;
    }
    KConfigGroup spellgroup = d->config->group("Spelling");
    spellgroup.writeEntry("checkerEnabledByDefault", false);
    spellgroup.writeEntry("defaultLanguage", KSpeller::defaultLanguage());
    spellgroup.writeEntry("personalWords", QStringList());
}

#include "moc_kspellconfigwidget.cpp"
