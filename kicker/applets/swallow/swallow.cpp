/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2000 Carsten Pfeiffer <pfeiffer@kde.org>
 *  based on keyes (C) 1999 by Jerome Tollet <tollet@magic.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */


#include <stdlib.h>

#include <qlayout.h>
#include <qstringlist.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kshell.h>
#include <kwin.h>
#include <kwinmodule.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "swallow.h"
#include "prefwidget.h"

template class QPtrList<SwallowApp>;
typedef QPtrListIterator<SwallowApp> SwallowAppListIterator;
template class QPtrList<SwallowCommand>;


// init static variables
SwallowAppList * SwallowApplet::appList = 0L;
SwallowAppList * SwallowApplet::embeddedList = 0L;
KWinModule * SwallowApplet::wModule = 0L;
SwallowApplet * SwallowApplet::self = 0L;

extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile) {
	return new SwallowApplet(configFile, parent, "kswallow applet");
    }
}


SwallowApplet::SwallowApplet( const QString& configFile,
			      QWidget *parent, const char *name )
    : KPanelApplet( configFile, Normal, Preferences, parent, name )
{
    resize( 30, 30 );
    kdDebug() << "**** constructing swallow applet (" << configFile << ") ****" << endl;
    self = this;
    m_swcList = new SwallowCommandList;
    m_swcList->setAutoDelete( true );
    wModule = new KWinModule(this);
    embeddedList = new SwallowAppList;
    embeddedList->setAutoDelete( false );
    appList = new SwallowAppList;
    appList->setAutoDelete( true );

    QBoxLayout::Direction d = (orientation() == Horizontal) ?
			     QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
    m_layout = new QBoxLayout( this, d, 0, 2 ); // make stretch configurable?
    m_layout->setAutoAdd( false );

    // read the config file and start all the configured apps
    createApps( readConfig() );

    if ( appList->count() == 0 ) {
	if ( KMessageBox::questionYesNo(0L, i18n("There is no swallowed application, "
		"do you want to configure or quit?"), i18n("No Swallowed Application"),
		KGuiItem(i18n("Configure"),"configure"), KStdGuiItem::quit()) == KMessageBox::Yes )
	    preferences();
	else {
	    delete this;
	    ::exit(0);
	}
    }

    emit updateLayout();
}

SwallowApplet::~SwallowApplet()
{
    kdDebug() << "********************** DELETING ************************" << endl;

    delete m_swcList;
    delete embeddedList;
    delete appList;
    delete wModule;
    wModule = 0L;
}


SwallowCommandList* SwallowApplet::readConfig()
{
    m_swcList->clear();
    KConfig *kc = config();

    kc->setGroup("General");
    int count = kc->readNumEntry("Number of apps");
    kdDebug() << "*** Registered " << count << " App(s) to be swallow'ed!" << endl;
    QString group = "SwallowApp %1";
    QString title, cmd;
    ushort errors = 0;
    SwallowCommand *swc = 0L;

    for ( int i = 1; i <= count; i++ ) {
	kc->setGroup( group.arg(i) );
	cmd = kc->readPathEntry("Commandline");
	title = kc->readEntry("Window title");
	kdDebug() << "*** Found Entry:  Cmd-Line: " << cmd << " Window-Title: " << title << endl;

	if ( !cmd.isEmpty() && !title.isEmpty() ) {
	    swc = new SwallowCommand;
	    swc->cmdline = cmd;
	    swc->title = title;
	    m_swcList->append( swc );
	}
	// remember items with non-null cmdline or title,
	// discard items with empty cmdline and empty title
	else if ( !(cmd.isEmpty() && title.isEmpty()) )
	    errors++;
    }

    if ( errors > 0 ) {
	QString entry = (errors == 1) ? i18n("entry") : i18n("entries");
	if ( KMessageBox::questionYesNo(0L, i18n("I found %1 invalid/incomplete %2\nin the configuration file.\n\nBoth the window title and the commandline\n of the to be swallowed application\nare required.\n\n.Do you want to correct this?").arg(errors).arg(entry), i18n("Configuration Error"),i18n("Correct"),i18n("Ignore Error")) == KMessageBox::Yes)
	    preferences();
    }

    return m_swcList;
}


void SwallowApplet::createApps( SwallowCommandList* list )
{
    SwallowApp *app = 0L;

    SwallowCommandListIterator it( *list );
    while ( (it.current()) ) {
	app = new SwallowApp( it.current(), this );
	app->hide();
	connect( app, SIGNAL( embedded(SwallowApp *)),
		 SLOT( embedded(SwallowApp *)));
	appList->append( app );
	++it;
	kapp->processEvents();
    }

    m_layout->activate();
}


