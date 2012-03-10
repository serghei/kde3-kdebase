/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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


#include "desktop.h"
#include "krootwm.h"
#include "bgmanager.h"
#include "bgsettings.h"
#include "startupid.h"
#include "kdiconview.h"
#include "minicli.h"
#include "kdesktopsettings.h"
#include "klaunchsettings.h"

#include <string.h>
#include <unistd.h>
#include <kcolordrag.h>
#include <kurldrag.h>

#include <qdir.h>
#include <qevent.h>
#include <qtooltip.h>

#include <netwm.h>
#include <dcopclient.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kimageio.h>
#include <kinputdialog.h>
#include <kipc.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kprocess.h>
#include <ksycoca.h>
#include <ktempfile.h>
#include <kmessagebox.h>
#include <kglobalaccel.h>
#include <kwinmodule.h>
#include <krun.h>
#include <kwin.h>
#include <kglobalsettings.h>
#include <kpopupmenu.h>
#include <kapplication.h>
// Create the equivalent of KAccelBase::connectItem
// and then remove this include and fix reconnects in initRoot() -- ellis
//#include <kaccelbase.h>

extern int kdesktop_screen_number;
extern QCString kdesktop_name, kicker_name, kwin_name;

KRootWidget::KRootWidget() : QObject()
{
     kapp->desktop()->installEventFilter(this);
     kapp->desktop()->setAcceptDrops( true );
}

bool KRootWidget::eventFilter ( QObject *, QEvent * e )
{
     if (e->type() == QEvent::MouseButtonPress)
     {
       QMouseEvent *me = static_cast<QMouseEvent *>(e);
       KRootWm::self()->mousePressed( me->globalPos(), me->button() );
       return true;
     }
     else if (e->type() == QEvent::Wheel)
     {
       QWheelEvent *we = static_cast<QWheelEvent *>(e);
       emit wheelRolled(we->delta());
       return true;
     }
     else if ( e->type() == QEvent::DragEnter )
     {
       QDragEnterEvent* de = static_cast<QDragEnterEvent *>( e );
       bool b = !KGlobal::config()->isImmutable() && !KGlobal::dirs()->isRestrictedResource( "wallpaper" );

       bool imageURL = false;
       if ( KURLDrag::canDecode( de ) )
       {
         KURL::List list;
         KURLDrag::decode( de, list );
         KURL url = list.first();
         KMimeType::Ptr mime = KMimeType::findByURL( url );
         if ( !KImageIO::type( url.path() ).isEmpty() ||
              KImageIO::isSupported( mime->name(), KImageIO::Reading ) || mime->is( "image/svg+xml" ) )
           imageURL = true;
       }

       b = b && ( KColorDrag::canDecode( de ) || QImageDrag::canDecode( de ) || imageURL );
       de->accept( b );
       return true;
     }
     else if ( e->type() == QEvent::Drop )
     {
       QDropEvent* de = static_cast<QDropEvent*>( e );
       if ( KColorDrag::canDecode( de ) ) 
         emit colorDropEvent( de );
       else if ( QImageDrag::canDecode( de ) )
         emit imageDropEvent( de );
       else if ( KURLDrag::canDecode( de ) ) {
	 KURL::List list;
         KURLDrag::decode( de, list );
         KURL url = list.first();
         emit newWallpaper( url );
       }
       return true;
     }
     return false; // Don't filter.
}

// -----------------------------------------------------------------------------
#define DEFAULT_DELETEACTION 1

KDesktop::WheelDirection KDesktop::m_eWheelDirection = KDesktop::m_eDefaultWheelDirection;
const char* KDesktop::m_wheelDirectionStrings[2] = { "Forward", "Reverse" };

