/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

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

//BEGIN Includes
#include "kateviewmanager.h"
#include "kateviewmanager.moc"

#include "katemainwindow.h"
#include "katedocmanager.h"
#include "kateviewspacecontainer.h"
#include "katetabwidget.h"

#include <dcopclient.h>
#include <kaction.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kdiroperator.h>
#include <kdockwidget.h>
#include <kencodingfiledialog.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstdaction.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kstdaccel.h>

#include <ktexteditor/encodinginterface.h>

#include <qobjectlist.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
//END Includes

KateViewManager::KateViewManager (KateMainWindow *parent)
 : QObject  (parent),
  showFullPath(false), m_mainWindow(parent)
{
  // while init
  m_init=true;

  // some stuff for the tabwidget
  m_mainWindow->tabWidget()->setTabReorderingEnabled( true );

  // important, set them up, as we use them in other methodes
  setupActions ();

  guiMergedView=0;

  m_viewManager = new Kate::ViewManager (this);
  m_currentContainer=0;
 connect(m_mainWindow->tabWidget(),SIGNAL(currentChanged(QWidget*)),this,SLOT(tabChanged(QWidget*)));
 slotNewTab();
 tabChanged(m_mainWindow->tabWidget()->currentPage());

  // no memleaks
  m_viewSpaceContainerList.setAutoDelete(true);

  // init done
  m_init=false;
}

KateViewManager::~KateViewManager ()
{
  m_viewSpaceContainerList.setAutoDelete(false);
}

void KateViewManager::setupActions ()
{
  KAction *a;

  /**
   * tabbing
   */
  a=new KAction ( i18n("New Tab"),"tab_new", 0, this, SLOT(slotNewTab()),
                  m_mainWindow->actionCollection(), "view_new_tab" );

  m_closeTab = new KAction ( i18n("Close Current Tab"),"tab_remove",0,this,SLOT(slotCloseTab()),
                             m_mainWindow->actionCollection(),"view_close_tab");

  m_activateNextTab
      = new KAction( i18n( "Activate Next Tab" ),
                     QApplication::reverseLayout() ? KStdAccel::tabPrev() : KStdAccel::tabNext(),
                     this, SLOT( activateNextTab() ), m_mainWindow->actionCollection(), "view_next_tab" );

  m_activatePrevTab
      = new KAction( i18n( "Activate Previous Tab" ),
                     QApplication::reverseLayout() ? KStdAccel::tabNext() : KStdAccel::tabPrev(),
                     this, SLOT( activatePrevTab() ), m_mainWindow->actionCollection(), "view_prev_tab" );

  /**
   * view splitting
   */
  a=new KAction ( i18n("Split Ve&rtical"), "view_right", CTRL+SHIFT+Key_L, this, SLOT(
                  slotSplitViewSpaceVert() ), m_mainWindow->actionCollection(), "view_split_vert");

  a->setWhatsThis(i18n("Split the currently active view vertically into two views."));

  a=new KAction ( i18n("Split &Horizontal"), "view_bottom", CTRL+SHIFT+Key_T, this, SLOT(
                  slotSplitViewSpaceHoriz() ), m_mainWindow->actionCollection(), "view_split_horiz");

  a->setWhatsThis(i18n("Split the currently active view horizontally into two views."));

  m_closeView = new KAction ( i18n("Cl&ose Current View"), "view_remove", CTRL+SHIFT+Key_R, this,
                    SLOT( slotCloseCurrentViewSpace() ), m_mainWindow->actionCollection(),
                    "view_close_current_space" );

  m_closeView->setWhatsThis(i18n("Close the currently active splitted view"));

  goNext=new KAction(i18n("Next View"),Key_F8,this,
                     SLOT(activateNextView()),m_mainWindow->actionCollection(),"go_next");

  goNext->setWhatsThis(i18n("Make the next split view the active one."));

  goPrev=new KAction(i18n("Previous View"),SHIFT+Key_F8, this, SLOT(activatePrevView()),m_mainWindow->actionCollection(),"go_prev");

  goPrev->setWhatsThis(i18n("Make the previous split view the active one."));

  /**
   * buttons for tabbing
   */
  QToolButton *b = new QToolButton( m_mainWindow->tabWidget() );
  connect( b, SIGNAL( clicked() ),
             this, SLOT( slotNewTab() ) );
  b->setIconSet( SmallIcon( "tab_new" ) );
  b->adjustSize();
  QToolTip::add(b, i18n("Open a new tab"));
  m_mainWindow->tabWidget()->setCornerWidget( b, TopLeft );

  b = m_closeTabButton = new QToolButton( m_mainWindow->tabWidget() );
  connect( b, SIGNAL( clicked() ),
            this, SLOT( slotCloseTab() ) );
  b->setIconSet( SmallIcon( "tab_remove" ) );
  b->adjustSize();
  QToolTip::add(b, i18n("Close the current tab"));
  m_mainWindow->tabWidget()->setCornerWidget( b, TopRight );
}

