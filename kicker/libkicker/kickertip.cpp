/*****************************************************************

Copyright (c) 2004 Zack Rusin <zrusin@kde.org>
                   Sami Kyostil <skyostil@kempele.fi>
                   Aaron J. Seigo <aseigo@kde.org>

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

#include <qapplication.h>
#include <qpainter.h>
#include <qsimplerichtext.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <kdialog.h>

#include "global.h"

#include "kickertip.h"
#include "kickerSettings.h"

// putting this #include higher results in compile errors
#include <netwm.h>

static const int DEFAULT_FRAMES_PER_SECOND = 30;

KickerTip* KickerTip::m_self = 0;
int KickerTip::m_tippingEnabled = 1;

void KickerTip::Client::updateKickerTip() const
{
    if (KickerTip::the()->isTippingFor(dynamic_cast<const QWidget*>(this)) &&
        KickerTip::the()->isVisible())
    {
        KickerTip::the()->display();
    }
}

KickerTip* KickerTip::the()
{
    if (!m_self)
    {
        m_self = new KickerTip(0);
    }

    return m_self;
}

KickerTip::KickerTip(QWidget * parent)
    : QWidget(parent, "animtt",WX11BypassWM),
      m_richText(0),
      m_mimeFactory(0),
      m_dissolveSize(0),
      m_dissolveDelta(-1),
      m_direction(KPanelApplet::Up),
      m_dirty(false),
      m_toolTipsEnabled(KickerSettings::showToolTips()),
      m_tippingFor(0)
{
    setFocusPolicy(NoFocus);
    setBackgroundMode(NoBackground);
    resize(0, 0);
    hide();
    connect(&m_frameTimer, SIGNAL(timeout()), SLOT(internalUpdate()));
}

KickerTip::~KickerTip()
{
    delete m_richText;
    delete m_mimeFactory;
}

void KickerTip::display()
{
    if (!tippingEnabled())
    {
        return;
    }

    {
        // prevent tips from showing when the active window is fullscreened
        NETRootInfo ri(qt_xdisplay(), NET::ActiveWindow);
        NETWinInfo wi(qt_xdisplay(), ri.activeWindow(), ri.rootWindow(), NET::WMState);
        if (wi.state() & NET::FullScreen)
        {
            return;
        }
    }

    QWidget *widget = const_cast<QWidget*>(m_tippingFor);
    KickerTip::Client *client = dynamic_cast<KickerTip::Client*>(widget);

    if (!client)
    {
        return;
    }

    // delete the mimefactory and create a new one so any old pixmaps used in the
    // richtext area are freed but the mimefactory is ready to be added to in 
    // the call to updateKickerTip
    delete m_mimeFactory;
    m_mimeFactory = new QMimeSourceFactory();

    // Declare interchange object and define defaults.
    Data data;
    data.maskEffect = Dissolve;
    data.duration = 2000;
    data.direction = KPanelApplet::Up;
    data.mimeFactory = m_mimeFactory;

    // Tickle the information out of the bastard.
    client->updateKickerTip(data);

    if (data.message.isEmpty() && data.subtext.isEmpty() && data.icon.isNull())
    {
        return;
    }

    delete m_richText;
    m_richText = new QSimpleRichText("<qt><h3>" + data.message + "</h3><p>" +
                                     data.subtext + "</p></qt>", font(), QString::null, 0,
                                     m_mimeFactory);
    m_richText->setWidth(400);
    m_direction = data.direction;

    if (KickerSettings::mouseOversShowIcon())
    {
        m_icon = data.icon;
    }
    else if (KickerSettings::mouseOversShowText())
    {
        m_icon = QPixmap();
    }
    else
    {
        // don't bother since we have NOTHING to show
        return;
    }

    m_maskEffect = isVisible() ? Plain : data.maskEffect;
    m_dissolveSize = 24;
    m_dissolveDelta = -1;

    displayInternal();

    m_frameTimer.start(1000 / DEFAULT_FRAMES_PER_SECOND);

    // close the message window after given mS
    if (data.duration > 0)
    {
        disconnect(&m_timer, SIGNAL(timeout()), 0, 0);
        connect(&m_timer, SIGNAL(timeout()), SLOT(hide()));
        m_timer.start(data.duration, true);
    }
    else
    {
        m_timer.stop();
    }

    move(KickerLib::popupPosition(m_direction, this, m_tippingFor));
    show();
}

void KickerTip::paintEvent(QPaintEvent * e)
{
    if (m_dirty)
    {
        displayInternal();
        m_dirty = false;
    }

    QPainter p(this);
    p.drawPixmap(e->rect().topLeft(), m_pixmap, e->rect());
}

void KickerTip::mousePressEvent(QMouseEvent * /*e*/)
{
    QToolTip::setGloballyEnabled(m_toolTipsEnabled);
    m_timer.stop();
    m_frameTimer.stop();
    hide();
}