KDesktop::KDesktop( bool x_root_hack, bool wait_for_kded ) :
    DCOPObject( "KDesktopIface" ),
    QWidget( 0L, "desktop", WResizeNoErase | ( x_root_hack ? (WStyle_Customize | WStyle_NoBorder) : 0) ),
    // those two WStyle_ break kdesktop when the root-hack isn't used (no Dnd)
   startup_id( NULL ), m_waitForKicker(0)
{
  m_bWaitForKded = wait_for_kded;
  m_miniCli = 0; // created on demand
  keys = 0; // created later
  KGlobal::locale()->insertCatalogue("kdesktop");
  KGlobal::locale()->insertCatalogue("libkonq"); // needed for apps using libkonq
  KGlobal::locale()->insertCatalogue("libdmctl");

  setCaption( "KDE Desktop");

  setAcceptDrops(true); // WStyle_Customize seems to disable that
  m_pKwinmodule = new KWinModule( this );

  kapp->dcopClient()->setNotifications(true);
  kapp->dcopClient()->connectDCOPSignal(kicker_name, kicker_name, "desktopIconsAreaChanged(QRect, int)",
                                        "KDesktopIface", "desktopIconsAreaChanged(QRect, int)", false);

  // Dont repaint on configuration changes during construction
  m_bInit = true;

  // It's the child widget that gets the focus, not us
  setFocusPolicy( NoFocus );

  if ( x_root_hack )
  {
    // this is a ugly hack to make Dnd work
    // Matthias told me that it won't be necessary with kwin
    // actually my first try with ICCCM (Dirk) :-)
    unsigned long data[2];
    data[0] = (unsigned long) 1;
    data[1] = (unsigned long) 0; // None; (Werner)
    Atom wm_state = XInternAtom(qt_xdisplay(), "WM_STATE", False);
    XChangeProperty(qt_xdisplay(), winId(), wm_state, wm_state, 32,
                    PropModeReplace, (unsigned char *)data, 2);

  }

  setGeometry( QApplication::desktop()->geometry() );
  lower();

  connect( kapp, SIGNAL( shutDown() ),
           this, SLOT( slotShutdown() ) );

  connect(kapp, SIGNAL(settingsChanged(int)),
          this, SLOT(slotSettingsChanged(int)));
  kapp->addKipcEventMask(KIPC::SettingsChanged);

  kapp->addKipcEventMask(KIPC::IconChanged);
  connect(kapp, SIGNAL(iconChanged(int)), this, SLOT(slotIconChanged(int)));

  connect(KSycoca::self(), SIGNAL(databaseChanged()),
          this, SLOT(slotDatabaseChanged()));

  m_pIconView = 0;
  m_pRootWidget = 0;
  bgMgr = 0;
  initRoot();

  QTimer::singleShot(0, this, SLOT( slotStart() ));

#if (QT_VERSION-0 >= 0x030200) // XRANDR support
  connect( kapp->desktop(), SIGNAL( resized( int )), SLOT( desktopResized()));
#endif
}

