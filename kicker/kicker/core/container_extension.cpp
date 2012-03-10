/*****************************************************************

Copyright (c) 2004-2005 Aaron J. Seigo <aseigo@kde.org>
Copyright (c) 2000-2001 the kicker authors. See file AUTHORS.

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
#include <math.h>

#include <qcursor.h>
#include <qfile.h>
#include <qlayout.h>
#include <qmovie.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qxembed.h>
#include <qcolor.h>

#include <dcopclient.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kglobal.h>
#include <kicker.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <netwm.h>
#include <fixx11h.h>
#include <kwinmodule.h>

#include "container_base.h"
#include "extensionmanager.h"
#include "extensionop_mnu.h"
#include "hidebutton.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "kickertip.h"
#include "pluginmanager.h"
#include "userrectsel.h"

#include "container_extension.h"

/* 1 is the initial speed, hide_show_animation is the top speed. */
#define PANEL_SPEED(x, c) (int)((1.0-2.0*fabs((x)-(c)/2.0)/c)*m_settings.hideAnimationSpeed()+1.0)

ExtensionContainer::ExtensionContainer(const AppletInfo& info,
                                       const QString& extensionId,
                                       QWidget *parent)
  : QFrame(parent, ("ExtensionContainer#" + extensionId).latin1(), WStyle_Customize | WStyle_NoBorder),
    m_settings(KSharedConfig::openConfig(info.configFile())),
    m_hideMode(ManualHide),
    m_unhideTriggeredAt(UnhideTrigger::None),
    _autoHidden(false),
    _userHidden(Unhidden),
    _block_user_input(false),
    _is_lmb_down(false),
    _in_autohide(false),
    _id(extensionId),
    _opMnu(0),
    _info(info),
    _ltHB(0),
    _rbHB(0),
    m_extension(0),
    m_maintainFocus(0),
    m_panelOrder(ExtensionManager::the()->nextPanelOrder())
{
    // now actually try to load the extension
    m_extension = PluginManager::the()->loadExtension(info, this);
    init();
}

ExtensionContainer::ExtensionContainer(KPanelExtension* extension,
                                       const AppletInfo& info,
                                       const QString& extensionId,
                                       QWidget *parent)
  : QFrame(parent, ("ExtensionContainer#" + extensionId).latin1(), WStyle_Customize | WStyle_NoBorder),
    m_settings(KSharedConfig::openConfig(info.configFile())),
    _autoHidden(false),
    _userHidden(Unhidden),
    _block_user_input(false),
    _is_lmb_down(false),
    _in_autohide(false),
    _id(extensionId),
    _opMnu(0),
    _info(info),
    _ltHB(0),
    _rbHB(0),
    m_extension(extension),
    m_maintainFocus(0),
    m_panelOrder(ExtensionManager::the()->nextPanelOrder())
{
    m_extension->reparent(this, QPoint(0, 0));
    init();
}

