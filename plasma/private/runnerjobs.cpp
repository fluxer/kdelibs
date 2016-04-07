/*
 *   Copyright (C) 2007, 2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
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

#include "runnerjobs_p.h"

#include <QTimer>

#include <kdebug.h>

#include "runnermanager.h"
#include "plasma/querymatch.h"

namespace Plasma {

////////////////////
// Jobs
////////////////////

FindMatchesJob::FindMatchesJob(Plasma::AbstractRunner *runner,
                               Plasma::RunnerContext *context, QObject *parent)
    : QThread(parent),
      m_context(*context, 0),
      m_runner(runner)
{
}

FindMatchesJob::~FindMatchesJob()
{
    wait(3000);
}

void FindMatchesJob::run()
{
    // kDebug() << "Running match for " << m_runner->objectName();
    if (m_context.isValid()) {
        m_runner->performMatch(m_context);
        emit done(this);
    }
}

int FindMatchesJob::priority() const
{
    return m_runner->priority();
}

Plasma::AbstractRunner* FindMatchesJob::runner() const
{
    return m_runner;
}

} // Plasma namespace

#include "moc_runnerjobs_p.cpp"
