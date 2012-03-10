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
AN ACTION OF CONTRACT, TORT OR OTDHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <stdlib.h>

#include <qstring.h>
#include <qfile.h>
#include <qobjectlist.h>
#include <qxembed.h>

#include <kapplication.h>
#include <kglobal.h>
#include <klibloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kpanelapplet.h>
#include <kaboutdata.h>
#include <qfileinfo.h>
#include <dcopclient.h>
#include <kwin.h>

#include "appletinfo.h"

#include "appletproxy.h"
#include "appletproxy.moc"

#include <X11/Xlib.h>

KPanelApplet::Position directionToPosition( int d )
{
    switch( d ) {
    case 1:   return KPanelApplet::pTop;
    case 2:   return KPanelApplet::pRight;
    case 3:   return KPanelApplet::pLeft;
    default:
    case 0:   return KPanelApplet::pBottom;
    }
}

static KCmdLineOptions options[] =
{
  { "+desktopfile", I18N_NOOP("The applet's desktop file"), 0 },
  { "configfile <file>", I18N_NOOP("The config file to be used"), 0 },
  { "callbackid <id>", I18N_NOOP("DCOP callback id of the applet container"), 0 },
  KCmdLineLastOption
};

extern "C" KDE_EXPORT int kdemain( int argc, char ** argv )
{
    KAboutData aboutData( "kicker", I18N_NOOP("Panel applet proxy.")
                          , "v0.1.0"
                          ,I18N_NOOP("Panel applet proxy.")
                          , KAboutData::License_BSD
                          , "(c) 2000, The KDE Developers");
    KCmdLineArgs::init(argc, argv, &aboutData );
    aboutData.addAuthor("Matthias Elter",0, "elter@kde.org");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
    KApplication::addCmdLineOptions();
    KCmdLineArgs::addCmdLineOptions(options); // Add our own options.

    KApplication a;
    a.disableSessionManagement();

    KGlobal::dirs()->addResourceType("applets", KStandardDirs::kde_default("data") +
				     "kicker/applets");

    // setup proxy object
    AppletProxy proxy(0, "appletproxywidget");

    // parse cmdline args
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( args->count() == 0 )
        KCmdLineArgs::usage(i18n("No desktop file specified") );

    // Perhaps we should use a konsole-like solution here (shell, list of args...)
    QString desktopfile( args->arg(0) );

    // load applet DSO
    if ( !QFile::exists( desktopfile ) &&
         !desktopfile.endsWith( ".desktop" ) )
        desktopfile.append( ".desktop" );

    if ( !QFile::exists( desktopfile ) )
        desktopfile = locate( "applets", desktopfile ).latin1();

    proxy.loadApplet( desktopfile, args->getOption("configfile"));

    // dock into our applet container
    QCString callbackid = args->getOption( "callbackid");
    if ( callbackid.isEmpty() )
	proxy.showStandalone();
    else
	proxy.dock(args->getOption("callbackid"));

    return a.exec();
}

AppletProxy::AppletProxy(QObject* parent, const char* name)
  : QObject(parent, name)
  , DCOPObject("AppletProxy")
  , _info(0)
  , _applet(0)
{
    // try to attach to DCOP server
    if (!kapp->dcopClient()->attach()) {
	kdError() << "Failed to attach to DCOP server." << endl;
        KMessageBox::error(0,
                           i18n("The applet proxy could not be started due to DCOP communication problems."),
                           i18n("Applet Loading Error"));
	exit(0);
    }

    if (!kapp->dcopClient()->registerAs("applet_proxy", true)) {
	kdError() << "Failed to register at DCOP server." << endl;
        KMessageBox::error(0,
                           i18n("The applet proxy could not be started due to DCOP registration problems."),
                           i18n("Applet Loading Error"));
	exit(0);
    }

    _bg = QPixmap();
}

AppletProxy::~AppletProxy()
{
    kapp->dcopClient()->detach();
    delete _info;
    delete _applet;
}