void ExtensionContainer::init()
{
    // panels live in the dock
    KWin::setType(winId(), NET::Dock);
    KWin::setState(winId(), NET::Sticky);
    KWin::setOnAllDesktops(winId(), true);

    connect(Kicker::the()->kwinModule(), SIGNAL(strutChanged()), this, SLOT(strutChanged()));
    connect(Kicker::the()->kwinModule(), SIGNAL(currentDesktopChanged(int)),
            this, SLOT( currentDesktopChanged(int)));

    setFrameStyle(NoFrame);
    setLineWidth(0);
    setMargin(0);

    connect(UnhideTrigger::the(), SIGNAL(triggerUnhide(UnhideTrigger::Trigger,int)),
            this, SLOT(unhideTriggered(UnhideTrigger::Trigger,int)));

    _popupWidgetFilter = new PopupWidgetFilter( this );
    connect(_popupWidgetFilter, SIGNAL(popupWidgetHiding()), SLOT(maybeStartAutoHideTimer()));

    // layout
    _layout = new QGridLayout(this, 3, 3, 0, 0);
    _layout->setResizeMode(QLayout::FreeResize);
    _layout->setRowStretch(1,10);
    _layout->setColStretch(1,10);

    // instantiate the autohide timer
    _autohideTimer = new QTimer(this, "_autohideTimer");
    connect(_autohideTimer, SIGNAL(timeout()), SLOT(autoHideTimeout()));

    // instantiate the updateLayout event compressor timer
    _updateLayoutTimer = new QTimer(this, "_updateLayoutTimer");
    connect(_updateLayoutTimer, SIGNAL(timeout()), SLOT(actuallyUpdateLayout()));

    installEventFilter(this); // for mouse event handling

    connect(Kicker::the(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(updateHighlightColor()));
    updateHighlightColor();

    // if we were hidden when kicker quit, let's start out hidden as well!
    KConfig *config = KGlobal::config();
    config->setGroup(extensionId());
    int tmp = config->readNumEntry("UserHidden", Unhidden);
    if (tmp > Unhidden && tmp <= RightBottom)
    {
        _userHidden = static_cast<UserHidden>(tmp);
    }

    if (m_extension)
    {
        // if we have an extension, we need to grab the extension-specific
        // defaults for position, size and custom size and override the
        // defaults in the settings object since the extension may differ
        // from the "normal" panels. for example, the universal sidebar's
        // preferred position is the left, not the bottom/top
        KConfigSkeleton::ItemInt* item = dynamic_cast<KConfigSkeleton::ItemInt*>(m_settings.findItem("Position"));
        if (item)
        {
            KPanelExtension::Position p = m_extension->preferedPosition();
            item->setDefaultValue(p);
            item->readConfig(m_settings.config());
        }

        item = dynamic_cast<KConfigSkeleton::ItemInt*>(m_settings.findItem("Size"));
        if (item)
        {
            item->setDefaultValue(m_extension->sizeSetting());
        }

        item = dynamic_cast<KConfigSkeleton::ItemInt*>(m_settings.findItem("CustomSize"));
        if (item)
        {
            item->setDefaultValue(m_extension->customSize());
        }

        connect(m_extension, SIGNAL(updateLayout()), SLOT(updateLayout()));
        connect(m_extension, SIGNAL(maintainFocus(bool)),
                SLOT(maintainFocus(bool)));

        _layout->addWidget(m_extension, 1, 1);
    }

    if (!m_settings.iExist())
    {
        m_settings.setIExist(true);
        m_settings.writeConfig();
    }
}

ExtensionContainer::~ExtensionContainer()
{
}

QSize ExtensionContainer::sizeHint(KPanelExtension::Position p, const QSize &maxSize) const
{
    int width = 0;
    int height = 0;
    if (p == KPanelExtension::Top || p == KPanelExtension::Bottom)
    {
        if (needsBorder())
        {
            height += 1; // border
        }

        if (m_settings.showLeftHideButton())
        {
            width += m_settings.hideButtonSize();
        }

        if (m_settings.showRightHideButton())
        {
            width += m_settings.hideButtonSize();
        }

        // don't forget we might have a border!
        width += _layout->colSpacing(0) + _layout->colSpacing(2);
    }
    else
    {
        if (needsBorder())
        {
            width += 1; // border
        }

        if (m_settings.showLeftHideButton())
        {
            height += m_settings.hideButtonSize();
        }

        if (m_settings.showRightHideButton())
        {
            height += m_settings.hideButtonSize();
        }

        // don't forget we might have a border!
        height += _layout->rowSpacing(0) + _layout->rowSpacing(2);
    }

    QSize size(width, height);
    size = size.boundedTo(maxSize);

    if (m_extension)
    {
        size = m_extension->sizeHint(p, maxSize - size) + size;
    }

    return size.boundedTo(maxSize);
}

static bool isnetwm12_below()
{
  NETRootInfo info( qt_xdisplay(), NET::Supported );
  return info.supportedProperties()[ NETRootInfo::STATES ] & NET::KeepBelow;
}

void ExtensionContainer::readConfig()
{
//    kdDebug(1210) << "ExtensionContainer::readConfig()" << endl;
    m_settings.readConfig();

    if (m_settings.autoHidePanel())
    {
        m_hideMode = AutomaticHide;
    }
    else if (m_settings.backgroundHide())
    {
        m_hideMode = BackgroundHide;
    }
    else
    {
        m_hideMode = ManualHide;
    }

    positionChange(position());
    alignmentChange(alignment());
    setSize(static_cast<KPanelExtension::Size>(m_settings.size()),
            m_settings.customSize());

    if (m_hideMode != AutomaticHide)
    {
        autoHide(false);
    }

    static bool netwm12 = isnetwm12_below();
    if (netwm12) // new netwm1.2 compliant way
    {
        if (m_hideMode == BackgroundHide)
        {
            KWin::setState( winId(), NET::KeepBelow );
            UnhideTrigger::the()->setEnabled( true );
        }
        else
        {
            KWin::clearState( winId(), NET::KeepBelow );
        }
    }
    else if (m_hideMode == BackgroundHide)
    {
        // old way
        KWin::clearState( winId(), NET::StaysOnTop );
        UnhideTrigger::the()->setEnabled( true );
    }
    else
    {
        // the other old way
        KWin::setState( winId(), NET::StaysOnTop );
    }

    actuallyUpdateLayout();
    maybeStartAutoHideTimer();
}

void ExtensionContainer::writeConfig()
{
//    kdDebug(1210) << "ExtensionContainer::writeConfig()" << endl;
    KConfig *config = KGlobal::config();
    config->setGroup(extensionId());

    config->writePathEntry("ConfigFile", _info.configFile());
    config->writePathEntry("DesktopFile", _info.desktopFile());
    config->writeEntry("UserHidden", userHidden());

    m_settings.writeConfig();
}

void ExtensionContainer::showPanelMenu( const QPoint& globalPos )
{
    if (!kapp->authorizeKAction("kicker_rmb"))
    {
        return;
    }

    if (m_extension && m_extension->customMenu())
    {
        // use the extenion's own custom menu
        Kicker::the()->setInsertionPoint(globalPos);
        m_extension->customMenu()->exec(globalPos);
        Kicker::the()->setInsertionPoint(QPoint());
        return;
    }

    if (!_opMnu)
    {
        KDesktopFile f(KGlobal::dirs()->findResource("extensions", _info.desktopFile()));
        _opMnu = new PanelExtensionOpMenu(f.readName(),
                                          m_extension ? m_extension->actions() : 0,
                                          this);
    }

    QPopupMenu *menu = KickerLib::reduceMenu(_opMnu);

    Kicker::the()->setInsertionPoint(globalPos);

    switch (menu->exec(globalPos))
    {
        case PanelExtensionOpMenu::Remove:
            emit removeme(this);
            break;
        case PanelExtensionOpMenu::About:
            about();
            break;
        case PanelExtensionOpMenu::Help:
            help();
            break;
        case PanelExtensionOpMenu::Preferences:
            preferences();
            break;
        case PanelExtensionOpMenu::ReportBug:
            reportBug();
            break;
        default:
            break;
    }
    Kicker::the()->setInsertionPoint(QPoint());
}

void ExtensionContainer::about()
{
    if (!m_extension)
    {
        return;
    }

    m_extension->action(KPanelExtension::About);
}

void ExtensionContainer::help()
{
    if (!m_extension)
    {
        return;
    }

    m_extension->action(KPanelExtension::Help);
}

void ExtensionContainer::preferences()
{
    if (!m_extension)
    {
        return;
    }

    m_extension->action(KPanelExtension::Preferences);
}

void ExtensionContainer::reportBug()
{
    if (!m_extension)
    {
        return;
    }

    m_extension->action(KPanelExtension::ReportBug);
}

void ExtensionContainer::removeSessionConfigFile()
{
    if (_info.configFile().isEmpty() || _info.isUniqueApplet())
    {
        return;
    }

    if (QFile::exists(locate("config", _info.configFile())))
    {
        QFile::remove(locate("config", _info.configFile()));
    }
}

void ExtensionContainer::moveMe()
{
    int screen = xineramaScreen();
    if (screen < 0)
    {
        screen = kapp->desktop()->screenNumber(this);
    }

    if (screen < 0)
    {
        // we aren't on any screen? um. ok.
        return;
    }

    stopAutoHideTimer();

    QApplication::syncX();
    UserRectSel::RectList rects;

    KPanelExtension::Position  positions[]  = { KPanelExtension::Left,
                                                KPanelExtension::Right,
                                                KPanelExtension::Top,
                                                KPanelExtension::Bottom };
    KPanelExtension::Alignment alignments[] = { KPanelExtension::LeftTop,
                                                KPanelExtension::Center,
                                                KPanelExtension::RightBottom };

    for (int s = 0; s < QApplication::desktop()->numScreens(); s++)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                // FIXME:
                // asking for initial geometry here passes bogus heightForWidth
                // and widthForHeight requests to applets and buttons. if they
                // need to make layout adjustments or need to calculate based
                // on other parameters this can lead to Bad Things(tm)
                //
                // we need to find a way to do this that doesn't result in
                // sizeHint's getting called on the extension =/
                //
                // or else we need to change the semantics for applets so that
                // they don't get their "you're changing position" signals through
                // heightForWidth/widthForHeight
                rects.append(UserRectSel::PanelStrut(initialGeometry(positions[i],
                                                                     alignments[j], s),
                                                     s, positions[i], alignments[j]));
            }
        }
    }

    UserRectSel::PanelStrut newStrut = UserRectSel::select(rects, rect().center(), m_highlightColor);
    arrange(newStrut.m_pos, newStrut.m_alignment, newStrut.m_screen);

    _is_lmb_down = false;

    // sometimes the HB's are not reset correctly
    if (_ltHB)
    {
        _ltHB->setDown(false);
    }

    if (_rbHB)
    {
        _rbHB->setDown(false);
    }

    maybeStartAutoHideTimer();
}

