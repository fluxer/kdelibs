/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
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

#include "dataenginemanager.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kservicetypetrader.h>

#include "datacontainer.h"
#include "pluginloader.h"
#include "private/dataengine_p.h"
#include "private/datacontainer_p.h"
#include "scripting/scriptengine.h"

namespace Plasma
{

class NullEngine : public DataEngine
{
    public:
        NullEngine(QObject *parent = 0)
            : DataEngine(parent)
        {
            setValid(false);

            // ref() ourselves to ensure we never get deleted
            d->ref();
        }
};

class DataEngineManagerPrivate
{
    public:
        DataEngineManagerPrivate()
            : nullEng(0)
        {}

        ~DataEngineManagerPrivate()
        {
            foreach (Plasma::DataEngine *engine, engines) {
                delete engine;
            }
            engines.clear();
            delete nullEng;
        }

        DataEngine *nullEngine()
        {
            if (!nullEng) {
                nullEng = new NullEngine;
            }

            return nullEng;
        }

        DataEngine::Dict engines;
        DataEngine *nullEng;
};

class DataEngineManagerSingleton
{
    public:
        DataEngineManager self;
};

K_GLOBAL_STATIC(DataEngineManagerSingleton, privateDataEngineManagerSelf)

DataEngineManager *DataEngineManager::self()
{
    return &privateDataEngineManagerSelf->self;
}

DataEngineManager::DataEngineManager()
    : d(new DataEngineManagerPrivate)
{
    //startTimer(30000);
}

DataEngineManager::~DataEngineManager()
{
    delete d;
}

Plasma::DataEngine *DataEngineManager::engine(const QString &name) const
{
    if (name.isEmpty()) {
        return d->nullEngine();
    }

    Plasma::DataEngine::Dict::const_iterator it = d->engines.constFind(name);
    if (it != d->engines.constEnd()) {
        // ref and return the engine
        //Plasma::DataEngine *engine = *it;
        return *it;
    }

    return d->nullEngine();
}

Plasma::DataEngine *DataEngineManager::loadEngine(const QString &name)
{
    Plasma::DataEngine::Dict::const_iterator it = d->engines.constFind(name);

    if (it != d->engines.constEnd()) {
        DataEngine *engine = *it;
        engine->d->ref();
        return engine;
    }

    DataEngine *engine = PluginLoader::loadDataEngine(name);
    if (!engine) {
        return d->nullEngine();
    }

    engine->init();
    d->engines[name] = engine;
    return engine;
}

void DataEngineManager::unloadEngine(const QString &name)
{
    Plasma::DataEngine::Dict::iterator it = d->engines.find(name);

    if (it != d->engines.end()) {
        Plasma::DataEngine *engine = *it;
        engine->d->deref();

        if (!engine->d->isUsed()) {
            d->engines.erase(it);
            delete engine;
        }
    }
}

QStringList DataEngineManager::listAllEngines(const QString &parentApp)
{
    QString constraint;

    if (parentApp.isEmpty()) {
        constraint.append("(not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '')");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine", constraint);

    QStringList engines;
    foreach (const KService::Ptr &service, offers) {
        QString name = service->property("X-KDE-PluginInfo-Name").toString();
        if (!name.isEmpty()) {
            engines.append(name);
        }
    }

    return engines;
}

KPluginInfo::List DataEngineManager::listEngineInfo(const QString &parentApp)
{
    return PluginLoader::listDataEngineInfo(parentApp);
}

KPluginInfo::List DataEngineManager::listEngineInfoByCategory(const QString &category, const QString &parentApp)
{
    QString constraint = QString("[X-KDE-PluginInfo-Category] == '%1'").arg(category);

    if (parentApp.isEmpty()) {
        constraint.append(" and not exist [X-KDE-ParentApp]");
    } else {
        constraint.append(" and [X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine", constraint);
    return KPluginInfo::fromServices(offers);
}

void DataEngineManager::timerEvent(QTimerEvent *)
{
#ifndef NDEBUG
    QString path = KGlobal::dirs()->locateLocal("appdata", "plasma_dataenginemanager_log");
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        kDebug() << "faild to open" << path;
        return;
    }

    QTextStream out(&f);

    QHashIterator<QString, DataEngine*> it(d->engines);
    out << "================================== " << KGlobal::locale()->formatDateTime(QDateTime::currentDateTime()) << endl;
    while (it.hasNext()) {
        it.next();
        DataEngine *engine = it.value();
        out << "DataEngine: " << it.key() << ' ' << (QString::fromLatin1("0x") + QString::number(quintptr(engine), 16)) << endl;
        out << "            Claimed # of sources: " << engine->sources().count() << endl;
        out << "            Actual # of sources: " << engine->containerDict().count() << endl;
        out << endl << "            Source Details" << endl;

        foreach (DataContainer *dc, engine->containerDict()) {
            out << "                * " << dc->objectName() << endl;
            out << "                       Data count: " << dc->d->data.count() << endl;
            const int directs = dc->receivers(SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data)));
            if (directs > 0) {
                out << "                       Direction Connections: " << directs << ' ' << endl;
            }

            const int relays = dc->d->relays.count();
            if (relays > 0) {
                out << "                       Relays: " << dc->d->relays.count() << endl;
                QString times;
                foreach (SignalRelay *relay, dc->d->relays) {
                    times.append(' ').append(QString::number(relay->m_interval));
                }
                out << "                       Relay Timeouts: " << times << ' '  << endl;
            }
        }

        out << endl << "-----" << endl;
    }
    out << endl << endl;
#endif
//    killTimer(event->timerId());
}

} // namespace Plasma

#include "moc_dataenginemanager.cpp"
