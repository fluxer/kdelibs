/* -*- c++ -*-
 * Copyright (C)2000 Daniel M. Duley <mosfet@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "krecentdocument.h"

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kmimetype.h>
#include <kdesktopfile.h>
#include <kde_file.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kdebug.h>

#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QRegExp>

#include <sys/types.h>

// see kdebug.areas
static const int s_krecentdocumentarea = 7005;

QString KRecentDocument::recentDocumentDirectory()
{
    // need to change this path, not sure where
    return KStandardDirs::locateLocal("data", QLatin1String("RecentDocuments/"));
}

QStringList KRecentDocument::recentDocuments()
{
    QDir d(
        recentDocumentDirectory(),
        "*.desktop",
        QDir::Time,
        QDir::Files | QDir::Readable | QDir::Hidden
    );

    if (!d.exists()) {
        d.mkdir(recentDocumentDirectory());
    }

    const QStringList list = d.entryList();
    QStringList fullList;

    foreach (const QString &it, list) {
        QString pathDesktop;
        if (it.startsWith(":")) {
            // FIXME: Remove when Qt will be fixed
            // http://bugreports.qt.nokia.com/browse/QTBUG-11223
            pathDesktop = KRecentDocument::recentDocumentDirectory() + it;
        } else {
           pathDesktop = d.absoluteFilePath(it);
        }
        KDesktopFile tmpDesktopFile(pathDesktop);
        KUrl urlDesktopFile(tmpDesktopFile.desktopGroup().readPathEntry("URL", QString()));
        if (urlDesktopFile.isLocalFile() && !QFile::exists(urlDesktopFile.toLocalFile())) {
            d.remove(pathDesktop);
        } else {
            fullList.append(pathDesktop);
        }
    }

    return fullList;
}

void KRecentDocument::add(const KUrl& url)
{
    KRecentDocument::add(url, KGlobal::mainComponent().componentName());
    // ### componentName might not match the service filename...
}

void KRecentDocument::add(const KUrl &url, const QString &desktopEntryName)
{
    if (url.isLocalFile() && KGlobal::dirs()->relativeLocation("tmp", url.toLocalFile()) != url.toLocalFile()) {
        // inside tmp resource, do not save
        kDebug(s_krecentdocumentarea) << "temporary resource" << url << "for" << desktopEntryName;
        return;
    }

    int maxEntries = KRecentDocument::maximumItems();
    if (maxEntries <= 0) {
        kDebug(s_krecentdocumentarea) << "disabled" << url;
        return;
    }

    const QString path = recentDocumentDirectory();
    const QString fileName = url.fileName();
    // don't create a file called ".desktop", it will lead to an empty name in kio_recentdocuments
    const QString dStr = path + (fileName.isEmpty() ? QString("unnamed") : fileName);

    QString ddesktop = dStr + QLatin1String(".desktop");

    int i=1;
    // check for duplicates
    while (QFile::exists(ddesktop)){
        // see if it points to the same file and application
        KDesktopFile tmp(ddesktop);
        if (tmp.desktopGroup().readEntry("X-KDE-LastOpenedWith") == desktopEntryName) {
            KDE::utime(ddesktop, NULL);
            kDebug(s_krecentdocumentarea) << "duplicate" << url << "for" << desktopEntryName;
            return;
        }
        // if not append a (num) to it
        ++i;
        if (i > maxEntries) {
            break;
        }
        ddesktop = dStr + QString::fromLatin1("[%1].desktop").arg(i);
    }

    QDir dir(path);
    // check for max entries, delete oldest files if exceeded
    const QStringList list = dir.entryList(QDir::Files | QDir::Hidden, QFlags<QDir::SortFlag>(QDir::Time | QDir::Reversed));
    i = list.count();
    if(i > maxEntries-1){
        QStringList::ConstIterator it;
        it = list.begin();
        while(i > maxEntries-1){
            QFile::remove(dir.absolutePath() + QLatin1String("/") + (*it));
            --i, ++it;
        }
    }

    QString openStr = url.url();
    openStr.replace(QRegExp("\\$"), "$$"); // Desktop files with type "Link" are $-variable expanded

    kDebug(s_krecentdocumentarea) << "adding URL" << url;
    // create the applnk
    KDesktopFile configFile(ddesktop);
    KConfigGroup conf = configFile.desktopGroup();
    conf.writeEntry("Type", QString::fromLatin1("Link"));
    conf.writePathEntry("URL", openStr );
    // If you change the line below, change the test in the above loop
    conf.writeEntry("X-KDE-LastOpenedWith", desktopEntryName);
    conf.writeEntry("Name", url.fileName());
    conf.writeEntry("Icon", KMimeType::iconNameForUrl(url));
}

void KRecentDocument::clear()
{
    const QStringList list = recentDocuments();
    QDir dir;
    foreach(const QString &it, list) {
        kDebug(s_krecentdocumentarea) << "clearing" << it;
        dir.remove(it);
    }
}

int KRecentDocument::maximumItems()
{
    KConfigGroup cg(KGlobal::config(), QLatin1String("RecentDocuments"));
    return cg.readEntry(QLatin1String("MaxEntries"), 10);
}
