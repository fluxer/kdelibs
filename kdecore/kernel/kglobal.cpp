/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
   Copyright (C) 2009 Olivier Goffart <ogoffart@kde.org>

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

/*
 * kglobal.cpp -- Implementation of namespace KGlobal.
 * Author:	Sirtaj Singh Kang
 * Generated:	Sat May  1 02:08:43 EST 1999
 */

#include "kglobal.h"
#include "kglobal_p.h"

#include <config.h>

#include <sys/stat.h>
#include <unistd.h> // umask

#include <QThread>
#include <QAtomicInt>
#include <QList>
#include <QSet>
#include <QCoreApplication>

#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include "kcmdlineargs.h"
#include "kcatalog_p.h"

mode_t s_umsk;

class KGlobalPrivate
{
    public:
        inline KGlobalPrivate()
            : locale(nullptr),
            charsets(nullptr),
            localeIsFromFakeComponent(false)
        {
            // the umask is read here before any threads are created to avoid race conditions
            mode_t tmp = 0;
            s_umsk = umask(tmp);
            umask(s_umsk);
        }

        inline ~KGlobalPrivate()
        {
            delete locale;
            locale = nullptr;
            delete charsets;
            charsets = nullptr;
        }

        KComponentData activeComponent;
        KComponentData mainComponent; // holds a refcount
        KLocale *locale;
        KCharsets *charsets;
        bool localeIsFromFakeComponent;
        QStringList catalogsToInsert;

        /**
         * This component may be used in applications that doesn't have a
         * main component (such as pure Qt applications).
         */
        static KComponentData initFakeComponent()
        {
            QByteArray name = QCoreApplication::applicationName().toLatin1();
            if (name.isEmpty()) {
                name = "kde";
            }
            return KComponentData(name, name, KComponentData::SkipMainComponentRegistration);
        }
};

KCatalogLoader::KCatalogLoader(const QString &catalogName)
{
    KGlobal::insertCatalog(catalogName);
}


K_GLOBAL_STATIC(KGlobalPrivate, globalData)
K_GLOBAL_STATIC_WITH_ARGS(KComponentData, fakeComponent, (KGlobalPrivate::initFakeComponent()))

#define PRIVATE_DATA KGlobalPrivate *d = globalData

KStandardDirs *KGlobal::dirs()
{
    PRIVATE_DATA;
    return d->mainComponent.isValid() ? d->mainComponent.dirs() : fakeComponent->dirs();
}

KSharedConfig::Ptr KGlobal::config()
{
    PRIVATE_DATA;
    return d->mainComponent.isValid() ? d->mainComponent.config() : fakeComponent->config();
}

const KComponentData &KGlobal::mainComponent()
{
    PRIVATE_DATA;
    return d->mainComponent.isValid() ? d->mainComponent : *fakeComponent;
}

bool KGlobal::hasMainComponent()
{
    if (globalData.isDestroyed()) {
        return false;
    }
    PRIVATE_DATA;
    return d->mainComponent.isValid();
}

void KGlobal::insertCatalog(const QString& catalog)
{
    PRIVATE_DATA;
    if (d->locale) {
        locale()->insertCatalog(catalog);
    } else {
        d->catalogsToInsert.append(catalog);
    }
}

#ifndef QT_NO_TRANSLATION
KDETranslator* s_kdetranslator = nullptr;

static void removeKDETranslator()
{
    QCoreApplication* coreApp = QCoreApplication::instance();
    if (coreApp && s_kdetranslator) {
        QCoreApplication::removeTranslator(s_kdetranslator);
        delete s_kdetranslator;
        s_kdetranslator = nullptr;
    }
}

static void installKDETranslator()
{
    QCoreApplication* coreApp = QCoreApplication::instance();
    if (coreApp && !s_kdetranslator) { // testcase: kwrite --help: no qcore app
        s_kdetranslator = new KDETranslator();
        QCoreApplication::installTranslator(s_kdetranslator);
        qAddPostRoutine(removeKDETranslator);
    }
}
#endif // QT_NO_TRANSLATION