void AppletProxy::loadApplet(const QString& desktopFile, const QString& configFile)
{
    QString df;

    // try simple path first
    QFileInfo finfo( desktopFile );
    if ( finfo.exists() ) {
	df = finfo.absFilePath();
    } else {
	// locate desktop file
	df = KGlobal::dirs()->findResource("applets", desktopFile);
    }

    QFile file(df);
    // does the config file exist?
    if (df.isNull() || !file.exists()) {
	kdError() << "Failed to locate applet desktop file: " << desktopFile << endl;
        KMessageBox::error(0,
                           i18n("The applet proxy could not load the applet information from %1.").arg(desktopFile), 
                           i18n("Applet Loading Error"));
	exit(0);
    }

    // create AppletInfo instance
    delete _info;
    _info = new AppletInfo(df);

    // set the config file
    if (!configFile.isNull())
	_info->setConfigFile(configFile);

    // load applet DSO
    _applet = loadApplet(*_info);

    // sanity check
    if (!_applet)
    {
	kdError() << "Failed to load applet: " << _info->library() << endl;
        KMessageBox::error(0,
                           i18n("The applet %1 could not be loaded via the applet proxy.").arg(_info->name()), 
                           i18n("Applet Loading Error"));
	exit(0);
    }

    // connect updateLayout signal
    connect(_applet, SIGNAL(updateLayout()), SLOT(slotUpdateLayout()));
    // connect requestFocus signal
    connect(_applet, SIGNAL(requestFocus()), SLOT(slotRequestFocus()));
}

KPanelApplet* AppletProxy::loadApplet(const AppletInfo& info)
{
    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->library(QFile::encodeName(info.library()));

    if (!lib)
    {
        kdWarning() << "cannot open applet: " << info.library()
                    << " because of " << loader->lastErrorMessage() << endl;
        return 0;
    }

    KPanelApplet* (*init_ptr)(QWidget *, const QString&);
    init_ptr = (KPanelApplet* (*)(QWidget *, const QString&))lib->symbol( "init" );

    if (!init_ptr)
    {
        kdWarning() << info.library() << " is not a kicker plugin!" << endl;
        return 0;
    }

    return init_ptr(0, info.configFile());
}

void AppletProxy::repaintApplet(QWidget* widget) 
{
    widget->repaint();
 
    const QObjectList* children = widget->children();

    if (!children)
    {
        return;
    }

    QObjectList::iterator it = children->begin();
    for (; it != children->end(); ++it)
    {
        QWidget *w = dynamic_cast<QWidget*>(*it);
        if (w)
        {
            repaintApplet(w);
        }
    }
}

void AppletProxy::dock(const QCString& callbackID)
{
    kdDebug(1210) << "Callback ID: " << callbackID << endl;

    _callbackID = callbackID;

    // try to attach to DCOP server
    DCOPClient* dcop = kapp->dcopClient();

    dcop->setNotifications(true);
    connect(dcop, SIGNAL(applicationRemoved(const QCString&)),
	    SLOT(slotApplicationRemoved(const QCString&)));

    WId win;

    // get docked
    {
	QCString replyType;
	QByteArray data, replyData;
	QDataStream dataStream( data, IO_WriteOnly );

	int actions = 0;
	if (_applet) actions = _applet->actions();
	dataStream << actions;

	int type = 0;
	if (_applet) type = static_cast<int>(_applet->type());
	dataStream << type;

	// we use "call" to know whether it was sucessful

	int screen_number = 0;
	if (qt_xdisplay())
	    screen_number = DefaultScreen(qt_xdisplay());
	QCString appname;
	if (screen_number == 0)
	    appname = "kicker";
	else
	    appname.sprintf("kicker-screen-%d", screen_number);

	if ( !dcop->call(appname, _callbackID, "dockRequest(int,int)",
			 data, replyType, replyData ) )
        {
            kdError() << "Failed to dock into the panel." << endl;
            KMessageBox::error(0,
                               i18n("The applet proxy could not dock into the panel due to DCOP communication problems."),
                               i18n("Applet Loading Error"));
            exit(0);
        }

	QDataStream reply( replyData, IO_ReadOnly );
	reply >> win;

        // request background
        dcop->send(appname, _callbackID, "getBackground()", data);
    }

    if (win)
    {
        if (_applet)
        {
            _applet->hide();
        }
        QXEmbed::initialize();
        QXEmbed::embedClientIntoWindow(_applet, win);
    }
    else
    {
        kdError() << "Failed to dock into the panel." << endl;
        KMessageBox::error(0,
                           i18n("The applet proxy could not dock into the panel."),
                           i18n("Applet Loading Error"));
        delete _applet;
        _applet = 0;

        exit(0);
    }

}

