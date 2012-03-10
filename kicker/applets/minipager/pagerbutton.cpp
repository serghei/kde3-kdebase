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

#include <stdlib.h>

#include <qcursor.h>
#include <qdrawutil.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qstylesheet.h>

#include <netwm.h>
#include <dcopclient.h>

#include <kwinmodule.h>
#include <ksharedpixmap.h>
#include <kpixmapio.h>
#include <kpixmapeffect.h>
#include <kstringhandler.h>
#include <kiconloader.h>

#include "global.h"
#include "kickertip.h"
#include "kickerSettings.h"
#include "kshadowengine.h"
#include "paneldrag.h"

#include "pagerapplet.h"
#include "pagerbutton.h"
#include "pagerbutton.moc"
#include "pagersettings.h"

#ifdef FocusOut
#undef FocusOut
#endif

KSharedPixmap* KMiniPagerButton::s_commonSharedPixmap;
KPixmap* KMiniPagerButton::s_commonBgPixmap;

KMiniPagerButton::KMiniPagerButton(int desk, bool useViewPorts, const QPoint& viewport,
        KMiniPager *parent, const char *name)
    : QButton(parent, name),
      m_pager(parent),
      m_desktop(desk),
      m_useViewports(useViewPorts),
      m_viewport(viewport),
      m_lineEdit(0),
      m_sharedPixmap(0),
      m_bgPixmap(0),
      m_isCommon(false),
      m_currentWindow(0),
      m_inside(false)
{
    setToggleButton(true);
    setAcceptDrops(true);
    setWFlags(WNoAutoErase);

    setBackgroundOrigin(AncestorOrigin);
    installEventFilter(KickerTip::the());

    m_desktopName = m_pager->kwin()->desktopName(m_desktop);

    connect(this, SIGNAL(clicked()), SLOT(slotClicked()));
    connect(this, SIGNAL(toggled(bool)), SLOT(slotToggled(bool)));
    connect(&m_dragSwitchTimer, SIGNAL(timeout()), this, SLOT(slotDragSwitch()));
    connect(&m_updateCompressor, SIGNAL(timeout()), this, SLOT(update()));

    if (m_pager->desktopPreview())
    {
        setMouseTracking(true);
    }
    loadBgPixmap();
}

KMiniPagerButton::~KMiniPagerButton()
{
    delete m_sharedPixmap;
    delete m_bgPixmap;
}

QRect KMiniPagerButton::mapGeometryToViewport(const KWin::WindowInfo& info) const
{
    if (!m_useViewports)
        return info.frameGeometry();

    // ### fix vertically layouted viewports
    QRect _r(info.frameGeometry());
    QPoint vx(m_pager->kwin()->currentViewport(m_pager->kwin()->currentDesktop()));

    _r.moveBy( - (m_desktop - vx.x()) * QApplication::desktop()->width(),
               0);

    if ((info.state() & NET::Sticky))
    {
        _r.moveTopLeft(QPoint(_r.x() % QApplication::desktop()->width(),
                    _r.y() % QApplication::desktop()->height()));

    }

    return _r;
}

QPoint KMiniPagerButton::mapPointToViewport(const QPoint& _p) const
{
    if (!m_useViewports) return _p;

    QPoint vx(m_pager->kwin()->currentViewport(m_pager->kwin()->currentDesktop()));

    // ### fix vertically layouted viewports
    QPoint p(_p);
    p.setX(p.x() + (m_desktop - vx.x()) * QApplication::desktop()->width());
    return p;
}

bool KMiniPagerButton::shouldPaintWindow( KWin::WindowInfo *info ) const
{
    if (!info)
      return false;

//  if (info->mappingState != NET::Visible)
//    return false;

    NET::WindowType type = info->windowType( NET::NormalMask | NET::DesktopMask
        | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
        | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask );

    if (type == NET::Desktop || type == NET::Dock || type == NET::TopMenu)
      return false;

    if (!m_useViewports && !info->isOnDesktop(m_desktop))
      return false;

    if (m_useViewports) {
        QRect r = mapGeometryToViewport(*info);

        if (!info->hasState(NET::Sticky) &&
            !QApplication::desktop()->geometry().contains(r.topLeft()) &&
            !QApplication::desktop()->geometry().contains(r.topRight()))
            return false;
    }

    if (info->state() & NET::SkipPager || info->state() & NET::Shaded )
      return false;

    if (info->win() == m_pager->winId())
      return false;

    if ( info->isMinimized() )
      return false;

    return true;
}

