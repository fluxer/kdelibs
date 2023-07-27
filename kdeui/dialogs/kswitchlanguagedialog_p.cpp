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

#include "kswitchlanguagedialog_p.h"

#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <QApplication>
#include <QEvent>

static void languageChanged(KSwitchLanguageDialog *dialog)
{
    KMessageBox::information(
        dialog,
        i18n("The language for this application has been changed. The change will take effect the next time the application is started."), // text
        i18n("Application Language Changed"), // caption
        "ApplicationLanguageChangedWarning" // dontShowAgainName
    );

    QEvent ev(QEvent::LanguageChange);
    QApplication::sendEvent(qApp, &ev);
}

KSwitchLanguageDialog::KSwitchLanguageDialog(QWidget *parent)
    : KDialog(parent),
    m_dialogwidget(nullptr),
    m_dialoglayout(nullptr),
    m_languagelabel(nullptr),
    m_languageedit(nullptr)
{
    setCaption(i18n("Switch Application Language"));
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);
    setDefaultButton(Ok);

    connect(this, SIGNAL(okClicked()), SLOT(slotOk()));
    connect(this, SIGNAL(defaultClicked()), SLOT(slotDefault()));

    m_dialogwidget = new QWidget(this);
    m_dialoglayout = new QVBoxLayout(m_dialogwidget);
    m_languagelabel = new QLabel(m_dialogwidget);
    m_languagelabel->setText(i18n("Please choose the language which should be used for this application:"));
    m_dialoglayout->addWidget(m_languagelabel);
    m_languageedit = new KEditListWidget(m_dialogwidget);
    m_dialoglayout->addWidget(m_languageedit);
    setMainWidget(m_dialogwidget);
}

KSwitchLanguageDialog::~KSwitchLanguageDialog()
{
}

void KSwitchLanguageDialog::slotOk()
{
    KConfigGroup localegroup(KGlobal::config(), "Locale");
    const QStringList oldtranslations = localegroup.readEntry("Translations", QStringList());

    const QStringList newtranslations = QStringList(); // TODO:
    if (oldtranslations != newtranslations) {
        localegroup.writeEntry("Translations", newtranslations);
        localegroup.sync();

        languageChanged(this);
    }

    accept();
}

void KSwitchLanguageDialog::slotDefault()
{
    KConfigGroup localegroup(KGlobal::config(), "Locale");
    const QStringList oldtranslations = localegroup.readEntry("Translations", QStringList());
    localegroup.revertToDefault("Translations");
    localegroup.sync();

    const QStringList newtranslations = localegroup.readEntry("Translations", QStringList());
    if (oldtranslations != newtranslations) {
        languageChanged(this);
    }

    accept();
}

#include "moc_kswitchlanguagedialog_p.cpp"