void ExtensionContainer::updateLayout()
{
    /*
       m_extension == 0 can happen for example if the constructor of a panel
       extension calls adjustSize(), resulting in a sendPostedEvents on the parent (us) and
       therefore this call. Happens with ksim for example. One can argue about ksim here, but
       kicker shouldn't crash in any case.
     */
    if (!m_extension || _updateLayoutTimer->isActive())
    {
        return;
    }

    // don't update our layout more than once every half a second...
    if (_in_autohide)
    {
        // ... unless we are autohiding
        _updateLayoutTimer->start(0,true);
    }
    else
    {
        _updateLayoutTimer->start(500,true);
    }
}

void ExtensionContainer::actuallyUpdateLayout()
{
//    kdDebug(1210) << "PanelContainer::updateLayout()" << endl;
    resetLayout();
    updateWindowManager();
}

void ExtensionContainer::enableMouseOverEffects()
{
    KickerTip::enableTipping(true);
    QPoint globalPos = QCursor::pos();
    QPoint localPos = mapFromGlobal(globalPos);
    QWidget* child = childAt(localPos);

    if (child)
    {
        QMouseEvent* e = new QMouseEvent(QEvent::Enter, localPos, globalPos, 0, 0);
        qApp->sendEvent(child, e);
    }
}

bool ExtensionContainer::shouldUnhideForTrigger(UnhideTrigger::Trigger t) const
{
    int loc = m_settings.unhideLocation();

    if (loc == t)
    {
        return true;
    }

    if (loc == UnhideTrigger::Bottom)
    {
        return t == UnhideTrigger::BottomLeft ||
               t == UnhideTrigger::BottomRight;
    }
    else if (loc == UnhideTrigger::Top)
    {
        return t == UnhideTrigger::TopLeft ||
               t == UnhideTrigger::TopRight;
    }
    else if (loc == UnhideTrigger::Left)
    {
        return t == UnhideTrigger::TopLeft ||
               t == UnhideTrigger::BottomLeft;
    }
    else if (loc == UnhideTrigger::Right)
    {
        return t == UnhideTrigger::TopRight ||
               t == UnhideTrigger::BottomRight;
    }

    return false;
}

void ExtensionContainer::unhideTriggered(UnhideTrigger::Trigger tr, int XineramaScreen)
{
    if (m_hideMode == ManualHide)
    {
        return;
    }
    else if (tr == UnhideTrigger::None)
    {
        if (m_settings.unhideLocation() != UnhideTrigger::None && _autoHidden)
        {
            UnhideTrigger::the()->setEnabled(false);
        }

        m_unhideTriggeredAt = UnhideTrigger::None;
        return;
    }

    if (xineramaScreen() != XineramaAllScreens &&
        XineramaScreen != xineramaScreen())
    {
        if (m_settings.unhideLocation() != UnhideTrigger::None)
        {
            m_unhideTriggeredAt = tr;
        }
        return;
    }

    // here we handle the case where the user has defined WHERE
    // the pannel can be popped up from.
    if (m_settings.unhideLocation() != UnhideTrigger::None)
    {
        if (_autoHidden)
        {
            UnhideTrigger::the()->setEnabled(true);
        }

        m_unhideTriggeredAt = tr;
        if (shouldUnhideForTrigger(tr))
        {
            UnhideTrigger::the()->triggerAccepted(tr, XineramaScreen);

            if (m_hideMode == BackgroundHide)
            {
                KWin::raiseWindow(winId());
            }
            else if (_autoHidden)
            {
                autoHide(false);
                maybeStartAutoHideTimer();
            }
        }

        return;
    }

    m_unhideTriggeredAt = UnhideTrigger::None;

    // Otherwise hide mode is automatic. The code below is slightly
    // complex so as to keep the same behavior as it has always had:
    // only unhide when the cursor position is within the widget geometry.
    // We can't just do geometry().contains(QCursor::pos()) because
    // now we hide the panel completely off screen.

    int x = QCursor::pos().x();
    int y = QCursor::pos().y();
    int t = geometry().top();
    int b = geometry().bottom();
    int r = geometry().right();
    int l = geometry().left();
    if (((tr == UnhideTrigger::Top ||
          tr == UnhideTrigger::TopLeft ||
          tr == UnhideTrigger::TopRight) &&
         position() == KPanelExtension::Top && x >= l && x <= r) ||
        ((tr == UnhideTrigger::Left ||
          tr == UnhideTrigger::TopLeft ||
          tr == UnhideTrigger::BottomLeft) &&
         position() == KPanelExtension::Left && y >= t && y <= b) ||
        ((tr == UnhideTrigger::Bottom ||
          tr == UnhideTrigger::BottomLeft ||
          tr == UnhideTrigger::BottomRight) &&
         position() == KPanelExtension::Bottom && x >= l && x <= r ) ||
        ((tr == UnhideTrigger::Right ||
          tr == UnhideTrigger::TopRight ||
          tr == UnhideTrigger::BottomRight) &&
         position() == KPanelExtension::Right && y >= t && y <= b ))
    {
        UnhideTrigger::the()->triggerAccepted(tr, XineramaScreen);

        if (_autoHidden)
        {
            autoHide(false);
            maybeStartAutoHideTimer();
        }
        else if (m_hideMode == BackgroundHide)
        {
            KWin::raiseWindow(winId());
        }
    }
}

void ExtensionContainer::autoHideTimeout()
{
//    kdDebug(1210) << "PanelContainer::autoHideTimeout() " << name() << endl;
    // Hack: If there is a popup open, don't autohide until it closes.
    QWidget* popup = QApplication::activePopupWidget();
    if (popup)
    {

    //    kdDebug(1210) << "popup detected" << endl;

        // Remove it first in case it was already installed.
        // Does nothing if it wasn't installed.
        popup->removeEventFilter( _popupWidgetFilter );

        // We will get a signal from the filter after the
        // popup is hidden. At that point, maybeStartAutoHideTimer()
        // will get called again.
        popup->installEventFilter( _popupWidgetFilter );

        // Stop the timer.
        stopAutoHideTimer();
        return;
    }

    if (m_hideMode != AutomaticHide ||
        _autoHidden ||
        _userHidden ||
        m_maintainFocus > 0)
    {
        return;
    }

    QRect r = geometry();
    QPoint p = QCursor::pos();
    if (!r.contains(p) &&
        (m_settings.unhideLocation() == UnhideTrigger::None ||
         !shouldUnhideForTrigger(m_unhideTriggeredAt)))
    {
        stopAutoHideTimer();
        autoHide(true);
        UnhideTrigger::the()->resetTriggerThrottle();
    }
}

