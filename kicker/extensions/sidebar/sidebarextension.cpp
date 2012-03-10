/***************************************************************************
                               sidebarextension.cpp
                             -------------------
    begin                : Sun July 20 16:00:00 CEST 2003
    copyright            : (C) 2003 Joseph Wenninger
    email                : jowenn@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "sidebarextension.h"
#include "sidebarextension.moc"
#include <kdebug.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <qlayout.h>
#include <konq_historymgr.h>
#include <krun.h>
#include <kurl.h>
#include <qvbox.h>
#include <qcursor.h>

extern "C"
{
   KDE_EXPORT KPanelExtension *init( QWidget *parent, const QString& configFile )
   {
      KGlobal::locale()->insertCatalogue("kickersidebarextension");
      KGlobal::locale()->insertCatalogue("konqueror");
      return new SidebarExtension( configFile,
				  KPanelExtension::Normal,
				  0,
				  parent, "kickersidebarextension");
   }
}

SidebarExtension::SidebarExtension( const QString& configFile,
                                  Type type,
                                  int actions,
                                  QWidget *parent, const char *name )
   : KPanelExtension( configFile, type, actions, parent, name ),m_resizing(false),m_expandedSize(200)
{
    kdDebug() << "SidebarExtension: Created '" << name << "', '" << configFile << "'" << endl;
    new KonqHistoryManager(0,"SidebarExtensionHistoryManager");
    m_layout=new QHBoxLayout(this);
    m_layout->activate();
    m_sbWrapper=new QVBox(this);
    KParts::ReadOnlyPart *p=KParts::ComponentFactory::createPartInstanceFromLibrary<KParts::ReadOnlyPart>(
						"konq_sidebar",
                                                 m_sbWrapper,
                                                 "SideBar_View",
                                                 this,
                                                 "Sidebar","universal");

    KParts::BrowserExtension *be=KParts::BrowserExtension::childObject(p);
    if (be) {
	connect(be,SIGNAL(openURLRequest( const KURL &, const KParts::URLArgs &)),
                        this,SLOT(openURLRequest( const KURL &, const KParts::URLArgs &)));
	connect(be,SIGNAL(createNewWindow( const KURL &, const KParts::URLArgs &)),
                        this,SLOT(openURLRequest( const KURL &, const KParts::URLArgs &)));

    }

    m_resizeHandle=new QFrame(this);
    m_resizeHandle->setFrameShape(QFrame::Panel);
    m_resizeHandle->setFrameShadow(QFrame::Raised);
    m_resizeHandle->setFixedWidth(6);
    m_resizeHandle->setCursor(QCursor(Qt::SizeHorCursor));
    connect(p->widget(),SIGNAL(panelHasBeenExpanded(bool)),this,SLOT(needLayoutUpdate(bool)));
    needLayoutUpdate(false);
    m_resizeHandle->installEventFilter(this);
    m_resizeHandle->setMouseTracking(true);
//   l->add(p->widget());
//   p->widget()->show();
//   l->activate();

}

void SidebarExtension::needLayoutUpdate(bool exp) {
	setReserveStrut(!exp); // only reserve a strut when we are collapsed
	if (exp) {
		m_currentWidth=m_expandedSize;
		m_resizeHandle->show();
		raise();
	} else {
		m_currentWidth=24;
		m_resizeHandle->hide();
	}
	topLevelWidget()->setFixedWidth(m_currentWidth);
	emit updateLayout();
}

void SidebarExtension::openURLRequest( const KURL &url, const KParts::URLArgs &) {
	KRun::runCommand("kfmclient openURL \""+url.prettyURL()+"\"", "kfmclient", "konqueror");

}


SidebarExtension::~SidebarExtension()
{
      KGlobal::locale()->removeCatalogue("kickersidebarextension");
      KGlobal::locale()->removeCatalogue("konqueror");
}

bool SidebarExtension::eventFilter( QObject *, QEvent *e ) {
	if (e->type()==QEvent::MouseButtonPress) {
		m_resizing=true;
		m_x=((QMouseEvent*)e)->globalX();
		return true;
	} else if (e->type()==QEvent::MouseButtonRelease) {
		m_resizing=false;
		m_expandedSize=topLevelWidget()->width();
		needLayoutUpdate(true);
		return true;
	} else if (e->type()==QEvent::MouseMove) {
		if (m_resizing) {
			Position p=position();
			if (p==Left) {
				int diff=((QMouseEvent*)e)->globalX()-m_x;
					if (abs(diff)>3) {
						topLevelWidget()->setFixedWidth(topLevelWidget()->width()+diff);
						m_x=((QMouseEvent*)e)->globalX();
					}
			} else if (p==Right) {
				int diff=((QMouseEvent*)e)->globalX()-m_x;
					if (abs(diff)>3) {
						topLevelWidget()->setFixedWidth(topLevelWidget()->width()-diff);
						topLevelWidget()->move(topLevelWidget()->x()+diff,topLevelWidget()->y());
						m_x=((QMouseEvent*)e)->globalX();
					}
			}
			return true;
		}
	}
	return false;
}

KPanelExtension::Position SidebarExtension::preferedPosition() const {
	kdDebug()<<"SidebarExtension::preferedPosition()***************"<<endl;
	return KPanelExtension::Left;
}

QSize SidebarExtension::sizeHint(Position, QSize maxSize ) const
{
	return QSize(m_currentWidth,maxSize.height());
}

void SidebarExtension::positionChange( Position  position ) {
	if (position == Right) {
		m_layout->remove(m_sbWrapper);
		m_layout->remove(m_resizeHandle);

		m_layout->add(m_resizeHandle);
		m_layout->add(m_sbWrapper);
	} else 	if (position == Left) {
		m_layout->remove(m_sbWrapper);
		m_layout->remove(m_resizeHandle);

		m_layout->add(m_sbWrapper);
		m_layout->add(m_resizeHandle);

	}

}

void SidebarExtension::about()
{
}

void SidebarExtension::preferences()
{
}

