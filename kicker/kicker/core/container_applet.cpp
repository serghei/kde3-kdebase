/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#include <qcursor.h>
#include <qxembed.h>
#include <qframe.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qfile.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpanelapplet.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kstandarddirs.h>

#include "applethandle.h"
#include "appletinfo.h"
#include "appletop_mnu.h"
#include "containerarea.h"
#include "global.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "pluginmanager.h"

#include "container_applet.h"
#include "container_applet.moc"

#define APPLET_MARGIN  1

AppletContainer::AppletContainer(const AppletInfo& info,
                                 QPopupMenu* opMenu,
                                 bool immutable,
                                 QWidget* parent )
  : BaseContainer(opMenu,
                  parent,
                  QString(info.library() + "container").latin1()),
    _info(info),
    _handle(0),
    _layout(0),
    _type(KPanelApplet::Normal),
    _widthForHeightHint(0),
    _heightForWidthHint(0),
    _firstuse(true)
{
    setBackgroundOrigin(AncestorOrigin);

    //setup appletframe
    _appletframe = new QHBox(this);
    _appletframe->setBackgroundOrigin( AncestorOrigin );
    _appletframe->setFrameStyle(QFrame::NoFrame);
    _appletframe->installEventFilter(this);

    if (orientation() == Horizontal)
    {
        _layout = new QBoxLayout(this, QBoxLayout::LeftToRight, 0, 0);
    }
    else
    {
        _layout = new QBoxLayout(this, QBoxLayout::TopToBottom, 0, 0);
    }

    _layout->setResizeMode( QLayout::FreeResize );

    _layout->addSpacing(APPLET_MARGIN);
    _handle = new AppletHandle(this);
    _layout->addWidget(_handle, 0);
    connect(_handle, SIGNAL(moveApplet(const QPoint&)),
            this, SLOT(moveApplet(const QPoint&)));
    connect(_handle, SIGNAL(showAppletMenu()), this, SLOT(showAppletMenu()));

    _layout->addWidget(_appletframe, 1);
    _layout->activate();

    _deskFile = info.desktopFile();
    _configFile = info.configFile();
    _applet = PluginManager::the()->loadApplet( info, _appletframe );

    if (!_applet)
    {
        _valid = false;
        KMessageBox::error(this,
                           i18n("The %1 applet could not be loaded. Please check your installation.")
                               .arg(info.name().isEmpty() ? _deskFile : info.name()),
                           i18n("Applet Loading Error"));
        return;
    }

    _valid = true;

    _applet->setPosition((KPanelApplet::Position)KickerLib::directionToPosition(popupDirection()));
    _applet->setAlignment((KPanelApplet::Alignment)alignment());

    _actions = _applet->actions();
    _type = _applet->type();

    setImmutable(immutable);

    connect(_applet, SIGNAL(updateLayout()), SLOT(slotUpdateLayout()));
    connect(_applet, SIGNAL(requestFocus()), SLOT(activateWindow()));
    connect(_applet, SIGNAL(requestFocus(bool)), SLOT(focusRequested(bool)));

    connect(Kicker::the(), SIGNAL(configurationChanged()),
            this, SLOT(slotReconfigure()));
}

void AppletContainer::configure()
{
    _handle->setPopupDirection(popupDirection());
    _handle->setFadeOutHandle(KickerSettings::fadeOutAppletHandles());

    if (isImmutable() ||
        KickerSettings::hideAppletHandles() ||
        !kapp->authorizeKAction("kicker_rmb"))
    {
        if (_handle->isVisibleTo(this))
        {
            _handle->hide();
            setBackground();
        }
    }
    else if (!_handle->isVisibleTo(this))
    {
        _handle->show();
        setBackground();
    }
}

void AppletContainer::slotReconfigure()
{
    configure();
}

void AppletContainer::setPopupDirection(KPanelApplet::Direction d)
{
    if (!_firstuse && _dir == d)
    {
        return;
    }

    _firstuse = false;

    BaseContainer::setPopupDirection(d);
    _handle->setPopupDirection(d);
    resetLayout();

    if (_applet)
    {
        _applet->setPosition((KPanelApplet::Position)KickerLib::directionToPosition(d));
    }
}

void AppletContainer::setOrientation(KPanelExtension::Orientation o)
{
    if (_orient == o) return;

    BaseContainer::setOrientation(o);
    setBackground();
    resetLayout();
}

void AppletContainer::resetLayout()
{
    _handle->resetLayout();

    if (orientation() == Horizontal)
    {
        _layout->setDirection( QBoxLayout::LeftToRight );
    }
    else
    {
        _layout->setDirection( QBoxLayout::TopToBottom );
    }

    _layout->activate();
}

void AppletContainer::moveApplet( const QPoint& moveOffset )
{
    _moveOffset = moveOffset;
    emit moveme(this);
}

void AppletContainer::signalToBeRemoved()
{
    emit removeme(this);
}

void AppletContainer::showAppletMenu()
{
    if (!kapp->authorizeKAction("kicker_rmb"))
    {
        return;
    }

    QPopupMenu *menu = opMenu();

    Kicker::the()->setInsertionPoint(_handle->mapToGlobal(_handle->rect().center()));

    switch(menu->exec(KickerLib::popupPosition(popupDirection(), menu, _handle)))
    {
        case PanelAppletOpMenu::Move:
            moveApplet(_handle->mapToParent(_handle->rect().center()));
            break;
        case PanelAppletOpMenu::Remove:
            Kicker::the()->setInsertionPoint(QPoint());
            emit removeme(this);
            return; // Above signal will cause this to be deleted.
            break;
        case PanelAppletOpMenu::Help:
            help();
            break;
        case PanelAppletOpMenu::About:
            about();
            break;
        case PanelAppletOpMenu::Preferences:
            preferences();
            break;
        case PanelAppletOpMenu::ReportBug:
            reportBug();
            break;
        default:
            break;
    }

    Kicker::the()->setInsertionPoint(QPoint());
    clearOpMenu();
}

