/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#include <qpainter.h>
#include <qtooltip.h>
#include <qlineedit.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qbuttongroup.h>

#include <dcopref.h>
#include <kglobalsettings.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <dcopclient.h>
#include <netwm.h>
#include <kmanagerselection.h>

#include "global.h"
#include "kickertip.h"
#include "kickerSettings.h"
#include "kshadowengine.h"
#include "kshadowsettings.h"
#include "paneldrag.h"
#include "taskmanager.h"

#include "pagerapplet.h"
#include "pagerapplet.moc"

#ifdef FocusOut
#undef FocusOut
#endif

const int knDesktopPreviewSize = 12;
const int knBtnSpacing = 1;

// The previews tend to have a 4/3 aspect ratio
static const int smallHeight = 32;
static const int smallWidth = 42;

// config menu id offsets
static const int rowOffset = 2000;
static const int labelOffset = 200;
static const int bgOffset = 300;

extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
      KGlobal::locale()->insertCatalogue("kminipagerapplet");
      return new KMiniPager(configFile, KPanelApplet::Normal, 0, parent, "kminipagerapplet");
    }
}

KMiniPager::KMiniPager(const QString& configFile, Type type, int actions,
                       QWidget *parent, const char *name)
    : KPanelApplet( configFile, type, actions, parent, name ),
      m_layout(0),
      m_desktopLayoutOwner( NULL ),
      m_shadowEngine(0),
      m_contextMenu(0),
      m_settings( new PagerSettings(sharedConfig()) )
{
    setBackgroundOrigin( AncestorOrigin );
    int scnum = QApplication::desktop()->screenNumber(this);
    QRect desk = QApplication::desktop()->screenGeometry(scnum);
    if (desk.width() <= 800)
    {
        KConfigSkeleton::ItemBool* item = dynamic_cast<KConfigSkeleton::ItemBool*>(m_settings->findItem("Preview"));
        if (item)
        {
            item->setDefaultValue(false);
        }
    }
    m_settings->readConfig();
    m_windows.setAutoDelete(true);
    if (m_settings->preview())
    {
        TaskManager::the()->trackGeometry();
    }

    m_group = new QButtonGroup(this);
    m_group->setBackgroundOrigin(AncestorOrigin);
    m_group->setFrameStyle(QFrame::NoFrame);
    m_group->setExclusive( true );

    setFont( KGlobalSettings::taskbarFont() );

    m_kwin = new KWinModule(this);
    m_activeWindow = m_kwin->activeWindow();
    m_curDesk = m_kwin->currentDesktop();

    if (m_curDesk == 0) // kwin not yet launched
    {
        m_curDesk = 1;
    }

    desktopLayoutOrientation = Qt::Horizontal;
    desktopLayoutX = -1;
    desktopLayoutY = -1;

    QSize s(m_kwin->numberOfViewports(m_kwin->currentDesktop()));
    m_useViewports = s.width() * s.height() > 1;

    drawButtons();

    connect( m_kwin, SIGNAL( currentDesktopChanged(int)), SLOT( slotSetDesktop(int) ) );
    connect( m_kwin, SIGNAL( currentDesktopViewportChanged(int, const QPoint&)),
            SLOT(slotSetDesktopViewport(int, const QPoint&)));
    connect( m_kwin, SIGNAL( numberOfDesktopsChanged(int)), SLOT( slotSetDesktopCount(int) ) );
    connect( m_kwin, SIGNAL( activeWindowChanged(WId)), SLOT( slotActiveWindowChanged(WId) ) );
    connect( m_kwin, SIGNAL( windowAdded(WId) ), this, SLOT( slotWindowAdded(WId) ) );
    connect( m_kwin, SIGNAL( windowRemoved(WId) ), this, SLOT( slotWindowRemoved(WId) ) );
    connect( m_kwin, SIGNAL( windowChanged(WId,unsigned int) ), this, SLOT( slotWindowChanged(WId,unsigned int) ) );
    connect( m_kwin, SIGNAL( desktopNamesChanged() ), this, SLOT( slotDesktopNamesChanged() ) );
    connect( kapp, SIGNAL(backgroundChanged(int)), SLOT(slotBackgroundChanged(int)) );

    if (kapp->authorizeKAction("kicker_rmb") && kapp->authorizeControlModule("kde-kcmtaskbar.desktop"))
    {
        m_contextMenu = new QPopupMenu();
        connect(m_contextMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowContextMenu()));
        connect(m_contextMenu, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));
        setCustomMenu(m_contextMenu);
    }

    QValueList<WId>::ConstIterator it;
    QValueList<WId>::ConstIterator itEnd = m_kwin->windows().end();
    for ( it = m_kwin->windows().begin(); it != itEnd; ++it)
    {
        slotWindowAdded( (*it) );
    }

    slotSetDesktop( m_curDesk );
    updateLayout();
}

