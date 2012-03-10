/*****************************************************************

Copyright (c) 1996-2003 the kicker authors. See file AUTHORS.

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

#include <algorithm>

#include <qlayout.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kmultipledrag.h>
#include <kpanelapplet.h>
#include <kurldrag.h>

#include "global.h"
#include "appletop_mnu.h"

#include "containerarea.h"
#include "panelbutton.h"
#include "bookmarksbutton.h"
#include "browserbutton.h"
#include "desktopbutton.h"
#include "extensionbutton.h"
#include "kbutton.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "kickertip.h"
#include "nonkdeappbutton.h"
#include "paneldrag.h"
#include "servicebutton.h"
#include "servicemenubutton.h"
#include "urlbutton.h"
#include "windowlistbutton.h"

#include "container_button.h"
#include "container_button.moc"

ButtonContainer::ButtonContainer(QPopupMenu* opMenu, QWidget* parent)
  : BaseContainer(opMenu, parent)
  , _button(0)
  , _layout(0)
  , _oldpos(0,0)
{
     setBackgroundOrigin(AncestorOrigin);
}

bool ButtonContainer::isValid() const
{
   if (_button)
   {
       return _button->isValid();
   }

   return false; // Can this happen?
}

// Buttons Shouldn't be square when larger than a certain size.
int ButtonContainer::widthForHeight(int height) const
{
    if (isValid())
    {
        return _button->widthForHeight(height);
    }

    return height;
}

int ButtonContainer::heightForWidth(int width)  const
{
    if (isValid())
    {
        return _button->heightForWidth(width);
    }

    return width;
}

void ButtonContainer::setBackground()
{
    PanelButton* b = button();
    if (!b)
        return;

    b->unsetPalette();
}

void ButtonContainer::configure()
{
    if (_button)
    {
        _button->configure();
    }
}

void ButtonContainer::doSaveConfiguration(KConfigGroup& config, bool layoutOnly) const
{
    // immutability is checked by ContainerBase
    if (_button && !layoutOnly)
    {
        _button->saveConfig(config);
    }
}

void ButtonContainer::setPopupDirection(KPanelApplet::Direction d)
{
    BaseContainer::setPopupDirection(d);

    if (_button)
    {
        _button->setPopupDirection(d);
    }
}

void ButtonContainer::setOrientation(Orientation o)
{
    BaseContainer::setOrientation(o);

    if(_button)
        _button->setOrientation(o);
}

void ButtonContainer::embedButton(PanelButton* b)
{
    if (!b) return;

    delete _layout;
    _layout = new QVBoxLayout(this);
    _button = b;

    _button->installEventFilter(this);
    _layout->add(_button);
    connect(_button, SIGNAL(requestSave()), SIGNAL(requestSave()));
    connect(_button, SIGNAL(hideme(bool)), SLOT(hideRequested(bool)));
    connect(_button, SIGNAL(removeme()), SLOT(removeRequested()));
    connect(_button, SIGNAL(dragme(const QPixmap)),
            SLOT(dragButton(const QPixmap)));
    connect(_button, SIGNAL(dragme(const KURL::List, const QPixmap)),
            SLOT(dragButton(const KURL::List, const QPixmap)));
}

QPopupMenu* ButtonContainer::createOpMenu()
{
    return new PanelAppletOpMenu(_actions, appletOpMenu(), 0, _button->title(),
                                 _button->icon(), this);
}

void ButtonContainer::removeRequested()
{
    if (isImmutable())
    {
        return;
    }

    emit removeme(this);
}

void ButtonContainer::hideRequested(bool shouldHide)
{
    if (shouldHide)
    {
        hide();
    }
    else
    {
        show();
    }
}

void ButtonContainer::dragButton(const KURL::List urls, const QPixmap icon)
{
    if (isImmutable())
    {
        return;
    }

    KMultipleDrag* dd = new KMultipleDrag(this);
    dd->addDragObject(new KURLDrag(urls, 0));
    dd->addDragObject(new PanelDrag(this, 0));
    dd->setPixmap(icon);
    grabKeyboard();
    dd->dragMove();
    releaseKeyboard();
}

void ButtonContainer::dragButton(const QPixmap icon)
{
    PanelDrag* dd = new PanelDrag(this, this);
    dd->setPixmap(icon);
    grabKeyboard();
    dd->drag();
    releaseKeyboard();
}

bool ButtonContainer::eventFilter(QObject *o, QEvent *e)
{
    if (o == _button && e->type() == QEvent::MouseButtonPress)
    {
        static bool sentinal = false;

        if (sentinal)
        {
            return false;
        }

        sentinal = true;
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        switch (me->button())
        {
        case MidButton:
        {
            if (isImmutable())
            {
                break;
            }

            _button->setDown(true);
            _moveOffset = me->pos();
            emit moveme(this);
            sentinal = false;
            return true;
        }

        case RightButton:
        {
            if (!kapp->authorizeKAction("kicker_rmb") ||
                isImmutable())
            {
                break;
            }

            QPopupMenu* menu = opMenu();
            connect( menu, SIGNAL( aboutToHide() ), this, SLOT( slotMenuClosed() ) );
            QPoint pos = KickerLib::popupPosition(popupDirection(), menu, this,
                                                  (orientation() == Horizontal) ?
                                                   QPoint(0, 0) : me->pos());

            Kicker::the()->setInsertionPoint(me->globalPos());

            KickerTip::enableTipping(false);
            switch (menu->exec(pos))
            {
            case PanelAppletOpMenu::Move:
                _moveOffset = rect().center();
                emit moveme(this);
                break;
            case PanelAppletOpMenu::Remove:
                emit removeme(this);
                break;
            case PanelAppletOpMenu::Help:
                help();
                break;
            case PanelAppletOpMenu::About:
                about();
                break;
            case PanelAppletOpMenu::Preferences:
                if (_button)
                {
                    _button->properties();
                }
                break;
            default:
                break;
            }
            KickerTip::enableTipping(true);

            Kicker::the()->setInsertionPoint(QPoint());
            clearOpMenu();
            sentinal = false;
            return true;
        }

        default:
            break;
        }

        sentinal = false;
    }
    return false;
}

void ButtonContainer::completeMoveOperation()
{
    if (_button)
    {
        _button->setDown(false);
        setBackground();
    }
}

void ButtonContainer::slotMenuClosed()
{
    if (_button)
        _button->setDown(false);
}

void ButtonContainer::checkImmutability(const KConfigGroup& config)
{
    m_immutable = config.groupIsImmutable() ||
                  config.entryIsImmutable("ConfigFile") ||
                  config.entryIsImmutable("FreeSpace2");
}

// KMenuButton containerpan
KMenuButtonContainer::KMenuButtonContainer(const KConfigGroup& config, QPopupMenu *opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    checkImmutability(config);
    embedButton( new KButton(this) );
    _actions = PanelAppletOpMenu::KMenuEditor;
}

KMenuButtonContainer::KMenuButtonContainer(QPopupMenu *opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    embedButton( new KButton(this) );
    _actions = PanelAppletOpMenu::KMenuEditor;
}

int KMenuButtonContainer::heightForWidth( int width ) const
{
    if ( width < 32 )
        return width + 10;
    else
        return ButtonContainer::heightForWidth(width);
}

// DesktopButton container
DesktopButtonContainer::DesktopButtonContainer(QPopupMenu *opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    embedButton( new DesktopButton(this) );
}

DesktopButtonContainer::DesktopButtonContainer(const KConfigGroup& config,
                                               QPopupMenu *opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    checkImmutability(config);
    embedButton( new DesktopButton(this) );
}

// ServiceButton container
ServiceButtonContainer::ServiceButtonContainer( const QString& desktopFile,
                                                QPopupMenu* opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    embedButton( new ServiceButton( desktopFile, this ) );
    _actions = KPanelApplet::Preferences;
}

ServiceButtonContainer::ServiceButtonContainer( const KService::Ptr &service,
                                                QPopupMenu* opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    embedButton( new ServiceButton( service, this ) );
    _actions = KPanelApplet::Preferences;
}

ServiceButtonContainer::ServiceButtonContainer( const KConfigGroup& config,
                                                QPopupMenu* opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    checkImmutability(config);
    embedButton( new ServiceButton( config, this ) );
    _actions = KPanelApplet::Preferences;
}

QString ServiceButtonContainer::icon() const
{
    return button()->icon();
}

QString ServiceButtonContainer::visibleName() const
{
    return button()->title();
}

// URLButton container
URLButtonContainer::URLButtonContainer( const QString& url, QPopupMenu* opMenu, QWidget* parent )
  : ButtonContainer(opMenu, parent)
{
    embedButton( new URLButton( url, this ) );
    _actions = KPanelApplet::Preferences;
}

URLButtonContainer::URLButtonContainer( const KConfigGroup& config, QPopupMenu* opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    checkImmutability(config);
    embedButton( new URLButton( config, this ) );
    _actions = KPanelApplet::Preferences;
}

QString URLButtonContainer::icon() const
{
    return button()->icon();
}

QString URLButtonContainer::visibleName() const
{
    return button()->title();
}

// BrowserButton container
BrowserButtonContainer::BrowserButtonContainer(const QString &startDir, QPopupMenu* opMenu, const QString& icon, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    embedButton( new BrowserButton(icon, startDir, this) );
    _actions = KPanelApplet::Preferences;
}

BrowserButtonContainer::BrowserButtonContainer( const KConfigGroup& config, QPopupMenu* opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    checkImmutability(config);
    embedButton( new BrowserButton(config, this) );
    _actions = KPanelApplet::Preferences;
}

// ServiceMenuButton container
ServiceMenuButtonContainer::ServiceMenuButtonContainer(const QString& relPath, QPopupMenu* opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    embedButton( new ServiceMenuButton(relPath, this) );
}

ServiceMenuButtonContainer::ServiceMenuButtonContainer( const KConfigGroup& config, QPopupMenu* opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    checkImmutability(config);
    embedButton( new ServiceMenuButton(config, this) );
}

QString ServiceMenuButtonContainer::icon() const
{
    return button()->icon();
}

QString ServiceMenuButtonContainer::visibleName() const
{
    return button()->title();
}

// WindowListButton container
WindowListButtonContainer::WindowListButtonContainer(const KConfigGroup& config, QPopupMenu *opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    checkImmutability(config);
    embedButton( new WindowListButton(this) );
}

WindowListButtonContainer::WindowListButtonContainer(QPopupMenu *opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    embedButton( new WindowListButton(this) );
}

// BookmarkButton container
BookmarksButtonContainer::BookmarksButtonContainer(const KConfigGroup& config, QPopupMenu *opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    checkImmutability(config);
    embedButton( new BookmarksButton(this) );
    _actions = PanelAppletOpMenu::BookmarkEditor;
}

BookmarksButtonContainer::BookmarksButtonContainer(QPopupMenu *opMenu, QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    embedButton( new BookmarksButton(this) );
    _actions = PanelAppletOpMenu::BookmarkEditor;
}

// NonKDEAppButton container
NonKDEAppButtonContainer::NonKDEAppButtonContainer(const QString &name,
                                                   const QString &description,
                                                   const QString &filePath,
                                                   const QString &icon,
                                                   const QString &cmdLine,
                                                   bool inTerm,
                                                   QPopupMenu* opMenu,
                                                   QWidget* parent)
  : ButtonContainer(opMenu, parent)
{
    embedButton(new NonKDEAppButton(name, description, filePath, icon, cmdLine,
                                    inTerm, this));
    _actions = KPanelApplet::Preferences;
}

NonKDEAppButtonContainer::NonKDEAppButtonContainer( const KConfigGroup& config, QPopupMenu* opMenu, QWidget *parent)
  : ButtonContainer(opMenu, parent)
{
    checkImmutability(config);
    embedButton( new NonKDEAppButton(config, this) );
    _actions = KPanelApplet::Preferences;
}

// ExtensionButton container
ExtensionButtonContainer::ExtensionButtonContainer(const QString& df, QPopupMenu* opMenu, QWidget *parent)
  : ButtonContainer(opMenu, parent)
{
    embedButton( new ExtensionButton(df, this) );
}

ExtensionButtonContainer::ExtensionButtonContainer( const KConfigGroup& config, QPopupMenu* opMenu, QWidget *parent)
  : ButtonContainer(opMenu, parent)
{
    checkImmutability(config);
    embedButton( new ExtensionButton(config, this) );
}

QString ExtensionButtonContainer::icon() const
{
    return button()->icon();
}

QString ExtensionButtonContainer::visibleName() const
{
    return button()->title();
}