void AppletContainer::slotRemoved(KConfig* config)
{
    BaseContainer::slotRemoved(config);

    // we must delete the applet first since it may write out a config file
    // in its dtor which can foil out plans to remove it's config file below
    delete _applet;
    _applet = 0;

    if (_configFile.isEmpty() ||
        _info.isUniqueApplet())
    {
        return;
    }

    QFile::remove(locateLocal("config", _configFile));
}

void AppletContainer::activateWindow()
{
    KWin::forceActiveWindow(topLevelWidget()->winId());
}

void AppletContainer::focusRequested(bool focus)
{
    if (focus)
    {
        KWin::forceActiveWindow(topLevelWidget()->winId());
    }

    emit maintainFocus(focus);
}

void AppletContainer::doLoadConfiguration( KConfigGroup& config )
{
    setWidthForHeightHint(config.readNumEntry("WidthForHeightHint", 0));
    setHeightForWidthHint(config.readNumEntry("HeightForWidthHint", 0));
}

void AppletContainer::doSaveConfiguration( KConfigGroup& config,
                                           bool layoutOnly ) const
{
    // immutability is checked by ContainerBase
    if (orientation() == Horizontal)
    {
        config.writeEntry( "WidthForHeightHint", widthForHeight(height()) );
    }
    else
    {
        config.writeEntry( "HeightForWidthHint", heightForWidth(width()) );
    }

    if (!layoutOnly)
    {
        config.writePathEntry( "ConfigFile", _configFile );
        config.writePathEntry( "DesktopFile", _deskFile );
    }
}

QPopupMenu* AppletContainer::createOpMenu()
{
    QPopupMenu* opMenu = new PanelAppletOpMenu(_actions, appletOpMenu(),
                                               appletsOwnMenu(),
                                               _info.name(), _info.icon(),
                                               this);

    connect(opMenu, SIGNAL(escapePressed()),
            _handle, SLOT(toggleMenuButtonOff()));

    return opMenu;
}

void AppletContainer::slotRemoveApplet()
{
    emit removeme(this);
}

void AppletContainer::slotUpdateLayout()
{
    updateGeometry();
    emit updateLayout();
}


const QPopupMenu* AppletContainer::appletsOwnMenu() const
{
    if (!_applet)
    {
        return 0;
    }

    return _applet->customMenu();
}

void AppletContainer::slotDelayedDestruct()
{
    delete this;
}

void AppletContainer::alignmentChange(KPanelExtension::Alignment a)
{
    if (!_applet)
    {
        return;
    }

    _applet->setAlignment( (KPanelApplet::Alignment)a );
}

int AppletContainer::widthForHeight(int h) const
{
    int handleSize = (_handle->isVisibleTo(const_cast<AppletContainer*>(this))?
                                             _handle->widthForHeight(h) : 0);

    if (!_applet)
    {
        if (_widthForHeightHint > 0)
        {
            return _widthForHeightHint + handleSize;
        }
        else
        {
            return h + handleSize;
        }
    }

    return _applet->widthForHeight(h) + handleSize + APPLET_MARGIN;
}

int AppletContainer::heightForWidth(int w) const
{
    int handleSize = (_handle->isVisibleTo(const_cast<AppletContainer*>(this))?
                                               _handle->heightForWidth(w) : 0);

    if (!_applet)
    {
        if (_heightForWidthHint > 0)
        {
            return _heightForWidthHint + handleSize;
        }
        else
        {
            return w + handleSize;
        }
    }

    return _applet->heightForWidth(w) + handleSize + APPLET_MARGIN;
}

void AppletContainer::about()
{
    if (!_applet) return;
    _applet->action( KPanelApplet::About );
}

void AppletContainer::help()
{
    if (!_applet) return;
    _applet->action( KPanelApplet::Help );
}

void AppletContainer::preferences()
{
    if (!_applet) return;
    _applet->action( KPanelApplet::Preferences );
}

void AppletContainer::reportBug()
{
    if (!_applet) return;
    _applet->action( KPanelApplet::ReportBug );
}

void AppletContainer::setBackground()
{
    // can happen in perverse moments when an applet isn't loaded but the contanier
    // get's asked to update it's bground anyways
    if (!_applet)
    {
        return;
    }

    _applet->unsetPalette();
    _handle->unsetPalette();

    setBackgroundOrigin(AncestorOrigin);
    _applet->update();
    _handle->update();
    
    if (KickerSettings::transparent())
    {
        // Trick to tell applets that they must refresh their transparent background if they need.
        QMoveEvent e(_applet->pos(), _applet->pos());
        QApplication::sendEvent(_applet, &e);
    }
}

void AppletContainer::setImmutable(bool immutable)
{
    // The menu applet must be kept immutable
    if (_deskFile == "menuapplet.desktop" && !immutable)
        return;

    BaseContainer::setImmutable(immutable);
    if (isImmutable() ||
        KickerSettings::hideAppletHandles() ||
        !kapp->authorizeKAction("kicker_rmb"))
    {
        if (_handle->isVisibleTo(this))
        {
            _handle->hide();
            setBackground();
        }
    }
    else if (!_handle->isVisibleTo(this))
    {
        QToolTip::add(_handle, _info.name());
        _handle->show();
        setBackground();
    }
}