KMiniPager::~KMiniPager()
{
    KGlobal::locale()->removeCatalogue("kminipagerapplet");
    delete m_contextMenu;
    delete m_settings;
    delete m_shadowEngine;
}

void KMiniPager::slotBackgroundChanged(int desk)
{
    unsigned numDesktops = m_kwin->numberOfDesktops();
    if (numDesktops != m_desktops.count())
    {
        slotSetDesktopCount(numDesktops);
    }

    if (desk < 1 || (unsigned) desk > m_desktops.count())
    {
        // should not happen, but better to be paranoid than crash
        return;
    }

    m_desktops[desk - 1]->backgroundChanged();
}

void KMiniPager::slotSetDesktop(int desktop)
{
    if (m_kwin->numberOfDesktops() > static_cast<int>(m_desktops.count()))
    {
        slotSetDesktopCount( m_kwin->numberOfDesktops() );
    }

    if (!m_useViewports && (desktop != KWin::currentDesktop()))
    {
        // this can happen when the user clicks on a desktop,
        // holds down the key combo to switch desktops, lets the
        // mouse go but keeps the key combo held. the desktop will switch
        // back to the desktop associated with the key combo and then it
        // becomes a race condition between kwin's signal and the button's
        // signal. usually kwin wins.
        return;
    }

    m_curDesk = desktop;
    if (m_curDesk < 1)
    {
        m_curDesk = 1;
    }

    KMiniPagerButton* button = m_desktops[m_curDesk - 1];
    if (!button->isOn())
    {
        button->toggle();
    }
}

void KMiniPager::slotSetDesktopViewport(int desktop, const QPoint& viewport)
{
    // ###
    Q_UNUSED(desktop);
    QSize s(m_kwin->numberOfViewports(m_kwin->currentDesktop()));
    slotSetDesktop((viewport.y()-1) * s.width() + viewport.x() );
}

void KMiniPager::slotButtonSelected( int desk )
{
    if (m_kwin->numberOfViewports(m_kwin->currentDesktop()).width() *
        m_kwin->numberOfViewports(m_kwin->currentDesktop()).height() > 1)
    {
        QPoint p;

        p.setX( (desk-1) * QApplication::desktop()->width());
        p.setY( 0 );

        KWin::setCurrentDesktopViewport(m_kwin->currentDesktop(), p);
    }
    else
        KWin::setCurrentDesktop( desk );

    slotSetDesktop( desk );
}

