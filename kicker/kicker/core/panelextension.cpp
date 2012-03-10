/*****************************************************************

Copyright (c) 2000 Matthias Elter
              2004 Aaron J. Seigo <aseigo@kde.org>

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

#include <qframe.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qstyle.h>

#include <kdebug.h>
#include <khelpmenu.h>
#include <klocale.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kstdguiitem.h>

#include "container_applet.h"
#include "container_extension.h"
#include "containerarea.h"
#include "extensionmanager.h"
#include "kicker.h"
#include "removecontainer_mnu.h"
#include "removeextension_mnu.h"

#include "addapplet_mnu.h"
#include "addbutton_mnu.h"
#include "addextension_mnu.h"

#include "panelextension.h"
#include "panelextension.moc"

// KDE4: make these say Panel_(somenumber)
PanelExtension::PanelExtension(const QString& configFile, QWidget *parent, const char *name)
    : DCOPObject(QCString("ChildPanel_") + QString::number((ulong)this).latin1()),
      KPanelExtension(configFile, KPanelExtension::Stretch, 0, parent, name),
      _opMnu(0),
      m_panelAddMenu(0),
      m_removeMnu(0),
      m_addExtensionMenu(0),
      m_removeExtensionMenu(0),
      _configFile(configFile),
      m_opMenuBuilt( false )
{
    setAcceptDrops(!Kicker::the()->isImmutable());
    setCustomMenu(opMenu());

    QVBoxLayout* _layout = new QVBoxLayout(this);

    // container area
    _containerArea = new ContainerArea( config(), this, opMenu() );
    connect(_containerArea, SIGNAL(maintainFocus(bool)), this, SIGNAL(maintainFocus(bool)));
    _layout->addWidget(_containerArea);

    _containerArea->setFrameStyle(QFrame::NoFrame);
    _containerArea->viewport()->installEventFilter(this);
    _containerArea->configure();

    // Make sure the containerarea has the right orientation from the
    // beginning.
    positionChange(position());

    connect(Kicker::the(), SIGNAL(configurationChanged()),
            SLOT(configurationChanged()));
    connect(Kicker::the(), SIGNAL(immutabilityChanged(bool)),
            SLOT(immutabilityChanged(bool)));

    // we wait to get back to the event loop to start up the container area so that
    // the main panel in ExtensionManager will be assigned and we can tell in a
    // relatively non-hackish way that we are (or aren't) the "main panel"
    QTimer::singleShot(0, this, SLOT(populateContainerArea()));
}

PanelExtension::~PanelExtension()
{
}

void PanelExtension::populateContainerArea()
{
    _containerArea->show();

    if (ExtensionManager::the()->isMainPanel(topLevelWidget()))
    {
        setObjId("Panel");
        _containerArea->initialize(true);
    }
    else
    {
        _containerArea->initialize(false);
    }
}

void PanelExtension::configurationChanged()
{
    _containerArea->configure();
}

void PanelExtension::immutabilityChanged(bool)
{
    m_opMenuBuilt = false;
}

QPopupMenu* PanelExtension::opMenu()
{
    if (_opMnu)
    {
        return _opMnu;
    }

    _opMnu = new QPopupMenu(this);
    connect(_opMnu, SIGNAL(aboutToShow()), this, SLOT(slotBuildOpMenu()));

    return _opMnu;
}

void PanelExtension::positionChange(Position p)
{
    _containerArea->setPosition(p);
}

QSize PanelExtension::sizeHint(Position p, QSize maxSize) const
{
    QSize size;

    if (p == Left || p == Right)
    {
        size = QSize(sizeInPixels(),
                     _containerArea->heightForWidth(sizeInPixels()));
    }
    else
    {
        size = QSize(_containerArea->widthForHeight(sizeInPixels()),
                     sizeInPixels());
    }

    return size.boundedTo( maxSize );
}

bool PanelExtension::eventFilter(QObject*, QEvent * e)
{
    if ( e->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* me = (QMouseEvent*) e;
        if ( me->button() == RightButton && kapp->authorize("action/kicker_rmb"))
        {
            Kicker::the()->setInsertionPoint(me->globalPos());
            opMenu()->exec(me->globalPos());
            Kicker::the()->setInsertionPoint(QPoint());
            return true;
        }
    }
    else
    if ( e->type() == QEvent::Resize )
    {
        emit updateLayout();
    }

    return false;
}

void PanelExtension::setPanelSize(int size)
{
    int custom = customSize();
    if (size > KPanelExtension::SizeCustom)
    {
        custom = size;
        size = KPanelExtension::SizeCustom;
    }

    setSize(static_cast<Size>(size), custom);

    // save the size setting here if it isn't a custom setting
    config()->setGroup("General");
    config()->writeEntry("Size", size);
    config()->sync();
}

void PanelExtension::addKMenuButton()
{
    _containerArea->addKMenuButton();
}

void PanelExtension::addDesktopButton()
{
    _containerArea->addDesktopButton();
}

void PanelExtension::addWindowListButton()
{
    _containerArea->addWindowListButton();
}

void PanelExtension::addURLButton(const QString &url)
{
    _containerArea->addURLButton(url);
}

void PanelExtension::addBrowserButton(const QString &startDir)
{
    _containerArea->addBrowserButton(startDir);
}

void PanelExtension::addServiceButton(const QString& desktopEntry)
{
    _containerArea->addServiceButton(desktopEntry);
}

void PanelExtension::addServiceMenuButton(const QString &,
                                          const QString& relPath)
{
    _containerArea->addServiceMenuButton(relPath);
}

void PanelExtension::addNonKDEAppButton(const QString &filePath,
                                        const QString &icon,
                                        const QString &cmdLine, bool inTerm)
{
    _containerArea->addNonKDEAppButton(filePath, QString::null, filePath, icon,
                                       cmdLine, inTerm);
}

void PanelExtension::addNonKDEAppButton(const QString &title,
                                        const QString &description,
                                        const QString &filePath,
                                        const QString &icon,
                                        const QString &cmdLine, bool inTerm)
{
    _containerArea->addNonKDEAppButton(title, description, filePath, icon,
                                       cmdLine, inTerm);
}

void PanelExtension::addApplet(const QString &desktopFile)
{
    _containerArea->addApplet(AppletInfo(desktopFile, QString::null, AppletInfo::Applet));
}

void PanelExtension::addAppletContainer(const QString &desktopFile)
{
    // KDE4: this appears in the DCOP interface.
    // but it's such a bad name, can this go away?
    addApplet(desktopFile);
}

bool PanelExtension::insertApplet(const QString& desktopFile, int index)
{
    return _containerArea->addApplet(desktopFile, false, index) != 0;
}

bool PanelExtension::insertImmutableApplet(const QString& desktopFile, int index)
{
    return _containerArea->addApplet(desktopFile, true, index) != 0;
}

QStringList PanelExtension::listApplets()
{
    return _containerArea->listContainers();
    BaseContainer::List containers = _containerArea->containers("All");
    QStringList names;

    for (BaseContainer::List::const_iterator it = containers.constBegin();
         it != containers.constEnd();
         ++it)
    {
        names.append((*it)->visibleName());
    }

    return names;
}

bool PanelExtension::removeApplet(int index)
{
    return _containerArea->removeContainer(index);
}

void PanelExtension::restart()
{
    Kicker::the()->restart();
}

void PanelExtension::configure()
{
    Kicker::the()->configure();
}

void PanelExtension::slotBuildOpMenu()
{
    const int REMOVE_EXTENSION_ID = 1000;
    if (m_opMenuBuilt || !_opMnu)
    {
        if (_opMnu)
        {
            bool haveExtensions = ExtensionManager::the()->containers().count() > 0;
            _opMnu->setItemEnabled(REMOVE_EXTENSION_ID, haveExtensions);
        }

        return;
    }

    _opMnu->clear();

    delete m_panelAddMenu;
    m_panelAddMenu = 0;
    delete m_removeMnu;
    m_removeMnu = 0;
    delete m_addExtensionMenu;
    m_addExtensionMenu = 0;
    delete m_removeExtensionMenu;
    m_removeExtensionMenu = 0;

    m_opMenuBuilt = true;
    bool kickerImmutable = Kicker::the()->isImmutable();
    bool isMenuBar = ExtensionManager::the()->isMenuBar(dynamic_cast<QWidget*>(parent()));

    if (!kickerImmutable)
    {
        // setup addmenu and removemenu
        if (_containerArea->canAddContainers())
        {
            _opMnu->insertItem(isMenuBar ? i18n("Add &Applet to Menubar...")
                                         : i18n("Add &Applet to Panel..."),
                               _containerArea, SLOT(showAddAppletDialog()));
            m_panelAddMenu = new PanelAddButtonMenu(_containerArea, this);
            _opMnu->insertItem(isMenuBar ? i18n("Add Appli&cation to Menubar")
                                         : i18n("Add Appli&cation to Panel"),
                               m_panelAddMenu);

            m_removeMnu = new RemoveContainerMenu(_containerArea, this);
            _opMnu->insertItem(isMenuBar ? i18n("&Remove From Menubar")
                                         : i18n("&Remove From Panel"),
                                         m_removeMnu);
            _opMnu->insertSeparator();

            m_addExtensionMenu = new PanelAddExtensionMenu(this);
            _opMnu->insertItem(i18n("Add New &Panel"), m_addExtensionMenu);
            m_removeExtensionMenu = new PanelRemoveExtensionMenu(this);
            _opMnu->insertItem(i18n("Remove Pa&nel"), m_removeExtensionMenu,
                               REMOVE_EXTENSION_ID);
            _opMnu->setItemEnabled(REMOVE_EXTENSION_ID,
                            ExtensionManager::the()->containers().count() > 0);
            _opMnu->insertSeparator();
        }

        _opMnu->insertItem(SmallIconSet("lock"), i18n("&Lock Panels"),
                Kicker::the(), SLOT(toggleLock()));
    }
    else if (!Kicker::the()->isKioskImmutable())
    {
        _opMnu->insertItem(kickerImmutable? SmallIconSet("unlock") :
                                            SmallIconSet("lock"),
                           kickerImmutable ? i18n("Un&lock Panels") :
                                             i18n("&Lock Panels"),
                           Kicker::the(), SLOT(toggleLock()));
    }

    if (!isMenuBar && !Kicker::the()->isKioskImmutable())
    {
        _opMnu->insertItem(SmallIconSet("configure"),
                            i18n("&Configure Panel..."),
                            this, SLOT(showConfig()));
        _opMnu->insertSeparator();
    }
    
    if (kapp->authorize("action/help"))
    {
        KHelpMenu* help = new KHelpMenu( this, KGlobal::instance()->aboutData(), false);
        _opMnu->insertItem(SmallIconSet("help"), KStdGuiItem::help().text(), help->menu());
    }
    _opMnu->adjustSize();
}

void PanelExtension::showConfig()
{
    Kicker::the()->showConfig(_configFile);
}

MenubarExtension::MenubarExtension(const AppletInfo& info)
    : PanelExtension(info.configFile()),
      m_menubar(0)
{
}

MenubarExtension::~MenubarExtension()
{
    if (m_menubar)
    {
        m_menubar->setImmutable(false);
        _containerArea->slotSaveContainerConfig();
    }
}

void MenubarExtension::populateContainerArea()
{
    PanelExtension::populateContainerArea();
    BaseContainer::List containers = _containerArea->containers("All");
    for (BaseContainer::Iterator it = containers.begin();
         it != containers.end();
         ++it)
    {
        if ((*it)->appletType() == "Applet")
        {
            AppletContainer* applet = dynamic_cast<AppletContainer*>(*it);
            if (applet && applet->info().desktopFile() == "menuapplet.desktop")
            {
                m_menubar = applet;
                break;
            }
        }
    }

    if (!m_menubar)
    {
        m_menubar = _containerArea->addApplet(AppletInfo("menuapplet.desktop",
                                                         QString::null,
                                                         AppletInfo::Applet));
    }

    // in the pathological case we may not have a menuapplet at all,
    // so check for it =/
    if (m_menubar)
    {
        m_menubar->setImmutable(true);
    }
}

