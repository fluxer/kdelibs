/*
    This file is part of the KDE File Manager

    Copyright (C) 1998- Waldo Bastian (bastian@kde.org)
    Copyright (C) 2000- Dawit Alemayehu (adawit@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//----------------------------------------------------------------------------
//
// KDE File Manager -- HTTP Cookie Dialogs

#ifndef KCOOKIEWIN_H
#define KCOOKIEWIN_H

#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>
#include <kdialog.h>
#include "kcookiejar.h"

class KLineEdit;
#include <QPushButton>

class KCookieDetail : public QGroupBox
{
    Q_OBJECT

public:
    KCookieDetail(KHttpCookieList cookieList, int cookieCount, QWidget *parent=0);
    ~KCookieDetail();


private Q_SLOTS:
    void slotNextCookie();

private:
    void displayCookieDetails();

    KLineEdit*   m_name;
    KLineEdit*   m_value;
    KLineEdit*   m_expires;
    KLineEdit*   m_domain;
    KLineEdit*   m_path;
    KLineEdit*   m_secure;

    KHttpCookieList m_cookieList;
    int m_cookieNumber;
};

class KCookieWin : public KDialog
{
    Q_OBJECT

public :
    KCookieWin( QWidget *parent, KHttpCookieList cookieList, int defaultButton=0,
                bool showDetails=false );
    ~KCookieWin();

    KCookieAdvice advice( KCookieJar *cookiejar, const KHttpCookie& cookie );

protected:
  virtual void slotButtonClicked(int button);

private :
    QRadioButton* m_onlyCookies, *m_allCookies, *m_allCookiesDomain;
    KCookieDetail* m_detailView;
};

#endif
