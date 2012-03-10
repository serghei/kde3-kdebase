/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001, 2005 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kateviewspace.h"
#include "kateviewspace.moc"

#include "katemainwindow.h"
#include "kateviewspacecontainer.h"
#include "katedocmanager.h"
#include "kateapp.h"
#include "katesession.h"

#include <kiconloader.h>
#include <klocale.h>
#include <ksqueezedtextlabel.h>
#include <kconfig.h>
#include <kdebug.h>

#include <qwidgetstack.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qpixmap.h>

//BEGIN KVSSBSep
/*
   "KateViewSpaceStatusBarSeparator"
   A 2 px line to separate the statusbar from the view.
   It is here to compensate for the lack of a frame in the view,
   I think Kate looks very nice this way, as QScrollView with frame
   looks slightly clumsy...
   Slight 3D effect. I looked for suitable QStyle props or methods,
   but found none, though maybe it should use QStyle::PM_DefaultFrameWidth
   for height (TRY!).
   It does look a bit funny with flat styles (Light, .Net) as is,
   but there are on methods to paint panel lines separately. And,
   those styles tends to look funny on their own, as a light line
   in a 3D frame next to a light contents widget is not functional.
   Also, QStatusBar is up to now completely ignorant to style.
   -anders
*/
class KVSSBSep : public QWidget {
public:
  KVSSBSep( KateViewSpace *parent=0) : QWidget(parent)
  {
    setFixedHeight( 2 );
  }
protected:
  void paintEvent( QPaintEvent *e )
  {
    QPainter p( this );
    p.setPen( colorGroup().shadow() );
    p.drawLine( e->rect().left(), 0, e->rect().right(), 0 );
    p.setPen( ((KateViewSpace*)parentWidget())->isActiveSpace() ? colorGroup().light() : colorGroup().midlight() );
    p.drawLine( e->rect().left(), 1, e->rect().right(), 1 );
  }
};
//END KVSSBSep

//BEGIN KateViewSpace
KateViewSpace::KateViewSpace( KateViewSpaceContainer *viewManager,
                              QWidget* parent, const char* name )
  : QVBox(parent, name),
    m_viewManager( viewManager )
{
  mViewList.setAutoDelete(false);

  stack = new QWidgetStack( this );
  setStretchFactor(stack, 1);
  stack->setFocus();
  //sep = new KVSSBSep( this );
  mStatusBar = new KateVSStatusBar(this);
  mIsActiveSpace = false;
  mViewCount = 0;

  setMinimumWidth (mStatusBar->minimumWidth());
  m_group = QString::null;
}

KateViewSpace::~KateViewSpace()
{
}

void KateViewSpace::polish()
{
  mStatusBar->show();
}

void KateViewSpace::addView(Kate::View* v, bool show)
{
  // restore the config of this view if possible
  if ( !m_group.isEmpty() )
  {
    QString fn = v->getDoc()->url().prettyURL();
    if ( ! fn.isEmpty() )
    {
      QString vgroup = QString("%1 %2").arg(m_group).arg(fn);

      KateSession::Ptr as = KateSessionManager::self()->activeSession ();
      if ( as->configRead() && as->configRead()->hasGroup( vgroup ) )
      {
        as->configRead()->setGroup( vgroup );
        v->readSessionConfig ( as->configRead() );
      }
    }
  }

  uint id = mViewList.count();
  stack->addWidget(v, id);
  if (show) {
    mViewList.append(v);
    showView( v );
  }
  else {
    Kate::View* c = mViewList.current();
    mViewList.prepend( v );
    showView( c );
  }
}

void KateViewSpace::removeView(Kate::View* v)
{
  disconnect( v->getDoc(), SIGNAL(modifiedChanged()),
              mStatusBar, SLOT(modifiedChanged()) );

  bool active = ( v == currentView() );

  mViewList.remove (v);
  stack->removeWidget (v);

  if ( ! active )
    return;

  if (currentView() != 0L)
    showView(mViewList.current());
  else if (mViewList.count() > 0)
    showView(mViewList.last());
}

bool KateViewSpace::showView(Kate::View* v)
{
  return showView( v->getDoc()->documentNumber() );
}

