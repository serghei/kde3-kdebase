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
#include <qlayout.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qpixmapcache.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qimage.h>

#include <kpushbutton.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "global.h"
#include "container_applet.h"
#include "kickerSettings.h"

#include "applethandle.h"

AppletHandle::AppletHandle(AppletContainer* parent)
    : QWidget(parent),
      m_applet(parent),
      m_menuButton(0),
      m_drawHandle(false),
      m_popupDirection(KPanelApplet::Up),
      m_handleHoverTimer(0)
{
    setBackgroundOrigin(AncestorOrigin);
    setMinimumSize(widthForHeight(0), heightForWidth(0));
    m_layout = new QBoxLayout(this, QBoxLayout::BottomToTop, 0, 0);

    m_dragBar = new AppletHandleDrag(this);
    m_dragBar->installEventFilter(this);
    m_layout->addWidget(m_dragBar);

    if (kapp->authorizeKAction("kicker_rmb"))
    {
        m_menuButton = new AppletHandleButton( this );
        m_menuButton->installEventFilter(this);
        m_layout->addWidget(m_menuButton);

        connect(m_menuButton, SIGNAL(pressed()),
                this, SLOT(menuButtonPressed()));
        QToolTip::add(m_menuButton, i18n("%1 menu").arg(parent->info().name()));
    }

    QToolTip::add(this, i18n("%1 applet handle").arg(parent->info().name()));
    resetLayout();
}

int AppletHandle::heightForWidth( int /* w */ ) const
{
    int size = style().pixelMetric(QStyle::PM_DockWindowHandleExtent, this);

    return size;
}

int AppletHandle::widthForHeight( int /* h */ ) const
{
    int size = style().pixelMetric(QStyle::PM_DockWindowHandleExtent, this);

    return size;
}

void AppletHandle::setPopupDirection(KPanelApplet::Direction d)
{
    Qt::ArrowType a = Qt::UpArrow;

    if (d == m_popupDirection || !m_menuButton)
    {
        return;
    }

    m_popupDirection = d;

    switch (m_popupDirection)
    {
        case KPanelApplet::Up:
            m_layout->setDirection(QBoxLayout::BottomToTop);
            a = Qt::UpArrow;
            break;
        case KPanelApplet::Down:
            m_layout->setDirection(QBoxLayout::TopToBottom);
            a = Qt::DownArrow;
            break;
        case KPanelApplet::Left:
            m_layout->setDirection(QBoxLayout::RightToLeft);
            a = Qt::LeftArrow;
            break;
        case KPanelApplet::Right:
            m_layout->setDirection(QBoxLayout::LeftToRight);
            a = Qt::RightArrow;
            break;
    }

    m_menuButton->setArrowType(a);
    m_layout->activate();
}

void AppletHandle::resetLayout()
{
    if (m_handleHoverTimer && !m_drawHandle)
    {
        m_dragBar->hide();

        if (m_menuButton)
        {
            m_menuButton->hide();
        }
    }
    else
    {
        m_dragBar->show();

        if (m_menuButton)
        {
            m_menuButton->show();
        }
    }
}

void AppletHandle::setFadeOutHandle(bool fadeOut)
{
    if (fadeOut)
    {
        if (!m_handleHoverTimer)
        {
            m_handleHoverTimer = new QTimer(this);
            connect(m_handleHoverTimer, SIGNAL(timeout()),
                    this, SLOT(checkHandleHover()));
            m_applet->installEventFilter(this);
        }
    }
    else
    {
        delete m_handleHoverTimer;
        m_handleHoverTimer = 0;
        m_applet->removeEventFilter(this);
    }

    resetLayout();
}