KLocale *KGlobal::locale()
{
    PRIVATE_DATA;
    if (d->locale == 0 || (d->localeIsFromFakeComponent && d->mainComponent.isValid() && d->mainComponent.config())) {
        // If you hit the warning below, here's how to debug it in gdb:
        // (gdb) set auto-solib-add on
        // (gdb) b i18n
        // (gdb) b KLocale::KLocale
        // Function "KLocale::KLocale" not defined.
        // Make breakpoint pending on future shared library load? (y or [n]) y
        // (gdb) run
        // And now it will stop at the first i18n call or more generally at the first construction of the KLocale object,
        // type bt or go up to find the guilty i18n call.
        if (d->locale != 0) {
            qWarning("KGlobal::locale(): Warning your global KLocale is being recreated with a valid main component"
                     " instead of a fake component, this usually means you tried to call i18n related functions before"
                     " your main component was created. You should not do that since it most likely will not work");
        }
        delete d->locale;
        d->locale = 0;
        d->locale = new KLocale(mainComponent().catalogName());
        d->localeIsFromFakeComponent = !d->mainComponent.isValid();
#ifndef QT_NO_TRANSLATION
        installKDETranslator();
#endif
        foreach(const QString &catalog, d->catalogsToInsert) {
            d->locale->insertCatalog(catalog);
        }
        d->catalogsToInsert.clear();
    }
    return d->locale;
}

bool KGlobal::hasLocale()
{
    if (globalData.isDestroyed()) {
        return false;
    }
    PRIVATE_DATA;
    return (d->locale != 0);
}

KCharsets *KGlobal::charsets()
{
    PRIVATE_DATA;
    if (d->charsets == 0) {
        d->charsets = new KCharsets();
    }

    return d->charsets;
}

mode_t KGlobal::umask()
{
    // Don't use PRIVATE_DATA here. This is called by ~KGlobalPrivate -> ~KConfig -> sync -> KSaveFile,
    // so there's no KGlobalPrivate anymore.
    return s_umsk;
}

KComponentData KGlobal::activeComponent()
{
    PRIVATE_DATA;
#ifndef NDEBUG
    if (!d->activeComponent.isValid()) {
        qFatal("Fatal error: you need to have a KComponentData object before\n"
               "you do anything that requires it! Examples of this are config\n"
               "objects, standard directories or translations.");
    }
#endif
    return d->activeComponent;
}

void KGlobal::setActiveComponent(const KComponentData &c)
{
    PRIVATE_DATA;
    d->activeComponent = c;
    if (c.isValid() && d->locale) {
        locale()->setActiveCatalog(c.catalogName());
    }
}

void KGlobal::newComponentData(const KComponentData &c)
{
    PRIVATE_DATA;
    if (d->mainComponent.isValid()) {
        return;
    }
    d->mainComponent = c;
    KGlobal::setActiveComponent(c);
}

void KGlobal::setLocale(KLocale *newLocale, CopyCatalogs copy)
{
    PRIVATE_DATA;
    if (copy == DoCopyCatalogs && d->locale)
        locale()->copyCatalogsTo(newLocale);
    delete d->locale;
    d->locale = newLocale;
}

QString KGlobal::caption()
{
    PRIVATE_DATA;
    // Caption set from command line ?
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    if (args && args->isSet("caption") && !args->getOption("caption").trimmed().isEmpty()) {
        return args->getOption("caption");
    } else if (d->mainComponent.isValid() && d->mainComponent.aboutData()) {
        // We have some about data ?
        return d->mainComponent.aboutData()->programName();
    }
    // Last resort : application name
    return QCoreApplication::applicationName();
}

/**
 * This counter indicates when to quit the application.
 * It starts at 0, is incremented by KMainWindow, systray icons, running jobs, etc.
 * and decremented again when those things are destroyed.
 * This mechanism allows dialogs and jobs to outlive the last window closed
 * e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client,
 * the job progress widget with "keep open" checked, etc.
 */
static QAtomicInt s_allowQuit(0); // this is used a bool
static QAtomicInt s_refCount(0);

void KGlobal::ref()
{
    s_refCount.fetchAndAddOrdered(1);
}

void KGlobal::deref()
{
    const int prevRefCount = s_refCount.fetchAndAddOrdered(-1);
    if (prevRefCount <= 1 && s_allowQuit.load()) {
        QCoreApplication::instance()->quit();
    }
}

void KGlobal::setAllowQuit(bool allowQuit)
{
    if (QThread::currentThread() == qApp->thread()) {
        s_allowQuit.fetchAndStoreOrdered(int(allowQuit));
    }
    else {
        qWarning() << "KGlobal::setAllowQuit may only be called from the main thread";
    }
}

#undef PRIVATE_DATA

QObject* KGlobal::findDirectChild_helper(const QObject* parent, const QMetaObject& mo)
{
    if (!parent)
        return 0;

    const QObjectList &children = parent->children();
    for (int i = 0; i < children.size(); ++i) {
        QObject* obj = children.at(i);
        if (mo.cast(obj)) {
            return obj;
        }
    }
    return 0;

}