void KateViewManager::updateViewSpaceActions ()
{
  if (!m_currentContainer) return;

  m_closeView->setEnabled (m_currentContainer->viewSpaceCount() > 1);
  goNext->setEnabled (m_currentContainer->viewSpaceCount() > 1);
  goPrev->setEnabled (m_currentContainer->viewSpaceCount() > 1);
}

void KateViewManager::tabChanged(QWidget* widget) {
  KateViewSpaceContainer *container=static_cast<KateViewSpaceContainer*>(widget->qt_cast("KateViewSpaceContainer"));
  Q_ASSERT(container);
  m_currentContainer=container;

  if (container) {
    container->reactivateActiveView();

  }

  m_closeTab->setEnabled(m_mainWindow->tabWidget()->count() > 1);
  m_activateNextTab->setEnabled(m_mainWindow->tabWidget()->count() > 1);
  m_activatePrevTab->setEnabled(m_mainWindow->tabWidget()->count() > 1);
  m_closeTabButton->setEnabled (m_mainWindow->tabWidget()->count() > 1);

  updateViewSpaceActions ();
}

void KateViewManager::slotNewTab()
{
  uint documentNumber=0;

  if (m_currentContainer)
  {
    if (m_currentContainer->activeView())
      documentNumber = m_currentContainer->activeView()->getDoc()->documentNumber();
  }

  KateViewSpaceContainer *container=new KateViewSpaceContainer (m_mainWindow->tabWidget(), this);
  m_viewSpaceContainerList.append(container);
  m_mainWindow->tabWidget()->addTab (container, "");

  connect(container,SIGNAL(viewChanged()),this,SIGNAL(viewChanged()));
  connect(container,SIGNAL(viewChanged()),m_viewManager,SIGNAL(viewChanged()));

  if (!m_init)
  {
    container->activateView(documentNumber);
    container->setShowFullPath(showFullPath);
    m_mainWindow->slotWindowActivated ();
  }
}

void KateViewManager::slotCloseTab()
{
  if (m_viewSpaceContainerList.count() <= 1) return;
  if (!m_currentContainer) return;

  int pos = m_viewSpaceContainerList.find (m_currentContainer);

  if (pos == -1)
    return;

  if (guiMergedView)
    m_mainWindow->guiFactory()->removeClient (guiMergedView );

  m_viewSpaceContainerList.remove (pos);

  if ((uint)pos >= m_viewSpaceContainerList.count())
    pos = m_viewSpaceContainerList.count()-1;

  tabChanged(m_viewSpaceContainerList.at (pos));
}

void KateViewManager::activateNextTab()
{
  if( m_mainWindow->tabWidget()->count() <= 1 ) return;

  int iTab = m_mainWindow->tabWidget()->currentPageIndex();

  iTab++;

  if( iTab == m_mainWindow->tabWidget()->count() )
    iTab = 0;

  m_mainWindow->tabWidget()->setCurrentPage( iTab );
}