void ExtensionContainer::hideLeft()
{
    animatedHide(true);
}

void ExtensionContainer::hideRight()
{
    animatedHide(false);
}

void ExtensionContainer::autoHide(bool hide)
{
//   kdDebug(1210) << "PanelContainer::autoHide( " << hide << " )" << endl;

    if (_in_autohide || hide == _autoHidden)
    {
        return;
    }

    //    kdDebug(1210) << "entering autohide for real" << endl;

    blockUserInput(true);

    QPoint oldpos = pos();
    QRect newextent = initialGeometry( position(), alignment(), xineramaScreen(), hide, Unhidden );
    QPoint newpos = newextent.topLeft();

    if (hide)
    {
        /* bail out if we are unable to hide */

        for (int s=0; s <  QApplication::desktop()->numScreens(); s++)
        {
            /* don't let it intersect with any screen in the hidden position
             * that it doesn't intesect in the shown position. Should prevent
             * panels from hiding by sliding onto other screens, while still
             * letting them show reveal buttons onscreen */
            QRect desktopGeom = QApplication::desktop()->screenGeometry(s);
            if (desktopGeom.intersects(newextent) &&
                !desktopGeom.intersects(geometry()))
            {
                blockUserInput( false );
                return;
            }
        }
    }

    _in_autohide = true;
    _autoHidden = hide;
    UnhideTrigger::the()->setEnabled(_autoHidden);
    KickerTip::enableTipping(false);

    if (hide)
    {
        // So we don't cover other panels
        lower();
    }
    else
    {
        // So we aren't covered by other panels
        raise();
    }

    if (m_settings.hideAnimation())
     {
        if (position() == KPanelExtension::Left || position() == KPanelExtension::Right)
        {
            for (int i = 0; i < abs(newpos.x() - oldpos.x());
                 i += PANEL_SPEED(i,abs(newpos.x() - oldpos.x())))
            {
                if (newpos.x() > oldpos.x())
                {
                    move(oldpos.x() + i, newpos.y());
                }
                else
                {
                    move(oldpos.x() - i, newpos.y());
                }

                qApp->syncX();
                qApp->processEvents();
            }
        }
        else
        {
            for (int i = 0; i < abs(newpos.y() - oldpos.y());
                    i += PANEL_SPEED(i,abs(newpos.y() - oldpos.y())))
            {
                if (newpos.y() > oldpos.y())
                {
                    move(newpos.x(), oldpos.y() + i);
                }
                else
                {
                    move(newpos.x(), oldpos.y() - i);
                }

                qApp->syncX();
                qApp->processEvents();
            }
        }
    }

    blockUserInput(false);

    updateLayout();

    // Sometimes tooltips don't get hidden
    QToolTip::hide();

    _in_autohide = false;

    QTimer::singleShot(100, this, SLOT(enableMouseOverEffects()));
}

void ExtensionContainer::animatedHide(bool left)
{
//    kdDebug(1210) << "PanelContainer::animatedHide()" << endl;
    KickerTip::enableTipping(false);
    blockUserInput(true);

    UserHidden newState;
    if (_userHidden != Unhidden)
    {
        newState = Unhidden;
    }
    else if (left)
    {
        newState = LeftTop;
    }
    else
    {
        newState = RightBottom;
    }

    QPoint oldpos = pos();
    QRect newextent = initialGeometry(position(), alignment(), xineramaScreen(), false, newState);
    QPoint newpos(newextent.topLeft());

    if (newState != Unhidden)
    {
        /* bail out if we are unable to hide */
        for(int s=0; s <  QApplication::desktop()->numScreens(); s++)
        {
            /* don't let it intersect with any screen in the hidden position
             * that it doesn't intesect in the shown position. Should prevent
             * panels from hiding by sliding onto other screens, while still
            * letting them show reveal buttons onscreen */
            if (QApplication::desktop()->screenGeometry(s).intersects(newextent) &&
                !QApplication::desktop()->screenGeometry(s).intersects(geometry()))
            {
                blockUserInput(false);
                QTimer::singleShot(100, this, SLOT(enableMouseOverEffects()));
                return;
            }
        }

        _userHidden = newState;

        // So we don't cover the mac-style menubar
        lower();
    }

    if (m_settings.hideAnimation())
    {
        if (position() == KPanelExtension::Left || position() == KPanelExtension::Right)
        {
            for (int i = 0; i < abs(newpos.y() - oldpos.y());
                 i += PANEL_SPEED(i, abs(newpos.y() - oldpos.y())))
            {
                if (newpos.y() > oldpos.y())
                {
                    move(newpos.x(), oldpos.y() + i);
                }
                else
                {
                    move(newpos.x(), oldpos.y() - i);
                }
                qApp->syncX();
                qApp->processEvents();
            }
        }
        else
        {
            for (int i = 0; i < abs(newpos.x() - oldpos.x());
                 i += PANEL_SPEED(i, abs(newpos.x() - oldpos.x())))
            {
                if (newpos.x() > oldpos.x())
                {
                    move(oldpos.x() + i, newpos.y());
                }
                else
                {
                    move(oldpos.x() - i, newpos.y());
                }
                qApp->syncX();
                qApp->processEvents();
            }
        }
    }

    blockUserInput( false );

    _userHidden = newState;

    actuallyUpdateLayout();
    qApp->syncX();
    qApp->processEvents();

    // save our hidden status so that when kicker starts up again
    // we'll come back in the same state
    KConfig *config = KGlobal::config();
    config->setGroup(extensionId());
    config->writeEntry("UserHidden", userHidden());

    QTimer::singleShot(100, this, SLOT(enableMouseOverEffects()));
}

bool ExtensionContainer::reserveStrut() const
{
    return !m_extension || m_extension->reserveStrut();
}

KPanelExtension::Alignment ExtensionContainer::alignment() const
{
    // KConfigXT really needs to get support for vars that are enums that
    // are defined in other classes
    return static_cast<KPanelExtension::Alignment>(m_settings.alignment());
}

