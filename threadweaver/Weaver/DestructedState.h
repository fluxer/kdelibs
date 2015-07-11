/* -*- C++ -*-

   This file declares the DestructedState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005-2013 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://creative-destruction.me $

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

   $Id: DestructedState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef DESTRUCTEDSTATE_H
#define DESTRUCTEDSTATE_H

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

#include "StateImplementation.h"

namespace ThreadWeaver {

    /** DestructedState is only active after the thread have been destroyed by
        the destructor, but before superclass destructors have finished.
    */
    class DestructedState : public StateImplementation
    {
    public:
	explicit DestructedState( WeaverInterface *weaver)
	    : StateImplementation (weaver)
	    {
	    }
	/** Suspend job processing. */
        virtual void suspend();
        /** Resume job processing. */
        virtual void resume();
        /** Assign a job to an idle thread. */
        virtual Job* applyForWork ( Thread *th,  Job* previous );
        /** Wait (by suspending the calling thread) until a job becomes available. */
        virtual void waitForAvailableJob ( Thread *th );

        /** reimpl */
        StateId stateId() const;
    };

}

#endif // DESTRUCTEDSTATE_H