void
KDesktop::initRoot()
{
  Display *dpy = qt_xdisplay();
  Window root = RootWindow(dpy, kdesktop_screen_number);
  XDefineCursor(dpy, root, cursor().handle());
  
  m_bDesktopEnabled = KDesktopSettings::desktopEnabled();
  if ( !m_bDesktopEnabled && !m_pRootWidget )
  {
     hide();
     delete bgMgr;
     bgMgr = 0;
     if ( m_pIconView )
        m_pIconView->saveIconPositions();
     delete m_pIconView;
     m_pIconView = 0;

     { // trigger creation of QToolTipManager, it does XSelectInput() on the root window
     QWidget w;
     QToolTip::add( &w, "foo" );
     }
     // NOTE: If mouse clicks stop working again, it's most probably something doing XSelectInput()
     // on the root window after this, and setting it to some fixed value instead of adding its mask.
     XWindowAttributes attrs;
     XGetWindowAttributes(dpy, root, &attrs);
     XSelectInput(dpy, root, attrs.your_event_mask | ButtonPressMask);

     m_pRootWidget = new KRootWidget;
     connect(m_pRootWidget, SIGNAL(wheelRolled(int)), this, SLOT(slotSwitchDesktops(int)));
     connect(m_pRootWidget, SIGNAL(colorDropEvent(QDropEvent*)), this, SLOT(handleColorDropEvent(QDropEvent*)) );
     connect(m_pRootWidget, SIGNAL(imageDropEvent(QDropEvent*)), this, SLOT(handleImageDropEvent(QDropEvent*)) );
     connect(m_pRootWidget, SIGNAL(newWallpaper(const KURL&)), this, SLOT(slotNewWallpaper(const KURL&)) );

     // Geert Jansen: backgroundmanager belongs here
     // TODO tell KBackgroundManager if we change widget()
     bgMgr = new KBackgroundManager( m_pIconView, m_pKwinmodule );
     bgMgr->setExport(1);
     connect( bgMgr, SIGNAL( initDone()), SLOT( backgroundInitDone()));
     if (!m_bInit)
     {
        delete KRootWm::self();
        KRootWm* krootwm = new KRootWm( this ); // handler for root menu (used by kdesktop on RMB click)
        keys->setSlot("Lock Session", krootwm, SLOT(slotLock()));
        keys->updateConnections();
     }
  }
  else if (m_bDesktopEnabled && !m_pIconView)
  {
     delete bgMgr;
     bgMgr = 0;
     delete m_pRootWidget;
     m_pRootWidget = 0;
     m_pIconView = new KDIconView( this, 0 );
     connect( m_pIconView, SIGNAL( imageDropEvent( QDropEvent * ) ),
              this, SLOT( handleImageDropEvent( QDropEvent * ) ) );
     connect( m_pIconView, SIGNAL( colorDropEvent( QDropEvent * ) ),
              this, SLOT( handleColorDropEvent( QDropEvent * ) ) );
     connect( m_pIconView, SIGNAL( newWallpaper( const KURL & ) ),
              this, SLOT( slotNewWallpaper( const KURL & ) ) );
     connect( m_pIconView, SIGNAL( wheelRolled( int ) ),
              this, SLOT( slotSwitchDesktops( int ) ) );

     // All the QScrollView/QWidget-specific stuff should go here, so that we can use
     // another qscrollview/widget instead of the iconview and use the same code
     m_pIconView->setVScrollBarMode( QScrollView::AlwaysOff );
     m_pIconView->setHScrollBarMode( QScrollView::AlwaysOff );
     m_pIconView->setDragAutoScroll( false );
     m_pIconView->setFrameStyle( QFrame::NoFrame );
     m_pIconView->viewport()->setBackgroundMode( X11ParentRelative );
     m_pIconView->setFocusPolicy( StrongFocus );
     m_pIconView->viewport()->setFocusPolicy( StrongFocus );
     m_pIconView->setGeometry( geometry() );
     m_pIconView->show();

     // Geert Jansen: backgroundmanager belongs here
     // TODO tell KBackgroundManager if we change widget()
     bgMgr = new KBackgroundManager( m_pIconView, m_pKwinmodule );
     bgMgr->setExport(1);
     connect( bgMgr, SIGNAL( initDone()), SLOT( backgroundInitDone()));

     // make sure it is initialized before we first call updateWorkArea()
     m_pIconView->initConfig( m_bInit );

     // set the size of the area for desktop icons placement
     {
        QByteArray data, result;
        QDataStream arg(data, IO_WriteOnly);
        arg << kdesktop_screen_number;
        QCString replyType;
        QRect area;

        if ( kapp->dcopClient()->call(kicker_name, kicker_name, "desktopIconsArea(int)",
                                       data, replyType, result, false, 2000) )
        {
          QDataStream res(result, IO_ReadOnly);
          res >> area;

          m_pIconView->updateWorkArea(area);
        }
        else
          if ( m_bInit )
          {
            // if we failed to get the information from kicker wait a little - probably
            // this is the KDE startup and kicker is simply not running yet
            m_waitForKicker = new QTimer(this);
            connect(m_waitForKicker, SIGNAL(timeout()), this, SLOT(slotNoKicker()));
            m_waitForKicker->start(15000, true);
          }
          else  // we are not called from the ctor, so kicker should already run
          {
            area = kwinModule()->workArea(kwinModule()->currentDesktop());
            m_pIconView->updateWorkArea(area);
          }
     }

     if (!m_bInit)
     {
        m_pIconView->start();
        delete KRootWm::self();
        KRootWm* krootwm = new KRootWm( this ); // handler for root menu (used by kdesktop on RMB click)
        keys->setSlot("Lock Session", krootwm, SLOT(slotLock()));
        keys->updateConnections();
     }
   } else {
     DCOPRef r( "ksmserver", "ksmserver" );
     r.send( "resumeStartup", QCString( "kdesktop" ));
   }

   KWin::setType( winId(), NET::Desktop );
   KWin::setState( winId(), NET::SkipPager );
   KWin::setOnAllDesktops( winId(), true );
}