bool AppletHandle::eventFilter(QObject *o, QEvent *e)
{
    if (o == parent())
    {
        switch (e->type())
        {
            case QEvent::Enter:
            {
                m_drawHandle = true;
                resetLayout();

                if (m_handleHoverTimer)
                {
                    m_handleHoverTimer->start(250);
                }
                break;
            }

            case QEvent::Leave:
            {
                if (m_menuButton && m_menuButton->isOn())
                {
                    break;
                }

                QWidget* w = dynamic_cast<QWidget*>(o);

                bool nowDrawIt = false;
                if (w)
                {
                    // a hack for applets that have out-of-process
                    // elements (e.g the systray) so that the handle
                    // doesn't flicker when moving over those elements
                    if (w->rect().contains(w->mapFromGlobal(QCursor::pos())))
                    {
                        nowDrawIt = true;
                    }
                }

                if (nowDrawIt != m_drawHandle)
                {
                    if (m_handleHoverTimer)
                    {
                        m_handleHoverTimer->stop();
                    }

                    m_drawHandle = nowDrawIt;
                    resetLayout();
                }
                break;
            }

            default:
                break;
        }

        return QWidget::eventFilter( o, e );
    }
    else if (o == m_dragBar)
    {
        if (e->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* ev = static_cast<QMouseEvent*>(e);
            if (ev->button() == LeftButton || ev->button() == MidButton)
            {
                emit moveApplet(m_applet->mapFromGlobal(ev->globalPos()));
            }
        }
    }

    if (m_menuButton && e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* ev = static_cast<QMouseEvent*>(e);
        if (ev->button() == RightButton)
        {
            if (!m_menuButton->isDown())
            {
                m_menuButton->setDown(true);
                menuButtonPressed();
            }

            return true;
        }
    }

    return QWidget::eventFilter(o, e);    // standard event processing
}

void AppletHandle::menuButtonPressed()
{
    if (!kapp->authorizeKAction("kicker_rmb"))
    {
        return;
    }

    emit showAppletMenu();

    if (!onMenuButton(QCursor::pos()))
    {
        toggleMenuButtonOff();
    }
}

void AppletHandle::checkHandleHover()
{
    if (!m_handleHoverTimer ||
        (m_menuButton && m_menuButton->isOn()) ||
        m_applet->geometry().contains(m_applet->mapToParent(
                                      m_applet->mapFromGlobal(QCursor::pos()))))
    {
        return;
    }

    m_handleHoverTimer->stop();
    m_drawHandle = false;
    resetLayout();
}

bool AppletHandle::onMenuButton(const QPoint& point) const
{
    return m_menuButton && (childAt(mapFromGlobal(point)) == m_menuButton);
}

void AppletHandle::toggleMenuButtonOff()
{
    if (!m_menuButton)
    {
        return;
    }

    m_menuButton->setDown(false);
}

AppletHandleDrag::AppletHandleDrag(AppletHandle* parent)
    : QWidget(parent),
      m_parent(parent),
      m_inside(false)
{
   setBackgroundOrigin( AncestorOrigin );
}

QSize AppletHandleDrag::minimumSizeHint() const
{
    int wh = style().pixelMetric(QStyle::PM_DockWindowHandleExtent, this);

    if (m_parent->orientation() == Horizontal)
    {
        return QSize(wh, 0);
    }

    return QSize(0, wh);
}

QSizePolicy AppletHandleDrag::sizePolicy() const
{
    if (m_parent->orientation() == Horizontal)
    {
        return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
    }

    return QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
}

void AppletHandleDrag::enterEvent( QEvent *e )
{
    m_inside = true;
    QWidget::enterEvent( e );
    update();
}

void AppletHandleDrag::leaveEvent( QEvent *e )
{
    m_inside = false;
    QWidget::enterEvent( e );
    update();
}

void AppletHandleDrag::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    
    if (!KickerSettings::transparent())
    {
        if (paletteBackgroundPixmap())
        {
            QPoint offset = backgroundOffset();
            int ox = offset.x();
            int oy = offset.y();
            p.drawTiledPixmap( 0, 0, width(), height(),*paletteBackgroundPixmap(), ox, oy);
        }
        
        QStyle::SFlags flags = QStyle::Style_Default;
        flags |= QStyle::Style_Enabled;
        if (m_parent->orientation() == Horizontal)
        {
            flags |= QStyle::Style_Horizontal;
        }
    
        QRect r = rect();
    
        style().drawPrimitive(QStyle::PE_DockWindowHandle, &p, r,
                            colorGroup(), flags);
    }
    else
    {
        KickerLib::drawBlendedRect(&p, QRect(0, 0, width(), height()), paletteForegroundColor(), m_inside ? 0x40 : 0x20);
    }
}

AppletHandleButton::AppletHandleButton(AppletHandle *parent)
  : SimpleArrowButton(parent),
    m_parent(parent)
{
}

QSize AppletHandleButton::minimumSizeHint() const
{
    int height = style().pixelMetric(QStyle::PM_DockWindowHandleExtent, this);
    int width = height;

    if (m_parent->orientation() == Horizontal)
    {
        return QSize(width, height);
    }

    return QSize(height, width);
}

QSizePolicy AppletHandleButton::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

#include "applethandle.moc"