int KMiniPager::widthForHeight(int h) const
{
    if (orientation() == Qt::Vertical)
    {
        return width();
    }

    int deskNum = m_kwin->numberOfDesktops() * m_kwin->numberOfViewports(0).width()
        * m_kwin->numberOfViewports(0).height();

    int rowNum = m_settings->numberOfRows();
    if (rowNum == 0)
    {
        if (h <= 32 || deskNum <= 1)
        {
            rowNum = 1;
        }
        else
        {
            rowNum = 2;
        }
    }

    int deskCols = deskNum/rowNum;
    if(deskNum == 0 || deskNum % rowNum != 0)
        deskCols += 1;

    int bw = (h / rowNum);
    if( m_settings->labelType() != PagerSettings::EnumLabelType::LabelName )
    {
        if (desktopPreview() || m_settings->backgroundType() == PagerSettings::EnumBackgroundType::BgLive)
        {
            bw = (int) ( bw * (double) QApplication::desktop()->width() / QApplication::desktop()->height() );
        }
    }
    else
    {
        // scale to desktop width as a minimum
        bw = (int) (bw * (double) QApplication::desktop()->width() / QApplication::desktop()->height());
        QFontMetrics fm = fontMetrics();
        for (int i = 1; i <= deskNum; i++)
        {
            int sw = fm.width( m_kwin->desktopName( i ) ) + 8;
            if (sw > bw)
            {
                bw = sw;
            }
        }
    }

    // we add one to the width for the spacing in between the buttons
    // however, the last button doesn't have a space on the end of it (it's
    // only _between_ buttons) so we then remove that one pixel
    return (deskCols * (bw + 1)) - 1;
}

int KMiniPager::heightForWidth(int w) const
{
    if (orientation() == Qt::Horizontal)
    {
        return height();
    }

    int deskNum = m_kwin->numberOfDesktops() * m_kwin->numberOfViewports(0).width()
        * m_kwin->numberOfViewports(0).height();
    int rowNum = m_settings->numberOfRows(); // actually these are columns now... oh well.
    if (rowNum == 0)
    {
        if (w <= 48 || deskNum == 1)
        {
            rowNum = 1;
        }
        else
        {
            rowNum = 2;
        }
    }

    int deskCols = deskNum/rowNum;
    if(deskNum == 0 || deskNum % rowNum != 0)
    {
        deskCols += 1;
    }

    int bh = (w/rowNum) + 1;
    if ( desktopPreview() )
    {
        bh = (int) ( bh *  (double) QApplication::desktop()->height() / QApplication::desktop()->width() );
    }
    else if ( m_settings->labelType() == PagerSettings::EnumLabelType::LabelName )
    {
       bh = fontMetrics().lineSpacing() + 8;
    }

    // we add one to the width for the spacing in between the buttons
    // however, the last button doesn't have a space on the end of it (it's
    // only _between_ buttons) so we then remove that one pixel
    int nHg = (deskCols * (bh + 1)) - 1;

    return nHg;
}

void KMiniPager::updateDesktopLayout(int o, int x, int y)
{
    if ((desktopLayoutOrientation == o) &&
       (desktopLayoutX == x) &&
       (desktopLayoutY == y))
    {
      return;
    }

    desktopLayoutOrientation = o;
    desktopLayoutX = x;
    desktopLayoutY = y;
    if( x == -1 ) // do-the-maths-yourself is encoded as 0 in the wm spec
        x = 0;
    if( y == -1 )
        y = 0;
    if( m_desktopLayoutOwner == NULL )
    { // must own manager selection before setting global desktop layout
        int screen = DefaultScreen( qt_xdisplay());
        m_desktopLayoutOwner = new KSelectionOwner( QString( "_NET_DESKTOP_LAYOUT_S%1" ).arg( screen ).latin1(),
            screen, this );
        if( !m_desktopLayoutOwner->claim( false ))
        {
            delete m_desktopLayoutOwner;
            m_desktopLayoutOwner = NULL;
            return;
        }
    }
    NET::Orientation orient = o == Qt::Horizontal ? NET::OrientationHorizontal : NET::OrientationVertical;
    NETRootInfo i( qt_xdisplay(), 0 );
    i.setDesktopLayout( orient, x, y, NET::DesktopLayoutCornerTopLeft );
}

