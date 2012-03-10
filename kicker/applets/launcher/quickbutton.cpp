/*****************************************************************

Copyright (c) 2000 Bill Nagel

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

#include "quickbutton.h"
#include "quickaddappsmenu.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qpopupmenu.h>
#include <qtooltip.h>

#include <kactionclasses.h>
#include <kickertip.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <krun.h>
#include <kiconeffect.h>
#include <kglobalsettings.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kipc.h>
#include <kiconloader.h>
#include <kurldrag.h>
#include <kstandarddirs.h>

#include <math.h>
#include <algorithm>

#ifdef DEBUG
   #define DEBUGSTR kdDebug()
#else
   #define DEBUGSTR kndDebug()
#endif

QuickURL::QuickURL(const QString &u)
{  DEBUGSTR<<"QuickURL::QuickURL("<<u<<")"<<endl<<flush;
   KService::Ptr _service=0;
   _menuId = u;
   if (_menuId.startsWith("file:") && _menuId.endsWith(".desktop")) {
      // this ensures that desktop entries are referenced by desktop name instead of by file name
      _menuId=KURL(_menuId).path();
   }
   if (_menuId.startsWith("/")) {
      // Absolute path
      _kurl.setPath(_menuId);

      if (_menuId.endsWith(".desktop")) {
         // Strip path
         QString s = _menuId;
         s = s.mid(s.findRev('/')+1);
         s = s.left(s.length()-8);
         _service = KService::serviceByStorageId(s);
         if (!_service) {
            _service = new KService(_menuId);
         } else {
         }
      }
   } else if (!KURL::isRelativeURL(_menuId)) {
      // Full URL
      _kurl = _menuId;
   } else {
      // menu-id
      _service = KService::serviceByMenuId(_menuId);
   }
   DEBUGSTR << "QuickURL: _service='"<<_service<<" _kurl="<<_kurl<<" _menuId="<<_menuId<<endl<<flush;

   if (_service) {
      if (!_service->isValid()) {
         DEBUGSTR << "QuickURL: _service is not valid"<<endl<<flush;
         // _service is a KShared pointer, don't try to delete it!
         _service = 0;
      } else {
         DEBUGSTR << "QuickURL: _service='"<<_service<<"' _service->desktopEntryPath()="<<_service->desktopEntryPath()<<endl<<flush;
         if (_kurl.path().length() == 0)
         {
            _kurl.setPath(locate("apps", _service->desktopEntryPath()));
         }
         if (!_service->menuId().isEmpty())
            _menuId = _service->menuId();

         m_genericName = _service->genericName();
         m_name = _service->name();
      }
   } else {
      m_name = _kurl.prettyURL();
   }
   DEBUGSTR<<"QuickURL::QuickURL("<<u<<") END"<<endl<<flush;
}

void QuickURL::run() const
{  kapp->propagateSessionManager();   // is this needed?
   if (_service)
      KRun::run(*(_service), KURL::List());
   else
      new KRun(_kurl, 0, _kurl.isLocalFile());
}

//similar to MimeType::pixmapForURL
QPixmap QuickURL::pixmap( mode_t _mode, KIcon::Group _group,
                          int _force_size, int _state, QString *) const
{  // Load icon
   QPixmap pxmap = KMimeType::pixmapForURL(_kurl, _mode, _group, _force_size, _state);
   // Resize to fit button
   pxmap.convertFromImage(pxmap.convertToImage().smoothScale(_force_size,_force_size, QImage::ScaleMin));
   return pxmap;
}


QuickButton::QuickButton(const QString &u, KAction* configAction, 
                         QWidget *parent, const char *name) : 
     SimpleButton(parent, name),
     m_flashCounter(0),
     m_sticky(false)
{
    installEventFilter(KickerTip::the());
    setMouseTracking(true);
    _highlight = false;
    _oldCursor = cursor();
    _qurl=new QuickURL(u);
    
    QToolTip::add(this, _qurl->name());
    resize(int(DEFAULT_ICON_DIM),int(DEFAULT_ICON_DIM));
    QBrush bgbrush(colorGroup().brush(QColorGroup::Background));
    
    QuickAddAppsMenu *addAppsMenu = new QuickAddAppsMenu(
        parent, this, _qurl->url());
    _popup = new QPopupMenu(this);
    _popup->insertItem(i18n("Add Application"), addAppsMenu);
    configAction->plug(_popup);
        _popup->insertSeparator();
    _popup->insertItem(SmallIcon("remove"), i18n("Remove"), 
            this, SLOT(removeApp()));

    m_stickyAction = new KToggleAction(i18n("Never Remove Automatically"),
        KShortcut(), this);
    connect(m_stickyAction, SIGNAL(toggled(bool)), 
        this, SLOT(slotStickyToggled(bool)));
    m_stickyAction->plug(_popup, 2);
    m_stickyId = _popup->idAt(2);

    connect(this, SIGNAL(clicked()), SLOT(launch()));
    connect(this, SIGNAL(removeApp(QuickButton *)), parent,
        SLOT(removeAppManually(QuickButton *)));
}

QuickButton::~QuickButton()
{
    delete _qurl;
}


QString QuickButton::url() const
{
    return _qurl->url();
}


QString QuickButton::menuId() const
{  return _qurl->menuId();}


void QuickButton::loadIcon()
{ 
    // Set Icon Dimension from size
   _iconDim=std::min(size().width(),size().height())-2*ICON_MARGIN;
   // Load icons
   _icon = _qurl->pixmap(0, KIcon::Panel, _iconDim, KIcon::DefaultState);
   _iconh = _qurl->pixmap(0, KIcon::Panel, _iconDim, KIcon::ActiveState);
   setPixmap(_icon);
}

void QuickButton::resizeEvent(QResizeEvent *e)
{
   loadIcon();
   SimpleButton::resizeEvent(e);
}

void QuickButton::mousePressEvent(QMouseEvent *e)
{
   if (e->button() == RightButton)
      _popup->popup(e->globalPos());
   else if (e->button() == LeftButton) {
      _dragPos = e->pos();
      QButton::mousePressEvent(e);
   }
}

void QuickButton::mouseMoveEvent(QMouseEvent *e)
{
   if ((e->state() & LeftButton) == 0) return;
   QPoint p(e->pos() - _dragPos);
   if (p.manhattanLength() <= KGlobalSettings::dndEventDelay())
      return;
   DEBUGSTR<<"dragstart"<<endl<<flush;
   setDown(false);
   if (_dragEnabled) {
       KURL::List uris;
       uris.append(_qurl->kurl());
       DEBUGSTR<<"creating KURLDrag"<<endl<<flush;
       KURLDrag *dd = new KURLDrag(uris,this);
       dd->setPixmap(_icon); //PIX
       DEBUGSTR<<"ready to drag"<<endl<<flush;
       grabKeyboard();
       dd->drag();
       releaseKeyboard();
   } else {
       setCursor(Qt::ForbiddenCursor);
   }
}

void QuickButton::slotIconChanged(int group)
{
   loadIcon();
   SimpleButton::slotIconChanged(group);
   update();
}

void QuickButton::launch()
{
   setDown(false);
   update();
   KIconEffect::visualActivate(this, rect());
   _qurl->run();
   emit executed(_qurl->menuId());
}

void QuickButton::setDragging(bool enable)
{
   setDown(enable);
   _highlight=enable;
   update();
}

void QuickButton::setEnableDrag(bool enable)
{
   _dragEnabled=enable;
}

void QuickButton::removeApp()
{
   emit removeApp(this);
}

void QuickButton::flash()
{
   m_flashCounter = 2000;
   QTimer::singleShot(0, this, SLOT(slotFlash()));
}

void QuickButton::slotFlash()
{
    static const int timeout = 500/4;
    if (m_flashCounter > 0)
    {
        m_flashCounter -= timeout;
        if (m_flashCounter < 0) m_flashCounter = 0;
        update();
        QTimer::singleShot(timeout, this, SLOT(slotFlash()));
    }
}

void QuickButton::slotStickyToggled(bool isSticky)
{
    m_sticky = isSticky;
    emit stickyToggled(isSticky);
}

void QuickButton::setSticky(bool sticky)
{
    m_stickyAction->setChecked(sticky);
    slotStickyToggled(sticky);
}

void QuickButton::updateKickerTip(KickerTip::Data &data)
{
    if (!_qurl)
    {
        return;
    }
    data.message = _qurl->name();
    data.direction = m_popupDirection;
    data.subtext = _qurl->genericName();
    if (data.subtext == QString())
    {
        data.subtext = data.message;
    }
    data.icon = KMimeType::pixmapForURL(_qurl->kurl(), 0, 
        KIcon::Panel, KIcon::SizeHuge, KIcon::DefaultState);
}

void QuickButton::setPopupDirection(KPanelApplet::Direction d)
{
    m_popupDirection = d;
}

void QuickButton::setDynamicModeEnabled(bool enabled)
{
    _popup->setItemVisible(m_stickyId, enabled);
}


#include "quickbutton.moc"