void KDesktop::slotNoKicker()
{
    kdDebug(1204) << "KDesktop::slotNoKicker ... kicker did not respond" << endl;
    // up till now, we got no desktopIconsArea from kicker - probably
    // it's not running, so use the area from KWinModule
    m_pIconView->updateWorkArea(kwinModule()->workArea(kwinModule()->currentDesktop()));
}

void
KDesktop::backgroundInitDone()
{
    //kdDebug(1204) << "KDesktop::backgroundInitDone" << endl;
    // avoid flicker
    if (m_bDesktopEnabled)
    {
       const QPixmap *bg = QApplication::desktop()->screen()->backgroundPixmap();
       if ( bg )
          m_pIconView->setErasePixmap( *bg );

       show();
       kapp->sendPostedEvents();
    }

    DCOPRef r( "ksmserver", "ksmserver" );
    r.send( "resumeStartup", QCString( "kdesktop" ));
}

void
KDesktop::slotStart()
{
  //kdDebug(1204) << "KDesktop::slotStart" << endl;
  if (!m_bInit) return;

  // In case we started without database
  KImageIO::registerFormats();

  initConfig();

//   if (m_bDesktopEnabled)
//   {
//      // We need to be visible in order to insert icons, even if the background isn't ready yet...

//      show();
//   }

  // Now we may react to configuration changes
  m_bInit = false;

  if (m_pIconView)
     m_pIconView->start();

  // Global keys
  keys = new KGlobalAccel( this );
  (void) new KRootWm( this );

#include "kdesktopbindings.cpp"

  keys->readSettings();
  keys->updateConnections();

  connect(kapp, SIGNAL(appearanceChanged()), SLOT(slotConfigure()));

  QTimer::singleShot(300, this, SLOT( slotUpAndRunning() ));
}

void
KDesktop::runAutoStart()
{
     // now let's execute all the stuff in the autostart folder.
     // the stuff will actually be really executed when the event loop is
     // entered, since KRun internally uses a QTimer
     QDir dir( KGlobalSettings::autostartPath() );
     QStringList entries = dir.entryList( QDir::Files );
     QStringList::Iterator it = entries.begin();
     QStringList::Iterator end = entries.end();
     for (; it != end; ++it )
     {
            // Don't execute backup files
            if ( (*it).right(1) != "~" && (*it).right(4) != ".bak" &&
                 ( (*it)[0] != '%' || (*it).right(1) != "%" ) &&
                 ( (*it)[0] != '#' || (*it).right(1) != "#" ) )
            {
                KURL url;
                url.setPath( dir.absPath() + '/' + (*it) );
                (void) new KRun( url, 0, true );
            }
     }
}

// -----------------------------------------------------------------------------

KDesktop::~KDesktop()
{
  delete m_miniCli;
  m_miniCli = 0; // see #120382
  delete bgMgr;
  bgMgr = 0;
  delete startup_id;
}

// -----------------------------------------------------------------------------