void ExtensionContainer::updateWindowManager()
{
    NETExtendedStrut strut;

    if (reserveStrut())
    {
        //    kdDebug(1210) << "PanelContainer::updateWindowManager()" << endl;
        // Set the relevant properties on the window.
        int w = 0;
        int h = 0;

        QRect geom = initialGeometry(position(), alignment(), xineramaScreen());
        QRect virtRect(QApplication::desktop()->geometry());
        QRect screenRect(QApplication::desktop()->screenGeometry(xineramaScreen()));

        if (m_hideMode == ManualHide && !userHidden())
        {
            w = width();
            h = height();
        }

        switch (position())
        {
            case KPanelExtension::Top:
                strut.top_width = geom.y() + h;
                strut.top_start = x();
                strut.top_end = x() + width() - 1;
                break;

            case KPanelExtension::Bottom:
                // also claim the non-visible part at the bottom
                strut.bottom_width = (virtRect.bottom() - geom.bottom()) + h;
                strut.bottom_start = x();
                strut.bottom_end = x() + width() - 1;
                break;

            case KPanelExtension::Right:
                strut.right_width = (virtRect.right() - geom.right()) + w;
                strut.right_start = y();
                strut.right_end = y() + height() - 1;
                break;

            case KPanelExtension::Left:
                strut.left_width = geom.x() + w;
                strut.left_start = y();
                strut.left_end = y() + height() - 1;
                break;

            case KPanelExtension::Floating:
                // should never be reached, anyways
                break;
        }
    }

    if (strut.left_width != _strut.left_width ||
        strut.left_start != _strut.left_start ||
        strut.left_end != _strut.left_end ||
        strut.right_width != _strut.right_width ||
        strut.right_start != _strut.right_start ||
        strut.right_end != _strut.right_end ||
        strut.top_width != _strut.top_width ||
        strut.top_start != _strut.top_start ||
        strut.top_end != _strut.top_end ||
        strut.bottom_width != _strut.bottom_width ||
        strut.bottom_start != _strut.bottom_start ||
        strut.bottom_end != _strut.bottom_end)
    {
        /*kdDebug(1210) << " === Panel sets new strut for pos " << position() << " ===" << endl;

       kdDebug(1210) << "strut for " << winId() << ": " << endl <<
            "\tleft  : " << strut.left_width << " " << strut.left_start << " " << strut.left_end << endl <<
            "\tright : " << strut.right_width << " " << strut.right_start << " " << strut.right_end << endl <<
            "\ttop   : " << strut.top_width << " " << strut.top_start << " " << strut.top_end << endl <<
            "\tbottom: " << strut.bottom_width << " " << strut.bottom_start << " " << strut.bottom_end << endl; */
        _strut = strut;

        KWin::setExtendedStrut(winId(),
            strut.left_width, strut.left_start, strut.left_end,
            strut.right_width, strut.right_start, strut.right_end,
            strut.top_width, strut.top_start, strut.top_end,
            strut.bottom_width, strut.bottom_start, strut.bottom_end);
        KWin::setStrut(winId(), strut.left_width, strut.right_width, strut.top_width, strut.bottom_width);
    }
    /*else
    {
        kdDebug(1210) << "Panel strut did NOT change!" << endl;
    }*/
}

void ExtensionContainer::currentDesktopChanged(int)
{
    //    kdDebug(1210) << "PanelContainer::currentDesktopChanged" << endl;
    if (m_settings.autoHideSwitch())
    {
        if (m_hideMode == AutomaticHide)
        {
            autoHide(false);
        }
        else if (m_hideMode == BackgroundHide)
        {
            KWin::raiseWindow(winId());
        }
    }

    // For some reason we don't always get leave events when the user
    // changes desktops and moves the cursor out of the panel at the
    // same time. Maybe always calling this will help.
    maybeStartAutoHideTimer();
}

void ExtensionContainer::strutChanged()
{
    //kdDebug(1210) << "PanelContainer::strutChanged()" << endl;
    QRect ig = currentGeometry();

    if (ig != geometry())
    {
        setGeometry(ig);
        updateLayout();
    }
}

void ExtensionContainer::blockUserInput( bool block )
{
    if (block == _block_user_input)
    {
        return;
    }

    // If we don't want any user input to be possible we should catch mouse
    // events and such. Therefore we install an eventfilter and let the
    // eventfilter discard those events.
    if ( block )
    {
        qApp->installEventFilter( this );
    }
    else
    {
        qApp->removeEventFilter( this );
    }

    _block_user_input = block;
}

void ExtensionContainer::maybeStartAutoHideTimer()
{
    if (m_hideMode != ManualHide &&
        !_autoHidden &&
        !_userHidden)
    {
        // kdDebug(1210) << "starting auto hide timer for " << name() << endl;
        if (m_settings.autoHideDelay() == 0)
        {
            _autohideTimer->start(250);
        }
        else
        {
            _autohideTimer->start(m_settings.autoHideDelay() * 1000);
        }
    }
}

void ExtensionContainer::stopAutoHideTimer()
{
    if (_autohideTimer->isActive())
    {
        //kdDebug(1210) << "stopping auto hide timer for " << name() << endl;
        _autohideTimer->stop();
    }
}

void ExtensionContainer::maintainFocus(bool maintain)
{
    if (maintain)
    {
        ++m_maintainFocus;

        if (_autoHidden)
        {
            autoHide(false);
        }
        else if (_userHidden == LeftTop)
        {
            animatedHide(true);
        }
        else if (_userHidden == RightBottom)
        {
            animatedHide(false);
        }
    }
    else if (m_maintainFocus > 0)
    {
        --m_maintainFocus;
    }
}

int ExtensionContainer::arrangeHideButtons()
{
    bool layoutEnabled = _layout->isEnabled();

    if (layoutEnabled)
    {
        _layout->setEnabled(false);
    }

    if (orientation() == Vertical)
    {
        int maxWidth = width();

        if (needsBorder())
        {
            --maxWidth;
        }

        if (_ltHB)
        {
            _ltHB->setMaximumWidth(maxWidth);
            _ltHB->setMaximumHeight(14);
            _layout->remove(_ltHB);
            _layout->addWidget(_ltHB, 0, 1, Qt::AlignBottom | Qt::AlignLeft);
        }

        if (_rbHB)
        {
            _rbHB->setMaximumWidth(maxWidth);
            _rbHB->setMaximumHeight(14);
            _layout->remove(_rbHB);
            _layout->addWidget(_rbHB, 2, 1);
        }
    }
    else
    {
        int maxHeight = height();

        if (needsBorder())
        {
            --maxHeight;
        }

        int vertAlignment = (position() == KPanelExtension::Top) ? Qt::AlignTop : 0;
        int leftAlignment = Qt::AlignRight;

        if (_ltHB)
        {
            _ltHB->setMaximumHeight(maxHeight);
            _ltHB->setMaximumWidth(14);
            _layout->remove(_ltHB);
            if (kapp->reverseLayout())
            {
                _layout->addWidget(_ltHB, 1, 2, vertAlignment);
            }
            else
            {
                _layout->addWidget(_ltHB, 1, 0, leftAlignment | vertAlignment);
            }
        }

        if (_rbHB)
        {
            _rbHB->setMaximumHeight(maxHeight);
            _rbHB->setMaximumWidth(14);
            _layout->remove(_rbHB);
            if (kapp->reverseLayout())
            {
                _layout->addWidget(_rbHB, 1, 0, leftAlignment | vertAlignment);
            }
            else
            {
                _layout->addWidget(_rbHB, 1, 2, vertAlignment);
            }
        }
    }

    int layoutOffset = setupBorderSpace();
    if (layoutEnabled)
    {
        _layout->setEnabled(true);
    }

    return layoutOffset;
}

