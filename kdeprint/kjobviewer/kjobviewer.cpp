/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kjobviewer.h"
#include <kdeprint/kmjobviewer.h>
#include <kdeprint/kmtimer.h>
#include <kdeprint/kmmanager.h>

#include <stdlib.h>
#include <qpixmap.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <ksystemtray.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kwin.h>
#include <kstartupinfo.h>

class JobTray : public KSystemTray
{
public:
	JobTray(KJobViewerApp *parent, const char *name = 0)
		: KSystemTray(0, name), m_app(parent) { connect( this, SIGNAL( quitSelected() ), kapp, SLOT( quit() ) ); }
protected:
	void mousePressEvent(QMouseEvent*);
private:
	KJobViewerApp	*m_app;
};

void JobTray::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::RightButton)
		KSystemTray::mousePressEvent(e);
	else if (m_app->m_views.count() > 0)
	{
		KPopupMenu	menu;
		QDictIterator<KMJobViewer>	it(m_app->m_views);
		QPtrList<KMJobViewer>	list;
		list.setAutoDelete(false);
		for (; it.current(); ++it)
		{
			menu.insertItem(KWin::icon(it.current()->winId(), 16, 16), it.currentKey(), list.count());
			if (it.current()->isVisible())
				menu.setItemChecked(list.count(), true);
			list.append(it.current());
		}

		if (list.count() == 1)
		{
			// special case, old behavior
			if (list.first()->isVisible())
				list.first()->hide();
			else
				list.first()->show();
		}
		else
		{
			int	choice = menu.exec(mapToGlobal(e->pos()));
			if (choice != -1)
			{
				KMJobViewer	*view = list.at(choice);
				if (view->isVisible())
					KWin::activateWindow(view->winId());
				else
					view->show();
			}
		}
	}
}

//-------------------------------------------------------------

KJobViewerApp::KJobViewerApp() : KUniqueApplication()
{
	m_views.setAutoDelete(true);
	m_tray = 0;
	m_timer = 0;
}

KJobViewerApp::~KJobViewerApp()
{
}

int KJobViewerApp::newInstance()
{
	initialize();
	return 0;
}

void KJobViewerApp::initialize()
{
	KCmdLineArgs	*args = KCmdLineArgs::parsedArgs();
	bool 	showIt = args->isSet("show");
	bool	all = args->isSet("all");
	QString	prname = args->getOption("d");
	KMJobViewer	*view(0);

	if (!m_timer)
	{
		m_timer = KMTimer::self();
		connect(m_timer,SIGNAL(timeout()),SLOT(slotTimer()));
	}

	if (prname.isEmpty() && all)
		prname = i18n("All Printers");

        if (prname.isEmpty()) {
            KMPrinter *prt =  KMManager::self()->defaultPrinter();
            if (prt)
                prname = prt->printerName();
            else {
                KMessageBox::error(0, i18n("There is no default printer. Start with --all to see all printers."), i18n("Print Error"));
                exit(1);
                return;
            }
        }

        if (!m_tray)
        {
            m_tray = new JobTray(this);
            m_tray->setPixmap(m_tray->loadIcon("fileprint"));

        }

        view = m_views.find(prname);
        if (!view)
        {
            kdDebug() << "creating new view: " << prname << endl;
            view = new KMJobViewer();
            connect(view, SIGNAL(jobsShown(KMJobViewer*,bool)), SLOT(slotJobsShown(KMJobViewer*,bool)));
            connect(view, SIGNAL(printerChanged(KMJobViewer*,const QString&)), SLOT(slotPrinterChanged(KMJobViewer*,const QString&)));
            connect(view, SIGNAL(refreshClicked()), SLOT(slotTimer()));
            connect(view, SIGNAL(viewerDestroyed(KMJobViewer*)), SLOT(slotViewerDestroyed(KMJobViewer*)));
            m_views.insert(prname, view);
        }

        if (showIt)
        {
            view->show();
            m_tray->show();
        }
        view->setPrinter(prname);

	//m_timer->release(true);
	m_timer->delay(10);
}

void KJobViewerApp::slotJobsShown(KMJobViewer *view, bool hasJobs)
{
	if (!hasJobs && !view->isVisible() && !view->isSticky())
	{
		kdDebug() << "removing view: " << view->printer() << endl;
		// the window is hidden and doesn't have any job shown -> destroy it
		// closing won't have any effect as the KMJobViewer overload closeEvent()
		m_views.remove(view->printer());
	}

	if (m_views.count() > 0)
	{
		if (!m_tray->isVisible())
			m_tray->show();
	}
	else {
		KStartupInfo::appStarted();
		kapp->quit();
	}
}

void KJobViewerApp::slotTimer()
{
	// Update printer list
	KMManager::self()->printerList(true);

	// Refresh views. The first time, job list is reloaded,
	// other views will simply use reloaded job list
	bool	trigger(true);
	QDictIterator<KMJobViewer>	it(m_views);
	for (; it.current(); ++it, trigger=false)
		it.current()->refresh(trigger);
}

void KJobViewerApp::slotPrinterChanged(KMJobViewer *view, const QString& prname)
{
	KMJobViewer	*other = m_views.find(prname);
	if (other)
	{
		if (other->isVisible())
			KWin::activateWindow(other->winId());
		else
			other->show();
	}
	else
	{
		m_views.take(view->printer());
		m_views.insert(prname, view);
		view->setPrinter(prname);
	}
}

void KJobViewerApp::reload()
{
	// trigger delayed refresh in all views
	m_timer->delay(10);
}

void KJobViewerApp::slotViewerDestroyed(KMJobViewer *view)
{
	if (view)
		m_views.take(view->printer());
	if (m_views.count() == 0)
		kapp->quit();
}

#include "kjobviewer.moc"