void KMiniPagerButton::resizeEvent(QResizeEvent *ev)
{
    if (m_lineEdit)
    {
        m_lineEdit->setGeometry(rect());
    }

    delete m_bgPixmap;
    m_bgPixmap = 0;

    QButton::resizeEvent(ev);
}

void KMiniPagerButton::windowsChanged()
{
    m_currentWindow = 0;

    if (!m_updateCompressor.isActive())
    {
        m_updateCompressor.start(50, true);
    }
}

void KMiniPagerButton::backgroundChanged()
{
    delete s_commonSharedPixmap;
    s_commonSharedPixmap = 0;
    delete s_commonBgPixmap;
    s_commonBgPixmap = 0;
    loadBgPixmap();
}

void KMiniPagerButton::loadBgPixmap()
{
    if (m_pager->bgType() != PagerSettings::EnumBackgroundType::BgLive)
        return; // not needed

    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
    {
        client->attach();
    }

    QCString kdesktop_name;
    int screen_number = DefaultScreen(qt_xdisplay());
    if (screen_number == 0)
        kdesktop_name = "kdesktop";
    else
        kdesktop_name.sprintf("kdesktop-screen-%d", screen_number);

    QByteArray data, replyData;
    QCString replyType;
    if (client->call(kdesktop_name, "KBackgroundIface", "isCommon()",
                     data, replyType, replyData))
    {
        if (replyType == "bool")
        {
            QDataStream reply(replyData, IO_ReadOnly);
            reply >> m_isCommon;
        }
    }

    if (m_isCommon)
    {
        if (s_commonBgPixmap)
        { // pixmap is already ready, just use it
            backgroundLoaded( true );
            return;
        }
        else if (s_commonSharedPixmap)
        { // other button is already fetching the pixmap
            connect(s_commonSharedPixmap, SIGNAL(done(bool)),
                    SLOT(backgroundLoaded(bool)));
            return;
        }
    }

    QDataStream args( data, IO_WriteOnly );
    args << 1;
    client->send(kdesktop_name, "KBackgroundIface", "setExport(int)", data);

    if (m_isCommon)
    {
        if (!s_commonSharedPixmap)
        {
            s_commonSharedPixmap = new KSharedPixmap;
            connect(s_commonSharedPixmap, SIGNAL(done(bool)),
                    SLOT(backgroundLoaded(bool)));
        }
        s_commonSharedPixmap->loadFromShared(QString("DESKTOP1"));
    }
    else
    {
        if (!m_sharedPixmap)
        {
            m_sharedPixmap = new KSharedPixmap;
            connect(m_sharedPixmap, SIGNAL(done(bool)),
                    SLOT(backgroundLoaded(bool)));
        }
        m_sharedPixmap->loadFromShared(QString("DESKTOP%1").arg(m_desktop));
    }
}

static QPixmap scalePixmap(const QPixmap &pixmap, int width, int height)
{
    if (pixmap.width()>100)
    {
        KPixmapIO io;
        QImage img( io.convertToImage( pixmap ) );
        return io.convertToPixmap( img.smoothScale( width, height ) );
    }

    QImage img( pixmap.convertToImage().smoothScale( width, height ) );
    QPixmap pix;
    pix.convertFromImage( img );

    return pix;
}

void KMiniPagerButton::backgroundLoaded( bool loaded )
{
    if (loaded)
    {
        if (!m_bgPixmap)
        {
            m_bgPixmap = new KPixmap;
        }
        if (m_isCommon)
        {
            if (!s_commonBgPixmap)
            {
                s_commonBgPixmap = new KPixmap;
                *s_commonBgPixmap = scalePixmap(*s_commonSharedPixmap, width(), height());
                s_commonSharedPixmap->deleteLater(); // let others get the signal too
                s_commonSharedPixmap = 0;
            }
            *m_bgPixmap = *s_commonBgPixmap;
        }
        else
        {
            *m_bgPixmap = scalePixmap(*m_sharedPixmap, width(), height());
            delete m_sharedPixmap;
            m_sharedPixmap = 0L;
        }

        update();
    }
    else
    {
        kdWarning() << "Error getting the background\n";
    }
}

void KMiniPagerButton::enterEvent(QEvent *)
{
    m_inside = true;
    update();
}

void KMiniPagerButton::leaveEvent(QEvent *)
{
    m_inside = false;
    update();
}

