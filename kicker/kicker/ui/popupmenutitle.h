/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>
                   Matthias Ettrich <ettrich@kde.org>

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

#ifndef POPUPMENUTITLE_H
#define POPUPMENUTITLE_H

#include <qfont.h>
#include <qstring.h>
#include <qstyle.h>
#include <qpainter.h>
#include <qmenudata.h>

#include <kapplication.h>

class PopupMenuTitle : public QCustomMenuItem
{
public:
    PopupMenuTitle(const QString &name, const QFont &font);

    bool fullSpan () const { return true; }

    void paint(QPainter* p, const QColorGroup& cg, 
               bool /* act */, bool /*enabled*/, 
               int x, int y, int w, int h)
    {
        p->save();
        QRect r(x, y, w, h);
        kapp->style().drawPrimitive(QStyle::PE_HeaderSection, 
                                    p, r, cg);

        if (!m_desktopName.isEmpty())
        {
            p->setPen(cg.buttonText());
            p->setFont(m_font);
            p->drawText(x, y, w, h,
                        AlignCenter | SingleLine, 
                        m_desktopName);
        }

        p->setPen(cg.highlight());
        p->drawLine(0, 0, r.right(), 0);
        p->restore();
    }

    void setFont(const QFont &font)
    {
        m_font = font;
        m_font.setBold(true);
    }

    QSize sizeHint()
    {
      QSize size = QFontMetrics(m_font).size(AlignHCenter, m_desktopName);
      size.setHeight(size.height() + 
                     (kapp->style().pixelMetric(QStyle::PM_DefaultFrameWidth) * 2 + 1));
      return size;
    }

  private:
    QString m_desktopName;
    QFont m_font;
};

#endif
