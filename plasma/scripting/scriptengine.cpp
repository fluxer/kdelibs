/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include "scripting/scriptengine.h"

#include <kdebug.h>
#include <kservice.h>
#include <kservicetypetrader.h>

#include "abstractrunner.h"
#include "applet.h"
#include "dataengine.h"
#include "package.h"
#include "scripting/appletscript.h"

#include "private/packages_p.h"

namespace Plasma
{

ScriptEngine::ScriptEngine(QObject *parent)
    : QObject(parent)
{
}

ScriptEngine::~ScriptEngine()
{
//    delete d;
}

bool ScriptEngine::init()
{
    return true;
}

const Package *ScriptEngine::package() const
{
    return 0;
}

QString ScriptEngine::mainScript() const
{
    return QString();
}

QStringList knownLanguages(ComponentTypes types)
{
    QString constraintTemplate = "'%1' in [X-Plasma-ComponentTypes]";
    QString constraint;

    if (types & AppletComponent) {
        constraint.append(constraintTemplate.arg("Applet"));
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine", constraint);
    //kDebug() << "Applet::knownApplets constraint was '" << constraint
    //         << "' which got us " << offers.count() << " matches";

    QStringList languages;
    foreach (const KService::Ptr &service, offers) {
        QString language = service->property("X-Plasma-API").toString();
        if (!languages.contains(language)) {
            languages.append(language);
        }
    }

    return languages;
}

KService::List engineOffers(const QString &language, ComponentType type)
{
    if (language.isEmpty()) {
        return KService::List();
    }

    QRegExp re("[^a-zA-Z0-9\\-_]");
    if (re.indexIn(language) != -1) {
        kDebug() << "invalid language attempted:" << language;
        return KService::List();
    }

    QString component;
    switch (type) {
        case AppletComponent: {
            component = "Applet";
            break;
        }
        default: {
            return KService::List();
        }
    }

    QString constraint = QString("[X-Plasma-API] == '%1' and "
                                 "'%2' in [X-Plasma-ComponentTypes]").arg(language, component);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine", constraint);
    /* kDebug() << "********************* loadingApplet with Plasma/ScriptEngine" << constraint
             << "resulting in" << offers.count() << "results";*/
    if (offers.isEmpty()) {
        kDebug() << "No offers for \"" << language << "\"";
    }

    return offers;
}

ScriptEngine *loadEngine(const QString &language, ComponentType type, QObject *parent)
{
    KService::List offers = engineOffers(language, type);

    QVariantList args;
    QString error;

    ScriptEngine *engine = 0;
    foreach (const KService::Ptr &service, offers) {
        switch (type) {
            case AppletComponent: {
                engine = service->createInstance<Plasma::AppletScript>(parent, args, &error);
                break;
            }
            default: {
                return 0;
            }
        }

        if (engine) {
            return engine;
        }

        kDebug() << "Couldn't load script engine for language " << language
                 << "! error reported: " << error;
    }

    return 0;
}

AppletScript *loadScriptEngine(const QString &language, Applet *applet)
{
    AppletScript *engine =
        static_cast<AppletScript*>(loadEngine(language, AppletComponent, applet));

    if (engine) {
        engine->setApplet(applet);
    }

    return engine;
}

PackageStructure::Ptr defaultPackageStructure(ComponentType type)
{
    switch (type) {
        case AppletComponent:
        case GenericComponent:
            return PackageStructure::Ptr(new PlasmoidPackage());
        default:
            // TODO: we don't have any special structures for other components yet
            break;
    }

    return PackageStructure::Ptr(new PackageStructure());
}

PackageStructure::Ptr packageStructure(const QString &language, ComponentType type)
{
    KService::List offers = engineOffers(language, type);

    if (offers.isEmpty()) {
        return defaultPackageStructure(type);
    }

    KService::Ptr offer = offers.first();
    QString packageFormat = offer->property("X-Plasma-PackageFormat").toString();

    if (packageFormat.isEmpty()) {
        return defaultPackageStructure(type);
    } else {
        PackageStructure::Ptr structure = PackageStructure::load(packageFormat);
        return structure;
    }
}

} // namespace Plasma

#include "moc_scriptengine.cpp"