void KMiniPagerButton::drawButton(QPainter *bp)
{
    int w = width();
    int h = height();
    bool on = isOn();
    bool down = isDown();

    QBrush background;

    bool liveBkgnd = m_pager->bgType() == PagerSettings::EnumBackgroundType::BgLive;
    bool transparent = m_pager->bgType() == PagerSettings::EnumBackgroundType::BgTransparent;

    // background

    if (backgroundPixmap())
    {
        QPoint pt = backgroundOffset();
        bp->drawTiledPixmap(0, 0, width(), height(), *backgroundPixmap(), pt.x(), pt.y());
    }
    else
    {
        bp->fillRect(0, 0, width(), height(), paletteBackgroundColor());
    }
    

    // desktop background
    
    if (liveBkgnd)
    {
        if (m_bgPixmap && !m_bgPixmap->isNull())
        {
            if (on)
            {
                KPixmap tmp = *m_bgPixmap;
                KPixmapEffect::intensity(tmp, 0.33);
                bp->drawPixmap(0, 0, tmp);
            }
            else
            {
                bp->drawPixmap(0, 0, *m_bgPixmap);
            }
        }
        else
        {
            liveBkgnd = false;
        }
    }

    if (!liveBkgnd)
    {
        if (transparent)
        {
            // transparent windows get an 1 pixel frame...
            if (on)
            {
                bp->setPen(colorGroup().midlight());
            }
            else if (down)
            {
                bp->setPen(KickerLib::blendColors(colorGroup().mid(),
                                                 colorGroup().midlight()));
            }
            else
            {
                bp->setPen(colorGroup().dark());
            }

            bp->drawRect(0, 0, w, h);
        }
        else
        {
            QBrush background;

            if (on)
            {
                background = colorGroup().brush(QColorGroup::Midlight);
            }
            else if (down)
            {
                background = KickerLib::blendColors(colorGroup().mid(),
                                                    colorGroup().midlight());
            }
            else
            {
                background = colorGroup().brush(QColorGroup::Mid);
            }

            bp->fillRect(0, 0, w, h, background);
        }
    }

    // window preview...
    if (m_pager->desktopPreview())
    {
        KWinModule* kwin = m_pager->kwin();
        KWin::WindowInfo *info = 0;
        int dw = QApplication::desktop()->width();
        int dh = QApplication::desktop()->height();

        QValueList<WId> windows = kwin->stackingOrder();
        QValueList<WId>::const_iterator itEnd = windows.constEnd();
        for (QValueList<WId>::ConstIterator it = windows.constBegin(); it != itEnd; ++it)
        {
            info = m_pager->info(*it);

            if (shouldPaintWindow(info))
            {
                QRect r = mapGeometryToViewport(*info);
                r = QRect(r.x() * width() / dw, 2 + r.y() * height() / dh,
                          r.width() * width() / dw, r.height() * height() / dh);

                if (kwin->activeWindow() == info->win())
                {
                    QBrush brush = colorGroup().brush(QColorGroup::Highlight);
                    qDrawShadeRect(bp, r, colorGroup(), false, 1, 0, &brush);
                }
                else
                {
                    QBrush brush = colorGroup().brush(QColorGroup::Button);

                    if (on)
                    {
                        brush.setColor(brush.color().light(120));
                    }

                    bp->fillRect(r, brush);
                    qDrawShadeRect(bp, r, colorGroup(), true, 1, 0);
                }

                if (m_pager->windowIcons() && r.width() > 15 && r.height() > 15)
                {
                    QPixmap icon = KWin::icon(*it, 16, 16, true);
                    if (!icon.isNull())
                    {
                        bp->drawPixmap(r.left() + ((r.width() - 16) / 2),
                                      r.top() + ((r.height() - 16) / 2),
                                      icon);
                    }
                }
            }
        }
    }

    if (liveBkgnd)
    {
        // draw a little border around the individual buttons
        // makes it look a bit more finished.
        if (on)
        {
            bp->setPen(colorGroup().midlight());
        }
        else
        {
            bp->setPen(colorGroup().mid());
        }

        bp->drawRect(0, 0, w, h);
    }

    if (m_pager->labelType() != PagerSettings::EnumLabelType::LabelNone)
    {
        QString label = (m_pager->labelType() == PagerSettings::EnumLabelType::LabelNumber) ?
                            QString::number(m_desktop) : m_desktopName;
        
        if (transparent || liveBkgnd)
        {
            bp->setPen(on ? colorGroup().midlight() : colorGroup().buttonText());
            m_pager->shadowEngine()->drawText(*bp, QRect(0, 0, w, h), AlignCenter, label, size());
        }
        else
            bp->drawText(0, 0, w, h, AlignCenter, label);
    }
    
    if (m_inside)
        KickerLib::drawBlendedRect(bp, QRect(1, 1, width() - 2, height() - 2), colorGroup().foreground());
}