void KDesktop::initConfig()
{
    if (m_pIconView)
       m_pIconView->initConfig( m_bInit );

    if ( keys )
    {
        keys->readSettings();
        keys->updateConnections();
    }

    KLaunchSettings::self()->readConfig();
    if( !KLaunchSettings::busyCursor() )
    {
        delete startup_id;
        startup_id = NULL;
    }
    else
    {
        if( startup_id == NULL )
            startup_id = new StartupId;
        startup_id->configure();
    }

    set_vroot = KDesktopSettings::setVRoot();
    slotSetVRoot(); // start timer

    m_bWheelSwitchesWorkspace = KDesktopSettings::wheelSwitchesWorkspace();

    const char* forward_string = m_wheelDirectionStrings[Forward];
    m_eWheelDirection =
        (KDesktopSettings::wheelDirection() == forward_string) ? Forward : Reverse;
}

// -----------------------------------------------------------------------------

void KDesktop::slotExecuteCommand()
{
    // this function needs to be duplicated since it appears that one
    // cannot have a 'slot' be a DCOP method.  if this changes in the
    // future, then 'slotExecuteCommand' and 'popupExecuteCommand' can
    // merge into one slot.
    popupExecuteCommand();
}

/*
  Shows minicli
 */
void KDesktop::popupExecuteCommand()
{
  popupExecuteCommand("");
}

void KDesktop::popupExecuteCommand(const QString& command)
{
  if (m_bInit)
      return;

  if (!kapp->authorize("run_command"))
      return;

  // Created on demand
  if ( !m_miniCli )
  {
      m_miniCli = new Minicli( this );
      m_miniCli->adjustSize(); // for the centering below
  }

  if (!command.isEmpty())
      m_miniCli->setCommand(command);

  // Move minicli to the current desktop
  NETWinInfo info( qt_xdisplay(), m_miniCli->winId(), qt_xrootwin(), NET::WMDesktop );
  int currentDesktop = kwinModule()->currentDesktop();
  if ( info.desktop() != currentDesktop )
      info.setDesktop( currentDesktop );

  if ( m_miniCli->isVisible() ) {
      KWin::forceActiveWindow( m_miniCli->winId() );
  } else {
      QRect rect = KGlobalSettings::desktopGeometry(QCursor::pos());
      m_miniCli->move(rect.x() + (rect.width() - m_miniCli->width())/2,
                      rect.y() + (rect.height() - m_miniCli->height())/2);
      m_miniCli->show(); // non-modal
  }
}

void KDesktop::slotSwitchUser()
{
     KRootWm::self()->slotSwitchUser();
}

void KDesktop::slotShowWindowList()
{
     KRootWm::self()->slotWindowList();
}

void KDesktop::slotShowTaskManager()
{
    //kdDebug(1204) << "Launching KSysGuard..." << endl;
    KProcess* p = new KProcess;
    Q_CHECK_PTR(p);

    *p << "ksysguard";
    *p << "--showprocesses";

    p->start(KProcess::DontCare);

    delete p;
}

// -----------------------------------------------------------------------------

void KDesktop::rearrangeIcons()
{
    if (m_pIconView)
        m_pIconView->rearrangeIcons();
}

void KDesktop::lineupIcons()
{
    if (m_pIconView)
        m_pIconView->lineupIcons();
}

void KDesktop::selectAll()
{
    if (m_pIconView)
        m_pIconView->selectAll( true );
}

void KDesktop::unselectAll()
{
    if (m_pIconView)
        m_pIconView->selectAll( false );
}

QStringList KDesktop::selectedURLs()
{
    if (m_pIconView)
        return m_pIconView->selectedURLs();
    return QStringList();
}

void KDesktop::refreshIcons()
{
    if (m_pIconView)
        m_pIconView->refreshIcons();
}

KActionCollection * KDesktop::actionCollection()
{
    if (!m_pIconView)
       return 0;
    return m_pIconView->actionCollection();
}

KURL KDesktop::url() const
{
    if (m_pIconView)
        return m_pIconView->url();
    return KURL();
}

// -----------------------------------------------------------------------------

void KDesktop::slotConfigure()
{
    configure();
}

