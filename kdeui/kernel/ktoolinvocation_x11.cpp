/* This file is part of the KDE libraries
    Copyright (c) 1997,1998 Matthias Kalle Dalheimer <kalle@kde.org>
    Copyright (c) 1999 Espen Sand <espen@kde.org>
    Copyright (c) 2000-2004 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Oswald Buddenhagen <ossi@kde.org>
    Copyright (c) 2006 Thiago Macieira <thiago@kde.org>
    Copyright (C) 2008 Aaron Seigo <aseigo@kde.org>

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

#include <config.h>

#include "ktoolinvocation.h"
#include "kcmdlineargs.h"
#include "kconfig.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kshell.h"
#include "kmacroexpander.h"
#include "klocale.h"
#include "kstandarddirs.h"
#include "kmessage.h"
#include "kservice.h"
#include "kconfiggroup.h"
#include "kmimetypetrader.h"
#include "kurl.h"

static QStringList splitEmailAddressList( const QString & aStr )
{
    // This is a copy of KPIM::splitEmailAddrList().
    // Features:
    // - always ignores quoted characters
    // - ignores everything (including parentheses and commas)
    //   inside quoted strings
    // - supports nested comments
    // - ignores everything (including double quotes and commas)
    //   inside comments

    QStringList list;

    if (aStr.isEmpty())
        return list;

    QString addr;
    uint addrstart = 0;
    int commentlevel = 0;
    bool insidequote = false;

    for (int index=0; index<aStr.length(); index++) {
        // the following conversion to latin1 is o.k. because
        // we can safely ignore all non-latin1 characters
        switch (aStr[index].toLatin1()) {
        case '"' : // start or end of quoted string
            if (commentlevel == 0)
                insidequote = !insidequote;
            break;
        case '(' : // start of comment
            if (!insidequote)
                commentlevel++;
            break;
        case ')' : // end of comment
            if (!insidequote) {
                if (commentlevel > 0)
                    commentlevel--;
                else {
                    // kDebug() << "Error in address splitting: Unmatched ')'";
                    return list;
                }
            }
            break;
        case '\\' : // quoted character
            index++; // ignore the quoted character
            break;
        case ',' :
            if (!insidequote && (commentlevel == 0)) {
                addr = aStr.mid(addrstart, index-addrstart);
                if (!addr.isEmpty())
                    list += addr.simplified();
                addrstart = index+1;
            }
            break;
        }
    }
    // append the last address to the list
    if (!insidequote && (commentlevel == 0)) {
        addr = aStr.mid(addrstart, aStr.length()-addrstart);
        if (!addr.isEmpty())
            list += addr.simplified();
    }
    //else
    //  kDebug() << "Error in address splitting: "
    //            << "Unexpected end of address list";

    return list;
}

void KToolInvocation::invokeMailer(const QString &to, const QString &cc,
                                   const QString &subject, const QString &body,
                                   const QStringList &attachURLs,
                                   const QByteArray &startup_id)
{
    KConfig config(QString::fromLatin1("emaildefaults"));
    KConfigGroup profileGrp(&config, "General");

    QString command = profileGrp.readPathEntry("EmailClient", QString::fromLatin1("kmail"));
    if( !command.contains( QLatin1Char('%') ))
    {
        command += QLatin1String(" %u");
    }

    if (profileGrp.readEntry("TerminalClient", false))
    {
        KConfigGroup confGroup( KGlobal::config(), "General" );
        QString preferredTerminal = confGroup.readPathEntry("TerminalApplication", QString::fromLatin1("konsole"));
        command = preferredTerminal + QString::fromLatin1(" -e ") + command;
    }

    QStringList cmdTokens = KShell::splitArgs(command);
    QString cmd = cmdTokens.takeFirst();

    KUrl url;
    //QStringList qry;
    if (!to.isEmpty())
    {
        QStringList tos = splitEmailAddressList( to );
        url.setPath( tos.first() );
        tos.erase( tos.begin() );
        for (QStringList::ConstIterator it = tos.constBegin(); it != tos.constEnd(); ++it)
            url.addQueryItem(QString::fromLatin1("to"), *it);
        //qry.append( "to=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    }
    const QStringList ccs = splitEmailAddressList( cc );
    for (QStringList::ConstIterator it = ccs.constBegin(); it != ccs.constEnd(); ++it)
        url.addQueryItem(QString::fromLatin1("cc"), *it);
    //qry.append( "cc=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    for (QStringList::ConstIterator it = attachURLs.constBegin(); it != attachURLs.constEnd(); ++it)
        url.addQueryItem(QString::fromLatin1("attach"), *it);
    //qry.append( "attach=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    if (!subject.isEmpty())
        url.addQueryItem(QString::fromLatin1("subject"), subject);
    //qry.append( "subject=" + QLatin1String(KUrl::toPercentEncoding( subject ) ));
    if (!body.isEmpty())
        url.addQueryItem(QString::fromLatin1("body"), body);
    //qry.append( "body=" + QLatin1String(KUrl::toPercentEncoding( body ) ));
    //url.setQuery( qry.join( "&" ) );

    if ( ! (to.isEmpty() && (!url.hasQuery())) )
        url.setScheme(QString::fromLatin1("mailto"));

    QHash<QChar, QString> keyMap;
    keyMap.insert(QLatin1Char('t'), to);
    keyMap.insert(QLatin1Char('s'), subject);
    keyMap.insert(QLatin1Char('c'), cc);
    keyMap.insert(QLatin1Char('B'), body);
    keyMap.insert(QLatin1Char('u'), url.url());

    QString attachlist = attachURLs.join(QString::fromLatin1(","));
    attachlist.prepend(QLatin1Char('\''));
    attachlist.append(QLatin1Char('\''));
    keyMap.insert(QLatin1Char('A'), attachlist);

    for (QStringList::Iterator it = cmdTokens.begin(); it != cmdTokens.end(); )
    {
        if (*it == QLatin1String("%A"))
        {
            if (it == cmdTokens.begin()) // better safe than sorry ...
                continue;
            QStringList::ConstIterator urlit = attachURLs.begin();
            QStringList::ConstIterator urlend = attachURLs.end();
            if ( urlit != urlend )
            {
                QStringList::Iterator previt = it;
                --previt;
                *it = *urlit;
                ++it;
                while ( ++urlit != urlend )
                {
                    cmdTokens.insert( it, *previt );
                    cmdTokens.insert( it, *urlit );
                }
            } else {
                --it;
                it = cmdTokens.erase( cmdTokens.erase( it ) );
            }
        } else {
            *it = KMacroExpander::expandMacros(*it, keyMap);
            ++it;
        }
    }

    QString error;
    // TODO this should check if cmd has a .desktop file, and use data from it, together
    // with sending more ASN data
    if (kdeinitExec(cmd, cmdTokens, &error, startup_id) != 0) {
        KMessage::message(
            KMessage::Error,
            i18n("Could not launch the mail client:\n\n%1", error),
            i18n("Could not launch Mail Client")
        );
    }
}

void KToolInvocation::invokeBrowser(const QString &url, const QByteArray& startup_id)
{
    QStringList args;
    args << url;
    QString error;

    // This method should launch a webbrowser, preferably without doing a mimetype
    // check first, like KRun (i.e. kde-open) would do.
    const KService::Ptr htmlApp = KMimeTypeTrader::self()->preferredService(QLatin1String("text/html"));
    if (htmlApp) {
        QString error;
        const int err = startServiceByDesktopPath(htmlApp->entryPath(), url, &error, startup_id);
        if (err != 0) {
            KMessage::message(
                KMessage::Error,
                // TODO: i18n("Could not launch %1:\n\n%2", htmlApp->exec(), error),
                i18n("Could not launch the browser:\n\n%1", error),
                i18n("Could not launch Browser"));
        }
        return;
    }

    QString exe = KStandardDirs::findExe(QString::fromLatin1("kde-open"));
    if (exe.isEmpty()) {
        exe = KStandardDirs::findExe(QString::fromLatin1("xdg-open"));
    }

    if (kdeinitExec(exe, args, &error, startup_id) != 0) {
        KMessage::message(
            KMessage::Error,
            // TODO: i18n("Could not launch %1:\n\n%2", exe, error),
            i18n("Could not launch the browser:\n\n%1", error),
            i18n("Could not launch Browser")
        );
    }
}

void KToolInvocation::invokeTerminal(const QString &command,
                                     const QString &workdir,
                                     const QByteArray &startup_id)
{
    KConfigGroup confGroup( KGlobal::config(), "General" );
    QString exec = confGroup.readPathEntry("TerminalApplication", QString::fromLatin1("konsole"));

    if (!command.isEmpty()) {
        if (exec == QLatin1String("konsole")) {
            exec += QString::fromLatin1(" --noclose");
        } else if (exec == QLatin1String("xterm")) {
            exec += QString::fromLatin1(" -hold");
        }

        exec += QString::fromLatin1(" -e ") + command;
    }

    QStringList cmdTokens = KShell::splitArgs(exec);
    QString cmd = cmdTokens.takeFirst();

    if (exec == QLatin1String("konsole") && !workdir.isEmpty()) {
        cmdTokens << QString::fromLatin1("--workdir");
        cmdTokens << workdir;
        // For other terminals like xterm, we'll simply change the working
        // directory before launching them, see below.
    }

    QString error;
    if (self()->startServiceInternal("kdeinit_exec_with_workdir",
                                     cmd, cmdTokens, &error, startup_id, workdir)) {
      KMessage::message(KMessage::Error,
                      i18n("Could not launch the terminal client:\n\n%1", error),
                      i18n("Could not launch Terminal Client"));
    }
}
