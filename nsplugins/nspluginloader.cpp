/*

  This is an encapsulation of the  Netscape plugin API.


  Copyright (c) 2000 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
                     Stefan Schimanski <1Stein@gmx.de>
  Copyright (c) 2002-2005 George Staikos <staikos@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


#include <qdir.h>


#include <kapplication.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <dcopstub.h>
#include <qlayout.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qxembed.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qregexp.h>

#include "nspluginloader.h"
#include "nspluginloader.moc"

#include "NSPluginClassIface_stub.h"

#include <config.h>

NSPluginLoader *NSPluginLoader::s_instance = 0;
int NSPluginLoader::s_refCount = 0;


NSPluginInstance::NSPluginInstance(QWidget *parent) : EMBEDCLASS(parent), _loader(NULL), shown(false), inited(false), resize_count(0), stub(NULL)
{
}

void NSPluginInstance::init(const QCString &app, const QCString &obj)
{
    stub = new NSPluginInstanceIface_stub(app, obj);
    QGridLayout *_layout = new QGridLayout(this, 1, 1);
    KConfig cfg("kcmnspluginrc", false);
    cfg.setGroup("Misc");
    if(cfg.readBoolEntry("demandLoad", false))
    {
        _button = new QPushButton(i18n("Start Plugin"), dynamic_cast< EMBEDCLASS * >(this));
        _layout->addWidget(_button, 0, 0);
        connect(_button, SIGNAL(clicked()), this, SLOT(doLoadPlugin()));
        show();
    }
    else
    {
        _button = 0L;
        // Protection against repeated NPSetWindow() - Flash v9,0,115,0 doesn't handle
        // repeated NPSetWindow() calls properly, which happens when NSPluginInstance is first
        // shown and then resized. Which is what happens with KHTML. Therefore use 'shown'
        // to detect whether the widget is shown and drop all resize events before that,
        // and use 'resize_count' to wait for that one more resize to come (plus a timer
        // for a possible timeout). Only then flash is actually initialized ('inited' is true).
        resize_count = 1;
        QTimer::singleShot(1000, this, SLOT(doLoadPlugin()));
    }
}


void NSPluginInstance::doLoadPlugin()
{
    if(!inited)
    {
        delete _button;
        _button = 0L;
        _loader = NSPluginLoader::instance();
        setBackgroundMode(QWidget::NoBackground);
        WId winid = stub->winId();
        if(winid != 0)
        {
            setProtocol(QXEmbed::XPLAIN);
            embed(winid);
        }
        else
        {
            setProtocol(QXEmbed::XEMBED);
        }
        // resize before showing, some plugins are stupid and can't handle repeated
        // NPSetWindow() calls very well (viewer will avoid the call if not shown yet)
        resizePlugin(width(), height());
        displayPlugin();
        show();
        inited = true;
    }
}


NSPluginInstance::~NSPluginInstance()
{
    kdDebug() << "-> NSPluginInstance::~NSPluginInstance" << endl;
    if(inited)
        shutdown();
    kdDebug() << "release" << endl;
    if(_loader)
        _loader->release();
    kdDebug() << "<- NSPluginInstance::~NSPluginInstance" << endl;
    delete stub;
}


void NSPluginInstance::windowChanged(WId w)
{
    setBackgroundMode(w == 0 ? QWidget::PaletteBackground : QWidget::NoBackground);
    if(w == 0)
    {
        // FIXME: Put a notice here to tell the user that it crashed.
        repaint();
    }
}


void NSPluginInstance::resizeEvent(QResizeEvent *event)
{
    if(shown == false) // ignore all resizes before being shown
        return;
    if(!inited && resize_count > 0)
    {
        if(--resize_count == 0)
            doLoadPlugin();
        else
            return;
    }
    EMBEDCLASS::resizeEvent(event);
    if(isVisible())
    {
        resizePlugin(width(), height());
    }
    kdDebug() << "NSPluginInstance(client)::resizeEvent" << endl;
}

void NSPluginInstance::showEvent(QShowEvent *event)
{
    EMBEDCLASS::showEvent(event);
    shown = true;
    if(!inited && resize_count == 0)
        doLoadPlugin();
    if(inited)
        resizePlugin(width(), height());
}

void NSPluginInstance::focusInEvent(QFocusEvent *event)
{
    stub->gotFocusIn();
}

void NSPluginInstance::focusOutEvent(QFocusEvent *event)
{
    stub->gotFocusOut();
}

void NSPluginInstance::displayPlugin()
{
    qApp->syncX(); // process pending X commands
    stub->displayPlugin();
}

void NSPluginInstance::resizePlugin(int w, int h)
{
    qApp->syncX();
    stub->resizePlugin(w, h);
}

void NSPluginInstance::shutdown()
{
    if(stub)
        stub->shutdown();
}

/*******************************************************************************/