void KateViewManager::activatePrevTab()
{
  if( m_mainWindow->tabWidget()->count() <= 1 ) return;

  int iTab = m_mainWindow->tabWidget()->currentPageIndex();

  iTab--;

  if( iTab == -1 )
    iTab = m_mainWindow->tabWidget()->count() - 1;

  m_mainWindow->tabWidget()->setCurrentPage( iTab );
}

void KateViewManager::activateSpace (Kate::View* v)
{
  if (m_currentContainer) {
    m_currentContainer->activateSpace(v);
  }
}

void KateViewManager::activateView ( Kate::View *view ) {
  if (m_currentContainer) {
    m_currentContainer->activateView(view);
  }
}

KateViewSpace* KateViewManager::activeViewSpace ()
{
  if (m_currentContainer) {
    return m_currentContainer->activeViewSpace();
  }
  return 0L;
}

Kate::View* KateViewManager::activeView ()
{
  if (m_currentContainer) {
    return m_currentContainer->activeView();
  }
  return 0L;
}

void KateViewManager::setActiveSpace ( KateViewSpace* vs )
{
  if (m_currentContainer) {
    m_currentContainer->setActiveSpace(vs);
  }

}

void KateViewManager::setActiveView ( Kate::View* view )
{
  if (m_currentContainer) {
    m_currentContainer->setActiveView(view);
  }

}


void KateViewManager::activateView( uint documentNumber )
{
  if (m_currentContainer) {
     m_currentContainer->activateView(documentNumber);
  }
}

uint KateViewManager::viewCount ()
{
  uint viewCount=0;
  for (uint i=0;i<m_viewSpaceContainerList.count();i++) {
    viewCount+=m_viewSpaceContainerList.at(i)->viewCount();
  }
  return viewCount;

}

uint KateViewManager::viewSpaceCount ()
{
  uint viewSpaceCount=0;
  for (uint i=0;i<m_viewSpaceContainerList.count();i++) {
    viewSpaceCount+=m_viewSpaceContainerList.at(i)->viewSpaceCount();
  }
  return viewSpaceCount;
}

void KateViewManager::setViewActivationBlocked (bool block)
{
  for (uint i=0;i<m_viewSpaceContainerList.count();i++)
    m_viewSpaceContainerList.at(i)->m_blockViewCreationAndActivation=block;
}

void KateViewManager::activateNextView()
{
  if (m_currentContainer) {
    m_currentContainer->activateNextView();
  }
}

void KateViewManager::activatePrevView()
{
  if (m_currentContainer) {
    m_currentContainer->activatePrevView();
  }
}

void KateViewManager::closeViews(uint documentNumber)
{
  for (uint i=0;i<m_viewSpaceContainerList.count();i++) {
    m_viewSpaceContainerList.at(i)->closeViews(documentNumber);
  }
  tabChanged(m_currentContainer);
}

void KateViewManager::slotDocumentNew ()
{
  if (m_currentContainer) m_currentContainer->createView ();
}

void KateViewManager::slotDocumentOpen ()
{
  Kate::View *cv = activeView();

  if (cv) {
    KEncodingFileDialog::Result r=KEncodingFileDialog::getOpenURLsAndEncoding(
      (cv ? KTextEditor::encodingInterface(cv->document())->encoding() : Kate::Document::defaultEncoding()),
       (cv ? cv->document()->url().url() : QString::null),
       QString::null,m_mainWindow,i18n("Open File"));

    uint lastID = 0;
    for (KURL::List::Iterator i=r.URLs.begin(); i != r.URLs.end(); ++i)
      lastID = openURL( *i, r.encoding, false );

    if (lastID > 0)
      activateView (lastID);
  }
}

void KateViewManager::slotDocumentClose ()
{
  // no active view, do nothing
  if (!activeView()) return;

  // prevent close document if only one view alive and the document of
  // it is not modified and empty !!!
  if ( (KateDocManager::self()->documents() == 1)
       && !activeView()->getDoc()->isModified()
       && activeView()->getDoc()->url().isEmpty()
       && (activeView()->getDoc()->length() == 0) )
  {
    activeView()->getDoc()->closeURL();
    return;
  }

  // close document
  KateDocManager::self()->closeDocument (activeView()->getDoc());
}