void KMiniPager::resizeEvent(QResizeEvent*)
{
    bool horiz = orientation() == Horizontal;

    int deskNum = m_desktops.count();
    int rowNum = m_settings->numberOfRows();
    if (rowNum == 0)
    {
        if (((horiz && height()<=32)||(!horiz && width()<=48)) || deskNum <= 1)
            rowNum = 1;
        else
            rowNum = 2;
    }

    int deskCols = deskNum/rowNum;
    if(deskNum == 0 || deskNum % rowNum != 0)
        deskCols += 1;

    if (m_layout)
    {
        delete m_layout;
        m_layout = 0;
    }

    int nDX, nDY;
    if (horiz)
    {
        nDX = rowNum;
        nDY = deskCols;
        updateDesktopLayout(Qt::Horizontal, -1, nDX);
    }
    else
    {
        nDX = deskCols;
        nDY = rowNum;
        updateDesktopLayout(Qt::Horizontal, nDY, -1);
    }

    // 1 pixel spacing.
    m_layout = new QGridLayout(this, nDX, nDY, 0, 1);

    QValueList<KMiniPagerButton*>::Iterator it = m_desktops.begin();
    QValueList<KMiniPagerButton*>::Iterator itEnd = m_desktops.end();
    int c = 0,
        r = 0;
    while( it != itEnd ) {
        c = 0;
        while( (it != itEnd) && (c < nDY) ) {
            m_layout->addWidget( *it, r, c );
            ++it;
            ++c;
        }
        ++r;
    }

    m_layout->activate();
    updateGeometry();
}

void KMiniPager::wheelEvent( QWheelEvent* e )
{
    int newDesk;
    int desktops = KWin::numberOfDesktops();
    if (m_kwin->numberOfViewports(0).width() * m_kwin->numberOfViewports(0).height() > 1 )
        desktops = m_kwin->numberOfViewports(0).width() * m_kwin->numberOfViewports(0).height();
    if (e->delta() < 0)
    {
        newDesk = m_curDesk % desktops + 1;
    }
    else
    {
        newDesk = (desktops + m_curDesk - 2) % desktops + 1;
    }

    slotButtonSelected(newDesk);
}

void KMiniPager::drawButtons()
{
    int deskNum = m_kwin->numberOfDesktops();
    KMiniPagerButton *desk;

    int count = 1;
    int i = 1;
    do
    {
        QSize viewportNum = m_kwin->numberOfViewports(i);
        for (int j = 1; j <= viewportNum.width() * viewportNum.height(); ++j)
        {
            QSize s(m_kwin->numberOfViewports(m_kwin->currentDesktop()));
            QPoint viewport( (j-1) % s.width(), (j-1) / s.width());
            desk = new KMiniPagerButton( count, m_useViewports, viewport, this );
            if ( m_settings->labelType() != PagerSettings::EnumLabelType::LabelName )
            {
                QToolTip::add( desk, desk->desktopName() );
            }

            m_desktops.append( desk );
            m_group->insert( desk, count );

            connect(desk, SIGNAL(buttonSelected(int)),
                    SLOT(slotButtonSelected(int)) );
            connect(desk, SIGNAL(showMenu(const QPoint&, int )),
                    SLOT(slotShowMenu(const QPoint&, int )) );

            desk->show();
            ++count;
        }
    }
    while ( ++i <= deskNum );
}

void KMiniPager::slotSetDesktopCount( int )
{
    QValueList<KMiniPagerButton*>::ConstIterator it;
    QValueList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
    for( it = m_desktops.begin(); it != itEnd; ++it )
    {
        delete (*it);
    }
    m_desktops.clear();

    drawButtons();

    m_curDesk = m_kwin->currentDesktop();
    if ( m_curDesk == 0 )
    {
        m_curDesk = 1;
    }

    resizeEvent(0);
    updateLayout();
}

void KMiniPager::slotActiveWindowChanged( WId win )
{
    if (desktopPreview())
    {
        KWin::WindowInfo* inf1 = m_activeWindow ? info( m_activeWindow ) : NULL;
        KWin::WindowInfo* inf2 = win ? info( win ) : NULL;
        m_activeWindow = win;

        QValueList<KMiniPagerButton*>::ConstIterator it;
        QValueList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
        for ( it = m_desktops.begin(); it != itEnd; ++it)
        {
            if ( ( inf1 && (*it)->shouldPaintWindow(inf1)) ||
                 ( inf2 && (*it)->shouldPaintWindow(inf2)) )
            {
                (*it)->windowsChanged();
            }
        }
    }
}