NSPluginLoader::NSPluginLoader() : QObject(), _mapping(7, false), _viewer(0)
{
    scanPlugins();
    _mapping.setAutoDelete(true);
    _filetype.setAutoDelete(true);

    // trap dcop register events
    kapp->dcopClient()->setNotifications(true);
    QObject::connect(kapp->dcopClient(), SIGNAL(applicationRegistered(const QCString &)), this, SLOT(applicationRegistered(const QCString &)));

    // load configuration
    KConfig cfg("kcmnspluginrc", false);
    cfg.setGroup("Misc");
    _useArtsdsp = cfg.readBoolEntry("useArtsdsp", false);
}


NSPluginLoader *NSPluginLoader::instance()
{
    if(!s_instance)
        s_instance = new NSPluginLoader;

    s_refCount++;
    kdDebug() << "NSPluginLoader::instance -> " << s_refCount << endl;

    return s_instance;
}


void NSPluginLoader::release()
{
    s_refCount--;
    kdDebug() << "NSPluginLoader::release -> " << s_refCount << endl;

    if(s_refCount == 0)
    {
        delete s_instance;
        s_instance = 0;
    }
}


NSPluginLoader::~NSPluginLoader()
{
    kdDebug() << "-> NSPluginLoader::~NSPluginLoader" << endl;
    unloadViewer();
    kdDebug() << "<- NSPluginLoader::~NSPluginLoader" << endl;
}


void NSPluginLoader::scanPlugins()
{
    QRegExp version(";version=[^:]*:");

    // open the cache file
    QFile cachef(locate("data", "nsplugins/cache"));
    if(!cachef.open(IO_ReadOnly))
    {
        kdDebug() << "Could not load plugin cache file!" << endl;
        return;
    }

    QTextStream cache(&cachef);

    // read in cache
    QString line, plugin;
    while(!cache.atEnd())
    {
        line = cache.readLine();
        if(line.isEmpty() || (line.left(1) == "#"))
            continue;

        if(line.left(1) == "[")
        {
            plugin = line.mid(1, line.length() - 2);
            continue;
        }

        QStringList desc = QStringList::split(':', line, TRUE);
        QString mime = desc[0].stripWhiteSpace();
        QStringList suffixes = QStringList::split(',', desc[1].stripWhiteSpace());
        if(!mime.isEmpty())
        {
            // insert the mimetype -> plugin mapping
            _mapping.insert(mime, new QString(plugin));

            // insert the suffix -> mimetype mapping
            QStringList::Iterator suffix;
            for(suffix = suffixes.begin(); suffix != suffixes.end(); ++suffix)
            {

                // strip whitspaces and any preceding '.'
                QString stripped = (*suffix).stripWhiteSpace();

                unsigned p = 0;
                for(; p < stripped.length() && stripped[p] == '.'; p++)
                    ;
                stripped = stripped.right(stripped.length() - p);

                // add filetype to list
                if(!stripped.isEmpty() && !_filetype.find(stripped))
                    _filetype.insert(stripped, new QString(mime));
            }
        }
    }
}


QString NSPluginLoader::lookupMimeType(const QString &url)
{
    QDictIterator< QString > dit2(_filetype);
    while(dit2.current())
    {
        QString ext = QString(".") + dit2.currentKey();
        if(url.right(ext.length()) == ext)
            return *dit2.current();
        ++dit2;
    }
    return QString::null;
}


QString NSPluginLoader::lookup(const QString &mimeType)
{
    QString plugin;
    if(_mapping[mimeType])
        plugin = *_mapping[mimeType];

    kdDebug() << "Looking up plugin for mimetype " << mimeType << ": " << plugin << endl;

    return plugin;
}


