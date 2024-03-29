/*
 Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
 
 Win32 port:
 Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>
 
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

#include "kkeyserver.h"

#include <config.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>

namespace KKeyServer {
//---------------------------------------------------------------------
// Array Structures
//---------------------------------------------------------------------
    
struct ModInfo
{
    int modQt;
    const char* psName;
    QString* sLabel; // this struct is used in static objects, so must use a pointer here.
};

//---------------------------------------------------------------------
// Arrays
//---------------------------------------------------------------------

// Key names with this context are extracted elsewhere,
// no need for I18N_NOOP2's here.
#define KEYCTXT "keyboard-key-name"
static ModInfo g_rgModInfo[4] =
{
    { Qt::SHIFT,   "Shift", 0 },
    { Qt::CTRL,    "Ctrl",  0 },
    { Qt::ALT,     "Alt",   0 },
    { Qt::META,    "Meta",  0 }
};

//---------------------------------------------------------------------
// Initialization
//---------------------------------------------------------------------
static bool g_bInitializedKKeyLabels = false;

static void intializeKKeyLabels()
{
    KConfigGroup cg(KGlobal::config(), "Keyboard");
    g_rgModInfo[0].sLabel = new QString(cg.readEntry("Label Shift", i18nc(KEYCTXT, g_rgModInfo[0].psName)));
    g_rgModInfo[1].sLabel = new QString(cg.readEntry("Label Ctrl", i18nc(KEYCTXT, g_rgModInfo[1].psName)));
    g_rgModInfo[2].sLabel = new QString(cg.readEntry("Label Alt", i18nc(KEYCTXT, g_rgModInfo[2].psName)));
    g_rgModInfo[3].sLabel = new QString(cg.readEntry("Label Win", i18nc(KEYCTXT, g_rgModInfo[3].psName)));
    g_bInitializedKKeyLabels = true;
}

//---------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------

static QString modToString(uint mod, bool bUserSpace)
{
    if (bUserSpace && !g_bInitializedKKeyLabels) {
        intializeKKeyLabels();
    }
    
    QString s;
    for(int i = 3; i >= 0; i--) {
        if (mod & g_rgModInfo[i].modQt) {
            if (!s.isEmpty() ) {
                s += '+';
            }
            s += (bUserSpace ? *g_rgModInfo[i].sLabel : QString::fromLatin1(g_rgModInfo[i].psName));
        }
    }
    return s;
}

QString modToStringUser(uint mod)
{
    return modToString(mod, true);
}

uint stringUserToMod(const QString &mod)
{
    for (int i = 3; i >= 0; i--) {
        if (mod.toLower() == g_rgModInfo[i].sLabel->toLower()) {
            return g_rgModInfo[i].modQt;
        }
    }
    return 0;
}

bool isShiftAsModifierAllowed(int keyQt)
{
    // remove any modifiers
    keyQt &= ~Qt::KeyboardModifierMask;

    // Shift only works as a modifier with certain keys. It's not possible
    // to enter the SHIFT+5 key sequence for me because this is handled as
    // '%' by qt on my keyboard.
    // The working keys are all hardcoded here :-(
    if (keyQt >= Qt::Key_F1 && keyQt <= Qt::Key_F35) {
        return true;
    }

    if (QChar(keyQt).isLetter()) {
        return true;
    }

    switch (keyQt) {
        case Qt::Key_Return:
        case Qt::Key_Space:
        case Qt::Key_Backspace:
        case Qt::Key_Backtab:
        case Qt::Key_Escape:
        case Qt::Key_Print:
        case Qt::Key_ScrollLock:
        case Qt::Key_Pause:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Insert:
        case Qt::Key_Delete:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Enter:
        case Qt::Key_SysReq:
        case Qt::Key_CapsLock:
        case Qt::Key_NumLock:
        case Qt::Key_Help:
        case Qt::Key_Clear:
        case Qt::Key_Select:
            return true;

        default:
            return false;
    }
}

}