void KDesktop::configure()
{
    // re-read configuration and apply it
    KGlobal::config()->reparseConfiguration();
    KDesktopSettings::self()->readConfig();

    // If we have done start() already, then re-configure.
    // Otherwise, start() will call initConfig anyway
    if (!m_bInit)
    {
       initRoot();
       initConfig();
       KRootWm::self()->initConfig();
    }

    if (keys)
    {
       keys->readSettings();
       keys->updateConnections();
    }
}

void KDesktop::slotSettingsChanged(int category)
{
    //kdDebug(1204) << "KDesktop::slotSettingsChanged" << endl;
    if (category == KApplication::SETTINGS_PATHS)
    {
        kdDebug(1204) << "KDesktop::slotSettingsChanged SETTINGS_PATHS" << endl;
        if (m_pIconView)
            m_pIconView->recheckDesktopURL();
    }
    else if (category == KApplication::SETTINGS_SHORTCUTS)
    {
        kdDebug(1204) << "KDesktop::slotSettingsChanged SETTINGS_SHORTCUTS" << endl;
        keys->readSettings();
        keys->updateConnections();
    }
}

void KDesktop::slotIconChanged(int group)
{
    if ( group == KIcon::Desktop )
    {
        kdDebug(1204) << "KDesktop::slotIconChanged" << endl;
        refresh();
    }
}

void KDesktop::slotDatabaseChanged()
{
    //kdDebug(1204) << "KDesktop::slotDatabaseChanged" << endl;
    if (m_bInit) // kded is done, now we can "start" for real
        slotStart();
    if (m_pIconView && KSycoca::isChanged("mimetypes"))
        m_pIconView->refreshMimeTypes();
}

void KDesktop::refresh()
{
  // George Staikos 3/14/01
  // This bit will just refresh the desktop and icons.  Now I have code
  // in KWin to do a complete refresh so this isn't really needed.
  // I'll leave it in here incase the plan is changed again
#if 0
  m_bNeedRepaint |= 1;
  updateWorkArea();
#endif
  kapp->dcopClient()->send( kwin_name, "", "refresh()", "");
  refreshIcons();
}

// -----------------------------------------------------------------------------

void KDesktop::slotSetVRoot()
{
    if (!m_pIconView)
        return;

    if (KWin::windowInfo(winId()).mappingState() == NET::Withdrawn) {
        QTimer::singleShot(100, this, SLOT(slotSetVRoot()));
        return;
    }

    unsigned long rw = RootWindowOfScreen(ScreenOfDisplay(qt_xdisplay(), qt_xscreen()));
    unsigned long vroot_data[1] = { m_pIconView->viewport()->winId() };
    static Atom vroot = XInternAtom(qt_xdisplay(), "__SWM_VROOT", False);

    Window rootReturn, parentReturn, *children;
    unsigned int numChildren;
    Window top = winId();
    while (1) {
        /*int ret = */XQueryTree(qt_xdisplay(), top , &rootReturn, &parentReturn,
                                 &children, &numChildren);
        if (children)
            XFree((char *)children);
        if (parentReturn == rw) {
            break;
        } else
            top = parentReturn;
    }
    if ( set_vroot )
        XChangeProperty(qt_xdisplay(), top, vroot, XA_WINDOW, 32,
                        PropModeReplace, (unsigned char *)vroot_data, 1);
    else
        XDeleteProperty (qt_xdisplay(), top, vroot);
}

// -----------------------------------------------------------------------------

void KDesktop::slotShutdown()
{
    if ( m_pIconView )
        m_pIconView->saveIconPositions();
    if ( m_miniCli )
        m_miniCli->saveConfig();
}

// don't hide when someone presses Alt-F4 on us
void KDesktop::closeEvent(QCloseEvent *e)
{
    e->ignore();
}

void KDesktop::desktopIconsAreaChanged(const QRect &area, int screen)
{
    // hurra! kicker is alive
    if ( m_waitForKicker ) m_waitForKicker->stop();

    // -2: all screens
    // -1: primary screen
    // else: screen number

    if (screen <= -2)
       screen = kdesktop_screen_number;
    else if (screen == -1)
       screen = kapp->desktop()->primaryScreen();

    // This is pretty broken, mixes Xinerama and non-Xinerama multihead
    // and generally doesn't seem to be required anyway => ignore screen.
    if ( /*(screen == kdesktop_screen_number) &&*/ m_pIconView )
        m_pIconView->updateWorkArea(area);
}