void KMiniPager::slotWindowAdded( WId win)
{
    if (desktopPreview())
    {
        KWin::WindowInfo* inf = info( win );

        if (inf->state() & NET::SkipPager)
        {
            return;
        }

        QValueList<KMiniPagerButton*>::ConstIterator it;
        QValueList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
        for ( it = m_desktops.begin(); it != itEnd; ++it)
        {
            if ( (*it)->shouldPaintWindow(inf) )
            {
                (*it)->windowsChanged();
            }
        }
    }
}

void KMiniPager::slotWindowRemoved(WId win)
{
    if (desktopPreview())
    {
        KWin::WindowInfo* inf = info(win);
        bool onAllDesktops = inf->onAllDesktops();
        bool onAllViewports = inf->hasState(NET::Sticky);
        bool skipPager = inf->state() & NET::SkipPager;
        int desktop = inf->desktop();

        if (win == m_activeWindow)
            m_activeWindow = 0;

        m_windows.remove((long) win);

        if (skipPager)
        {
            return;
        }

        QValueList<KMiniPagerButton*>::ConstIterator it;
        QValueList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
        for (it = m_desktops.begin(); it != itEnd; ++it)
        {
            if (onAllDesktops || onAllViewports || desktop == (*it)->desktop())
            {
                (*it)->windowsChanged();
            }
        }
    }
    else
    {
        m_windows.remove(win);
        return;
    }

}

void KMiniPager::slotWindowChanged( WId win , unsigned int properties )
{
    if ((properties & (NET::WMState | NET::XAWMState | NET::WMDesktop)) == 0 &&
        (!desktopPreview() || (properties & NET::WMGeometry) == 0) &&
        !(desktopPreview() && windowIcons() &&
         (properties & NET::WMIcon | NET::WMIconName | NET::WMVisibleIconName) == 0))
    {
        return;
    }

    if (desktopPreview())
    {
        KWin::WindowInfo* inf = m_windows[win];
        bool skipPager = inf->hasState(NET::SkipPager);
        QMemArray<bool> old_shouldPaintWindow(m_desktops.size());
        QValueList<KMiniPagerButton*>::ConstIterator it;
        QValueList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
        int i = 0;
        for ( it = m_desktops.begin(); it != itEnd; ++it)
        {
            old_shouldPaintWindow[i++] = (*it)->shouldPaintWindow(inf);
        }

        m_windows.remove(win);
        inf = info(win);

        if (inf->hasState(NET::SkipPager) || skipPager)
        {
            return;
        }

        for ( i = 0, it = m_desktops.begin(); it != itEnd; ++it)
        {
            if ( old_shouldPaintWindow[i++] || (*it)->shouldPaintWindow(inf))
            {
                (*it)->windowsChanged();
            }
        }
    }
    else
    {
        m_windows.remove(win);
        return;
    }
}

KWin::WindowInfo* KMiniPager::info( WId win )
{
    if (!m_windows[win])
    {
        KWin::WindowInfo* info = new KWin::WindowInfo( win,
            NET::WMWindowType | NET::WMState | NET::XAWMState | NET::WMDesktop | NET::WMGeometry | NET::WMKDEFrameStrut, 0 );

        m_windows.insert(win, info);
        return info;
    }

    return m_windows[win];
}

KTextShadowEngine* KMiniPager::shadowEngine()
{
    if (!m_shadowEngine)
        m_shadowEngine = new KTextShadowEngine();

    return m_shadowEngine;
}

void KMiniPager::refresh()
{
    QValueList<KMiniPagerButton*>::ConstIterator it;
    QValueList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
    for ( it = m_desktops.begin(); it != itEnd; ++it)
    {
        (*it)->update();
    }
}

