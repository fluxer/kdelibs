/*  This file is part of the KDE project
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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

#include "kpluginloader.h"

#include "kaboutdata.h"
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include "kpluginfactory.h"
#include <kservice.h>
#include <kdebug.h>

#include <QtCore/QLibrary>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

class KPluginLoaderPrivate
{
public:
    KPluginLoaderPrivate(const QString &libname)
        : name(libname), pluginVersion(0)
    {}
    ~KPluginLoaderPrivate()
    {
    }

    const QString name;
    quint32 pluginVersion;
    QString errorString;
};

static inline QString makeLibName( const QString &libname )
{
    int pos = libname.lastIndexOf(QLatin1Char('/'));
    if (pos < 0)
        pos = 0;
    if (libname.indexOf(QLatin1Char('.'), pos) < 0) {
        const char* const extList[] = { ".so", ".sl" };
        for (uint i = 0; i < sizeof(extList) / sizeof(*extList); ++i) {
            const QString lib = libname + QString::fromLatin1(extList[i]);
            if (QLibrary::isLibrary(lib))
                return lib;
        }
    }
    return libname;
}


static QString findLibraryInternal(const QString &name, const KComponentData &cData)
{
    // Convert name to a valid platform libname
    QString libname = makeLibName(name);
    QFileInfo fileinfo(name);
    bool hasPrefix = fileinfo.fileName().startsWith(QLatin1String("lib"));

    if (hasPrefix)
        kDebug() << "plugins should not have a 'lib' prefix:" << libname;

    // If it is a absolute path just return it
    if (!QDir::isRelativePath(libname))
        return libname;

    // Check for kde modules/plugins?
    QString libfile = cData.dirs()->findResource("module", libname);
    if (!libfile.isEmpty())
        return libfile;

    // Now look where they don't belong but sometimes are
    if (!hasPrefix)
        libname = fileinfo.path() + QLatin1String("/lib") + fileinfo.fileName();

    libfile = cData.dirs()->findResource("lib", libname);
    if (!libfile.isEmpty()) {
        kDebug() << "library" << libname << "not found under 'module' but under 'lib'";
        return libfile;
    }

    // Nothing found
    return QString();
}

KPluginLoader::KPluginLoader(const QString &plugin, const KComponentData &componentdata, QObject *parent)
    : QPluginLoader(findLibraryInternal(plugin, componentdata), parent), d_ptr(new KPluginLoaderPrivate(plugin))
{
    Q_D(KPluginLoader);

    // No lib, no fun.
    if (fileName().isEmpty()) {
        d->errorString = i18n(
                "Could not find plugin '%1' for application '%2'",
                plugin,
                componentdata.aboutData()->appName());
        return;
    }
}


KPluginLoader::KPluginLoader(const KService &service, const KComponentData &componentdata, QObject *parent)
: QPluginLoader(findLibraryInternal(service.library(), componentdata), parent), d_ptr(new KPluginLoaderPrivate(service.library()))
{
    Q_D(KPluginLoader);

    // It's probably to late to check this because service.library() is used
    // above.
    if (!service.isValid()) {
        d->errorString = i18n("The provided service is not valid");
        return;
    }

    // service.library() is used to find the lib. So first check if it is empty.
    if (service.library().isEmpty()) {
        d->errorString = i18n("The service '%1' provides no library or the Library key is missing", service.entryPath());
        return;
    }

    // No lib, no fun. service.library() was set but we were still unable to
    // find the lib.
    if (fileName().isEmpty()) {
        d->errorString = i18n(
                "Could not find plugin '%1' for application '%2'",
                service.name(),
                componentdata.aboutData()->appName());
        return;
    }
}

KPluginLoader::~KPluginLoader()
{
    delete d_ptr;
}

KPluginFactory *KPluginLoader::factory()
{
    Q_D(KPluginLoader);

    if (!load())
        return 0;


    QObject *obj = instance();

    if (!obj)
        return 0;

    KPluginFactory *factory = qobject_cast<KPluginFactory *>(obj);

    if (!factory) {
        kDebug() << "Expected a KPluginFactory, got a" << obj->metaObject()->className();
        delete obj;
        d->errorString = i18n("The library %1 does not offer a KDE 4 compatible factory." , d->name);
    }

    return factory;
}

bool KPluginLoader::load()
{
    Q_D(KPluginLoader);

    if (isLoaded())
        return true;

    if (!QPluginLoader::load()) {
        return false;
    }

    Q_ASSERT(!fileName().isEmpty());
    QLibrary lib(fileName());
    Q_ASSERT(lib.isLoaded()); // already loaded by QPluginLoader::load()

    KDEPluginVerificationData *verificationData = (KDEPluginVerificationData *) lib.resolve("kde_plugin_verification_data");
    if (verificationData) {
        if (verificationData->dataVersion < KDEPluginVerificationData::PluginVerificationDataVersion
            || ((verificationData->KDEVersion & 0xFFFF00) > (KDE_VERSION & 0xFFFF00))  // newer minor version
            || (KDE_VERSION_MAJOR << 16 != (verificationData->KDEVersion & 0xFF0000))) // different major version
        {
            d->errorString = i18n("The plugin '%1' uses an incompatible KDE library (%2).",
                                  d->name, QString::fromLatin1(verificationData->KDEVersionString));
            unload();
            return false;
        }
        d->pluginVersion = (verificationData->KDEVersion & 0xFFFF00);
    } else {
        kDebug() << "The plugin" << d->name << "doesn't contain a kde_plugin_verification_data structure";
        d->pluginVersion = 0;
    }

    return true;
}

QString KPluginLoader::errorString() const
{
    Q_D(const KPluginLoader);

    if (!d->errorString.isEmpty())
        return d->errorString;

    return QPluginLoader::errorString();
}

quint32 KPluginLoader::pluginVersion() const
{
    Q_D(const KPluginLoader);
    const_cast<KPluginLoader*>(this)->load();
    return d->pluginVersion;
}

QString KPluginLoader::pluginName() const
{
    Q_D(const KPluginLoader);
    const_cast<KPluginLoader*>(this)->load();
    return d->name;
}

#include "moc_kpluginloader.cpp"
