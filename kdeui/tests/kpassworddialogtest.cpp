/* This file is part of the KDE libraries
    Copyright (C) 2007 Olivier Goffart  <ogoffart at kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kpassworddialog.h>
#include <klocale.h>
#include <iostream>

int main( int argc, char *argv[] )
{
    KAboutData about("KNewPasswordDialogTest", 0, ki18n("KNewPasswordDialogTest"), "1");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication a;

    //step 1  simple password
    {
        KPasswordDialog dlg(0, KPasswordDialog::ShowKeepPassword);
        dlg.setPrompt(i18n("This is a long prompt line. It is important it to be long so we can test the dialog does not get broken because of multiline labels. Please enter a password:"));
        dlg.addCommentLine(i18n("This is a rather large left comment line") , i18n("Right part of the comment line has to be long too so be test the layouting works really ok. Please visit http://www.kde.org"));
        
        if( dlg.exec() )
        {
            std::cout << "Entered password: " << (const char*)dlg.password().toLatin1() << std::endl;
        }
        else
        {
            std::cout << "No password" << std::endl;
            return -1;
        }
    }
    
    //step 2 readonly username
    {
        KPasswordDialog dlg(0, KPasswordDialog::ShowUsernameLine | KPasswordDialog::UsernameReadOnly);
        dlg.setPrompt(i18n("Enter a password for the test"));
        dlg.setUsername("konqui");
        dlg.addCommentLine( i18n("Site") , i18n("http://www.kde.org") );
        
        if( dlg.exec() )
        {
            std::cout << "Entered password: " << (const char*)dlg.password().toLatin1() << std::endl;
        }
        else
        {
            std::cout << "No password" << std::endl;
            return -1;
        }
    }

    
    //step 3 with some username preset
    {
        KPasswordDialog dlg(0, KPasswordDialog::ShowUsernameLine);
        dlg.setPrompt(i18n("Enter a password for the test"));
        QMap<QString,QString> logins;
        logins.insert("konqui" , "foo");
        logins.insert("watson" , "bar");
        logins.insert("ogoffart" , "");
        
        dlg.setKnownLogins(logins);

        if( dlg.exec() )
        {
            std::cout << "Entered password: " << (const char*)dlg.password().toLatin1() << " for username " <<  (const  char*)dlg.username().toAscii() <<std::endl;
        }
        else
        {
            std::cout << "No password" << std::endl;
            return -1;
        }
    }

    
    return 0;
    
    

}