void KickerTip::plainMask()
{
    QPainter maskPainter(&m_mask);

    m_mask.fill(Qt::black);

    maskPainter.setBrush(Qt::white);
    maskPainter.setPen(Qt::white);
    maskPainter.drawRoundRect(m_mask.rect(), 1600 / m_mask.rect().width(),
                              1600 / m_mask.rect().height());
    setMask(m_mask);
    m_frameTimer.stop();
}

void KickerTip::dissolveMask()
{
    QPainter maskPainter(&m_mask);

    m_mask.fill(Qt::black);

    maskPainter.setBrush(Qt::white);
    maskPainter.setPen(Qt::white);
    maskPainter.drawRoundRect(m_mask.rect(), 1600 / m_mask.rect().width(),
                              1600 / m_mask.rect().height());

    m_dissolveSize += m_dissolveDelta;

    if (m_dissolveSize > 0)
    {
        maskPainter.setRasterOp(Qt::EraseROP);

        int x, y, s;
        const int size = 16;

        for (y = 0; y < height() + size; y += size)
        {
            x = width();
            s = m_dissolveSize * x / 128;
            for (; x > -size; x -= size, s -= 2)
            {
                if (s < 0)
                {
                    break;
                }
                maskPainter.drawEllipse(x - s / 2, y - s / 2, s, s);
            }
        }
    }
    else if (m_dissolveSize < 0)
    {
        m_frameTimer.stop();
        m_dissolveDelta = 1;
    }

    setMask(m_mask);
}

void KickerTip::displayInternal()
{
    // we need to check for m_tippingFor here as well as m_richText
    // since if one is really persistant and moves the mouse around very fast
    // you can trigger a situation where m_tippingFor gets reset to 0 but
    // before display() is called!
    if (!m_tippingFor || !m_richText)
    {
        return;
    }

    // determine text rectangle
    QRect textRect(0, 0, 0, 0);
    if (KickerSettings::mouseOversShowText())
    {
        textRect.setWidth(m_richText->widthUsed());
        textRect.setHeight(m_richText->height());
    }

    int margin = KDialog::marginHint();
    int height = QMAX(m_icon.height(), textRect.height()) + 2 * margin;
    int textX = m_icon.isNull() ? margin : 2 + m_icon.width() + 2 * margin;
    int width = textX + textRect.width() + margin;
    int textY = (height - textRect.height()) / 2;

    // resize pixmap, mask and widget
    bool firstTime = m_dissolveSize == 24;
    if (firstTime)
    {
        m_mask.resize(width, height);
        m_pixmap.resize(width, height);
        resize(width, height);
        if (isVisible())
        {
            // we've already been shown before, but we may grow larger.
            // in the case of Up or Right displaying tips, this growth can
            // result in the tip occluding the panel and causing it to redraw
            // once we return back to display() causing horrid flicker
            move(KickerLib::popupPosition(m_direction, this, m_tippingFor));
        }
    }

    // create and set transparency mask
    switch(m_maskEffect)
    {
        case Plain:
            plainMask();
            break;

        case Dissolve:
            dissolveMask();
            break;
    }

    // draw background
    QPainter bufferPainter(&m_pixmap);
    bufferPainter.setPen(Qt::black);
    bufferPainter.setBrush(colorGroup().background());
    bufferPainter.drawRoundRect(0, 0, width, height,
                                1600 / width, 1600 / height);

    // draw icon if present
    if (!m_icon.isNull())
    {
        bufferPainter.drawPixmap(margin,
                                 margin,
                                 m_icon, 0, 0,
                                 m_icon.width(), m_icon.height());
    }

    if (KickerSettings::mouseOversShowText())
    {
        // draw text shadow
        QColorGroup cg = colorGroup();
        cg.setColor(QColorGroup::Text, cg.background().dark(115));
        int shadowOffset = QApplication::reverseLayout() ? -1 : 1;
        m_richText->draw(&bufferPainter, 5 + textX + shadowOffset, textY + 1, QRect(), cg);

        // draw text
        cg = colorGroup();
        m_richText->draw(&bufferPainter, 5 + textX, textY, rect(), cg);
    }
}

