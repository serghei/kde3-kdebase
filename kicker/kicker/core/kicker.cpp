/*****************************************************************

  Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

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

#include <stdlib.h>
#include <unistd.h>

#include <qfile.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <dcopclient.h>
#include <kconfig.h>
#include <kcmdlineargs.h>
#include <kcmultidialog.h>
#include <kcrash.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <kwinmodule.h>

#include "extensionmanager.h"
#include "pluginmanager.h"
#include "menumanager.h"
#include "k_mnu.h"
#include "showdesktop.h"
#include "panelbutton.h"

#include "kicker.h"
#include "kickerSettings.h"

#include "kicker.moc"

Kicker* Kicker::the() { return static_cast<Kicker*>(kapp); }

Kicker::Kicker()
    : KUniqueApplication(),
      keys(0),
      m_kwinModule(0),
      m_configDialog(0),
      m_canAddContainers(true)
{
    // initialize the configuration object
    KickerSettings::instance(instanceName() + "rc");

    if (KCrash::crashHandler() == 0 )
    {
        // this means we've most likely crashed once. so let's see if we
        // stay up for more than 2 minutes time, and if so reset the
        // crash handler since the crash isn't a frequent offender
        QTimer::singleShot(120000, this, SLOT(setCrashHandler()));
    }
    else
    {
        // See if a crash handler was installed. It was if the -nocrashhandler
        // argument was given, but the app eats the kde options so we can't
        // check that directly. If it wasn't, don't install our handler either.
        setCrashHandler();
    }

    // Make kicker immutable if configuration modules have been marked immutable
    if (isKioskImmutable() && kapp->authorizeControlModules(Kicker::configModules(true)).isEmpty())
    {
        config()->setReadOnly(true);
        config()->reparseConfiguration();
    }

    dcopClient()->setDefaultObject("Panel");
    disableSessionManagement();
    QString dataPathBase = KStandardDirs::kde_default("data").append("kicker/");
    KGlobal::dirs()->addResourceType("mini", dataPathBase + "pics/mini");
    KGlobal::dirs()->addResourceType("icon", dataPathBase + "pics");
    KGlobal::dirs()->addResourceType("builtinbuttons", dataPathBase + "builtins");
    KGlobal::dirs()->addResourceType("specialbuttons", dataPathBase + "menuext");
    KGlobal::dirs()->addResourceType("applets", dataPathBase + "applets");
    KGlobal::dirs()->addResourceType("tiles", dataPathBase + "tiles");
    KGlobal::dirs()->addResourceType("extensions", dataPathBase +  "extensions");

    KImageIO::registerFormats();

    KGlobal::iconLoader()->addExtraDesktopThemes();

    KGlobal::locale()->insertCatalogue("libkonq");
    KGlobal::locale()->insertCatalogue("libdmctl");
    KGlobal::locale()->insertCatalogue("libtaskbar");

    // initialize our keys
    // note that this creates the KMenu by calling MenuManager::the()
    keys = new KGlobalAccel( this );
#define KICKER_ALL_BINDINGS
#include "kickerbindings.cpp"
    keys->readSettings();
    keys->updateConnections();

    // set up our global settings
    configure();

    connect(this, SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)));
    connect(this, SIGNAL(kdisplayPaletteChanged()), SLOT(paletteChanged()));
    connect(this, SIGNAL(kdisplayStyleChanged()), SLOT(slotStyleChanged()));

#if (QT_VERSION-0 >= 0x030200) // XRANDR support
    connect(desktop(), SIGNAL(resized(int)), SLOT(slotDesktopResized()));
#endif

    // the panels, aka extensions
    QTimer::singleShot(0, ExtensionManager::the(), SLOT(initialize()));

    connect(ExtensionManager::the(), SIGNAL(desktopIconsAreaChanged(const QRect &, int)),
            this, SLOT(slotDesktopIconsAreaChanged(const QRect &, int)));
}

Kicker::~Kicker()
{
    // order of deletion here is critical to avoid crashes
    delete ExtensionManager::the();
    delete MenuManager::the();
}

void Kicker::setCrashHandler()
{
    KCrash::setEmergencySaveFunction(Kicker::crashHandler);
}

void Kicker::crashHandler(int /* signal */)
{
    fprintf(stderr, "kicker: crashHandler called\n");

    DCOPClient::emergencyClose();
    sleep(1);
    system("kicker --nocrashhandler &"); // try to restart
}

void Kicker::slotToggleShowDesktop()
{
    // don't connect directly to the ShowDesktop::toggle() slot
    // so that the ShowDesktop object doesn't get created if
    // this feature is never used, and isn't created until after
    // startup even if it is
    ShowDesktop::the()->toggle();
}

void Kicker::toggleLock()
{
    KickerSettings::self()->setLocked(!KickerSettings::locked());
    KickerSettings::self()->writeConfig();
    emit immutabilityChanged(isImmutable());
}

void Kicker::toggleShowDesktop()
{
    ShowDesktop::the()->toggle();
}

bool Kicker::desktopShowing()
{
    return ShowDesktop::the()->desktopShowing();
}

