/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000 Michael Matz <matz@kde.org>
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de.org>

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
#include "klibrary.h"

#include <QtCore/QDir>
#include <QtCore/QPointer>

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kpluginfactory.h>
#include <kdebug.h>

extern QString makeLibName( const QString &libname );
extern QString findLibraryInternal(const QString &name, const KComponentData &cData);

int kLibraryDebugArea() {
    static int s_area = KDebug::registerArea("kdecore (KLibrary)");
    return s_area;
}

//static
QString findLibrary(const QString &name, const KComponentData &cData)
{
    return findLibraryInternal(name, cData);;
}


KLibrary::KLibrary(QObject *parent)
    : QLibrary(parent), d_ptr(0)
{
}

KLibrary::KLibrary(const QString &name, const KComponentData &cData, QObject *parent)
    : QLibrary(findLibrary(name, cData), parent), d_ptr(0)
{
}

KLibrary::KLibrary(const QString &name, int verNum, const KComponentData &cData, QObject *parent)
    : QLibrary(findLibrary(name, cData), verNum, parent), d_ptr(0)
{
}

KLibrary::~KLibrary()
{
}

static KPluginFactory *kde4Factory(KLibrary *lib)
{
    const QByteArray symname("qt_plugin_instance");

    typedef QObject* (*t_func)();
    t_func func = reinterpret_cast<t_func>(lib->resolveFunction(symname));
    if ( !func )
    {
        kDebug(kLibraryDebugArea()) << "The library" << lib->fileName() << "does not offer a qt_plugin_instance function.";
        return 0;
    }

    QObject* instance = func();
    KPluginFactory *factory = qobject_cast<KPluginFactory *>(instance);

    if( !factory )
    {
        if (instance)
            kDebug(kLibraryDebugArea()) << "Expected a KPluginFactory, got a" << instance->metaObject()->className();
        kDebug(kLibraryDebugArea()) << "The library" << lib->fileName() << "does not offer a KDE 4 compatible factory.";
        return 0;
    }
    return factory;
}

// deprecated
KPluginFactory* KLibrary::factory(const char* factoryname)
{
    Q_UNUSED(factoryname);

    if (fileName().isEmpty()) {
        return NULL;
    }

    return kde4Factory(this);
}

KLibrary::void_function_ptr KLibrary::resolveFunction( const char* symname )
{
    void *psym = resolve( symname );
    if (!psym)
        return 0;

    // Cast the void* to non-pointer type first - it's not legal to
    // cast a pointer-to-object directly to a pointer-to-function.
    ptrdiff_t tmp = reinterpret_cast<ptrdiff_t>(psym);
    void_function_ptr sym = reinterpret_cast<void_function_ptr>(tmp);

    return sym;
}

void KLibrary::setFileName(const QString &name, const KComponentData &data)
{
    QLibrary::setFileName(findLibrary(name, data));
}

#include "moc_klibrary.cpp"
