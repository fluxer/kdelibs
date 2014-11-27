/* This file is part of the KDE libraries
   Copyright (C) 2007 Urs Wolfer <uwolfer at kde.org>

   Parts of this class have been take from the KAboutKDE class, which was
   Copyright (C) 2000 Espen Sand <espen@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kaboutkdedialog_p.h"

#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QTabWidget>

#include <kdeversion.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktitlewidget.h>

namespace KDEPrivate {


KAboutKdeDialog::KAboutKdeDialog(QWidget *parent)
  : KDialog(parent),
    d( 0 )
{
    setPlainCaption(i18n("About Katana"));
    setButtons(KDialog::Close);

    KTitleWidget *titleWidget = new KTitleWidget(this);
    titleWidget->setText(i18n("<html><font size=\"5\">Katana - Be Free and Be Fast!</font><br /><b>Platform Version %1</b></html>",
                         QString(KDE_VERSION_STRING)));
    titleWidget->setPixmap(KIcon("kde").pixmap(48), KTitleWidget::ImageLeft);

    QLabel *about = new QLabel;
    about->setMargin(10);
    about->setAlignment(Qt::AlignTop);
    about->setWordWrap(true);
    about->setOpenExternalLinks(true);
    about->setTextInteractionFlags(Qt::TextBrowserInteraction);
    about->setText(i18n("<html>"
        "<b>Katana</b> is fork of KDE Software Distribution with emphasis on speed."
        "<br /><br />"
        "Software can always be improved, and the Katana team is ready to do so. "
        "However, you - the user - must tell us when something does not work as "
        "expected or could be done better. You do not have to be a software developer "
        "to be a member of the Katana team. You can join the national teams that "
        "translate program interfaces. You can provide graphics, themes, sounds, and "
        "improved documentation. You decide!</html>"
        "<br /><br />"
        "Visit <a href=\"%2\">%2</a> to learn more about about Katana.</html>",
        QLatin1String("http://fluxer.github.io/katana/"),
        QLatin1String("http://fluxer.github.io/katana/")));

    QLabel *image = new QLabel;
    image->setPixmap(KStandardDirs::locate("data", "kdeui/pics/aboutkde.png"));

    QHBoxLayout *midLayout = new QHBoxLayout;
    midLayout->addWidget(image);
    midLayout->addWidget(about);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(titleWidget);
    mainLayout->addLayout(midLayout);
    mainLayout->setMargin(0);

    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);
}

}

#include "kaboutkdedialog_p.moc"
