//----------------------------------------------------------------------------
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
// Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>
//
// KDE screensaver engine
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xautolock.h"
#include "xautolock.moc"

#include <kapplication.h>
#include <kdebug.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <ctime>
#include "xautolock_c.h"

#ifdef HAVE_DPMS
extern "C" {
#include <X11/Xmd.h>
#ifndef Bool
#define Bool BOOL
#endif
#include <X11/extensions/dpms.h>

#ifndef HAVE_DPMSINFO_PROTO
Status DPMSInfo ( Display *, CARD16 *, BOOL * );
#endif
}
#endif

int xautolock_useXidle = 0;
int xautolock_useMit = 0;
xautolock_corner_t xautolock_corners[ 4 ];

static XAutoLock* self = NULL;

extern "C" {
static int catchFalseAlarms(Display *, XErrorEvent *)
{
    return 0;
}
}

//===========================================================================
//
// Detect user inactivity.
// Named XAutoLock after the program that it is based on.
//
XAutoLock::XAutoLock()
{
    self = this;
    xautolock_useXidle = 0;
    xautolock_useMit = 0;
#ifdef HAVE_XIDLE
    int dummy1;
    xautolock_useXidle = XidleQueryExtension( qt_xdisplay(), &dummy1, &dummy1 );
#endif
#ifdef HAVE_XSCREENSAVER
    int dummy2;
    if( !xautolock_useXidle )
        xautolock_useMit = XScreenSaverQueryExtension( qt_xdisplay(), &dummy2, &dummy2 );
#endif
    if( !xautolock_useXidle && !xautolock_useMit )
    {
        kapp->installX11EventFilter( this );
        int (*oldHandler)(Display *, XErrorEvent *);
        oldHandler = XSetErrorHandler(catchFalseAlarms);
        XSync(qt_xdisplay(), False );
        xautolock_initDiy( qt_xdisplay());
        XSync(qt_xdisplay(), False );
        XSetErrorHandler(oldHandler);
    }

    mTimeout = DEFAULT_TIMEOUT;
    mDPMS = true;
    resetTrigger();

    time(&mLastTimeout);
    mActive = false;

    mTimerId = startTimer( CHECK_INTERVAL );

}

//---------------------------------------------------------------------------
//
// Destructor.
//
XAutoLock::~XAutoLock()
{
  self = NULL;
}

//---------------------------------------------------------------------------
//
// The time in seconds of continuous inactivity.
//
void XAutoLock::setTimeout(int t)
{
    mTimeout = t;
    resetTrigger();
}

void XAutoLock::setDPMS(bool s)
{
#ifdef HAVE_DPMS
    BOOL on;
    CARD16 state;
    DPMSInfo( qt_xdisplay(), &state, &on );
    if (!on)
        s = false;
#endif
    mDPMS = s;
}

//---------------------------------------------------------------------------
//
// Start watching Activity
//
void XAutoLock::start()
{
    resetTrigger();
    time(&mLastTimeout);
    mActive = true;
}

//---------------------------------------------------------------------------
//
// Stop watching Activity
//
void XAutoLock::stop()
{
    mActive = false;
}

//---------------------------------------------------------------------------
//
// Reset the trigger time.
//
void XAutoLock::resetTrigger()
{
    mTrigger = time(0) + mTimeout;
}

//---------------------------------------------------------------------------
//
// Move the trigger time in order to postpone (repeat) emitting of timeout().
//
void XAutoLock::postpone()
{
    mTrigger = time(0) + 60; // delay by 60sec
}

//---------------------------------------------------------------------------
//
// Set the remaining time to 't', if it's shorter than already set.
//
void XAutoLock::setTrigger( time_t t )
{
    if( t < mTrigger )
        mTrigger = t;
}

//---------------------------------------------------------------------------
//
// Process new windows and check the mouse.
//
void XAutoLock::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() != mTimerId)
    {
        return;
    }

    int (*oldHandler)(Display *, XErrorEvent *) = NULL;
    if( !xautolock_useXidle && !xautolock_useMit )
    { // only the diy way needs special X handler
        XSync( qt_xdisplay(), False );
        oldHandler = XSetErrorHandler(catchFalseAlarms);
    }

    xautolock_processQueue();

    time_t now = time(0);
    if ((now > mLastTimeout && now - mLastTimeout > TIME_CHANGE_LIMIT) ||
        (mLastTimeout > now && mLastTimeout - now > TIME_CHANGE_LIMIT+1))
    {
        /* the time has changed in one large jump.  This could be because
           the date was changed, or the machine was suspended.  We'll just
           reset the triger. */
        resetTrigger();
    }

    mLastTimeout = now;

    xautolock_queryIdleTime( qt_xdisplay());
    xautolock_queryPointer( qt_xdisplay());

    if( !xautolock_useXidle && !xautolock_useMit )
        XSetErrorHandler(oldHandler);

    bool activate = false;

    //kdDebug() << now << " " << mTrigger << endl;
    if (now >= mTrigger)
    {
        resetTrigger();
        activate = true;
    }

#ifdef HAVE_DPMS
    BOOL on;
    CARD16 state;
    DPMSInfo( qt_xdisplay(), &state, &on );

    //kdDebug() << "DPMSInfo " << state << " " << on << endl;
    // If DPMS is active, it makes XScreenSaverQueryInfo() report idle time
    // that is always smaller than DPMS timeout (X bug I guess). So if DPMS
    // saving is active, simply always activate our saving too, otherwise
    // this could prevent locking from working.
    if(state == DPMSModeStandby || state == DPMSModeSuspend || state == DPMSModeOff)
        activate = true;
    if(!on && mDPMS) {
        activate = false;
#ifdef HAVE_XSCREENSAVER
        XForceScreenSaver(qt_xdisplay(), ScreenSaverReset );
#endif
        resetTrigger();
    }
#endif
    
#ifdef HAVE_XSCREENSAVER
    static XScreenSaverInfo* mitInfo = 0;
    if (!mitInfo) mitInfo = XScreenSaverAllocInfo ();
    if (XScreenSaverQueryInfo (qt_xdisplay(), DefaultRootWindow (qt_xdisplay()), mitInfo)) {
        //kdDebug() << "XScreenSaverQueryInfo " << mitInfo->state << " " << ScreenSaverDisabled << endl;
        if (mitInfo->state == ScreenSaverDisabled)
            activate = false;
    }
#endif

    if(mActive && activate)
        emit timeout();
}

bool XAutoLock::x11Event( XEvent* ev )
{
    xautolock_processEvent( ev );
// don't futher process key events that were received only because XAutoLock wants them
    if( ev->type == KeyPress && !ev->xkey.send_event
        && !xautolock_useXidle && !xautolock_useMit
        && !QWidget::find( ev->xkey.window ))
        return true;
    return false;
}

bool XAutoLock::ignoreWindow( WId w )
{
    if( w != qt_xrootwin() && QWidget::find( w ))
        return true;
    return false;
}

extern "C"
void xautolock_resetTriggers()
{
  self->resetTrigger();
}

extern "C"
void xautolock_setTrigger( time_t t )
{
  self->setTrigger( t );
}

extern "C"
int xautolock_ignoreWindow( Window w )
{
   return self->ignoreWindow( w );
}