int ExtensionContainer::setupBorderSpace()
{
    _layout->setRowSpacing(0, 0);
    _layout->setRowSpacing(2, 0);
    _layout->setColSpacing(0, 0);
    _layout->setColSpacing(2, 0);

    if (!needsBorder())
    {
        return 0;
    }

    int layoutOffset = 0;
    QRect r = QApplication::desktop()->screenGeometry(xineramaScreen());
    QRect h = geometry();

    if (orientation() == Vertical)
    {
        if (h.top() > 0)
        {
            int topHeight = (_ltHB && _ltHB->isVisibleTo(this)) ? _ltHB->height() + 1 : 1;
            _layout->setRowSpacing(0, topHeight);
            ++layoutOffset;
        }

        if (h.bottom() < r.bottom())
        {
            int bottomHeight = (_rbHB && _rbHB->isVisibleTo(this)) ? _rbHB->height() + 1 : 1;
            _layout->setRowSpacing(1, bottomHeight);
            ++layoutOffset;
        }
    }
    else
    {
        if (h.left() > 0)
        {
            int leftWidth = (_ltHB && _ltHB->isVisibleTo(this)) ? _ltHB->width() + 1 : 1;
            _layout->setColSpacing(0, leftWidth);
            ++layoutOffset;
        }

        if (h.right() < r.right())
        {
            int rightWidth = (_rbHB && _rbHB->isVisibleTo(this)) ? _rbHB->width() + 1 : 1;
            _layout->setColSpacing(1, rightWidth);
            ++layoutOffset;
        }
    }

    switch (position())
    {
        case KPanelExtension::Left:
            _layout->setColSpacing(2, 1);
        break;

        case KPanelExtension::Right:
            _layout->setColSpacing(0, 1);
        break;

        case KPanelExtension::Top:
            _layout->setRowSpacing(2, 1);
        break;

        case KPanelExtension::Bottom:
        default:
            _layout->setRowSpacing(0, 1);
        break;
    }

    return layoutOffset;
}

void ExtensionContainer::positionChange(KPanelExtension::Position p)
{
    arrangeHideButtons();

    if (m_extension)
    {
        m_extension->setPosition(p);
    }

    update();
}

void ExtensionContainer::updateHighlightColor()
{
    KConfig *config = KGlobal::config();
    config->setGroup("WM");
    QColor color = QApplication::palette().active().highlight();
    m_highlightColor = config->readColorEntry("activeBackground", &color);
    update();
}

void ExtensionContainer::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    if (needsBorder())
    {
        // draw border
        QPainter p(this);
        if (KickerSettings::useBackgroundTheme() && KickerSettings::colorizeBackground())
            p.setPen(m_highlightColor);
        else
            p.setPen(palette().color(QPalette::Active, QColorGroup::Mid));
        p.drawRect(0, 0, width(), height());
    }
}

void ExtensionContainer::leaveEvent(QEvent*)
{
    maybeStartAutoHideTimer();
}

void ExtensionContainer::alignmentChange(KPanelExtension::Alignment a)
{
    if (!m_extension)
    {
        return;
    }

    m_extension->setAlignment(a);
}

void ExtensionContainer::setSize(KPanelExtension::Size size, int custom)
{
    if (!m_extension)
    {
        return;
    }

    m_settings.setSize(size);
    m_settings.setCustomSize(custom);
    m_extension->setSize(size, custom);
}

KPanelExtension::Size ExtensionContainer::size() const
{
    // KConfigXT really needs to get support for vars that are enums that
    // are defined in other classes
    return static_cast<KPanelExtension::Size>(m_settings.size());
}

ExtensionContainer::HideMode ExtensionContainer::hideMode() const
{
    return m_hideMode;
}

void ExtensionContainer::unhideIfHidden(int showForAtLeastHowManyMS)
{
    if (_autoHidden)
    {
        autoHide(false);
        QTimer::singleShot(showForAtLeastHowManyMS,
                           this, SLOT(maybeStartAutoHideTimer()));
        return;
    }

    if (_userHidden == LeftTop)
    {
        animatedHide(true);
    }
    else if (_userHidden == RightBottom)
    {
        animatedHide(false);
    }
}

void ExtensionContainer::setHideButtons(bool showLeft, bool showRight)
{
    if (m_settings.showLeftHideButton() == showLeft &&
        m_settings.showRightHideButton() == showRight)
    {
        return;
    }

    m_settings.setShowLeftHideButton(showLeft);
    m_settings.setShowRightHideButton(showRight);
    resetLayout();
}

bool ExtensionContainer::event(QEvent* e)
{
    // Update the layout when we receive a LayoutHint. This way we can adjust
    // to changes of the layout of the main widget.
    if (e->type() == QEvent::LayoutHint)
    {
        updateLayout();
    }

    return QFrame::event(e);
}

void ExtensionContainer::closeEvent(QCloseEvent* e)
{
    // Prevent being closed via Alt-F4
    e->ignore();
}

void ExtensionContainer::arrange(KPanelExtension::Position p,
                                 KPanelExtension::Alignment a,
                                 int XineramaScreen)
{
    if (p == m_settings.position() &&
        a == m_settings.alignment() &&
        XineramaScreen == xineramaScreen())
    {
        return;
    }

    bool positionChanged = p != m_settings.position();
    if (positionChanged)
    {
        m_settings.setPosition(p);
    }
    else if (!needsBorder())
    {
        // this ensures that the layout gets rejigged
        // even if position doesn't change
        _layout->setRowSpacing(0, 0);
        _layout->setRowSpacing(2, 0);
        _layout->setColSpacing(0, 0);
        _layout->setColSpacing(2, 0);
    }

    if (a != m_settings.alignment())
    {
        m_settings.setAlignment(a);
        setAlignment(a);
    }

    if (XineramaScreen != xineramaScreen())
    {
        m_settings.setXineramaScreen(XineramaScreen);
        xineramaScreenChange(XineramaScreen);
    }

    actuallyUpdateLayout();
    
    if (positionChanged)
    {
        positionChange(p);
    }
    writeConfig();
}

KPanelExtension::Orientation ExtensionContainer::orientation() const
{
    if (position() == KPanelExtension::Top || position() == KPanelExtension::Bottom)
    {
        return Horizontal;
    }
    else
    {
        return Vertical;
    }
}

