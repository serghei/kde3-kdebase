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

#include <qapplication.h>
#include <qpainter.h>

#include "userrectsel.h"
#include "userrectsel.moc"

UserRectSel::UserRectSel(const RectList& rects, const QPoint& _offset, const QColor& color)
  : QWidget(0, 0, WStyle_Customize | WX11BypassWM),
    rectangles(rects),
    offset(_offset)
{
    setGeometry(-10, -10, 2, 2);
    _color = color;
    for (int i = 0; i < 8; i++)
        _frame[i] = 0;
}

UserRectSel::~UserRectSel()
{
    for (int i = 0; i < 8; i++)
        delete _frame[i];
}

void UserRectSel::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button() == LeftButton)
    {
        qApp->exit_loop();
    }
}

void UserRectSel::mouseMoveEvent(QMouseEvent * e)
{
    PanelStrut nearest = current;
    int diff = -1;
    QPoint p = e->globalPos(); // + offset;
    for (RectList::const_iterator it = rectangles.constBegin();
         it != rectangles.constEnd();
         ++it)
    {
        PanelStrut r = *it;
        int ndiff = (r.m_rect.center() - p).manhattanLength();

        if (diff < 0 || ndiff < diff)
        {
            diff = ndiff;
            nearest = r;
        }
    }

    if (nearest != current)
    {
        paintCurrent();
        current = nearest;
        paintCurrent();
    }
}

void UserRectSel::paintCurrent()
{
    int i;
    int x, y, w, h;
    
    if (!_frame[0])
    {
        for (i = 0; i < 4; i++)
        {
            _frame[i] = new QWidget(0, 0, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WX11BypassWM);
            _frame[i]->setPaletteBackgroundColor(Qt::black);
        }
        for (i = 4; i < 8; i++)
        {
            _frame[i] = new QWidget(0, 0, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WX11BypassWM);
            _frame[i]->setPaletteBackgroundColor(_color);
        }
    }
    
    x = current.m_rect.x();
    y = current.m_rect.y();
    w = current.m_rect.width();
    h = current.m_rect.height();
    
    if (w > 0 && h > 0)
    {
        _frame[0]->setGeometry(x, y, w, 4);
        _frame[1]->setGeometry(x, y, 4, h);
        _frame[2]->setGeometry(x + w - 4, y, 4, h);
        _frame[3]->setGeometry(x, y + h - 4, w, 4);
        
        for (i = 0; i < 4; i++)
            _frame[i]->show();
    }
    
    x += 1;
    y += 1;
    w -= 2;
    h -= 2;
    
    if (w > 0 && h > 0)
    {
        _frame[4]->setGeometry(x, y, w, 2);
        _frame[5]->setGeometry(x, y, 2, h);
        _frame[6]->setGeometry(x + w - 2, y, 2, h);
        _frame[7]->setGeometry(x, y + h - 2, w, 2);
    
        for (i = 4; i < 8; i++)
            _frame[i]->show();
    }
    
}

UserRectSel::PanelStrut UserRectSel::select(const RectList& rects, const QPoint& offset, const QColor& color)
{
    UserRectSel sel(rects, offset, color);
    sel.show();
    sel.grabMouse();
    sel.paintCurrent();
    qApp->enter_loop();
    sel.paintCurrent();
    sel.releaseMouse();
    qApp->syncX();
    return sel.current;
}

