/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <qwidget.h>
#include <qtooltip.h>
#include <kwin.h>
#include <qvalidator.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kdebug.h>

#include "dockcontainer.h"
#include "dockcontainer.moc"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


DockContainer::DockContainer( QString command, QWidget *parent,
                              QString resname, QString resclass, bool undocked_style )
    : QFrame( parent, resname.ascii(),
              undocked_style ? WStyle_Customize |
              WStyle_StaysOnTop | WStyle_Tool |
              WStyle_NoBorder | WX11BypassWM : 0 ),
      _embeddedWinId(0),
      _command(command),
      _resName(resname),
      _resClass(resclass)
{
    XSelectInput( qt_xdisplay(), winId(),
		  KeyPressMask | KeyReleaseMask |
		  ButtonPressMask | ButtonReleaseMask |
		  KeymapStateMask |
		  ButtonMotionMask |
		  PointerMotionMask |
		  EnterWindowMask | LeaveWindowMask |
		  FocusChangeMask |
		  ExposureMask |
		  StructureNotifyMask |
		  SubstructureRedirectMask |
		  SubstructureNotifyMask );
    if (!undocked_style) {
        setFrameStyle(StyledPanel | Raised);
        setLineWidth(border());
        QToolTip::add(this, command);
    } else {
        setFrameStyle(StyledPanel | Plain);
        setLineWidth(1);
    }
    resize(sz(),sz());
}

void DockContainer::embed( WId id )
{
    if( id == _embeddedWinId || id == 0)
        return;
    QRect geom = KWin::windowInfo(id,NET::WMKDEFrameStrut).frameGeometry();

    // does the same as KWM::prepareForSwallowing()
    XWithdrawWindow( qt_xdisplay(), id, qt_xscreen() );
    while( KWin::windowInfo(id, NET::XAWMState).mappingState() != NET::Withdrawn );

    XReparentWindow( qt_xdisplay(), id, winId(), 0, 0 );

    // resize if window is bigger than frame
    if( (geom.width() > width()) ||
        (geom.height() > height()) )
        XResizeWindow( qt_xdisplay(), id, width(), height() );
    else
        XMoveWindow(qt_xdisplay(), id,
                    (sz() -  geom.width())/2 - border(),
                    (sz() - geom.height())/2 - border());
    XMapWindow( qt_xdisplay(), id );
    XUngrabButton( qt_xdisplay(), AnyButton, AnyModifier, winId() );

    _embeddedWinId = id;
}

void DockContainer::unembed()
{
    if( _embeddedWinId )
        XReparentWindow( qt_xdisplay(), _embeddedWinId, qt_xrootwin(), 0, 0 );
}

void DockContainer::kill()
{
    if ( _embeddedWinId ) {
        XKillClient( qt_xdisplay(), _embeddedWinId );
        _embeddedWinId = 0; // in case the window does not exist anymore..
    }
    else emit embeddedWindowDestroyed(this); /* enable killing of empty windows.. */
}

bool DockContainer::x11Event( XEvent *e )
{
    switch( e->type ) {
    case DestroyNotify:
	if( e->xdestroywindow.window == _embeddedWinId || _embeddedWinId == 0) {
	    _embeddedWinId = 0;
	    emit embeddedWindowDestroyed(this);
	}
	break;
    case UnmapNotify:
        if ( e->xunmap.window == _embeddedWinId ) {
            kdDebug() << "Unmap Notify !!! I hate smart dockapps as wmpinboard " << command() << endl;
            _embeddedWinId = 0;
        }
        break;
    case ReparentNotify:
	if( _embeddedWinId &&
	    (e->xreparent.window == _embeddedWinId) &&
	    (e->xreparent.parent != winId()) ) {
	    _embeddedWinId = 0;
	}
	else if( e->xreparent.parent == winId() ) {
	    _embeddedWinId = e->xreparent.window;
	    embed( _embeddedWinId );
	}
	break;
    }

    return false;
}

void DockContainer::askNewCommand(bool bad_command)
{
    bool ok;
    QString title( i18n("Enter Command Line for Applet %1.%2").arg(resName()).arg(resClass()) );
    QString description( i18n("This applet does not behave correctly and the dockbar was unable to "
                              "find the command line necessary to launch it the next time KDE starts up") );
    QString cmd;

    /*
      I was not able to figure out why valgrind complains inside the getText call..
      (invalid read of size 1 in Xmb.. functions)
    */
    if (bad_command) {
        cmd = KInputDialog::getText( title, description,
                                     command(), &ok, this );
    } else {
        cmd = KInputDialog::getText( title, QString::null,
                                     command(), &ok, this );
    }
    if (ok) { _command = cmd; emit settingsChanged(this); }
}

void DockContainer::popupMenu(QPoint p)
{
    int r;
    {
        KPopupMenu pm(this);
        pm.insertItem( i18n("Kill This Applet"), 0);
        pm.insertItem( i18n("Change Command"), 1);
        r = pm.exec(p);
        /* pm is destroyed now .. if it is destroyed later,
           there is a risk that kill() double-frees it */
    }
    switch (r) {
        case 0: {
            kill();
        } break;
        case 1: {
            askNewCommand(false);
        } break;
    }
}

int& DockContainer::sz() {
    static int _sz = 66;
    return _sz;
}

int& DockContainer::border() {
    static int _border = 1;
    return _border;
}
