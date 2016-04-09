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

#ifndef PLASMA_RUNNERJOBS_P_H
#define PLASMA_RUNNERJOBS_P_H

#include <QHash>
#include <QMutex>
#include <QSet>
#include <QRunnable>

#include "abstractrunner.h"

namespace Plasma {
// Queue policies

/*
 * FindMatchesJob class
 * Class to run queries in different threads
 */
class FindMatchesJob : public QRunnable
{
public:
    FindMatchesJob(Plasma::AbstractRunner *runner,
                   Plasma::RunnerContext *context);

    Plasma::AbstractRunner* runner() const;
    bool isFinished();

protected:
    void run();

private:
    Plasma::RunnerContext m_context;
    Plasma::AbstractRunner *m_runner;
    bool m_finished;
};

}

#endif // PLASMA_RUNNERJOBS_P_H