void SwallowApplet::embedded( SwallowApp *app )
{
    kdDebug() << " -> embedding " << app << ", current size is: " << width() << ", " << height() << endl;
    if ( orientation() == Horizontal )
        app->resize(height() * app->sizeRatio(), height() );
    else
        app->resize(width(), width() * app->sizeRatio());

    kdDebug() << "--> ratio: " << app->sizeRatio() << endl;
    kdDebug() << "**** " << app << " is embedded now, with (" << app->width() << ", " << app->height() << ")" << endl;

    disconnect( app, SIGNAL( embedded(SwallowApp *)),
		this, SLOT( embedded(SwallowApp *)));

    embeddedList->append( app );

    if ( orientation() == Horizontal )
	resize( widthForHeight( height() ), height() );
    else
	resize( width(), heightForWidth( width() ));

    m_layout->addWidget( app );
    app->show();
    updateGeometry();
    emit updateLayout();
}

void SwallowApplet::preferences()
{
	PreferencesWidget *prefs = new PreferencesWidget(m_swcList,this);
	prefs->show();
}


int SwallowApplet::widthForHeight(int he)
{
    kdDebug() << "**** width for h: " << he << endl;
    int w = embeddedList->isEmpty() ? 30 : 0;
    layoutApps();
    SwallowAppListIterator it( *embeddedList );
    while ( it.current() ) {
	kdDebug() << "current: " << it.current()->width() << endl;
	w += (it.current())->width();
	++it;
    }

    kdDebug() << "**** wfh: " << w << " : count: " << embeddedList->count() << endl;
    return w;
}


int SwallowApplet::heightForWidth(int)
{
    int h = embeddedList->isEmpty() ? 30 : 0;
    layoutApps();
    SwallowAppListIterator it( *embeddedList );
    while ( it.current() ) {
	h += (it.current())->height();
	++it;
    }

    kdDebug() << "**** hfw: " << h << endl;
    return h;
}

void SwallowApplet::layoutApps()
{
    if ( KPanelApplet::orientation() == Horizontal )
	m_layout->setDirection( QBoxLayout::LeftToRight );
    else
	m_layout->setDirection( QBoxLayout::TopToBottom );
}


void SwallowApplet::removeApplet( SwallowApp *app )
{
    embeddedList->removeRef( app );
    appList->remove( app );
    emit self->updateLayout();
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


static void parseCommand(KProcess *proc, QString cmd)
{
    int pos;

    cmd += " ";

    pos = cmd.find(' ');
    *proc << cmd.left(pos);
    cmd.remove(0,pos);
    cmd = cmd.stripWhiteSpace();
    *proc << KShell::splitArgs(cmd, KShell::TildeExpand | KShell::AbortOnMeta);
}


SwallowApp::SwallowApp(const SwallowCommand *swc, QWidget* parent,
                       const char* /* name */)
    : QXEmbed( parent )
{
    wh_ratio = 1;
    setAutoDelete( false );
    QXEmbed::initialize();

    winTitle = swc->title;
    connect(SwallowApplet::winModule(), SIGNAL(windowAdded(WId)),
	    this, SLOT(windowAdded(WId)));

    if (!swc->cmdline.isEmpty()) {
	KProcess *process = new KProcess;
	parseCommand(process, swc->cmdline);

	// move window out of sight
	//	*process << "-geometry";
	//	*process << QString("32x32+%1+%2").arg(kapp->desktop()->width()).arg(kapp->desktop()->height());

	connect(process, SIGNAL(processExited(KProcess*)),
		this, SLOT(processExited(KProcess*)));

	process->start();
    }
}


SwallowApp::~SwallowApp()
{
    delete process;
}


void SwallowApp::windowAdded(WId win)
{
    // determine title of newly mapped window
    XTextProperty nameProp;
    XGetWMName(qt_xdisplay(), win, &nameProp);
    char **names;
    int count;
    XTextPropertyToStringList(&nameProp, &names, &count);
    if (count < 1) {
	XFreeStringList(names);
	return;
    }

    // is this our client?
    if (winTitle == names[0]) {
	kdDebug()<< "embedding window with title: "<<winTitle.latin1() << endl;

    QRect r = KWin::windowInfo(win).geometry();
	int h = r.height();
	if ( h == 0 ) h = 1;
	wh_ratio = (float) r.width() / (float) h;
	kdDebug() << " - - - win is: " << r.width() << ", " << r.height() << endl;
	resize( r.width(), r.height() );

	embed(win);
	XReparentWindow(qt_xdisplay(), win, winId(), 0, 0);

	disconnect(SwallowApplet::winModule(), SIGNAL(windowAdded(WId)),
		   this, SLOT(windowAdded(WId)));

	emit embedded( this );
    }

    XFreeStringList(names);
}


void SwallowApp::processExited(KProcess *)
{
    SwallowApplet::removeApplet( this ); // also deletes this app
}

#include "swallow.moc"
