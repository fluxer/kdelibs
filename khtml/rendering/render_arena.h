/*
 * Copyright (C) 2002 Apple Computer, Inc.
 * Copyright (C) 2003 Dirk Mueller (mueller@kde.org)
 *
 * Portions are Copyright (C) 1998 Netscape Communications Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 */

#ifndef RENDERARENA_H
#define RENDERARENA_H

#include "misc/shared.h"
#include "wtf/AlwaysInline.h"

#include <stdlib.h>

namespace khtml {

class RenderArena: public Shared<RenderArena> {
public:
#ifdef NDEBUG
    RenderArena() { };
    ~RenderArena() { };

    ALWAYS_INLINE void* allocate(size_t size) {
        return ::malloc(size);
    }
    ALWAYS_INLINE void deallocate(void* ptr) {
        ::free(ptr);
    }
#else
    RenderArena() { m_alloccount = 0; };
    // if you get assert here the application is leaking memory
    ~RenderArena() { assert(m_alloccount == 0); };

    NEVER_INLINE void* allocate(size_t size) {
        m_alloccount++;
        return ::malloc(size);
    }
    NEVER_INLINE void deallocate(void* ptr) {
        m_alloccount--;
        assert(this);
        ::free(ptr);
    }
#endif
private:
    int m_alloccount;
};

} // namespace


#endif

