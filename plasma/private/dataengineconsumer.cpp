/*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
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

#include "dataengineconsumer_p.h"

#include <kdebug.h>

#include "plasma/dataenginemanager.h"

namespace Plasma
{

DataEngineConsumer::DataEngineConsumer()
{
}

DataEngineConsumer::~DataEngineConsumer()
{
    foreach (const QString &engine, m_loadedEngines) {
        DataEngineManager::self()->unloadEngine(engine);
    }
}

DataEngine *DataEngineConsumer::dataEngine(const QString &name)
{
    if (m_loadedEngines.contains(name)) {
        DataEngine *engine = DataEngineManager::self()->engine(name);
        if (engine->isValid()) {
            return engine;
        } else {
            m_loadedEngines.remove(name);
        }
    }

    DataEngine *engine = DataEngineManager::self()->loadEngine(name);
    if (engine->isValid()) {
        m_loadedEngines.insert(name);
    }

    return engine;
}

} // namespace Plasma