KPanelExtension::Position ExtensionContainer::position() const
{
    // KConfigXT really needs to get support for vars that are enums that
    // are defined in other classes
    return static_cast<KPanelExtension::Position>(m_settings.position());
}

void ExtensionContainer::resetLayout()
{
    QRect g = initialGeometry(position(), alignment(), xineramaScreen(),
                              autoHidden(), userHidden());

    // Disable the layout while we rearrange the panel.
    // Necessary because the children may be
    // relayouted with the wrong size.

    _layout->setEnabled(false);

    if (geometry() != g)
    {
        setGeometry(g);
        ExtensionManager::the()->extensionSizeChanged(this);
    }

    // layout
    bool haveToArrangeButtons = false;
    bool showLeftHideButton = m_settings.showLeftHideButton() || userHidden() == RightBottom;
    bool showRightHideButton = m_settings.showRightHideButton() || userHidden() == LeftTop;

    // left/top hide button
    if (showLeftHideButton)
    {
        if (!_ltHB)
        {
            _ltHB = new HideButton(this);
            _ltHB->installEventFilter(this);
            _ltHB->setEnabled(true);
            connect(_ltHB, SIGNAL(clicked()), this, SLOT(hideLeft()));
            haveToArrangeButtons = true;
        }

        if (orientation() == Horizontal)
        {
            _ltHB->setArrowType(Qt::LeftArrow);
            _ltHB->setFixedSize(m_settings.hideButtonSize(), height());
        }
        else
        {
            _ltHB->setArrowType(Qt::UpArrow);
            _ltHB->setFixedSize(width(), m_settings.hideButtonSize());
        }

        _ltHB->show();
    }
    else if (_ltHB)
    {
        _ltHB->hide();
    }

    // right/bottom hide button
    if (showRightHideButton)
    {
        if (!_rbHB)
        {
            // right/bottom hide button
            _rbHB = new HideButton(this);
            _rbHB->installEventFilter(this);
            _rbHB->setEnabled(true);
            connect(_rbHB, SIGNAL(clicked()), this, SLOT(hideRight()));
            haveToArrangeButtons = true;
        }

        if ( orientation() == Horizontal)
        {
            _rbHB->setArrowType(Qt::RightArrow);
            _rbHB->setFixedSize(m_settings.hideButtonSize(), height());
        }
        else
        {
            _rbHB->setArrowType(Qt::DownArrow);
            _rbHB->setFixedSize(width(), m_settings.hideButtonSize());
        }

        _rbHB->show();
    }
    else if (_rbHB)
    {
        _rbHB->hide();
    }

    if (_ltHB)
    {
        QToolTip::remove(_ltHB);
        if (userHidden())
        {
            QToolTip::add(_ltHB, i18n("Show panel"));
        }
        else
        {
            QToolTip::add(_ltHB, i18n("Hide panel"));
        }
    }

    if (_rbHB)
    {
        QToolTip::remove( _rbHB );
        if (userHidden())
        {
            QToolTip::add(_rbHB, i18n("Show panel"));
        }
        else
        {
            QToolTip::add(_rbHB, i18n("Hide panel"));
        }
    }

    updateGeometry();
    int endBorderWidth = haveToArrangeButtons ? arrangeHideButtons() : setupBorderSpace();

    if (orientation() == Horizontal)
    {
        if (m_extension)
        {
            int maxWidth = width() - endBorderWidth;

            if (showLeftHideButton)
            {
                maxWidth -= _ltHB->width();
            }

            if (showRightHideButton)
            {
                maxWidth -= _rbHB->width();
            }

            m_extension->setMaximumWidth(maxWidth);

            if (needsBorder())
            {
                m_extension->setFixedHeight(height() - 1);
            }
            else
            {
                m_extension->setFixedHeight(height());
            }
        }
    }
    else if (m_extension)
    {
        int maxHeight = height() - endBorderWidth;

        if (showLeftHideButton)
        {
            maxHeight -= _ltHB->height();
        }

        if (showRightHideButton)
        {
            maxHeight -= _rbHB->height();
        }

        m_extension->setMaximumHeight(maxHeight);

        if (needsBorder())
        {
            m_extension->setFixedWidth(width() - 1);
        }
        else
        {
            m_extension->setFixedWidth(width());
        }
    }

    _layout->setEnabled(true);
}

bool ExtensionContainer::needsBorder() const
{
    return !KickerSettings::transparent();
           //&& !KickerSettings::useBackgroundTheme();
}

QSize ExtensionContainer::initialSize(KPanelExtension::Position p, QRect workArea) const
{
    /*kdDebug(1210) << "initialSize() Work Area: (" << workArea.topLeft().x() <<
        ", " << workArea.topLeft().y() << ") to (" << workArea.bottomRight().x() <<
        ", " << workArea.bottomRight().y() << ")" << endl;*/

    QSize hint = sizeHint(p, workArea.size()).boundedTo(workArea.size());
    int width = 0;
    int height = 0;

    if (p == KPanelExtension::Left || p == KPanelExtension::Right)
    {
        width = hint.width();
        height = (workArea.height() * m_settings.sizePercentage()) / 100;

        if (m_settings.expandSize())
        {
            height = QMAX(height, hint.height());
        }
    }
    else
    {
        width = (workArea.width() * m_settings.sizePercentage()) / 100;
        height = hint.height();

        if (m_settings.expandSize())
        {
            width = QMAX( width, hint.width() );
        }
    }

    return QSize(width, height);
}