void KDesktop::slotSwitchDesktops(int delta)
{
    if(m_bWheelSwitchesWorkspace && KWin::numberOfDesktops() > 1)
    {
      int newDesk, curDesk = KWin::currentDesktop();

      if( (delta < 0 && m_eWheelDirection == Forward) || (delta > 0 && m_eWheelDirection == Reverse) )
        newDesk = curDesk % KWin::numberOfDesktops() + 1;
      else
        newDesk = ( KWin::numberOfDesktops() + curDesk - 2 ) % KWin::numberOfDesktops() + 1;

      KWin::setCurrentDesktop( newDesk );
    }
}

void KDesktop::handleColorDropEvent(QDropEvent * e)
{
    KPopupMenu popup;
    popup.insertItem(SmallIconSet("colors"),i18n("Set as Primary Background Color"), 1);
    popup.insertItem(SmallIconSet("colors"),i18n("Set as Secondary Background Color"), 2);
    int result = popup.exec(e->pos());

    QColor c;
    KColorDrag::decode(e, c);
    switch (result) {
      case 1: bgMgr->setColor(c, true); break;
      case 2: bgMgr->setColor(c, false); break;
      default: return;
    }
    bgMgr->setWallpaper(0,0);
}

void KDesktop::handleImageDropEvent(QDropEvent * e)
{
    KPopupMenu popup;
    if ( m_pIconView )
    popup.insertItem(SmallIconSet("filesave"),i18n("&Save to Desktop..."), 1);
    if ( ( m_pIconView && m_pIconView->maySetWallpaper() ) || m_pRootWidget )
       popup.insertItem(SmallIconSet("background"),i18n("Set as &Wallpaper"), 2);
    popup.insertSeparator();
    popup.insertItem(SmallIconSet("cancel"), i18n("&Cancel"));
    int result = popup.exec(e->pos());

    if (result == 1)
    {
        bool ok = true;
        QString filename = KInputDialog::getText(QString::null, i18n("Enter a name for the image below:"), QString::null, &ok, m_pIconView);

        if (!ok)
        {
            return;
        }

        if (filename.isEmpty())
        {
            filename = i18n("image.png");
        }
        else if (filename.right(4).lower() != ".png")
        {
            filename += ".png";
        }

        QImage i;
        QImageDrag::decode(e, i);
        KTempFile tmpFile(QString::null, filename);
        i.save(tmpFile.name(), "PNG");
        // We pass 0 as parent window because passing the desktop is not a good idea
        KURL src;
        src.setPath( tmpFile.name() );
        KURL dest( KDIconView::desktopURL() );
        dest.addPath( filename );
        KIO::NetAccess::copy( src, dest, 0 );
        tmpFile.unlink();
    }
    else if (result == 2)
    {
        QImage i;
        QImageDrag::decode(e, i);
        KTempFile tmpFile(KGlobal::dirs()->saveLocation("wallpaper"), ".png");
        i.save(tmpFile.name(), "PNG");
        kdDebug(1204) << "KDesktop::contentsDropEvent " << tmpFile.name() << endl;
        bgMgr->setWallpaper(tmpFile.name());
    }
}

void KDesktop::slotNewWallpaper(const KURL &url)
{
    // This is called when a file containing an image is dropped
    // (called by KonqOperations)
    if ( url.isLocalFile() )
        bgMgr->setWallpaper( url.path() );
    else
    {
        // Figure out extension
        QString fileName = url.fileName();
        QFileInfo fileInfo( fileName );
        QString ext = fileInfo.extension();
        // Store tempfile in a place where it will still be available after a reboot
        KTempFile tmpFile( KGlobal::dirs()->saveLocation("wallpaper"), "." + ext );
        KURL localURL; localURL.setPath( tmpFile.name() );
        // We pass 0 as parent window because passing the desktop is not a good idea
        KIO::NetAccess::file_copy( url, localURL, -1, true /*overwrite*/ );
        bgMgr->setWallpaper( localURL.path() );
    }
}

