/* This file is part of the KDE project
   Copyright (C) 2001 Lubos Lunak <l.lunak@kde.org>

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

#include <config.h>

#include "startupid.h"
#include "klaunchsettings.h"

#include <kiconloader.h>
#include <qcursor.h>
#include <kapplication.h>
#include <qimage.h>
#include <qbitmap.h>
#include <kconfig.h>
#include <X11/Xlib.h>

#define KDE_STARTUP_ICON "kmenu"

#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif

enum kde_startup_status_enum { StartupPre, StartupIn, StartupDone };
static kde_startup_status_enum kde_startup_status = StartupPre;
static Atom kde_splash_progress;

StartupId::StartupId( QWidget* parent, const char* name )
    :   QWidget( parent, name ),
	startup_info( KStartupInfo::CleanOnCantDetect ),
	startup_widget( NULL ),
	blinking( true ),
	bouncing( false )
    {
    hide(); // is QWidget only because of x11Event()
    if( kde_startup_status == StartupPre )
        {
        kde_splash_progress = XInternAtom( qt_xdisplay(), "_KDE_SPLASH_PROGRESS", False );
        XWindowAttributes attrs;
        XGetWindowAttributes( qt_xdisplay(), qt_xrootwin(), &attrs);
        XSelectInput( qt_xdisplay(), qt_xrootwin(), attrs.your_event_mask | SubstructureNotifyMask);
        kapp->installX11EventFilter( this );
        }
    connect( &update_timer, SIGNAL( timeout()), SLOT( update_startupid()));
    connect( &startup_info,
        SIGNAL( gotNewStartup( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( gotNewStartup( const KStartupInfoId&, const KStartupInfoData& )));
    connect( &startup_info,
        SIGNAL( gotStartupChange( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( gotStartupChange( const KStartupInfoId&, const KStartupInfoData& )));
    connect( &startup_info,
        SIGNAL( gotRemoveStartup( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( gotRemoveStartup( const KStartupInfoId& )));
    }

StartupId::~StartupId()
    {
    stop_startupid();
    }
    
void StartupId::configure()
    {
    startup_info.setTimeout( KLaunchSettings::timeout());
    blinking = KLaunchSettings::blinking();
    bouncing = KLaunchSettings::bouncing();
    }

void StartupId::gotNewStartup( const KStartupInfoId& id_P, const KStartupInfoData& data_P )
    {
    QString icon = data_P.findIcon();
    current_startup = id_P;
    startups[ id_P ] = icon;
    start_startupid( icon );
    }

void StartupId::gotStartupChange( const KStartupInfoId& id_P, const KStartupInfoData& data_P )
    {
    if( current_startup == id_P )
        {
        QString icon = data_P.findIcon();
        if( !icon.isEmpty() && icon != startups[ current_startup ] )
            {
            startups[ id_P ] = icon;
            start_startupid( icon );
            }
        }
    }

void StartupId::gotRemoveStartup( const KStartupInfoId& id_P )
    {
    startups.remove( id_P );
    if( startups.count() == 0 )
        {
        current_startup = KStartupInfoId(); // null
        if( kde_startup_status == StartupIn )
            start_startupid( KDE_STARTUP_ICON );
        else
            stop_startupid();
        return;
        }
    current_startup = startups.begin().key();
    start_startupid( startups[ current_startup ] );
    }

bool StartupId::x11Event( XEvent* e )
    {
    if( e->type == ClientMessage && e->xclient.window == qt_xrootwin()
        && e->xclient.message_type == kde_splash_progress )
        {
        const char* s = e->xclient.data.b;
        if( strcmp( s, "kicker" ) == 0 && kde_startup_status == StartupPre )
            {
            kde_startup_status = StartupIn;
            if( startups.count() == 0 )
                start_startupid( KDE_STARTUP_ICON );
            // 60(?) sec timeout - shouldn't be hopefully needed anyway, ksmserver should have it too
            QTimer::singleShot( 60000, this, SLOT( finishKDEStartup()));
            }
        else if( strcmp( s, "session ready" ) == 0 && kde_startup_status < StartupDone )
            QTimer::singleShot( 2000, this, SLOT( finishKDEStartup()));
        }
    return false;
    }

void StartupId::finishKDEStartup()
    {
    kde_startup_status = StartupDone;
    kapp->removeX11EventFilter( this );
    if( startups.count() == 0 )
        stop_startupid();
    }

void StartupId::stop_startupid()
    {
    delete startup_widget;
    startup_widget = NULL;
    if( blinking )
        for( int i = 0;
             i < NUM_BLINKING_PIXMAPS;
             ++i )
            pixmaps[ i ] = QPixmap(); // null
    update_timer.stop();
    }

static QPixmap scalePixmap( const QPixmap& pm, int w, int h )
{
#if QT_VERSION >= 0x030200
	QPixmap result( 20, 20, pm.depth() );
	result.setMask( QBitmap( 20, 20, true ) );
	QPixmap scaled( pm.convertToImage().smoothScale( w, h ) );
	copyBlt( &result, (20 - w) / 2, (20 - h) / 2, &scaled, 0, 0, w, h );
	return result;
#else
	Q_UNUSED(w);
	Q_UNUSED(h);
	return pm;
#endif
}

void StartupId::start_startupid( const QString& icon_P )
    {

    const QColor startup_colors[ StartupId::NUM_BLINKING_PIXMAPS ]
    = { Qt::black, Qt::darkGray, Qt::lightGray, Qt::white, Qt::white };


    QPixmap icon_pixmap = KGlobal::iconLoader()->loadIcon( icon_P, KIcon::Small, 0,
        KIcon::DefaultState, 0, true ); // return null pixmap if not found
    if( icon_pixmap.isNull())
        icon_pixmap = SmallIcon( "exec" );
    if( startup_widget == NULL )
        {
        startup_widget = new QWidget( NULL, NULL, WX11BypassWM );
        XSetWindowAttributes attr;
        attr.save_under = True; // useful saveunder if possible to avoid redrawing
        XChangeWindowAttributes( qt_xdisplay(), startup_widget->winId(), CWSaveUnder, &attr );
        }
    startup_widget->resize( icon_pixmap.width(), icon_pixmap.height());
    if( blinking )
        {
        startup_widget->clearMask();
        int window_w = icon_pixmap.width();
        int window_h = icon_pixmap.height();
        for( int i = 0;
             i < NUM_BLINKING_PIXMAPS;
             ++i )
            {
            pixmaps[ i ] = QPixmap( window_w, window_h );
            pixmaps[ i ].fill( startup_colors[ i ] );
            bitBlt( &pixmaps[ i ], 0, 0, &icon_pixmap );
            }
        color_index = 0;
        }
    else if( bouncing )
        {
        startup_widget->resize( 20, 20 );
        pixmaps[ 0 ] = scalePixmap( icon_pixmap, 16, 16 );
        pixmaps[ 1 ] = scalePixmap( icon_pixmap, 14, 18 );
        pixmaps[ 2 ] = scalePixmap( icon_pixmap, 12, 20 );
        pixmaps[ 3 ] = scalePixmap( icon_pixmap, 18, 14 );
        pixmaps[ 4 ] = scalePixmap( icon_pixmap, 20, 12 );
        frame = 0;
        }
    else
        {
        if( icon_pixmap.mask() != NULL )
            startup_widget->setMask( *icon_pixmap.mask());
        else
            startup_widget->clearMask();
        startup_widget->setBackgroundPixmap( icon_pixmap );
        startup_widget->erase();
        }
    update_startupid();
    }

namespace
{
const int X_DIFF = 15;
const int Y_DIFF = 15;
const int color_to_pixmap[] = { 0, 1, 2, 3, 2, 1 };
const int frame_to_yoffset[] =
  {
    -5, -1, 2, 5, 8, 10, 12, 13, 15, 15, 15, 15, 14, 12, 10, 8, 5, 2, -1, -5
  };
const int frame_to_pixmap[] =
  {
    0, 0, 0, 1,  2,  2,  1,  0,  3,  4,  4,  3,  0,  1,  2,  2,  1,  0, 0, 0
  };
}

void StartupId::update_startupid()
    {
    int yoffset = 0;
    if( blinking )
        {
        startup_widget->setBackgroundPixmap( pixmaps[ color_to_pixmap[ color_index ]] );
        if( ++color_index >= ( sizeof( color_to_pixmap ) / sizeof( color_to_pixmap[ 0 ] )))
            color_index = 0;
        }
    else if( bouncing )
        {
        yoffset = frame_to_yoffset[ frame ];
        QPixmap pm = pixmaps[ frame_to_pixmap[ frame ] ];
        startup_widget->setBackgroundPixmap( pm );
        if ( pm.mask() != NULL )
            startup_widget->setMask( *pm.mask() );
        else
            startup_widget->clearMask();
        if ( ++frame >= ( sizeof( frame_to_yoffset ) / sizeof( frame_to_yoffset[ 0 ] ) ) )
            frame = 0;
        }
    Window dummy1, dummy2;
    int x, y;
    int dummy3, dummy4;
    unsigned int dummy5;
    if( !XQueryPointer( qt_xdisplay(), qt_xrootwin(), &dummy1, &dummy2, &x, &y, &dummy3, &dummy4, &dummy5 ))
        {
        startup_widget->hide();
        update_timer.start( 100, true );
        return;
        }
    QPoint c_pos( x, y );
    int cursor_size = 0;
#ifdef HAVE_XCURSOR
    cursor_size = XcursorGetDefaultSize( qt_xdisplay());
#endif
    int X_DIFF;
    if( cursor_size <= 16 )
        X_DIFF = 8 + 7;
    else if( cursor_size <= 32 )
        X_DIFF = 16 + 7;
    else if( cursor_size <= 48 )
        X_DIFF = 24 + 7;
    else
        X_DIFF = 32 + 7;
    int Y_DIFF = X_DIFF;
    if( startup_widget->x() != c_pos.x() + X_DIFF
        || startup_widget->y() != c_pos.y() + Y_DIFF + yoffset )
        startup_widget->move( c_pos.x() + X_DIFF, c_pos.y() + Y_DIFF + yoffset );
    startup_widget->show();
    XRaiseWindow( qt_xdisplay(), startup_widget->winId());
    update_timer.start( bouncing ? 30 : 100, true );
    QApplication::flushX();
    }

#include "startupid.moc"