void KickerTip::tipFor(const QWidget* w)
{
    if (m_tippingFor)
    {
        disconnect(m_tippingFor, SIGNAL(destroyed(QObject*)),
                   this, SLOT(tipperDestroyed(QObject*)));
    }

    m_tippingFor = w;

    if (m_tippingFor)
    {
        connect(m_tippingFor, SIGNAL(destroyed(QObject*)),
                this, SLOT(tipperDestroyed(QObject*)));
    }
}

void KickerTip::untipFor(const QWidget* w)
{
    if (isTippingFor(w))
    {
        tipFor(0);
        m_timer.stop();
        hide();
    }
}

bool KickerTip::isTippingFor(const QWidget* w) const
{
    return m_tippingFor == w;
}

void KickerTip::tipperDestroyed(QObject* o)
{
    // we can't do a dynamic cast because we are in the process of dieing
    // so static it is.
    untipFor(static_cast<QWidget*>(o));
}

void KickerTip::internalUpdate()
{
    m_dirty = true;
    repaint(false);
}

void KickerTip::enableTipping(bool tip)
{
    if (tip)
    {
        m_tippingEnabled++;
    }
    else
    {
        m_tippingEnabled--;
    }

    if (m_tippingEnabled < 1 && m_self)
    {
        m_self->hide();
    }
}

bool KickerTip::tippingEnabled()
{
    return m_tippingEnabled > 0;
}

void KickerTip::hide()
{
    m_tippingFor = 0;
    m_frameTimer.stop();
    QWidget::hide();
}

bool KickerTip::eventFilter(QObject *object, QEvent *event)
{
    if (!tippingEnabled())
    {
        return false;
    }

    if (!object->isWidgetType())
    {
        return false;
    }

    QWidget *widget = static_cast<QWidget*>(object);

    switch (event->type())
    {
        case QEvent::Enter:
            if (!KickerSettings::showMouseOverEffects())
            {
                return false;
            }

            if (!mouseGrabber() &&
                !qApp->activePopupWidget() &&
                !isTippingFor(widget))
            {
                m_toolTipsEnabled = QToolTip::isGloballyEnabled();
                QToolTip::setGloballyEnabled(false);

                tipFor(widget);
                m_timer.stop();
                disconnect(&m_timer, SIGNAL(timeout()), 0, 0);
                connect(&m_timer, SIGNAL(timeout()), SLOT(display()));

                // delay to avoid false starts
                // e.g. when the user quickly zooms their mouse over
                // a button then out of kicker
                if (isVisible())
                {
                    m_timer.start(150, true);
                }
                else
                {
                    m_timer.start(KickerSettings::mouseOversShowDelay(), true);
                }
            }
            break;
        case QEvent::Leave:
            QToolTip::setGloballyEnabled(m_toolTipsEnabled);

            m_timer.stop();

            if (isTippingFor(widget) && isVisible())
            {
                disconnect(&m_timer, SIGNAL(timeout()), 0, 0);
                connect(&m_timer, SIGNAL(timeout()), SLOT(hide()));
                m_timer.start(KickerSettings::mouseOversHideDelay(), true);
            }

            tipFor(0);
            break;
        case QEvent::MouseButtonPress:
            QToolTip::setGloballyEnabled(m_toolTipsEnabled);
            m_timer.stop();
            m_frameTimer.stop();
            hide();
        default:
            break;
    }

    return false;
}

#include <kickertip.moc>

