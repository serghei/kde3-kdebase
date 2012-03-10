/*****************************************************************

Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>

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

#include <qlayout.h>
#include <qtimer.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kwindowlistmenu.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "simplebutton.h"

#include "taskbar.h"
#include "taskbarsettings.h"

#include "taskbarcontainer.h"
#include "taskbarcontainer.moc"

TaskBarContainer::TaskBarContainer( bool enableFrame, QWidget *parent, const char *name )
    : QFrame(parent, name),
      direction( KPanelApplet::Up ),
      showWindowListButton( true ),
      windowListButton(0),
      windowListMenu(0)
{
    setBackgroundOrigin( AncestorOrigin );
    
    uint margin;
    if ( enableFrame )
    {
        setFrameStyle( Sunken | StyledPanel );
        margin = frameWidth();
    }
    else
    {
        setFrameStyle( NoFrame );
        margin = 0;
    }

    layout = new QBoxLayout( this, QApplication::reverseLayout() ?
                                   QBoxLayout::RightToLeft :
                                   QBoxLayout::LeftToRight );
    layout->setMargin( margin );

    // scrollable taskbar
    taskBar = new TaskBar(this);
    layout->addWidget( taskBar );

    connect( taskBar, SIGNAL( containerCountChanged() ), SIGNAL( containerCountChanged() ) );

    setBackground();

    // read settings and setup layout
    configure();

    connectDCOPSignal("", "", "kdeTaskBarConfigChanged()",
                      "configChanged()", false);
}

TaskBarContainer::~TaskBarContainer()
{
    delete windowListMenu;
}

void TaskBarContainer::configure()
{
    setFont(TaskBarSettings::taskbarFont());
    showWindowListButton = TaskBarSettings::showWindowListBtn();

    if (!showWindowListButton)
    {
        delete windowListButton;
        windowListButton = 0;
        delete windowListMenu;
        windowListMenu = 0;
    }
    else if (windowListButton == 0)
    {
        // window list button
        windowListButton = new SimpleButton(this);
        windowListMenu= new KWindowListMenu;
        connect(windowListButton, SIGNAL(pressed()),
                SLOT(showWindowListMenu()));
        connect(windowListMenu, SIGNAL(aboutToHide()),
                SLOT(windowListMenuAboutToHide()));

        // geometry
        QString icon;
        switch (direction)
        {
            case KPanelApplet::Up:
                icon = "1uparrow";
                windowListButton->setMaximumHeight(BUTTON_MAX_WIDTH);
                break;
            case KPanelApplet::Down:
                icon = "1downarrow";
                windowListButton->setMaximumHeight(BUTTON_MAX_WIDTH);
                break;
            case KPanelApplet::Left:
                icon = "1leftarrow";
                windowListButton->setMaximumWidth(BUTTON_MAX_WIDTH);
                break;
            case KPanelApplet::Right:
                icon = "1rightarrow";
                windowListButton->setMaximumWidth(BUTTON_MAX_WIDTH);
                break;
        }

        windowListButton->setPixmap(kapp->iconLoader()->loadIcon(icon,
                                                                 KIcon::Panel,
                                                                 16));
        windowListButton->setMinimumSize(windowListButton->sizeHint());
        layout->insertWidget(0, windowListButton);
        windowListButton->show();
    }
}

void TaskBarContainer::configChanged()
{
    // we have a separate method here to connect to the DCOP signal
    // instead of connecting direclty to taskbar so that Taskbar
    // doesn't have to also connect to the DCOP signal (less places
    // to change/fix it if/when it changes) without calling
    // configure() twice on taskbar on start up
    TaskBarSettings::self()->readConfig();

    configure();
    taskBar->configure();
}

void TaskBarContainer::preferences()
{
    QByteArray data;

    if (!kapp->dcopClient()->isAttached())
    {
        kapp->dcopClient()->attach();
    }

    kapp->dcopClient()->send("kicker", "kicker", "showTaskBarConfig()", data);
}

void TaskBarContainer::orientationChange(Orientation o)
{
    if (o == Horizontal)
     {
        if (windowListButton)
        {
            windowListButton->setFixedWidth(WINDOWLISTBUTTON_SIZE);
            windowListButton->setMaximumHeight(BUTTON_MAX_WIDTH);
        }
        layout->setDirection(QApplication::reverseLayout() ?
                                QBoxLayout::RightToLeft :
                                QBoxLayout::LeftToRight);
    }
    else
    {
        if (windowListButton)
        {
            windowListButton->setMaximumWidth(BUTTON_MAX_WIDTH);
            windowListButton->setFixedHeight(WINDOWLISTBUTTON_SIZE);
        }
        layout->setDirection(QBoxLayout::TopToBottom);
    }

    taskBar->setOrientation(o);
    if (windowListButton)
    {
        windowListButton->setOrientation(o);
    }
    layout->activate();
}

void TaskBarContainer::popupDirectionChange(KPanelApplet::Direction d)
{
    direction = d;
    ArrowType at = UpArrow;

    QString icon;
    switch (d)
    {
        case KPanelApplet::Up:
            icon = "1uparrow";
            at = UpArrow;
            break;
        case KPanelApplet::Down:
            icon = "1downarrow";
            at = DownArrow;
            break;
        case KPanelApplet::Left:
            icon = "1leftarrow";
            at = LeftArrow;
            break;
        case KPanelApplet::Right:
            icon = "1rightarrow";
            at = RightArrow;
            break;
    }

    taskBar->setArrowType(at);

    if (windowListButton)
    {
        windowListButton->setPixmap(kapp->iconLoader()->loadIcon(icon,
                                                                 KIcon::Panel,
                                                                 16));
        windowListButton->setMinimumSize(windowListButton->sizeHint());
    }
}

void TaskBarContainer::showWindowListMenu()
{
    if (!windowListMenu)
        return;

    windowListMenu->init();

    // calc popup menu position
    QPoint pos( mapToGlobal( QPoint(0,0) ) );

    switch( direction ) {
        case KPanelApplet::Right:
            pos.setX( pos.x() + width() );
            break;
        case KPanelApplet::Left:
            pos.setX( pos.x() - windowListMenu->sizeHint().width() );
            break;
        case KPanelApplet::Down:
            pos.setY( pos.y() + height() );
            break;
        case KPanelApplet::Up:
            pos.setY( pos.y() - windowListMenu->sizeHint().height() );
        default:
            break;
    }

    disconnect( windowListButton, SIGNAL( pressed() ), this, SLOT( showWindowListMenu() ) );
    windowListMenu->exec( pos );
    QTimer::singleShot(100, this, SLOT(reconnectWindowListButton()));
}

void TaskBarContainer::windowListMenuAboutToHide()
{
    // this ensures that when clicked AGAIN, the window list button doesn't cause the
    // window list menu to show again. usability, you see. hoorah.
    windowListButton->setDown( false );
}

void TaskBarContainer::reconnectWindowListButton()
{
    connect( windowListButton, SIGNAL( pressed() ), SLOT( showWindowListMenu() ) );
}

QSize TaskBarContainer::sizeHint( KPanelExtension::Position p, QSize maxSize) const
{
    QSize size = taskBar->sizeHint( p, maxSize );
    if ( (p == KPanelExtension::Left || p == KPanelExtension::Right) && showWindowListButton ) {
        return QSize( size.width(), size.height() + WINDOWLISTBUTTON_SIZE );
    }
    return size;
}

void TaskBarContainer::setBackground()
{
    taskBar->setBackground();
}