bool KateViewSpace::showView(uint documentNumber)
{
  QPtrListIterator<Kate::View> it (mViewList);
  it.toLast();
  for( ; it.current(); --it ) {
    if (((Kate::Document*)it.current()->getDoc())->documentNumber() == documentNumber) {
      if ( currentView() )
        disconnect( currentView()->getDoc(), SIGNAL(modifiedChanged()),
                    mStatusBar, SLOT(modifiedChanged()) );

      Kate::View* kv = it.current();
      connect( kv->getDoc(), SIGNAL(modifiedChanged()),
               mStatusBar, SLOT(modifiedChanged()) );

      mViewList.removeRef( kv );
      mViewList.append( kv );
      stack->raiseWidget( kv );
      kv->show();
      mStatusBar->modifiedChanged();
      return true;
    }
  }
   return false;
}


Kate::View* KateViewSpace::currentView()
{
  if (mViewList.count() > 0)
    return (Kate::View*)stack->visibleWidget();

  return 0L;
}

bool KateViewSpace::isActiveSpace()
{
  return mIsActiveSpace;
}

void KateViewSpace::setActive( bool active, bool )
{
  mIsActiveSpace = active;

  // change the statusbar palette and make sure it gets updated
  QPalette pal( palette() );
  if ( ! active )
  {
    pal.setColor( QColorGroup::Background, pal.active().mid() );
    pal.setColor( QColorGroup::Light, pal.active().midlight() );
  }

  mStatusBar->setPalette( pal );
  mStatusBar->update();
  //sep->update();
}

bool KateViewSpace::event( QEvent *e )
{
  if ( e->type() == QEvent::PaletteChange )
  {
    setActive( mIsActiveSpace );
    return true;
  }
  return QVBox::event( e );
}

void KateViewSpace::slotStatusChanged (Kate::View *view, int r, int c, int ovr, bool block, int mod, const QString &msg)
{
  if ((QWidgetStack *)view->parentWidget() != stack)
    return;
  mStatusBar->setStatus( r, c, ovr, block, mod, msg );
}

void KateViewSpace::saveConfig ( KConfig* config, int myIndex ,const QString& viewConfGrp)
{
//   kdDebug()<<"KateViewSpace::saveConfig("<<myIndex<<", "<<viewConfGrp<<") - currentView: "<<currentView()<<")"<<endl;
  QString group = QString(viewConfGrp+"-ViewSpace %1").arg( myIndex );

  config->setGroup (group);
  config->writeEntry ("Count", mViewList.count());

  if (currentView())
    config->writeEntry( "Active View", currentView()->getDoc()->url().prettyURL() );

  // Save file list, includeing cursor position in this instance.
  QPtrListIterator<Kate::View> it(mViewList);

  int idx = 0;
  for (; it.current(); ++it)
  {
    if ( !it.current()->getDoc()->url().isEmpty() )
    {
      config->setGroup( group );
      config->writeEntry( QString("View %1").arg( idx ), it.current()->getDoc()->url().prettyURL() );

      // view config, group: "ViewSpace <n> url"
      QString vgroup = QString("%1 %2").arg(group).arg(it.current()->getDoc()->url().prettyURL());
      config->setGroup( vgroup );
      it.current()->writeSessionConfig( config );
    }

    idx++;
  }
}

void KateViewSpace::modifiedOnDisc(Kate::Document *, bool, unsigned char)
{
  if ( currentView() )
    mStatusBar->updateMod( currentView()->getDoc()->isModified() );
}

void KateViewSpace::restoreConfig ( KateViewSpaceContainer *viewMan, KConfig* config, const QString &group )
{
  config->setGroup (group);
  QString fn = config->readEntry( "Active View" );

  if ( !fn.isEmpty() )
  {
    Kate::Document *doc = KateDocManager::self()->findDocumentByUrl (KURL(fn));

    if (doc)
    {
      // view config, group: "ViewSpace <n> url"
      QString vgroup = QString("%1 %2").arg(group).arg(fn);
      config->setGroup( vgroup );

      viewMan->createView (doc);

      Kate::View *v = viewMan->activeView ();

      if (v)
        v->readSessionConfig( config );
    }
  }

  if (mViewList.isEmpty())
    viewMan->createView (KateDocManager::self()->document(0));

  m_group = group; // used for restroing view configs later
}
//END KateViewSpace

