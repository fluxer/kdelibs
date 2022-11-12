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

#include <config-kkeyserver.h>

#include "kkeyserver_x11.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtGui/qx11info_x11.h>
# define XK_MISCELLANY
# define XK_XKB_KEYS
# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
#ifdef HAVE_XKB
# include <X11/XKBlib.h>
#endif
# include <X11/keysymdef.h>
# include <X11/XF86keysym.h>

// #define KKEYSERVER_DEBUG 1



namespace KKeyServer
{

//---------------------------------------------------------------------
// Data Structures
//---------------------------------------------------------------------

struct Mod
{
    int m_mod;
};

//---------------------------------------------------------------------
// Array Structures
//---------------------------------------------------------------------

struct X11ModInfo
{
    int modQt;
    int modX;
};

struct SymVariation
{
    uint sym, symVariation;
    bool bActive;
};

struct SymName
{
    uint sym;
    const char* psName;
};

struct TransKey {
    int keySymQt;
    uint keySymX;
};

//---------------------------------------------------------------------
// Arrays
//---------------------------------------------------------------------

static X11ModInfo g_rgX11ModInfo[4] =
{
    { Qt::SHIFT,   ShiftMask },
    { Qt::CTRL,    ControlMask },
    { Qt::ALT,     Mod1Mask },
    { Qt::META,    Mod4Mask }
};

// These are the X equivalents to the Qt keycodes 0x1000 - 0x1026
static const TransKey g_rgQtToSymX[] =
{
    { Qt::Key_Escape,     XK_Escape },
    { Qt::Key_Tab,        XK_Tab },
    { Qt::Key_Backtab,    XK_ISO_Left_Tab },
    { Qt::Key_Backspace,  XK_BackSpace },
    { Qt::Key_Return,     XK_Return },
    { Qt::Key_Enter,      XK_KP_Enter },
    { Qt::Key_Insert,     XK_Insert },
    { Qt::Key_Delete,     XK_Delete },
    { Qt::Key_Pause,      XK_Pause },
#ifdef Q_OS_SOLARIS
    { Qt::Key_Print,      XK_F22 },
#else
    { Qt::Key_Print,      XK_Print },
#endif
    { Qt::Key_SysReq,     XK_Sys_Req },
    { Qt::Key_Home,       XK_Home },
    { Qt::Key_End,        XK_End },
    { Qt::Key_Left,       XK_Left },
    { Qt::Key_Up,         XK_Up },
    { Qt::Key_Right,      XK_Right },
    { Qt::Key_Down,       XK_Down },
    { Qt::Key_PageUp,      XK_Prior },
    { Qt::Key_PageDown,       XK_Next },
    //{ Qt::Key_Shift,      0 },
    //{ Qt::Key_Control,    0 },
    //{ Qt::Key_Meta,       0 },
    //{ Qt::Key_Alt,        0 },
    { Qt::Key_CapsLock,   XK_Caps_Lock },
    { Qt::Key_NumLock,    XK_Num_Lock },
    { Qt::Key_ScrollLock, XK_Scroll_Lock },
    { Qt::Key_F1,         XK_F1 },
    { Qt::Key_F2,         XK_F2 },
    { Qt::Key_F3,         XK_F3 },
    { Qt::Key_F4,         XK_F4 },
    { Qt::Key_F5,         XK_F5 },
    { Qt::Key_F6,         XK_F6 },
    { Qt::Key_F7,         XK_F7 },
    { Qt::Key_F8,         XK_F8 },
    { Qt::Key_F9,         XK_F9 },
    { Qt::Key_F10,        XK_F10 },
    { Qt::Key_F11,        XK_F11 },
    { Qt::Key_F12,        XK_F12 },
    { Qt::Key_F13,        XK_F13 },
    { Qt::Key_F14,        XK_F14 },
    { Qt::Key_F15,        XK_F15 },
    { Qt::Key_F16,        XK_F16 },
    { Qt::Key_F17,        XK_F17 },
    { Qt::Key_F18,        XK_F18 },
    { Qt::Key_F19,        XK_F19 },
    { Qt::Key_F20,        XK_F20 },
    { Qt::Key_F21,        XK_F21 },
    { Qt::Key_F22,        XK_F22 },
    { Qt::Key_F23,        XK_F23 },
    { Qt::Key_F24,        XK_F24 },
    { Qt::Key_F25,        XK_F25 },
    { Qt::Key_F26,        XK_F26 },
    { Qt::Key_F27,        XK_F27 },
    { Qt::Key_F28,        XK_F28 },
    { Qt::Key_F29,        XK_F29 },
    { Qt::Key_F30,        XK_F30 },
    { Qt::Key_F31,        XK_F31 },
    { Qt::Key_F32,        XK_F32 },
    { Qt::Key_F33,        XK_F33 },
    { Qt::Key_F34,        XK_F34 },
    { Qt::Key_F35,        XK_F35 },
    { Qt::Key_Super_L,    XK_Super_L },
    { Qt::Key_Super_R,    XK_Super_R },
    { Qt::Key_Menu,       XK_Menu },
    { Qt::Key_Hyper_L,    XK_Hyper_L },
    { Qt::Key_Hyper_R,    XK_Hyper_R },
    { Qt::Key_Help,       XK_Help },

    { '/',                XK_KP_Divide },
    { '*',                XK_KP_Multiply },
    { '-',                XK_KP_Subtract },
    { '+',                XK_KP_Add },
    { Qt::Key_Return,     XK_KP_Enter },

    // All of the stuff below really has to match qkeymapper_x11.cpp in Katie!s
    { Qt::Key_Clear,  XF86XK_Clear },
    { Qt::Key_Select,  XF86XK_Select },
};

//---------------------------------------------------------------------
// Debugging
//---------------------------------------------------------------------
#ifndef NDEBUG
inline void checkDisplay()
{
    // Some non-GUI apps might try to use us.
    if ( !QX11Info::display() ) {
        kError() << "QX11Info::display() returns 0.  I'm probably going to crash now.";
        kError() << "If this is a KApplication initialized without GUI stuff, change it to be "
                    "initialized with GUI stuff.";
    }
}
#else // NDEBUG
# define checkDisplay()
#endif

//---------------------------------------------------------------------
// Initialization
//---------------------------------------------------------------------

static bool g_bInitializedMods;
static uint g_modXNumLock, g_modXScrollLock, g_modXModeSwitch, g_alt_mask, g_meta_mask, g_super_mask, g_hyper_mask;

bool initializeMods()
{
    // Reinitialize the masks
    g_modXNumLock = 0;
    g_modXScrollLock = 0;
    g_modXModeSwitch = 0;
    g_alt_mask = 0;
    g_meta_mask = 0;
    g_super_mask = 0;
    g_hyper_mask = 0;

    checkDisplay();
    XModifierKeymap* xmk = XGetModifierMapping( QX11Info::display() );

    int min_keycode, max_keycode;
    int keysyms_per_keycode = 0;

    XDisplayKeycodes( QX11Info::display(), &min_keycode, &max_keycode );
    XFree( XGetKeyboardMapping( QX11Info::display(), min_keycode, 1, &keysyms_per_keycode ));

    for( int i = Mod1MapIndex; i < 8; i++ ) {
        uint mask = (1 << i);
        uint keySymX = NoSymbol;

        // This used to be only XKeycodeToKeysym( ... , 0 ), but that fails with XFree4.3.99
        // and X.org R6.7 , where for some reason only ( ... , 1 ) works. I have absolutely no
        // idea what the problem is, but searching all possibilities until something valid is
        // found fixes the problem.
        for( int j = 0; j < xmk->max_keypermod; ++j ) {

            for( int k = 0; k < keysyms_per_keycode; ++k ) {

#ifdef HAVE_XKB
                keySymX = XkbKeycodeToKeysym( QX11Info::display(), xmk->modifiermap[xmk->max_keypermod * i + j], 0, k );
#else
                keySymX = XKeycodeToKeysym( QX11Info::display(), xmk->modifiermap[xmk->max_keypermod * i + j], k );
#endif

                switch( keySymX ) {
                    case XK_Alt_L:
                    case XK_Alt_R:       g_alt_mask |= mask; break;

                    case XK_Super_L:
                    case XK_Super_R:     g_super_mask |= mask; break;

                    case XK_Hyper_L:
                    case XK_Hyper_R:     g_hyper_mask |= mask; break;

                    case XK_Meta_L:
                    case XK_Meta_R:      g_meta_mask |= mask; break;

                    case XK_Num_Lock:    g_modXNumLock |= mask; break;
                    case XK_Scroll_Lock: g_modXScrollLock |= mask; break;
                    case XK_Mode_switch: g_modXModeSwitch |= mask; break;
                }
            }
        }
    }

#ifdef KKEYSERVER_DEBUG
    kDebug() << "Alt:" << g_alt_mask;
    kDebug() << "Meta:" << g_meta_mask;
    kDebug() << "Super:" << g_super_mask;
    kDebug() << "Hyper:" << g_hyper_mask;
    kDebug() << "NumLock:" << g_modXNumLock;
    kDebug() << "ScrollLock:" << g_modXScrollLock;
    kDebug() << "ModeSwitch:" << g_modXModeSwitch;
#endif

    // Check if hyper overlaps with super or meta or alt
    if (g_hyper_mask&(g_super_mask|g_meta_mask|g_alt_mask)) {
#ifdef KKEYSERVER_DEBUG
        kDebug() << "Hyper conflicts with super, meta or alt.";
#endif
        // Remove the conflicting masks
        g_hyper_mask &= ~(g_super_mask|g_meta_mask|g_alt_mask);
    }

    // Check if super overlaps with meta or alt
    if (g_super_mask&(g_meta_mask|g_alt_mask)) {
#ifdef KKEYSERVER_DEBUG
        kDebug() << "Super conflicts with meta or alt.";
#endif
        // Remove the conflicting masks
        g_super_mask &= ~(g_meta_mask|g_alt_mask);
    }


    // Check if meta overlaps with alt
    if (g_meta_mask|g_alt_mask) {
#ifdef KKEYSERVER_DEBUG
        kDebug() << "Meta conflicts with alt.";
#endif
        // Remove the conflicting masks
        g_meta_mask &= ~(g_alt_mask);
    }

    if (!g_meta_mask) {
#ifdef KKEYSERVER_DEBUG
        kDebug() << "Meta is not set or conflicted with alt.";
#endif
        if (g_super_mask) {
#ifdef KKEYSERVER_DEBUG
            kDebug() << "Using super for meta";
#endif
            // Use Super
            g_meta_mask = g_super_mask;
        } else if (g_hyper_mask) {
#ifdef KKEYSERVER_DEBUG
            kDebug() << "Using hyper for meta";
#endif
            // User Hyper
            g_meta_mask = g_hyper_mask;
        } else {
            // ???? Nothing left
            g_meta_mask = 0;
        }
    }

#ifdef KKEYSERVER_DEBUG
    kDebug() << "Alt:" << g_alt_mask;
    kDebug() << "Meta:" << g_meta_mask;
    kDebug() << "Super:" << g_super_mask;
    kDebug() << "Hyper:" << g_hyper_mask;
    kDebug() << "NumLock:" << g_modXNumLock;
    kDebug() << "ScrollLock:" << g_modXScrollLock;
    kDebug() << "ModeSwitch:" << g_modXModeSwitch;
#endif

    if (!g_meta_mask) {
        kWarning() << "Your keyboard setup doesn't provide a key to use for meta. See 'xmodmap -pm' or 'xkbcomp $DISPLAY'";
    }

    g_rgX11ModInfo[2].modX = g_alt_mask;
    g_rgX11ModInfo[3].modX = g_meta_mask;

    XFreeModifiermap( xmk );
    g_bInitializedMods = true;

    return true;
}


//---------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------


uint modXShift()      { return ShiftMask; }
uint modXCtrl()       { return ControlMask; }
uint modXAlt()        { if( !g_bInitializedMods ) { initializeMods(); } return g_alt_mask; }
uint modXMeta()       { if( !g_bInitializedMods ) { initializeMods(); } return g_meta_mask; }

uint modXNumLock()    { if( !g_bInitializedMods ) { initializeMods(); } return g_modXNumLock; }
uint modXLock()       { return LockMask; }
uint modXScrollLock() { if( !g_bInitializedMods ) { initializeMods(); } return g_modXScrollLock; }
uint modXModeSwitch() { if( !g_bInitializedMods ) { initializeMods(); } return g_modXModeSwitch; }

bool keyboardHasMetaKey() { return modXMeta() != 0; }


uint getModsRequired(uint sym)
{
    uint mod = 0;

    // FIXME: This might not be true on all keyboard layouts!
    if( sym == XK_Sys_Req ) return Qt::ALT;
    if( sym == XK_Break ) return Qt::CTRL;

    if( sym < 0x3000 ) {
        QChar c(sym);
        if( c.isLetter() && c.toLower() != c.toUpper() && sym == c.toUpper().unicode() )
            return Qt::SHIFT;
    }

    uchar code = XKeysymToKeycode( QX11Info::display(), sym );
    if( code ) {
        // need to check index 0 before the others, so that a null-mod
        //  can take precedence over the others, in case the modified
        //  key produces the same symbol.
#ifdef HAVE_XKB
        if( sym == XkbKeycodeToKeysym( QX11Info::display(), code, 0, 0 ) )
            ;
        else if( sym == XkbKeycodeToKeysym( QX11Info::display(), code, 0, 1 ) )
            mod = Qt::SHIFT;
        else if( sym == XkbKeycodeToKeysym( QX11Info::display(), code, 0, 2 ) )
            mod = MODE_SWITCH;
        else if( sym == XkbKeycodeToKeysym( QX11Info::display(), code, 0, 3 ) )
            mod = Qt::SHIFT | MODE_SWITCH;
#else
        if( sym == XKeycodeToKeysym( QX11Info::display(), code, 0 ) )
            ;
        else if( sym == XKeycodeToKeysym( QX11Info::display(), code, 1 ) )
            mod = Qt::SHIFT;
        else if( sym == XKeycodeToKeysym( QX11Info::display(), code, 2 ) )
            mod = MODE_SWITCH;
        else if( sym == XKeycodeToKeysym( QX11Info::display(), code, 3 ) )
            mod = Qt::SHIFT | MODE_SWITCH;
#endif
    }
    return mod;
}

bool keyQtToCodeX( int keyQt, int* keyCode )
{
    int sym;
    uint mod;
    keyQtToSymX(keyQt, &sym);
    keyQtToModX(keyQt, &mod);

    // Get any extra mods required by the sym.
    //  E.g., XK_Plus requires SHIFT on the en layout.
    uint modExtra = getModsRequired(sym);
    // Get the X modifier equivalent.
    if( !sym || !keyQtToModX( (keyQt & Qt::KeyboardModifierMask) | modExtra, &mod ) ) {
        *keyCode = 0;
        return false;
    }

    *keyCode = XKeysymToKeycode( QX11Info::display(), sym );
    return true;
}

bool keyQtToSymX( int keyQt, int* keySym )
{
    int symQt = keyQt & ~Qt::KeyboardModifierMask;

    if( symQt < 0x1000 ) {
        *keySym = QChar(symQt).toUpper().unicode();
        return true;
    }


    for( uint i = 0; i < sizeof(g_rgQtToSymX)/sizeof(TransKey); i++ ) {
        if( g_rgQtToSymX[i].keySymQt == symQt ) {
            *keySym = g_rgQtToSymX[i].keySymX;
            return true;
        }
    }

    *keySym = 0;
    if( symQt != Qt::Key_Shift && symQt != Qt::Key_Control && symQt != Qt::Key_Alt && symQt != Qt::Key_Meta) {
        kDebug(125) << "Sym::initQt( " << QString::number(keyQt,16) << " ): failed to convert key.";
    }
    return false;
}

bool symXToKeyQt( uint keySym, int* keyQt )
{
    *keyQt = Qt::Key_unknown;
    if( keySym < 0x1000 ) {
        if( keySym >= 'a' && keySym <= 'z' )
            *keyQt = QChar(keySym).toUpper().unicode();
        else
            *keyQt = keySym;
    }

    else if( keySym < 0x3000 )
        *keyQt = keySym;

    else {
        for( uint i = 0; i < sizeof(g_rgQtToSymX)/sizeof(TransKey); i++ )
            if( g_rgQtToSymX[i].keySymX == keySym ) {
                *keyQt = g_rgQtToSymX[i].keySymQt;
                break;
            }
    }

    return (*keyQt != Qt::Key_unknown);
}

/* are these things actually used anywhere?  there's no way
   they can do anything on non-X11 */

bool keyQtToModX( int modQt, uint* modX )
{
    if( !g_bInitializedMods )
        initializeMods();

    *modX = 0;
    for( int i = 0; i < 4; i++ ) {

        if( modQt & g_rgX11ModInfo[i].modQt ) {
            if( g_rgX11ModInfo[i].modX ) {
                *modX |= g_rgX11ModInfo[i].modX;
            } else {
                // The qt modifier has no x equivalent. Return false
                return false;
            }
        }
    }
    return true;
}

bool modXToQt( uint modX, int* modQt )
{
    if( !g_bInitializedMods )
        initializeMods();

    *modQt = 0;
    for( int i = 0; i < 4; i++ ) {
        if( modX & g_rgX11ModInfo[i].modX ) {
            *modQt |= g_rgX11ModInfo[i].modQt;
            continue;
        }
    }
    return true;
}


bool codeXToSym( uchar codeX, uint modX, uint* sym )
{
    KeySym keySym;
    XKeyPressedEvent event;

    checkDisplay();

    event.type = KeyPress;
    event.display = QX11Info::display();
    event.state = modX;
    event.keycode = codeX;

    XLookupString( &event, 0, 0, &keySym, 0 );
    *sym = (uint) keySym;
    return true;
}


uint accelModMaskX()
{
    return modXShift() | modXCtrl() | modXAlt() | modXMeta();
}


bool xEventToQt( XEvent* e, int* keyQt )
{
    Q_ASSERT(e->type == KeyPress || e->type == KeyRelease);

    uchar keyCodeX = e->xkey.keycode;
    uint keyModX = e->xkey.state & (accelModMaskX() | MODE_SWITCH);

    KeySym keySym;
    char buffer[16];
    XLookupString( (XKeyEvent*) e, buffer, 15, &keySym, 0 );
    uint keySymX = (uint)keySym;

    // If numlock is active and a keypad key is pressed, XOR the SHIFT state.
    //  e.g., KP_4 => Shift+KP_Left, and Shift+KP_4 => KP_Left.
    if( e->xkey.state & modXNumLock() ) {
#ifdef HAVE_XKB
        uint sym = XkbKeycodeToKeysym( QX11Info::display(), keyCodeX, 0, 0 );
#else
        uint sym = XKeycodeToKeysym( QX11Info::display(), keyCodeX, 0 );
#endif
        // TODO: what's the xor operator in c++?
        // If this is a keypad key,
        if( sym >= XK_KP_Space && sym <= XK_KP_9 ) {
            switch( sym ) {
                // Leave the following keys unaltered
                // FIXME: The proper solution is to see which keysyms don't change when shifted.
                case XK_KP_Multiply:
                case XK_KP_Add:
                case XK_KP_Subtract:
                case XK_KP_Divide:
                    break;
                default:
                    if( keyModX & modXShift() )
                        keyModX &= ~modXShift();
                    else
                        keyModX |= modXShift();
            }
        }
    }

    int keyCodeQt;
    int keyModQt;
    symXToKeyQt(keySymX, &keyCodeQt);
    modXToQt(keyModX, &keyModQt);

    *keyQt = keyCodeQt | keyModQt;
    return true;
}


} // end of namespace KKeyServer block