void KMiniPagerButton::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == RightButton)
    {
        // prevent LMB down -> RMB down -> LMB up sequence
        if ((e->state() & MouseButtonMask ) == NoButton)
        {
            emit showMenu(e->globalPos(), m_desktop);
            return;
        }
    }

    if (m_pager->desktopPreview())
    {
        m_pager->clickPos = e->pos();
    }

    QButton::mousePressEvent(e);
}

void KMiniPagerButton::mouseReleaseEvent(QMouseEvent* e)
{
    m_pager->clickPos = QPoint();
    QButton::mouseReleaseEvent(e);
}

void KMiniPagerButton::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_pager->desktopPreview())
    {
        return;
    }

    int dw = QApplication::desktop()->width();
    int dh = QApplication::desktop()->height();
    int w = width();
    int h = height();

    QPoint pos(m_pager->clickPos.isNull() ? mapFromGlobal(QCursor::pos()) : m_pager->clickPos);
    QPoint p = mapPointToViewport(QPoint(pos.x() * dw / w, pos.y() * dh / h));

    Task::Ptr wasWindow = m_currentWindow;
    m_currentWindow = TaskManager::the()->findTask(m_useViewports ? 1 : m_desktop, p);

    if (wasWindow != m_currentWindow)
    {
        KickerTip::Client::updateKickerTip();
    }

    if (m_currentWindow && !m_pager->clickPos.isNull() &&
        (m_pager->clickPos - e->pos()).manhattanLength() > KGlobalSettings::dndEventDelay())
    {
        QRect r = m_currentWindow->geometry();

        // preview window height, window width
        int ww = r.width() * w / dw;
        int wh = r.height() * h / dh;
        QPixmap windowImage(ww, wh);
        QPainter bp(&windowImage, this);

        bp.setPen(colorGroup().foreground());
        bp.drawRect(0, 0, ww, wh);
        bp.fillRect(1, 1, ww - 2, wh - 2, colorGroup().background());

        Task::List tasklist;
        tasklist.append(m_currentWindow);
        TaskDrag* drag = new TaskDrag(tasklist, this);
        QPoint offset(m_pager->clickPos.x() - (r.x() * w / dw),
                m_pager->clickPos.y() - (r.y() * h / dh));
        drag->setPixmap(windowImage, offset);
        drag->dragMove();

        if (isDown())
        {
            setDown(false);
        }

        m_pager->clickPos = QPoint();
    }
}

void KMiniPagerButton::dragEnterEvent(QDragEnterEvent* e)
{
    if (PanelDrag::canDecode(e))
    {
        // ignore container drags
        return;
    }
    else if (TaskDrag::canDecode(e))
    {
        // if it's a task drag don't switch the desktop, just accept it
        e->accept();
        setDown(true);
    }
    else
    {
        // if a dragitem is held for over a pager button for two seconds,
        // activate corresponding desktop
        m_dragSwitchTimer.start(1000, true);
        QButton::dragEnterEvent(e);
    }
}

void KMiniPagerButton::dropEvent(QDropEvent* e)
{
    if (TaskDrag::canDecode(e))
    {
        e->accept();
        Task::List tasks(TaskDrag::decode(e));

        if ((m_useViewports || e->source() == this) && tasks.count() == 1)
        {
            Task::Ptr task = tasks[0];
            int dw = QApplication::desktop()->width();
            int dh = QApplication::desktop()->height();
            int w = width();
            int h = height();
            QRect location = mapGeometryToViewport(task->info());
            QPoint pos = mapPointToViewport(e->pos());
            int deltaX = pos.x() - m_pager->clickPos.x();
            int deltaY = pos.y() - m_pager->clickPos.y();

            if (abs(deltaX) < 3)
            {
                deltaX = 0;
            }
            else
            {
                deltaX = deltaX * dw / w;
            }

            if (abs(deltaY) < 3)
            {
                deltaY = 0;
            }
            else
            {
                deltaY = deltaY * dh / h;
            }

            location.moveBy(deltaX, deltaY);

            XMoveWindow(x11Display(), task->window(), location.x(), location.y());
            if ((e->source() != this || !task->isOnAllDesktops()) &&
                task->desktop() != m_desktop)
            {
                task->toDesktop(m_desktop);
            }
        }
        else
        {
            Task::List::iterator itEnd = tasks.end();
            for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
            {
                (*it)->toDesktop(m_desktop);
            }
        }

        setDown(false);
    }

    QButton::dropEvent( e );
}