// for dcop interface backward compatibility
void KDesktop::logout()
{
    logout( KApplication::ShutdownConfirmDefault,
            KApplication::ShutdownTypeNone );
}

void KDesktop::logout( KApplication::ShutdownConfirm confirm,
                       KApplication::ShutdownType sdtype )
{
    if( !kapp->requestShutDown( confirm, sdtype ) )
        // this i18n string is also in kicker/applets/run/runapplet
        KMessageBox::error( this, i18n("Could not log out properly.\nThe session manager cannot "
                                        "be contacted. You can try to force a shutdown by pressing "
                                        "Ctrl+Alt+Backspace; note, however, that your current session "
                                        "will not be saved with a forced shutdown." ) );
}

void KDesktop::slotLogout()
{
    logout( KApplication::ShutdownConfirmDefault,
            KApplication::ShutdownTypeDefault );
}

void KDesktop::slotLogoutNoCnf()
{
    logout( KApplication::ShutdownConfirmNo,
            KApplication::ShutdownTypeNone );
}

void KDesktop::slotHaltNoCnf()
{
    logout( KApplication::ShutdownConfirmNo,
            KApplication::ShutdownTypeHalt );
}

void KDesktop::slotRebootNoCnf()
{
    logout( KApplication::ShutdownConfirmNo,
            KApplication::ShutdownTypeReboot );
}

void KDesktop::setVRoot( bool enable )
{
    if ( enable == set_vroot )
        return;

    set_vroot = enable;
    kdDebug(1204) << "setVRoot " << enable << endl;
    KDesktopSettings::setSetVRoot( set_vroot );
    KDesktopSettings::writeConfig();
    slotSetVRoot();
}

void KDesktop::clearCommandHistory()
{
    if ( m_miniCli )
        m_miniCli->clearHistory();
}

void KDesktop::setIconsEnabled( bool enable )
{
    if ( enable == m_bDesktopEnabled )
        return;

    m_bDesktopEnabled = enable;
    kdDebug(1204) << "setIcons " << enable << endl;
    KDesktopSettings::setDesktopEnabled( m_bDesktopEnabled );
    KDesktopSettings::writeConfig();
    if (!enable) {
        delete m_pIconView;
        m_pIconView = 0;
    }
    configure();
}

void KDesktop::desktopResized()
{
    resize(kapp->desktop()->size());

    if ( m_pIconView )
    {
        // the sequence of actions is important:
        // remove all icons, resize desktop, tell kdiconview new iconsArea size
        // tell kdiconview to reget all icons
        m_pIconView->slotClear();
        m_pIconView->resize(kapp->desktop()->size());

        // get new desktopIconsArea from kicker
        QByteArray data, result;
        QDataStream arg(data, IO_WriteOnly);
        arg << kdesktop_screen_number;
        QCString replyType;
        QRect area;

        if ( kapp->dcopClient()->call(kicker_name, kicker_name, "desktopIconsArea(int)",
                                       data, replyType, result, false, 2000) )
        {
            QDataStream res(result, IO_ReadOnly);
            res >> area;
        }
        else
            area = kwinModule()->workArea(kwinModule()->currentDesktop());

        m_pIconView->updateWorkArea(area);
        m_pIconView->startDirLister();
    }
}

void KDesktop::switchDesktops( int delta )
{
    bool old = m_bWheelSwitchesWorkspace;
    m_bWheelSwitchesWorkspace = true;
    slotSwitchDesktops(delta);
    m_bWheelSwitchesWorkspace = old;
}

bool KDesktop::event(QEvent * e)
{
    if ( e->type() == QEvent::WindowDeactivate)
    {
        if (m_pIconView)
            m_pIconView->clearSelection();
    }
    return QWidget::event(e);
}

#include "desktop.moc"