QPoint ExtensionContainer::initialLocation(KPanelExtension::Position p,
                                           KPanelExtension::Alignment a,
                                           int XineramaScreen,
                                           const QSize &s,
                                           QRect workArea,
                                           bool autohidden,
                                           UserHidden userHidden) const
{
    QRect wholeScreen;
    if (XineramaScreen == XineramaAllScreens)
    {
        wholeScreen = QApplication::desktop()->geometry();
    }
    else
    {
        wholeScreen = QApplication::desktop()->screenGeometry(XineramaScreen);
    }

    /*kdDebug(1210) << "initialLocation() Work Area: (" <<
                        workArea.topLeft().x() << ", " <<
                        area.topLeft().y() << ") to (" <<
                        workArea.bottomRight().x() << ", " <<
                        workArea.bottomRight().y() << ")" << endl;*/

    int left;
    int top;

    // If the panel is horizontal
    if (p == KPanelExtension::Top || p == KPanelExtension::Bottom)
    {
        // Get the X coordinate
        switch (a)
        {
            case KPanelExtension::LeftTop:
                left = workArea.left();
            break;

            case KPanelExtension::Center:
            {
                left = wholeScreen.left() + ( wholeScreen.width() - s.width() ) / 2;
                int right = left + s.width();
                if (right > workArea.right())
                {
                    left = left - (right - workArea.right());
                }

                if (left < workArea.left())
                {
                    left = workArea.left();
                }

            break;
            }

            case KPanelExtension::RightBottom:
                left = workArea.right() - s.width() + 1;
            break;

            default:
                left = workArea.left();
            break;
        }

        // Get the Y coordinate
        if (p == KPanelExtension::Top)
        {
            top = workArea.top();
        }
        else
        {
            top = workArea.bottom() - s.height() + 1;
        }
    }
    else // vertical panel
    {
        // Get the Y coordinate
        switch (a)
        {
            case KPanelExtension::LeftTop:
                top = workArea.top();
            break;

            case KPanelExtension::Center:
            {
                top = wholeScreen.top() + ( wholeScreen.height() - s.height() ) / 2;
                int bottom = top + s.height();
                if (bottom > workArea.bottom())
                {
                    top = top - (bottom - workArea.bottom());
                }

                if (top < workArea.top())
                {
                    top = workArea.top();
                }
            break;
            }

            case KPanelExtension::RightBottom:
                top = workArea.bottom() - s.height() + 1;
            break;

            default:
                top = workArea.top();
        }

            // Get the X coordinate
        if (p == KPanelExtension::Left)
        {
            left = workArea.left();
        }
        else
        {
            left = workArea.right() - s.width() + 1;
        }
    }

    // Correct for auto hide
    if (autohidden)
    {
        switch (position())
        {
            case KPanelExtension::Left:
                left -= s.width();
            break;

            case KPanelExtension::Right:
                left += s.width();
            break;

            case KPanelExtension::Top:
                top -= s.height();
            break;

            case KPanelExtension::Bottom:
            default:
                top += s.height();
            break;
        }
        // Correct for user hide
    }
    else if (userHidden == LeftTop)
    {
        if (position() == KPanelExtension::Left || position() == KPanelExtension::Right)
        {
            top = workArea.top() - s.height() + m_settings.hideButtonSize();
        }
        else
        {
            left = workArea.left() - s.width() + m_settings.hideButtonSize();
        }
    }
    else if (userHidden == RightBottom)
    {
        if (position() == KPanelExtension::Left || position() == KPanelExtension::Right)
        {
            top = workArea.bottom() - m_settings.hideButtonSize() + 1;
        }
        else
        {
            left = workArea.right() - m_settings.hideButtonSize() + 1;
        }
    }

    return QPoint( left, top );
}

int ExtensionContainer::xineramaScreen() const
{
    // sanitize at runtime only, since many Xinerama users
    // turn it on and off and don't want kicker to lose their configs

    /* -2 means all screens, -1 primary screens, the rest are valid screen numbers */
    if (XineramaAllScreens <= m_settings.xineramaScreen() &&
        m_settings.xineramaScreen() < QApplication::desktop()->numScreens())
    {
        return m_settings.xineramaScreen();
    }
    else
    {
        /* force invalid screen locations onto the primary screen */
        return QApplication::desktop()->primaryScreen();
    }
}

void ExtensionContainer::setXineramaScreen(int screen)
{
    if (m_settings.isImmutable("XineramaScreen"))
    {
        return;
    }

    arrange(position(),alignment(), screen);
}

QRect ExtensionContainer::currentGeometry() const
{
    return initialGeometry(position(), alignment(), xineramaScreen(),
                           autoHidden(), userHidden());
}

QRect ExtensionContainer::initialGeometry(KPanelExtension::Position p,
                                          KPanelExtension::Alignment a,
                                          int XineramaScreen,
                                          bool autoHidden,
                                          UserHidden userHidden) const
{
    //RESEARCH: is there someway to cache the results of the repeated calls to this method?

    /*kdDebug(1210) << "initialGeometry() Computing geometry for " << name() <<
        " on screen " << XineramaScreen << endl;*/
    QRect workArea = ExtensionManager::the()->workArea(XineramaScreen, this);
    QSize size = initialSize(p, workArea);
    QPoint point = initialLocation(p, a, XineramaScreen,
                                   size, workArea,
                                   autoHidden, userHidden);

    //kdDebug(1210) << "Size: " << size.width() << " x " << size.height() << endl;
    //kdDebug(1210) << "Pos: (" << point.x() << ", " << point.y() << ")" << endl;

    return QRect(point, size);
}

bool ExtensionContainer::eventFilter( QObject*, QEvent * e)
{
    if (autoHidden())
    {
        switch ( e->type() )
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        return true; // ignore;
        default:
        break;
        }
    }

    QEvent::Type eventType = e->type();
    if (_block_user_input)
    {
        return (eventType == QEvent::MouseButtonPress ||
                eventType == QEvent::MouseButtonRelease ||
                eventType == QEvent::MouseButtonDblClick ||
                eventType == QEvent::MouseMove ||
                eventType == QEvent::KeyPress ||
                eventType == QEvent::KeyRelease ||
                eventType == QEvent::Enter ||
                eventType == QEvent::Leave);
    }

    switch (eventType)
    {
        case QEvent::MouseButtonPress:
        {
            QMouseEvent* me = static_cast<QMouseEvent*>(e);
            if ( me->button() == LeftButton )
            {
                _last_lmb_press = me->globalPos();
                _is_lmb_down = true;
            }
            else if (me->button() == RightButton)
            {
                showPanelMenu(me->globalPos());
                return true; // don't crash!
            }
        }
        break;

        case QEvent::MouseButtonRelease:
        {
            QMouseEvent* me = static_cast<QMouseEvent*>(e);
            if ( me->button() == LeftButton )
            {
                _is_lmb_down = false;
            }
        }
        break;

        case QEvent::MouseMove:
        {
            QMouseEvent* me = (QMouseEvent*) e;
            if (_is_lmb_down &&
                ((me->state() & LeftButton) == LeftButton) &&
                !Kicker::the()->isImmutable() &&
                !m_settings.config()->isImmutable() &&
                !ExtensionManager::the()->isMenuBar(this))
            {
                QPoint p(me->globalPos() - _last_lmb_press);
                int x_threshold = width();
                int y_threshold = height();

                if (x_threshold > y_threshold)
                {
                     x_threshold = x_threshold / 3;
                     y_threshold *= 2;
                }
                else
                {
                    y_threshold = y_threshold / 3;
                    x_threshold *= 2;
                }

                if ((abs(p.x()) > x_threshold) ||
                    (abs(p.y()) > y_threshold))
                {
                    moveMe();
                    return true;
                }
            }
        }
        break;

        default:
        break;
    }

    return false;
}

PopupWidgetFilter::PopupWidgetFilter( QObject *parent )
  : QObject( parent, "PopupWidgetFilter" )
{
}

bool PopupWidgetFilter::eventFilter( QObject*, QEvent* e )
{
    if (e->type() == QEvent::Hide)
    {
        emit popupWidgetHiding();
    }
    return false;
}

#include "container_extension.moc"

