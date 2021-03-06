/****************************************************************************

 Copyright (C) 2003 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#include "kmanagerselection.h"

#include <config.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <QtCore/QObject>
#ifdef Q_WS_X11 // FIXME(E)

#include <qx11info_x11.h>
#include <qwidget.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kxerrorhandler.h>
#include <X11/Xatom.h>


class KSelectionOwner::Private : public QWidget
{
public:
    Private( KSelectionOwner* owner_P, Atom selection_P, int screen_P )
        : selection( selection_P ),
          screen( screen_P >= 0 ? screen_P : DefaultScreen( QX11Info::display() ) ),
          window( None ),
          timestamp( CurrentTime ),
          extra1( 0 ),
          extra2( 0 ),
          owner( owner_P )
    {
        kapp->installX11EventFilter( this );
    }

    const Atom selection;
    const int screen;
    Window window;
    Time timestamp;
    long extra1, extra2;
    static Atom manager_atom;
    static Atom xa_multiple;
    static Atom xa_targets;
    static Atom xa_timestamp;

protected:
    virtual bool x11Event( XEvent* ev_P )
    {
        return owner->filterEvent( ev_P );
    }

private:
    KSelectionOwner* owner;
};

    
KSelectionOwner::KSelectionOwner( Atom selection_P, int screen_P, QObject* parent_P )
    :   QObject( parent_P ),
        d( new Private( this, selection_P, screen_P ) )
{
}

KSelectionOwner::KSelectionOwner( const char* selection_P, int screen_P, QObject* parent_P )
    :   QObject( parent_P ),
        d( new Private( this, XInternAtom( QX11Info::display(), selection_P, False ), screen_P ) )
{
}

KSelectionOwner::~KSelectionOwner()
{
    release();
    delete d;
}

bool KSelectionOwner::claim( bool force_P, bool force_kill_P )
    {
    if( Private::manager_atom == None )
        getAtoms();
    if( d->timestamp != CurrentTime )
        release();
    Display* const dpy = QX11Info::display();
    Window prev_owner = XGetSelectionOwner( dpy, d->selection );
    if( prev_owner != None )
        {
        if( !force_P )
            {
//            kDebug() << "Selection already owned, failing";
            return false;
            }
        XSelectInput( dpy, prev_owner, StructureNotifyMask );
        }
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    d->window = XCreateWindow( dpy, RootWindow( dpy, d->screen ), 0, 0, 1, 1, 
        0, CopyFromParent, InputOnly, CopyFromParent, CWOverrideRedirect, &attrs );
//    kDebug() << "Using owner window " << window;
    Atom tmp = XA_ATOM;
    XSelectInput( dpy, d->window, PropertyChangeMask );
    XChangeProperty( dpy, d->window, XA_ATOM, XA_ATOM, 32, PropModeReplace,
        reinterpret_cast< unsigned char* >( &tmp ), 1 );
    XEvent ev;
    XSync( dpy, False );
    XCheckTypedWindowEvent( dpy, d->window, PropertyNotify, &ev ); // get a timestamp
    d->timestamp = ev.xproperty.time;
    XSelectInput( dpy, d->window, StructureNotifyMask ); // for DestroyNotify
    XSetSelectionOwner( dpy, d->selection, d->window, d->timestamp );
    Window new_owner = XGetSelectionOwner( dpy, d->selection );
    if( new_owner != d->window )
        {
//        kDebug() << "Failed to claim selection : " << new_owner;
        XDestroyWindow( dpy, d->window );
        d->timestamp = CurrentTime;
        return false;
        }
    if( prev_owner != None )
        {
//        kDebug() << "Waiting for previous owner to disown";
        for( int cnt = 0;
             ;
             ++cnt )
            {
            if( XCheckTypedWindowEvent( dpy, prev_owner, DestroyNotify, &ev ) == True )
                break;
            struct timeval tm = { 0, 50000 }; // 50 ms
            select( 0, NULL, NULL, NULL, &tm );
            if( cnt == 19 )
                {
                if( force_kill_P )
                    {
                    KXErrorHandler err;
//                    kDebug() << "Killing previous owner";
                    XKillClient( dpy, prev_owner );
                    err.error( true ); // ignore errors when killing
                    }
                break;
                }
            }
        }
    ev.type = ClientMessage;
    ev.xclient.window = RootWindow( dpy, d->screen );
    ev.xclient.display = dpy;
    ev.xclient.message_type = Private::manager_atom;
    ev.xclient.format = 32;
    ev.xclient.data.l[ 0 ] = d->timestamp;
    ev.xclient.data.l[ 1 ] = d->selection;
    ev.xclient.data.l[ 2 ] = d->window;
    ev.xclient.data.l[ 3 ] = d->extra1;
    ev.xclient.data.l[ 4 ] = d->extra2;
    XSendEvent( dpy, RootWindow( dpy, d->screen ), False, StructureNotifyMask, &ev );
//    kDebug() << "Claimed selection";
    return true;
    }

// destroy resource first
void KSelectionOwner::release()
    {
    if( d->timestamp == CurrentTime )
        return;
    XDestroyWindow( QX11Info::display(), d->window ); // also makes the selection not owned
//    kDebug() << "Releasing selection";
    d->timestamp = CurrentTime;
    }

Window KSelectionOwner::ownerWindow() const
    {
    if( d->timestamp == CurrentTime )
        return None;
    return d->window;
    }

void KSelectionOwner::setData( long extra1_P, long extra2_P )
    {
    d->extra1 = extra1_P;
    d->extra2 = extra2_P;
    }
    
bool KSelectionOwner::filterEvent( XEvent* ev_P )
    {
    if( d->timestamp != CurrentTime && ev_P->xany.window == d->window )
        {
        if( handleMessage( ev_P ))
            return true;
        }
    switch( ev_P->type )
	{
	case SelectionClear:
	    {
	    if( d->timestamp == CurrentTime || ev_P->xselectionclear.selection != d->selection )
	        return false;
	    d->timestamp = CurrentTime;
//	    kDebug() << "Lost selection";
            Window window = d->window;
	    emit lostOwnership();
	    XSelectInput( QX11Info::display(), window, 0 );
	    XDestroyWindow( QX11Info::display(), window );
	  return true;
	    }
	case DestroyNotify:
	    {
	    if( d->timestamp == CurrentTime || ev_P->xdestroywindow.window != d->window )
	        return false;
	    d->timestamp = CurrentTime;
//	    kDebug() << "Lost selection (destroyed)";
	    emit lostOwnership();
	  return true;
	    }
	case SelectionNotify:
	    {
	    if( d->timestamp == CurrentTime || ev_P->xselection.selection != d->selection )
	        return false;
	    // ignore?
	  return false;
	    }
	case SelectionRequest:
	    filter_selection_request( ev_P->xselectionrequest );
	  return false;
	}
    return false;
    }

bool KSelectionOwner::handleMessage( XEvent* )
    {
    return false;
    }

void KSelectionOwner::filter_selection_request( XSelectionRequestEvent& ev_P )
    {
    if( d->timestamp == CurrentTime || ev_P.selection != d->selection )
        return;
    if( ev_P.time != CurrentTime
        && ev_P.time - d->timestamp > 1U << 31 )
        return; // too old or too new request
//    kDebug() << "Got selection request";
    bool handled = false;
    if( ev_P.target == Private::xa_multiple )
        {
        if( ev_P.property != None )
            {
            const int MAX_ATOMS = 100; // no need to handle more?
            int format;
            Atom type;
            unsigned long items;
            unsigned long after;
            unsigned char* data;
            if( XGetWindowProperty( QX11Info::display(), ev_P.requestor, ev_P.property, 0,
                MAX_ATOMS, False, AnyPropertyType, &type, &format, &items, &after,
                &data ) == Success && format == 32 && items % 2 == 0 )
                {
                bool handled_array[ MAX_ATOMS ];
                Atom* atoms = reinterpret_cast< Atom* >( data );
                for( unsigned int i = 0;
                     i < items / 2;
                     ++i )
                    handled_array[ i ] = handle_selection(
                        atoms[ i * 2 ], atoms[ i * 2 + 1 ], ev_P.requestor );
                bool all_handled = true;
                for( unsigned int i = 0;
                     i < items / 2;
                     ++i )
                    if( !handled_array[ i ] )
                        {
                        all_handled = false;
                        atoms[ i * 2 + 1 ] = None;
                        }
                if( !all_handled )
                    XChangeProperty( QX11Info::display(), ev_P.requestor, ev_P.property, XA_ATOM,
                        32, PropModeReplace, reinterpret_cast< unsigned char* >( atoms ), items );
                handled = true;
                XFree( data );
                }
            }
        }
    else
        {
        if( ev_P.property == None ) // obsolete client
            ev_P.property = ev_P.target;
        handled = handle_selection( ev_P.target, ev_P.property, ev_P.requestor );
        }
    XEvent ev;
    ev.xselection.selection = ev_P.selection;
    ev.xselection.type = SelectionNotify;
    ev.xselection.display = QX11Info::display();
    ev.xselection.requestor = ev_P.requestor;
    ev.xselection.target = ev_P.target;
    ev.xselection.property = handled ? ev_P.property : None;
    XSendEvent( QX11Info::display(), ev_P.requestor, False, 0, &ev );
    }

bool KSelectionOwner::handle_selection( Atom target_P, Atom property_P, Window requestor_P )
    {
    if( target_P == Private::xa_timestamp )
        {
//        kDebug() << "Handling timestamp request";
        XChangeProperty( QX11Info::display(), requestor_P, property_P, XA_INTEGER, 32,
            PropModeReplace, reinterpret_cast< unsigned char* >( &d->timestamp ), 1 );
        }
    else if( target_P == Private::xa_targets )
        replyTargets( property_P, requestor_P );
    else if( genericReply( target_P, property_P, requestor_P ))
        ; // handled
    else
        return false; // unknown
    return true;
    }

void KSelectionOwner::replyTargets( Atom property_P, Window requestor_P )
    {
    Atom atoms[ 3 ] = { Private::xa_multiple, Private::xa_timestamp, Private::xa_targets };
//    kDebug() << "Handling targets request";
    XChangeProperty( QX11Info::display(), requestor_P, property_P, XA_ATOM, 32, PropModeReplace,
        reinterpret_cast< unsigned char* >( atoms ), 3 );
    }

bool KSelectionOwner::genericReply( Atom, Atom, Window )
    {
    return false;
    }

void KSelectionOwner::getAtoms()
    {
    if( Private::manager_atom == None )
        {
        Atom atoms[ 4 ];
        const char* const names[] =
            { "MANAGER", "MULTIPLE", "TARGETS", "TIMESTAMP" };
        XInternAtoms( QX11Info::display(), const_cast< char** >( names ), 4, False, atoms );
        Private::manager_atom = atoms[ 0 ];
        Private::xa_multiple = atoms[ 1];
        Private::xa_targets = atoms[ 2 ];
        Private::xa_timestamp = atoms[ 3 ];
        }
    }

Atom KSelectionOwner::Private::manager_atom = None;
Atom KSelectionOwner::Private::xa_multiple = None;
Atom KSelectionOwner::Private::xa_targets = None;
Atom KSelectionOwner::Private::xa_timestamp = None;

//*******************************************
// KSelectionWatcher
//*******************************************


class KSelectionWatcher::Private : public QWidget
{
public:
    Private( KSelectionWatcher* watcher_P, Atom selection_P, int screen_P )
        : selection( selection_P ),
          screen( screen_P >= 0 ? screen_P : DefaultScreen( QX11Info::display())),
          selection_owner( None ),
          watcher( watcher_P )
    {
        kapp->installX11EventFilter( this );
    }

    const Atom selection;
    const int screen;
    Window selection_owner;
    static Atom manager_atom;

protected:
    virtual bool x11Event( XEvent* ev_P )
    {
        watcher->filterEvent( ev_P );
        return false;
    }

private:
    KSelectionWatcher* watcher;
};

KSelectionWatcher::KSelectionWatcher( Atom selection_P, int screen_P, QObject* parent_P )
    :   QObject( parent_P ),
        d( new Private( this, selection_P, screen_P ))
    {
    init();
    }
    
KSelectionWatcher::KSelectionWatcher( const char* selection_P, int screen_P, QObject* parent_P )
    :   QObject( parent_P ),
        d( new Private( this, XInternAtom( QX11Info::display(), selection_P, False ), screen_P ))
    {
    init();
    }

KSelectionWatcher::~KSelectionWatcher()
    {
    delete d;
    }
    
void KSelectionWatcher::init()
    {
    if( Private::manager_atom == None )
        {
        Display* const dpy = QX11Info::display();
        Private::manager_atom = XInternAtom( dpy, "MANAGER", False );
        XWindowAttributes attrs;
        XGetWindowAttributes( dpy, RootWindow( dpy, d->screen ), &attrs );
        long event_mask = attrs.your_event_mask;
        // StructureNotifyMask on the root window is needed
        XSelectInput( dpy, RootWindow( dpy, d->screen ), event_mask | StructureNotifyMask );
        }
    owner(); // trigger reading of current selection status
    }    

Window KSelectionWatcher::owner()
    {
    Display* const dpy = QX11Info::display();
    KXErrorHandler handler;
    Window current_owner = XGetSelectionOwner( dpy, d->selection );
    if( current_owner == None )
        return None;
    if( current_owner == d->selection_owner )
        return d->selection_owner;
    XSelectInput( dpy, current_owner, StructureNotifyMask );
    if( !handler.error( true ) && current_owner == XGetSelectionOwner( dpy, d->selection ))
        {
//        kDebug() << "isOwner: " << current_owner;
        d->selection_owner = current_owner;
        emit newOwner( d->selection_owner );
        }
    else
        d->selection_owner = None;
    return d->selection_owner;
    }

// void return value in order to allow more watchers in one process
void KSelectionWatcher::filterEvent( XEvent* ev_P )
    {
    if( ev_P->type == ClientMessage )
        {
//        kDebug() << "got ClientMessage";
        if( ev_P->xclient.message_type != Private::manager_atom
            || ev_P->xclient.data.l[ 1 ] != static_cast< long >( d->selection ))
            return;
//        kDebug() << "handling message";
        if( static_cast< long >( owner()) == ev_P->xclient.data.l[ 2 ] )
            {
            // owner() emits newOwner() if needed, no need to do it twice
            }
        return;
        }
    if( ev_P->type == DestroyNotify )
        {
        if( d->selection_owner == None || ev_P->xdestroywindow.window != d->selection_owner )
            return;
        d->selection_owner = None; // in case the exactly same ID gets reused as the owner
        if( owner() == None )
            emit lostOwner(); // it must be safe to delete 'this' in a slot
        return;
        }
    return;
    }

Atom KSelectionWatcher::Private::manager_atom = None;

#include "moc_kmanagerselection.cpp"
#endif