bool NSPluginLoader::loadViewer()
{
    kdDebug() << "NSPluginLoader::loadViewer" << endl;

    _running = false;
    _process = new KProcess;

    // get the dcop app id
    int pid = (int)getpid();
    _dcopid.sprintf("nspluginviewer-%d", pid);

    connect(_process, SIGNAL(processExited(KProcess *)), this, SLOT(processTerminated(KProcess *)));

    // find the external viewer process
    QString viewer = KGlobal::dirs()->findExe("nspluginviewer");
    if(!viewer)
    {
        kdDebug() << "can't find nspluginviewer" << endl;
        delete _process;
        return false;
    }

    // find the external artsdsp process
    if(_useArtsdsp)
    {
        kdDebug() << "trying to use artsdsp" << endl;
        QString artsdsp = KGlobal::dirs()->findExe("artsdsp");
        if(!artsdsp)
        {
            kdDebug() << "can't find artsdsp" << endl;
        }
        else
        {
            kdDebug() << artsdsp << endl;
            *_process << artsdsp;
        }
    }
    else
        kdDebug() << "don't using artsdsp" << endl;

    *_process << viewer;

    // tell the process it's parameters
    *_process << "-dcopid";
    *_process << _dcopid;

    // run the process
    kdDebug() << "Running nspluginviewer" << endl;
    _process->start();

    // wait for the process to run
    int cnt = 0;
    while(!kapp->dcopClient()->isApplicationRegistered(_dcopid))
    {
// kapp->processEvents(); // would lead to recursive calls in khtml
#ifdef HAVE_USLEEP
        usleep(50 * 1000);
#else
        sleep(1);
        kdDebug() << "sleep" << endl;
#endif
        cnt++;
#ifdef HAVE_USLEEP
        if(cnt >= 100)
#else
        if(cnt >= 10)
#endif
        {
            kdDebug() << "timeout" << endl;
            delete _process;
            return false;
        }

        if(!_process->isRunning())
        {
            kdDebug() << "nspluginviewer terminated" << endl;
            delete _process;
            return false;
        }
    }

    // get viewer dcop interface
    _viewer = new NSPluginViewerIface_stub(_dcopid, "viewer");

    return _viewer != 0;
}


void NSPluginLoader::unloadViewer()
{
    kdDebug() << "-> NSPluginLoader::unloadViewer" << endl;

    if(_viewer)
    {
        _viewer->shutdown();
        kdDebug() << "Shutdown viewer" << endl;
        delete _viewer;
        delete _process;
        _viewer = 0;
        _process = 0;
    }

    kdDebug() << "<- NSPluginLoader::unloadViewer" << endl;
}


void NSPluginLoader::applicationRegistered(const QCString &appId)
{
    kdDebug() << "DCOP application " << appId.data() << " just registered!" << endl;

    if(_dcopid == appId)
    {
        _running = true;
        kdDebug() << "plugin now running" << endl;
    }
}


void NSPluginLoader::processTerminated(KProcess *proc)
{
    if(_process == proc)
    {
        kdDebug() << "Viewer process  terminated" << endl;
        delete _viewer;
        delete _process;
        _viewer = 0;
        _process = 0;
    }
}


NSPluginInstance *NSPluginLoader::newInstance(QWidget *parent, QString url, QString mimeType, bool embed, QStringList argn, QStringList argv,
                                              QString appId, QString callbackId, bool reload, bool doPost, QByteArray postData)
{
    kdDebug() << "-> NSPluginLoader::NewInstance( parent=" << (void *)parent << ", url=" << url << ", mime=" << mimeType << ", ...)" << endl;

    if(!_viewer)
    {
        // load plugin viewer process
        loadViewer();

        if(!_viewer)
        {
            kdDebug() << "No viewer dcop stub found" << endl;
            return 0;
        }
    }

    // check the mime type
    QString mime = mimeType;
    if(mime.isEmpty())
    {
        mime = lookupMimeType(url);
        argn << "MIME";
        argv << mime;
    }
    if(mime.isEmpty())
    {
        kdDebug() << "Unknown MimeType" << endl;
        return 0;
    }

    // lookup plugin for mime type
    QString plugin_name = lookup(mime);
    if(plugin_name.isEmpty())
    {
        kdDebug() << "No suitable plugin" << endl;
        return 0;
    }

    // get plugin class object
    DCOPRef cls_ref = _viewer->newClass(plugin_name);
    if(cls_ref.isNull())
    {
        kdDebug() << "Couldn't create plugin class" << endl;
        return 0;
    }
    NSPluginClassIface_stub *cls = new NSPluginClassIface_stub(cls_ref.app(), cls_ref.object());

    // handle special plugin cases
    if(mime == "application/x-shockwave-flash")
        embed = true; // flash doesn't work in full mode :(

    NSPluginInstance *plugin = new NSPluginInstance(parent);
    kdDebug() << "<- NSPluginLoader::NewInstance = " << (void *)plugin << endl;

    // get plugin instance
    DCOPRef inst_ref = cls->newInstance(url, mime, embed, argn, argv, appId, callbackId, reload, doPost, postData, plugin->winId());
    if(inst_ref.isNull())
    {
        kdDebug() << "Couldn't create plugin instance" << endl;
        delete plugin;
        return 0;
    }

    plugin->init(inst_ref.app(), inst_ref.object());

    return plugin;
}

// vim: ts=4 sw=4 et