bool AppletProxy::process(const QCString &fun, const QByteArray &data,
                          QCString& replyType, QByteArray &replyData)
{
    if ( fun == "widthForHeight(int)" )
	{
	    QDataStream dataStream( data, IO_ReadOnly );
	    int height;
	    dataStream >> height;
	    QDataStream reply( replyData, IO_WriteOnly );
	    replyType = "int";

	    if (!_applet)
		reply << height;
	    else
		reply << _applet->widthForHeight(height);

	    return true;
	}
    else if ( fun == "heightForWidth(int)" )
	{
	    QDataStream dataStream( data, IO_ReadOnly );
	    int width;
	    dataStream >> width;
	    QDataStream reply( replyData, IO_WriteOnly );
	    replyType = "int";

	    if(!_applet)
		reply << width;
	    else
		reply << _applet->heightForWidth(width);

	    return true;
	}
    else if ( fun == "setDirection(int)" )
	{
	    QDataStream dataStream( data, IO_ReadOnly );
	    int dir;
	    dataStream >> dir;

	    if(_applet) {
		_applet->setPosition(directionToPosition(dir));
	    }
	    return true;
	}
    else if ( fun == "setAlignment(int)" )
	{
	    QDataStream dataStream( data, IO_ReadOnly );
	    int alignment;
	    dataStream >> alignment;

	    if(_applet) {
		_applet->setAlignment( (KPanelApplet::Alignment)alignment );
	    }
	    return true;
	}
    else if ( fun == "removedFromPanel()" )
	{
            delete _applet;
            _applet = 0;
            exit(0);
	    return true;
	}
    else if ( fun == "about()" )
	{
	    if(_applet) _applet->action( KPanelApplet::About );
	    return true;
	}
    else if ( fun == "help()" )
	{
	    if(_applet) _applet->action( KPanelApplet::Help );
	    return true;
	}
    else if ( fun == "preferences()" )
	{
	    if(_applet) _applet->action( KPanelApplet::Preferences );
	    return true;
	}
    else if (fun == "reportBug()" )
  {
      if(_applet) _applet->action( KPanelApplet::ReportBug );
      return true;
  }
    else if ( fun == "actions()" )
	{
	    QDataStream reply( replyData, IO_WriteOnly );
	    int actions = 0;
	    if(_applet) actions = _applet->actions();
	    reply << actions;
	    replyType = "int";
	    return true;
	}
    else if ( fun == "type()" )
	{
	    QDataStream reply( replyData, IO_WriteOnly );
	    int type = 0;
	    if (_applet) type = static_cast<int>(_applet->type());
	    reply << type;
	    replyType = "int";
	    return true;
	}
    else if ( fun == "setBackground(QPixmap)" )
        {
            QDataStream dataStream( data, IO_ReadOnly ); 
            dataStream >> _bg;
            if(_applet)
                if ( _bg.isNull() ) { // no transparency
		    _applet->unsetPalette();
		    _applet->repaint();
		}
                else { //transparency
		    _applet->blockSignals(true);
		    _applet->setBackgroundMode(Qt::FixedPixmap);
		    _applet->setPaletteBackgroundPixmap(_bg);
		    repaintApplet(_applet);
		    _applet->blockSignals(false);
                }
            return true;
        }
    return false;
}

void AppletProxy::slotUpdateLayout()
{
    if(_callbackID.isNull()) return;

    QByteArray data;
    int screen_number = 0;
    if (qt_xdisplay())
	screen_number = DefaultScreen(qt_xdisplay());
    QCString appname;
    if (screen_number == 0)
	appname = "kicker";
    else
	appname.sprintf("kicker-screen-%d", screen_number);

    kapp->dcopClient()->send(appname, _callbackID, "updateLayout()", data);
}

void AppletProxy::slotRequestFocus()
{
    if(_callbackID.isNull()) return;

    QByteArray data;
    int screen_number = 0;
    if (qt_xdisplay())
	screen_number = DefaultScreen(qt_xdisplay());
    QCString appname;
    if (screen_number == 0)
	appname = "kicker";
    else
	appname.sprintf("kicker-screen-%d", screen_number);

    kapp->dcopClient()->send(appname, _callbackID, "requestFocus()", data);
}

void AppletProxy::slotApplicationRemoved(const QCString& appId)
{
    int screen_number = 0;
    if (qt_xdisplay())
	screen_number = DefaultScreen(qt_xdisplay());
    QCString appname;
    if (screen_number == 0)
	appname = "kicker";
    else
	appname.sprintf("kicker-screen-%d", screen_number);

    if(appId == appname) {
	kdDebug(1210) << "Connection to kicker lost, shutting down" << endl;
	kapp->quit();
    }
}

void AppletProxy::showStandalone()
{
    if (!_applet)
    {
        return;
    }

    _applet->resize( _applet->widthForHeight( 48 ), 48 );
    _applet->setMinimumSize( _applet->size() );
    _applet->setCaption( _info->name() );
    kapp->setMainWidget( _applet );
    _applet->show();
}