void KMiniPagerButton::enabledChange( bool oldEnabled )
{
    if (m_pager->bgType() == PagerSettings::EnumBackgroundType::BgLive)
    {
        m_pager->refresh();
    }

    QButton::enabledChange(oldEnabled);
}

void KMiniPagerButton::dragLeaveEvent( QDragLeaveEvent* e )
{
    m_dragSwitchTimer.stop();

    if (m_pager->kwin()->currentDesktop() != m_desktop)
    {
        setDown(false);
    }

    QButton::dragLeaveEvent( e );
}

void KMiniPagerButton::slotDragSwitch()
{
    emit buttonSelected(m_desktop);
}

void KMiniPagerButton::slotClicked()
{
    emit buttonSelected(m_desktop);
}

void KMiniPagerButton::rename()
{
  if ( !m_lineEdit ) {
    m_lineEdit = new QLineEdit( this );
    connect( m_lineEdit, SIGNAL( returnPressed() ), m_lineEdit, SLOT( hide() ) );
    m_lineEdit->installEventFilter( this );
  }
  m_lineEdit->setGeometry( rect() );
  m_lineEdit->setText(m_desktopName);
  m_lineEdit->show();
  m_lineEdit->setFocus();
  m_lineEdit->selectAll();
  m_pager->emitRequestFocus();
}

void KMiniPagerButton::slotToggled( bool b )
{
    if ( !b && m_lineEdit )
    {
        m_lineEdit->hide();
    }
}

bool KMiniPagerButton::eventFilter( QObject *o, QEvent * e)
{
    if (o && o == m_lineEdit &&
        (e->type() == QEvent::FocusOut || e->type() == QEvent::Hide))
    {
        m_pager->kwin()->setDesktopName( m_desktop, m_lineEdit->text() );
        m_desktopName = m_lineEdit->text();
        QTimer::singleShot( 0, m_lineEdit, SLOT( deleteLater() ) );
        m_lineEdit = 0;
        return true;
    }

    return QButton::eventFilter(o, e);
}

void KMiniPagerButton::updateKickerTip(KickerTip::Data &data)
{
    Task::Dict tasks = TaskManager::the()->tasks();
    Task::Dict::iterator taskEnd = tasks.end();
    uint taskCounter = 0;
    uint taskLimiter = 4;
    QString lastWindow;

    for (Task::Dict::iterator it = tasks.begin(); it != taskEnd; ++it)
    {
        if (it.data()->desktop() == m_desktop || it.data()->isOnAllDesktops())
        {
            taskCounter++;
            if (taskCounter > taskLimiter)
            {
                lastWindow = it.data()->visibleName();
                continue;
            }

            QPixmap winIcon = it.data()->pixmap();
            QString bullet;

            if (winIcon.isNull())
            {
                bullet = "&bull;";
            }
            else
            {
                data.mimeFactory->setPixmap(QString::number(taskCounter), winIcon);
                bullet = QString("<img src=\"%1\" width=\"%2\" height=\"%3\">").arg(taskCounter).arg(16).arg(16);
            }

            QString name = KStringHandler::cPixelSqueeze(it.data()->visibleName(), fontMetrics(), 400);
            name = QStyleSheet::escape(name);
            if (it.data() == m_currentWindow)
            {
                data.subtext.append(QString("<br>%1&nbsp; <u>").arg(bullet));
                data.subtext.append(name).append("</u>");
            }
            else
            {
                data.subtext.append(QString("<br>%1&nbsp; ").arg(bullet));
                data.subtext.append(name);
            }
        }
    }

    if (taskCounter > taskLimiter)
    {
        if (taskCounter - taskLimiter == 1)
        {
            data.subtext.append("<br>&bull; ").append(lastWindow);
        }
        else
        {
            data.subtext.append("<br>&bull; <i>")
                        .append(i18n("and 1 other", "and %n others", taskCounter - taskLimiter))
                        .append("</i>");
        }
    }

    if (taskCounter > 0)
    {
        data.subtext.prepend(i18n("One window:",
                                  "%n windows:",
                                  taskCounter));
    }

    data.duration = 4000;
    data.icon = DesktopIcon("window_list", KIcon::SizeMedium);
    data.message = QStyleSheet::escape(m_desktopName);
    data.direction = m_pager->popupDirection();
}