void KMiniPager::aboutToShowContextMenu()
{
    m_contextMenu->clear();

    m_contextMenu->insertItem(SmallIcon("kpager"), i18n("&Launch Pager"), LaunchExtPager);
    m_contextMenu->insertSeparator();

    m_contextMenu->insertItem(i18n("&Rename Desktop \"%1\"")
                                   .arg(kwin()->desktopName(m_rmbDesk)), RenameDesktop);
    m_contextMenu->insertSeparator();

    KPopupMenu* showMenu = new KPopupMenu(m_contextMenu);
    showMenu->setCheckable(true);
    showMenu->insertTitle(i18n("Pager Layout"));

    QPopupMenu* rowMenu = new QPopupMenu(showMenu);
    rowMenu->setCheckable(true);
    rowMenu->insertItem(i18n("&Automatic"), 0 + rowOffset);
    rowMenu->insertItem(i18n("one row or column", "&1"), 1 + rowOffset);
    rowMenu->insertItem(i18n("two rows or columns", "&2"), 2 + rowOffset);
    rowMenu->insertItem( i18n("three rows or columns", "&3"), 3 + rowOffset);
    connect(rowMenu, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));
    showMenu->insertItem((orientation()==Horizontal) ? i18n("&Rows"):
                                                       i18n("&Columns"),
                         rowMenu);

    showMenu->insertItem(i18n("&Window Thumbnails"), WindowThumbnails);
    showMenu->insertItem(i18n("&Window Icons"), WindowIcons);

    showMenu->insertTitle(i18n("Text Label"));
    showMenu->insertItem(i18n("Desktop N&umber"),
                         PagerSettings::EnumLabelType::LabelNumber + labelOffset);
    showMenu->insertItem(i18n("Desktop N&ame"),
                         PagerSettings::EnumLabelType::LabelName + labelOffset);
    showMenu->insertItem(i18n("N&o Label"),
                         PagerSettings::EnumLabelType::LabelNone + labelOffset);

    showMenu->insertTitle(i18n("Background"));
    showMenu->insertItem(i18n("&Elegant"),
                         PagerSettings::EnumBackgroundType::BgPlain + bgOffset);
    showMenu->insertItem(i18n("&Transparent"),
                         PagerSettings::EnumBackgroundType::BgTransparent + bgOffset);
    showMenu->insertItem(i18n("&Desktop Wallpaper"),
                         PagerSettings::EnumBackgroundType::BgLive + bgOffset);
    connect(showMenu, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));
    m_contextMenu->insertItem(i18n("&Pager Options"),showMenu);

    m_contextMenu->insertItem(SmallIcon("configure"),
                              i18n("&Configure Desktops..."),
                              ConfigureDesktops);

    rowMenu->setItemChecked(m_settings->numberOfRows() + rowOffset, true);
    m_contextMenu->setItemChecked(m_settings->labelType() + labelOffset, showMenu);
    m_contextMenu->setItemChecked(m_settings->backgroundType() + bgOffset, showMenu);

    m_contextMenu->setItemChecked(WindowThumbnails, m_settings->preview());
    m_contextMenu->setItemChecked(WindowIcons, m_settings->icons());
    m_contextMenu->setItemEnabled(WindowIcons, m_settings->preview());
    m_contextMenu->setItemEnabled(RenameDesktop,
                                  m_settings->labelType() ==
                                  PagerSettings::EnumLabelType::LabelName);
}

void KMiniPager::slotShowMenu(const QPoint& pos, int desktop)
{
    if (!m_contextMenu)
    {
        return;
    }

    m_rmbDesk = desktop;
    m_contextMenu->exec(pos);
    m_rmbDesk = -1;
}

