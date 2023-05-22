/* This file is part of the KDE libraries
  Copyright (C) 2004, 2010 Joseph Wenninger <jowenn@kde.org>

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

#include "templateinterface.h"
#include "document.h"
#include "view.h"

#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kcalendarsystem.h>
#include <kemailsettings.h>
#include <kuser.h>
#include <kdebug.h>

#include <QString>
#include <QDateTime>
#include <QRegExp>
#include <QHostInfo>

#define DUMMY_VALUE "!KTE:TEMPLATEHANDLER_DUMMY_VALUE!"

using namespace KTextEditor;

bool TemplateInterface::insertTemplateText(const Cursor& insertPosition, const QString &templateString, const QMap<QString, QString> &initialValues)
{
  // NOTE: THE IMPLEMENTATION WILL HANDLE cursor AND selection

  QDateTime datetime = QDateTime::currentDateTime();
  QDate date = datetime.date();
  QTime time = datetime.time();
  QWidget *parentWindow = dynamic_cast<QWidget*>(this);

  QMap<QString, QString> enhancedInitValues(initialValues);
  if (templateString.contains(QLatin1String("%{loginname}"))) {
    enhancedInitValues["loginname"] = KUser().loginName();
  }

  if (templateString.contains(QLatin1String("%{fullname}"))) {
    KEMailSettings mailsettings;
    const QString fullname = mailsettings.getSetting(KEMailSettings::RealName);
    if (fullname.isEmpty()) {
      KMessageBox::sorry(parentWindow,i18n("The template needs information about you but it is not available.\n The information can be set set from system settings."));
      return false;
    }
    enhancedInitValues["fullname"] = fullname;
  }

  if (templateString.contains(QLatin1String("%{email}"))) {
    KEMailSettings mailsettings;
    const QString email = mailsettings.getSetting(KEMailSettings::EmailAddress);
    if (email.isEmpty()) {
      KMessageBox::sorry(parentWindow,i18n("The template needs information about you but it is not available.\n The information can be set set from system settings."));
      return false;
    }
    enhancedInitValues["email"] = email;
  }

  if (templateString.contains(QLatin1String("%{date}"))) {
    enhancedInitValues["date"] = KGlobal::locale()->formatDate(date, KLocale::ShortDate);
  }

  if (templateString.contains(QLatin1String("%{time}"))) {
    enhancedInitValues["time"] = KGlobal::locale()->formatTime(time, true, false);
  }

  if (templateString.contains(QLatin1String("%{year}"))) {
    enhancedInitValues["year"] = KGlobal::locale()->calendar()->formatDate(date, KLocale::Year, KLocale::LongNumber);
  }

  if (templateString.contains(QLatin1String("%{month}"))) {
    enhancedInitValues["month"] = QString::number(KGlobal::locale()->calendar()->month(date));
  }

  if (templateString.contains(QLatin1String("%{day}"))) {
    enhancedInitValues["day"] = QString::number(KGlobal::locale()->calendar()->day(date));
  }

  if (templateString.contains(QLatin1String("%{hostname}"))) {
    enhancedInitValues["hostname"] = QHostInfo::localHostName();
  }

  return insertTemplateTextImplementation(insertPosition, templateString, enhancedInitValues);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
