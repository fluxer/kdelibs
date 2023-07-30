/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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

#include "kpluginfactory.h"
#include "kpluginfactory_p.h"
#include <kdebug.h>
#include <kglobal.h>

#include <QtCore/QObjectCleanupHandler>

K_GLOBAL_STATIC(QObjectCleanupHandler, factorycleanup)

KPluginFactory::KPluginFactory(const char *componentName, const char *catalogName, QObject *parent)
    : QObject(parent), d_ptr(new KPluginFactoryPrivate)
{
    Q_D(KPluginFactory);
    d->q_ptr = this;

    if (componentName)
        d->componentData = KComponentData(componentName, catalogName);

    factorycleanup->add(this);
}


KPluginFactory::KPluginFactory(const KAboutData &aboutData, QObject *parent)
    : QObject(parent), d_ptr(new KPluginFactoryPrivate)
{
    Q_D(KPluginFactory);
    d->q_ptr = this;
    d->componentData = KComponentData(aboutData);

    factorycleanup->add(this);
}


KPluginFactory::KPluginFactory(KPluginFactoryPrivate &d, QObject *parent)
    : QObject(parent), d_ptr(&d)
{
    factorycleanup->add(this);
}

KPluginFactory::~KPluginFactory()
{
    delete d_ptr;
}

KComponentData KPluginFactory::componentData() const
{
    Q_D(const KPluginFactory);
    return d->componentData;
}

void KPluginFactory::registerPlugin(const QString &keyword, const QMetaObject *metaObject, CreateInstanceFunction instanceFunction)
{
    Q_D(KPluginFactory);

    Q_ASSERT(metaObject);

    // we allow different interfaces to be registered without keyword
    if (!keyword.isEmpty()) {
        if (d->createInstanceHash.contains(keyword)) {
            kFatal() << "A plugin with the keyword" << keyword << "was already registered. A keyword must be unique!";
        }
        d->createInstanceHash.insert(keyword, KPluginFactoryPrivate::Plugin(metaObject, instanceFunction));
    } else {
        QList<KPluginFactoryPrivate::Plugin> clashes(d->createInstanceHash.values(keyword));
        const QMetaObject *superClass = metaObject->superClass();
        if (superClass) {
            foreach (const KPluginFactoryPrivate::Plugin &plugin, clashes) {
                for (const QMetaObject *otherSuper = plugin.first->superClass(); otherSuper;
                        otherSuper = otherSuper->superClass()) {
                    if (superClass == otherSuper) {
                        kFatal() << "Two plugins with the same interface(" << superClass->className() << ") were registered. Use keywords to identify the plugins.";
                    }
                }
            }
        }
        foreach (const KPluginFactoryPrivate::Plugin &plugin, clashes) {
            superClass = plugin.first->superClass();
            if (superClass) {
                for (const QMetaObject *otherSuper = metaObject->superClass(); otherSuper;
                        otherSuper = otherSuper->superClass()) {
                    if (superClass == otherSuper) {
                        kFatal() << "Two plugins with the same interface(" << superClass->className() << ") were registered. Use keywords to identify the plugins.";
                    }
                }
            }
        }
        d->createInstanceHash.insertMulti(keyword, KPluginFactoryPrivate::Plugin(metaObject, instanceFunction));
    }
}



QObject *KPluginFactory::create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
{
    Q_D(KPluginFactory);

    QObject *obj = 0;

    const QList<KPluginFactoryPrivate::Plugin> candidates(d->createInstanceHash.values(keyword));
    // for !keyword.isEmpty() candidates.count() is 0 or 1

    foreach (const KPluginFactoryPrivate::Plugin &plugin, candidates) {
        for (const QMetaObject *current = plugin.first; current; current = current->superClass()) {
            if (0 == qstrcmp(iface, current->className())) {
                if (obj) {
                    kFatal() << "ambiguous interface requested from a DSO containing more than one plugin";
                }
                obj = plugin.second(parentWidget, parent, args);
                break;
            }
        }
    }

    if (obj) {
        emit objectCreated(obj);
    }
    return obj;
}

void KPluginFactory::setComponentData(const KComponentData &kcd)
{
    Q_D(KPluginFactory);
    d->componentData = kcd;
}

#include "moc_kpluginfactory.cpp"
