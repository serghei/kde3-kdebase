/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003 <ravi@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kwin.h>

#include <qevent.h>
#include <qfile.h>
#include <qwidget.h>

#include <X11/Xlib.h>

#ifdef HAVE_XCURSOR
# include <X11/Xlib.h>
# include <X11/Xcursor/Xcursor.h>
#endif

#include "objkstheme.h"
#include "themeengine.h"
#include "themeengine.moc"

struct ThemeEngine::ThemeEnginePrivate
{
    QValueList< Window > mSplashWindows;
};

ThemeEngine::ThemeEngine( QWidget *, const char *, const QStringList& args )
  : QVBox( 0, "wndSplash", WStyle_Customize|WX11BypassWM ), d(0)
{
  d = new ThemeEnginePrivate;
  kapp->installX11EventFilter( this );
  kapp->installEventFilter( this );
  (void)kapp->desktop();
  XWindowAttributes rootAttr;
  XGetWindowAttributes(qt_xdisplay(), RootWindow(qt_xdisplay(),
                        qt_xscreen()), &rootAttr);
  XSelectInput( qt_xdisplay(), qt_xrootwin(),
        SubstructureNotifyMask | rootAttr.your_event_mask );
  if (args.isEmpty())
    mTheme = new ObjKsTheme( "Default" );
  else
    mTheme = new ObjKsTheme( args.first() );
  mTheme->loadCmdLineArgs( KCmdLineArgs::parsedArgs() );
}

ThemeEngine::~ThemeEngine()
{
    delete d;
}

/*
 This is perhaps a bit crude, but I'm not aware of any better way
 of fixing #85030 and keeping backwards compatibility if there
 are any 3rd party splashscreens. Check all toplevel windows,
 force them to be WX11BypassWM (so that ksplash can handle their stacking),
 and keep them on top, even above all windows handled by KWin.
*/
bool ThemeEngine::eventFilter( QObject* o, QEvent* e )
{
    if( e->type() == QEvent::Show && o->isWidgetType())
        addSplashWindow( static_cast< QWidget* >( o ));
    return false;
}

namespace
{
class HackWidget : public QWidget { friend class ::ThemeEngine; };
}

void ThemeEngine::addSplashWindow( QWidget* w )
{
    if( !w->isTopLevel())
        return;
    if( d->mSplashWindows.contains( w->winId()))
        return;
    if( !w->testWFlags( WX11BypassWM ))
    { // All toplevel widgets should be probably required to be WX11BypassWM
      // for KDE4 instead of this ugly hack.
        static_cast< HackWidget* >( w )->setWFlags( WX11BypassWM );
        XSetWindowAttributes attrs;
        attrs.override_redirect = True;
        XChangeWindowAttributes( qt_xdisplay(), w->winId(), CWOverrideRedirect, &attrs );
    }
    d->mSplashWindows.prepend( w->winId());
    connect( w, SIGNAL( destroyed( QObject* )), SLOT( splashWindowDestroyed( QObject* )));
    w->raise();
}

void ThemeEngine::splashWindowDestroyed( QObject* obj )
{
    d->mSplashWindows.remove( static_cast< QWidget* >( obj )->winId());
}

bool ThemeEngine::x11Event( XEvent* e )
{
    if( e->type != ConfigureNotify && e->type != MapNotify )
        return false;
    if( e->type == ConfigureNotify && e->xconfigure.event != qt_xrootwin())
        return false;
    if( e->type == MapNotify && e->xmap.event != qt_xrootwin())
        return false;
    if( d->mSplashWindows.count() == 0 )
        return false;
    // this restacking is written in a way so that
    // if the stacking positions actually don't change,
    // all restacking operations will be no-op,
    // and no ConfigureNotify will be generated,
    // thus avoiding possible infinite loops
    XRaiseWindow( qt_xdisplay(), d->mSplashWindows.first()); // raise topmost
    // and stack others below it
    Window* stack = new Window[ d->mSplashWindows.count() ];
    int count = 0;
    for( QValueList< Window >::ConstIterator it = d->mSplashWindows.begin();
         it != d->mSplashWindows.end();
         ++it )
        stack[ count++ ] = *it;
    XRestackWindows( x11Display(), stack, count );
    delete[] stack;
    return false;
}