uint KateViewManager::openURL (const KURL &url, const QString& encoding, bool activate, bool isTempFile )
{
  uint id = 0;
  Kate::Document *doc = KateDocManager::self()->openURL (url, encoding, &id, isTempFile );

  if (!doc->url().isEmpty())
    m_mainWindow->fileOpenRecent->addURL( doc->url() );

  if (activate)
    activateView( id );

  return id;
}

void KateViewManager::openURL (const KURL &url)
{
  openURL (url, QString::null);
}

void KateViewManager::removeViewSpace (KateViewSpace *viewspace)
{
  if (m_currentContainer) {
    m_currentContainer->removeViewSpace(viewspace);
  }
}

void KateViewManager::slotCloseCurrentViewSpace()
{
  if (m_currentContainer) {
    m_currentContainer->slotCloseCurrentViewSpace();
  }
}

void KateViewManager::slotSplitViewSpaceVert()
{
  if (m_currentContainer) {
    m_currentContainer->slotSplitViewSpaceVert();
  }
}

void KateViewManager::slotSplitViewSpaceHoriz()
{
  if (m_currentContainer) {
    m_currentContainer->slotSplitViewSpaceHoriz();
  }
}

void KateViewManager::setShowFullPath( bool enable )
{
  showFullPath=enable;
  for (uint i=0;i<m_viewSpaceContainerList.count();i++) {
    m_viewSpaceContainerList.at(i)->setShowFullPath(enable);
  }
  m_mainWindow->slotWindowActivated ();
 }

/**
 * session config functions
 */
// FIXME 3.0 - make those config goups more streamlined: "objN:objN..."
void KateViewManager::saveViewConfiguration(KConfig *config,const QString& grp)
{
  // Use the same group name for view configuration as usual for sessions.
  // (When called by session manager grp is a 1-based index for the main window)
  QString group = grp;
  bool ok = false;
  int n = group.toInt( &ok );
  if ( ok )
    group = QString( "MainWindow%1" ).arg( n-1 );

  config->setGroup(group);
  config->writeEntry("ViewSpaceContainers",m_viewSpaceContainerList.count());
  config->writeEntry("Active ViewSpaceContainer", m_mainWindow->tabWidget()->currentPageIndex());
  for (uint i=0;i<m_viewSpaceContainerList.count();i++) {
    m_viewSpaceContainerList.at(i)->saveViewConfiguration(config,group+QString(":ViewSpaceContainer-%1:").arg(i));
  }
}

void KateViewManager::restoreViewConfiguration (KConfig *config, const QString& grp)
{
  // Use the same group name for view configuration as usual for sessions.
  // (When called by session manager grp is a 1-based index for the main window)
  QString group = grp;
  bool ok = false;
  int n = group.toInt( &ok );
  if ( ok )
    group = QString( "MainWindow%1" ).arg( n-1 );

  config->setGroup(group);
  uint tabCount=config->readNumEntry("ViewSpaceContainers",0);
  int activeOne=config->readNumEntry("Active ViewSpaceContainer",0);
  if (tabCount==0) return;
  m_viewSpaceContainerList.at(0)->restoreViewConfiguration(config,group+QString(":ViewSpaceContainer-0:"));
  for (uint i=1;i<tabCount;i++) {
    slotNewTab();
    m_viewSpaceContainerList.at(i)->restoreViewConfiguration(config,group+QString(":ViewSpaceContainer-%1:").arg(i));
  }

  if (activeOne != m_mainWindow->tabWidget()->currentPageIndex())
    m_mainWindow->tabWidget()->setCurrentPage (activeOne);

  updateViewSpaceActions();
}

KateMainWindow *KateViewManager::mainWindow() {
        return m_mainWindow;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
