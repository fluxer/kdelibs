/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "pluginloader.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>
#include <kplugininfo.h>

#include "applet.h"
#include "abstractrunner.h"
#include "containment.h"
#include "packagestructure.h"
#include "popupapplet.h"
#include "private/applet_p.h"
#include "private/extenderapplet_p.h"

namespace Plasma {

Applet *PluginLoader::loadApplet(const QString &name, uint appletId, const QVariantList &args)
{ 
    // the application-specific appletLoader failed to create an applet, here we try with our own logic.
    if (name.isEmpty()) {
        return 0;
    }

    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    bool isContainment = false;
    if (offers.isEmpty()) {
        offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
        if (offers.count() > 0) {
            isContainment = true;
        }
    }

    /* if (offers.count() > 1) {
        kDebug() << "hey! we got more than one! let's blindly take the first one";
    } */

    if (offers.isEmpty()) {
        kDebug() << "offers is empty for " << name;
        return 0;
    }

    KService::Ptr offer = offers.first();
 
    if (appletId == 0) {
        appletId = ++AppletPrivate::s_maxAppletId;
    }

    QVariantList allArgs;
    allArgs << offer->storageId() << appletId << args;

    if (!offer->property("X-Plasma-API").toString().isEmpty()) {
        kDebug() << "we have a script using the"
                 << offer->property("X-Plasma-API").toString() << "API";
        if (isContainment) {
            return new Containment(0, allArgs);
        } else {
            if (offer->serviceTypes().contains("Plasma/Containment")) {
                return new Containment(0, allArgs);
            } else if (offer->serviceTypes().contains("Plasma/PopupApplet")) {
                return new PopupApplet(0, allArgs);
            } else {
                return new Applet(0, allArgs);
            }
        }
    }

    Applet *applet = 0;
    QString error;
    if (name == "internal:extender") {
        applet = new ExtenderApplet(0, allArgs);
    } else {
        applet = offer->createInstance<Plasma::Applet>(0, allArgs, &error);
    }

    if (!applet) {
        kWarning() << "Could not load applet" << name << "! reason given:" << error;
    }

    return applet;
}

DataEngine *PluginLoader::loadDataEngine(const QString &name)
{ 
    // load the engine, add it to the engines
    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine",
                                                              constraint);

    DataEngine *engine = 0;
    QString error;
    if (offers.isEmpty()) {
        kDebug() << "offers are empty for " << name << " with constraint " << constraint;
    } else {
        QVariantList allArgs;
        allArgs << offers.first()->storageId();
        QString api = offers.first()->property("X-Plasma-API").toString();
        if (api.isEmpty()) {
            if (offers.first()) {
                engine = offers.first()->createInstance<Plasma::DataEngine>(0, allArgs, &error);
            }
        } else {
            engine = new DataEngine(0, offers.first());
        }
    }

    if (!engine) {
        kDebug() << "Couldn't load engine \"" << name << "\". Error given: " << error;
    }

    return engine;
}

AbstractRunner *PluginLoader::loadRunner(const QString &name)
{
    // FIXME: RunnerManager is all wrapped around runner loading; that should be sorted out
    // and the actual plugin loading added here
    return 0;
}

KPluginInfo::List PluginLoader::listAppletInfo(const QString &category, const QString &parentApp)
{
    QString constraint = AppletPrivate::parentAppConstraint(parentApp);

    //note: constraint guaranteed non-empty from here down
    if (category.isEmpty()) { //use all but the excluded categories
        KConfigGroup group(KGlobal::config(), "General");
        QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
        foreach (const QString &category, excluded) {
            constraint.append(" and [X-KDE-PluginInfo-Category] != '").append(category).append("'");
        }
    } else { //specific category (this could be an excluded one - is that bad?)
        constraint.append(" and [X-KDE-PluginInfo-Category] == '").append(category).append("'");
        if (category == "Miscellaneous") {
            constraint.append(" or (not exist [X-KDE-PluginInfo-Category] or [X-KDE-PluginInfo-Category] == '')");
        }
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    //kDebug() << "Applet::listAppletInfo constraint was '" << constraint
    //         << "' which got us " << offers.count() << " matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List PluginLoader::listDataEngineInfo(const QString &parentApp)
{
    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine", constraint);
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List PluginLoader::listRunnerInfo(const QString &parentApp)
{
    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner", constraint);
    return KPluginInfo::fromServices(offers);
}

} // Plasma Namespace