//BEGIN KateVSStatusBar
KateVSStatusBar::KateVSStatusBar ( KateViewSpace *parent, const char *name )
  : KStatusBar( parent, name ),
    m_viewSpace( parent )
{
  m_lineColLabel = new QLabel( this );
  addWidget( m_lineColLabel, 0, false );
  m_lineColLabel->setAlignment( Qt::AlignCenter );
  m_lineColLabel->installEventFilter( this );

  m_modifiedLabel = new QLabel( QString("   "), this );
  addWidget( m_modifiedLabel, 0, false );
  m_modifiedLabel->setAlignment( Qt::AlignCenter );
  m_modifiedLabel->installEventFilter( this );

  m_insertModeLabel = new QLabel( i18n(" INS "), this );
  addWidget( m_insertModeLabel, 0, false );
  m_insertModeLabel->setAlignment( Qt::AlignCenter );
  m_insertModeLabel->installEventFilter( this );

  m_selectModeLabel = new QLabel( i18n(" NORM "), this );
  addWidget( m_selectModeLabel, 0, false );
  m_selectModeLabel->setAlignment( Qt::AlignCenter );
  m_selectModeLabel->installEventFilter( this );

  m_fileNameLabel=new KSqueezedTextLabel( this );
  addWidget( m_fileNameLabel, 1, true );
  m_fileNameLabel->setMinimumSize( 0, 0 );
  m_fileNameLabel->setSizePolicy(QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed ));
  m_fileNameLabel->setAlignment( /*Qt::AlignRight*/Qt::AlignLeft );
  m_fileNameLabel->installEventFilter( this );

  installEventFilter( this );
  m_modPm = SmallIcon("modified");
  m_modDiscPm = SmallIcon("modonhd");
  m_modmodPm = SmallIcon("modmod");
  m_noPm = SmallIcon("null");
}

KateVSStatusBar::~KateVSStatusBar ()
{
}

void KateVSStatusBar::setStatus( int r, int c, int ovr, bool block, int, const QString &msg )
{
  m_lineColLabel->setText(
    i18n(" Line: %1 Col: %2 ").arg(KGlobal::locale()->formatNumber(r+1, 0))
                              .arg(KGlobal::locale()->formatNumber(c+1, 0)) );

  if (ovr == 0)
    m_insertModeLabel->setText( i18n(" R/O ") );
  else if (ovr == 1)
    m_insertModeLabel->setText( i18n(" OVR ") );
  else if (ovr == 2)
    m_insertModeLabel->setText( i18n(" INS ") );

//   updateMod( mod );

  m_selectModeLabel->setText( block ? i18n(" BLK ") : i18n(" NORM ") );

  m_fileNameLabel->setText( msg );
}

void KateVSStatusBar::updateMod( bool mod )
{
  Kate::View *v = m_viewSpace->currentView();
  if ( v )
  {
    const KateDocumentInfo *info
      = KateDocManager::self()->documentInfo ( v->getDoc() );

    bool modOnHD = info && info->modifiedOnDisc;

    m_modifiedLabel->setPixmap(
        mod ?
          info && modOnHD ?
            m_modmodPm :
            m_modPm :
          info && modOnHD ?
            m_modDiscPm :
        m_noPm
        );
  }
}

void KateVSStatusBar::modifiedChanged()
{
  Kate::View *v = m_viewSpace->currentView();
  if ( v )
    updateMod( v->getDoc()->isModified() );
}

void KateVSStatusBar::showMenu()
{
   KMainWindow* mainWindow = static_cast<KMainWindow*>( topLevelWidget() );
   QPopupMenu* menu = static_cast<QPopupMenu*>( mainWindow->factory()->container("viewspace_popup", mainWindow ) );

   if (menu)
     menu->exec(QCursor::pos());
}

bool KateVSStatusBar::eventFilter(QObject*,QEvent *e)
{
  if (e->type()==QEvent::MouseButtonPress)
  {
    if ( m_viewSpace->currentView() )
      m_viewSpace->currentView()->setFocus();

    if ( ((QMouseEvent*)e)->button()==RightButton)
      showMenu();

    return true;
  }

  return false;
}
//END KateVSStatusBar
// kate: space-indent on; indent-width 2; replace-tabs on;