void KMiniPager::contextMenuActivated(int result)
{
    if (result < 1)
    {
        return;
    }

    switch (result)
    {
        case LaunchExtPager:
            showPager();
            return;

        case ConfigureDesktops:
            kapp->startServiceByDesktopName("desktop");
            return;

        case RenameDesktop:
            m_desktops[(m_rmbDesk == -1) ? m_curDesk - 1 : m_rmbDesk - 1]->rename();
            return;
    }

    if (result >= rowOffset)
    {
        m_settings->setNumberOfRows(result - rowOffset);
        resizeEvent(0);
    }

    switch (result)
    {
        case WindowThumbnails:
            m_settings->setPreview(!m_settings->preview());
            TaskManager::the()->trackGeometry();
            break;

        case WindowIcons:
            m_settings->setIcons(!m_settings->icons());
            break;

        case PagerSettings::EnumBackgroundType::BgPlain + bgOffset:
            m_settings->setBackgroundType(PagerSettings::EnumBackgroundType::BgPlain);
            break;
        case PagerSettings::EnumBackgroundType::BgTransparent + bgOffset:
            m_settings->setBackgroundType(PagerSettings::EnumBackgroundType::BgTransparent);
            break;
        case PagerSettings::EnumBackgroundType::BgLive + bgOffset:
        {
            m_settings->setBackgroundType(PagerSettings::EnumBackgroundType::BgLive);
            QValueList<KMiniPagerButton*>::ConstIterator it;
            QValueList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();
            for( it = m_desktops.begin(); it != itEnd; ++it )
            {
                (*it)->backgroundChanged();
            }
            break;
        }

        case PagerSettings::EnumLabelType::LabelNone + labelOffset:
            m_settings->setLabelType(PagerSettings::EnumLabelType::LabelNone);
            break;
        case PagerSettings::EnumLabelType::LabelNumber + labelOffset:
            m_settings->setLabelType(PagerSettings::EnumLabelType::LabelNumber);
            break;
        case PagerSettings::EnumLabelType::LabelName + labelOffset:
            m_settings->setLabelType(PagerSettings::EnumLabelType::LabelName);
            break;
    }

    m_settings->writeConfig();
    updateGeometry();
    refresh();
}

void KMiniPager::slotDesktopNamesChanged()
{
    QValueList<KMiniPagerButton*>::ConstIterator it = m_desktops.begin();
    QValueList<KMiniPagerButton*>::ConstIterator itEnd = m_desktops.end();

    for (int i = 1; it != itEnd; ++it, ++i)
    {
        QString name = m_kwin->desktopName(i);
        (*it)->setDesktopName(name);
        (*it)->repaint();
        QToolTip::remove((*it));
        QToolTip::add((*it), name);
    }

    updateLayout();
}

void KMiniPager::showPager()
{
    DCOPClient *dcop=kapp->dcopClient();

    if (dcop->isApplicationRegistered("kpager"))
    {
       showKPager(true);
    }
    else
    {
    // Let's run kpager if it isn't running
        connect( dcop, SIGNAL( applicationRegistered(const QCString &) ), this, SLOT(applicationRegistered(const QCString &)) );
        dcop->setNotifications(true);
        QString strAppPath(locate("exe", "kpager"));
        if (!strAppPath.isEmpty())
        {
            KProcess process;
            process << strAppPath;
            process << "--hidden";
            process.start(KProcess::DontCare);
        }
    }
}

void KMiniPager::showKPager(bool toggleShow)
{
    QPoint pt;
    switch ( position() )
    {
        case pTop:
            pt = mapToGlobal( QPoint(x(), y() + height()) );
            break;
        case pLeft:
            pt = mapToGlobal( QPoint(x() + width(), y()) );
            break;
        case pRight:
        case pBottom:
        default:
            pt=mapToGlobal( QPoint(x(), y()) );
    }

    DCOPClient *dcop=kapp->dcopClient();

    QByteArray data;
    QDataStream arg(data, IO_WriteOnly);
    arg << pt.x() << pt.y() ;
    if (toggleShow)
    {
        dcop->send("kpager", "KPagerIface", "toggleShow(int,int)", data);
    }
    else
    {
        dcop->send("kpager", "KPagerIface", "showAt(int,int)", data);
    }
}

void KMiniPager::applicationRegistered( const QCString  & appName )
{
    if (appName == "kpager")
    {
        disconnect( kapp->dcopClient(), SIGNAL( applicationRegistered(const QCString &) ),
                    this, SLOT(applicationRegistered(const QCString &)) );
        showKPager(false);
    }
}