void Kicker::slotSettingsChanged(int category)
{
    if (category == (int)KApplication::SETTINGS_SHORTCUTS)
    {
        keys->readSettings();
        keys->updateConnections();
    }
}

void Kicker::paletteChanged()
{
    KConfigGroup c(KGlobal::config(), "General");
    KickerSettings::setTintColor(c.readColorEntry("TintColor",
                                           &palette().active().mid()));
    KickerSettings::self()->writeConfig();
}

void Kicker::slotStyleChanged()
{
	restart();
}

bool Kicker::highlightMenuItem(const QString &menuId)
{
    return MenuManager::the()->kmenu()->highlightMenuItem( menuId );
}

void Kicker::showKMenu()
{
    MenuManager::the()->showKMenu();
}

void Kicker::popupKMenu(const QPoint &p)
{
    MenuManager::the()->popupKMenu(p);
}

void Kicker::configure()
{
    static bool notFirstConfig = false;

    KConfig* c = KGlobal::config();
    c->reparseConfiguration();
    c->setGroup("General");
    m_canAddContainers = !c->entryIsImmutable("Applets2");

    KickerSettings::self()->readConfig();

    QToolTip::setGloballyEnabled(KickerSettings::showToolTips());

    if (notFirstConfig)
    {
        emit configurationChanged();
        {
            QByteArray data;
            emitDCOPSignal("configurationChanged()", data);
        }
    }

    notFirstConfig = true;
//    kdDebug(1210) << "tooltips " << ( _showToolTips ? "enabled" : "disabled" ) << endl;
}

void Kicker::quit()
{
    exit(1);
}

void Kicker::restart()
{
    // do this on a timer to give us time to return true
    QTimer::singleShot(0, this, SLOT(slotRestart()));
}

void Kicker::slotRestart()
{
    // since the child will awaken before we do, we need to
    // clear the untrusted list manually; can't rely on the
    // dtor's to this for us.
    PluginManager::the()->clearUntrustedLists();

    char ** o_argv = new char*[2];
    o_argv[0] = strdup("kicker");
    o_argv[1] = 0L;
    execv(QFile::encodeName(locate("exe", "kdeinit_wrapper")), o_argv);

    exit(1);
}

bool Kicker::isImmutable() const
{
    return config()->isImmutable() || KickerSettings::locked();
}

bool Kicker::isKioskImmutable() const
{
    return config()->isImmutable();
}

void Kicker::addExtension( const QString &desktopFile )
{
   ExtensionManager::the()->addExtension( desktopFile );
}

QStringList Kicker::configModules(bool controlCenter)
{
    QStringList args;

    if (controlCenter)
    {
        args << "kde-panel.desktop";
    }
    else
    {
        args << "kde-kicker_config_arrangement.desktop"
             << "kde-kicker_config_hiding.desktop"
             << "kde-kicker_config_menus.desktop"
             << "kde-kicker_config_appearance.desktop";
    }
    args << "kde-kcmtaskbar.desktop";
    return args;
}

QPoint Kicker::insertionPoint()
{
    return m_insertionPoint;
}

void Kicker::setInsertionPoint(const QPoint &p)
{
    m_insertionPoint = p;
}


void Kicker::showConfig(const QString& configPath, int page)
{
    if (!m_configDialog)
    {
         m_configDialog = new KCMultiDialog(0);

         QStringList modules = configModules(false);
         QStringList::ConstIterator end(modules.end());
         for (QStringList::ConstIterator it = modules.begin(); it != end; ++it)
         {
            m_configDialog->addModule(*it);
         }

         connect(m_configDialog, SIGNAL(finished()), SLOT(configDialogFinished()));
    }

    if (!configPath.isEmpty())
    {
        QByteArray data;
        QDataStream stream(data, IO_WriteOnly);
        stream << configPath;
        emitDCOPSignal("configSwitchToPanel(QString)", data);
    }

    KWin::setOnDesktop(m_configDialog->winId(), KWin::currentDesktop());
    m_configDialog->show();
    m_configDialog->raise();
    if (page > -1)
    {
        m_configDialog->showPage(page);
    }
}

void Kicker::showTaskBarConfig()
{
    showConfig(QString(), 4);
}

void Kicker::configureMenubar()
{
    ExtensionManager::the()->configureMenubar(false);
}

void Kicker::configDialogFinished()
{
    m_configDialog->delayedDestruct();
    m_configDialog = 0;
}

void Kicker::slotDesktopResized()
{
    configure(); // reposition on the desktop
}

void Kicker::clearQuickStartMenu()
{
    MenuManager::the()->kmenu()->clearRecentMenuItems();
}

KWinModule* Kicker::kwinModule()
{
    if (!m_kwinModule)
    {
        m_kwinModule = new KWinModule();
    }

    return m_kwinModule;
}

QRect Kicker::desktopIconsArea(int screen) const
{
    return ExtensionManager::the()->desktopIconsArea(screen);
}

void Kicker::slotDesktopIconsAreaChanged(const QRect &area, int screen)
{
    QByteArray params;
    QDataStream stream(params, IO_WriteOnly);
    stream << area;
    stream << screen;
    emitDCOPSignal("desktopIconsAreaChanged(QRect, int)", params);
}
