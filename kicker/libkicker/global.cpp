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
#include <qbitmap.h>
#include <qfile.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qpainter.h>

#include <kiconeffect.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <kservice.h>

#include "global.h"
#include "kickerSettings.h"

namespace KickerLib
{

KPanelExtension::Position directionToPosition(KPanelApplet::Direction d )
{
    switch (d)
    {
        case KPanelApplet::Down:
            return KPanelExtension::Top;
        break;

        case KPanelApplet::Left:
            return KPanelExtension::Right;
        break;

        case KPanelApplet::Right:
            return KPanelExtension::Left;
        break;

        case KPanelApplet::Up:
        default:
            return KPanelExtension::Bottom;
        break;
    }
}

KPanelExtension::Position directionToPopupPosition(KPanelApplet::Direction d)
{
    switch (d)
    {
        case KPanelApplet::Up:
            return KPanelExtension::Top;
        break;

        case KPanelApplet::Down:
            return KPanelExtension::Bottom;
        break;

        case KPanelApplet::Left:
            return KPanelExtension::Left;
        break;

        case KPanelApplet::Right:
        default:
            return KPanelExtension::Right;
        break;
    }
}

KPanelApplet::Direction positionToDirection(KPanelExtension::Position p)
{
    switch (p)
    {
        case KPanelExtension::Top:
            return KPanelApplet::Down;
        break;

        case KPanelExtension::Right:
            return KPanelApplet::Left;
        break;

        case KPanelExtension::Left:
            return KPanelApplet::Right;
        break;

        case KPanelExtension::Bottom:
        default:
            return KPanelApplet::Up;
        break;
    }
}

KPanelApplet::Direction arrowToDirection(Qt::ArrowType p)
{
    switch (p)
    {
        case Qt::DownArrow:
            return KPanelApplet::Down;
        break;

        case Qt::LeftArrow:
            return KPanelApplet::Left;
        break;

        case Qt::RightArrow:
            return KPanelApplet::Right;
        break;

        case Qt::UpArrow:
        default:
            return KPanelApplet::Up;
        break;
    }
}

int sizeValue(KPanelExtension::Size s)
{
    switch (s)
    {
        case KPanelExtension::SizeTiny:
            return 24;
        break;

        case KPanelExtension::SizeSmall:
            return 30;
        break;

        case KPanelExtension::SizeNormal:
            return 46;
        break;

        case KPanelExtension::SizeLarge:
        default:
            return 58;
        break;
    }
}

int maxButtonDim()
{
    return (2 * KickerSettings::iconMargin()) + KIcon::SizeLarge;
}

QString newDesktopFile(const KURL& url)
{
   QString base = url.fileName();
   if (base.endsWith(".desktop"))
      base.truncate(base.length()-8);
   QRegExp r("(.*)(?=-\\d+)");
   if (r.search(base) > -1)
      base = r.cap(1);

   QString file = base + ".desktop";

   for(int n = 1; ++n; )
   {
      QString path = locate("appdata", file);
      if (path.isEmpty())
         break;

      file = QString("%2-%1.desktop").arg(n).arg(base);
   }
   file = locateLocal("appdata", file);
   return file;
}

QString copyDesktopFile(const KURL& url)
{
   QString file = newDesktopFile(url);
   KURL dest;
   dest.setPath(file);
   KIO::NetAccess::upload(url.path(), dest, 0);
   return file;
}

QPopupMenu* reduceMenu(QPopupMenu *menu)
{
    if (menu->count() != 1)
    {
       return menu;
    }

    QMenuItem *item = menu->findItem(menu->idAt(0));

    if (item->popup())
    {
       return reduceMenu(item->popup());
    }

    return menu;
}

QPoint popupPosition(KPanelApplet::Direction d,
                     const QWidget* popup,
                     const QWidget* source,
                     const QPoint& offset)
{
    QRect r;
    if (source->isTopLevel())
    {
        r = source->geometry();
    }
    else
    {
        r = QRect(source->mapToGlobal(QPoint(0, 0)),
                  source->mapToGlobal(QPoint(source->width(), source->height())));

        switch (d)
        {
            case KPanelApplet::Left:
            case KPanelApplet::Right:
                r.setLeft( source->topLevelWidget()->x() );
                r.setWidth( source->topLevelWidget()->width() );
                break;
            case KPanelApplet::Up:
            case KPanelApplet::Down:
                r.setTop( source->topLevelWidget()->y() );
                r.setHeight( source->topLevelWidget()->height() );
                break;
        }
    }

    switch (d)
    {
        case KPanelApplet::Left:
        case KPanelApplet::Right:
        {
            QDesktopWidget* desktop = QApplication::desktop();
            QRect screen = desktop->screenGeometry(desktop->screenNumber(const_cast<QWidget*>(source)));
            int x = (d == KPanelApplet::Left) ? r.left() - popup->width() :
                                                r.right() + 1;
            int y = r.top() + offset.y();

            // try to keep this on screen
            if (y + popup->height() > screen.bottom())
            {
                y = r.bottom() - popup->height() + offset.y();

                if (y < screen.top())
                {
                    y = screen.bottom() - popup->height();

                    if (y < screen.top())
                    {
                        y = screen.top();
                    }
                }
            }

            return QPoint(x, y);
        }
        case KPanelApplet::Up:
        case KPanelApplet::Down:
        default:
        {
            int x = 0;
            int y = (d == KPanelApplet::Up) ? r.top() - popup->height() :
                                              r.bottom() + 1;

            if (QApplication::reverseLayout())
            {
                x = r.right() - popup->width() + 1;

                if (offset.x() > 0)
                {
                    x -= r.width() - offset.x();
                }

                // try to keep this on the screen
                if (x - popup->width() < 0)
                {
                    x = r.left();
                }

                return QPoint(x, y);
            }
            else
            {
                QDesktopWidget* desktop = QApplication::desktop();
                QRect screen = desktop->screenGeometry(desktop->screenNumber(const_cast<QWidget*>(source)));
                x = r.left() + offset.x();

                // try to keep this on the screen
                if (x + popup->width() > screen.right())
                {
                    x = r.right() - popup->width() + 1 + offset.x();

                    if (x < screen.left())
                    {
                        x = screen.left();
                    }
                }
            }

            return QPoint(x, y);
        }
    }
}

void colorize(QImage& image)
{
    KConfig *config = KGlobal::config();
    config->setGroup("WM");
    QColor color = QApplication::palette().active().highlight();
    QColor activeTitle = config->readColorEntry("activeBackground", &color);
    QColor inactiveTitle = config->readColorEntry("inactiveBackground", &color);

    // figure out which color is most suitable for recoloring to
    int h1, s1, v1, h2, s2, v2, h3, s3, v3;
    activeTitle.hsv(&h1, &s1, &v1);
    inactiveTitle.hsv(&h2, &s2, &v2);
    QApplication::palette().active().background().hsv(&h3, &s3, &v3);

    if ( (kAbs(h1-h3)+kAbs(s1-s3)+kAbs(v1-v3) < kAbs(h2-h3)+kAbs(s2-s3)+kAbs(v2-v3)) &&
            ((kAbs(h1-h3)+kAbs(s1-s3)+kAbs(v1-v3) < 32) || (s1 < 32)) && (s2 > s1))
        color = inactiveTitle;
    else
        color = activeTitle;

    // limit max/min brightness
    int r, g, b;
    color.rgb(&r, &g, &b);
    int gray = qGray(r, g, b);
    if (gray > 180) {
        r = (r - (gray - 180) < 0 ? 0 : r - (gray - 180));
        g = (g - (gray - 180) < 0 ? 0 : g - (gray - 180));
        b = (b - (gray - 180) < 0 ? 0 : b - (gray - 180));
    } else if (gray < 76) {
        r = (r + (76 - gray) > 255 ? 255 : r + (76 - gray));
        g = (g + (76 - gray) > 255 ? 255 : g + (76 - gray));
        b = (b + (76 - gray) > 255 ? 255 : b + (76 - gray));
    }
    color.setRgb(r, g, b);
    KIconEffect::colorize(image, color, 1.0);
}

QColor blendColors(const QColor& c1, const QColor& c2)
{
    int r1, g1, b1;
    int r2, g2, b2;

    c1.rgb(&r1, &g1, &b1);
    c2.rgb(&r2, &g2, &b2);

    r1 += (int) (.5 * (r2 - r1));
    g1 += (int) (.5 * (g2 - g1));
    b1 += (int) (.5 * (b2 - b1));

    return QColor(r1, g1, b1);
}

QColor shadowColor(const QColor& c)
{
    int r = c.red();
    int g = c.green();
    int b = c.blue();

    if ( r < 128 )
        r = 255;
    else
        r = 0;

    if ( g < 128 )
        g = 255;
    else
        g = 0;

    if ( b < 128 )
        b = 255;
    else
        b = 0;

    return QColor( r, g, b );
}

QIconSet menuIconSet(const QString& icon)
{
    QIconSet iconset;
    int iconSize = KickerSettings::menuEntryHeight();

    if (iconSize < 0)
    {
        return iconset;
    }

    if (icon != "unknown")
    {
        if (iconSize > 0)
        {
            iconset = KGlobal::iconLoader()->loadIconSet(icon,
                                                     KIcon::NoGroup,
                                                     iconSize, true);
        }
        else if (iconSize == 0)
        {
            QPixmap normal = KGlobal::iconLoader()->loadIcon(icon,
                                                         KIcon::Small,
                                                         0,
                                                         KIcon::DefaultState,
                                                         0,
                                                         true);

            QPixmap active = KGlobal::iconLoader()->loadIcon(icon,
                                                         KIcon::Small,
                                                         0,
                                                         KIcon::ActiveState,
                                                         0,
                                                         true);

            // make sure they are not larger than 20x20
            if (normal.width() > 20 || normal.height() > 20)
            {
                normal.convertFromImage(normal.convertToImage().smoothScale(20,20));
            }

            if (active.width() > 20 || active.height() > 20)
            {
                active.convertFromImage(active.convertToImage().smoothScale(20,20));
            }

            iconset.setPixmap(normal, QIconSet::Small, QIconSet::Normal);
            iconset.setPixmap(active, QIconSet::Small, QIconSet::Active);
        }
    }

    if (iconset.isNull())
    {
        QPixmap pix(iconSize, iconSize);
        QBitmap map(iconSize, iconSize, true);
        pix.setMask(map);
        iconset = QIconSet(pix, pix);
    }

    return iconset;
}

void drawBlendedRect(QPainter *p, const QRect &r, const QColor &color, int alpha)
{
    static QPixmap pix;
    static QColor last_color = Qt::black;
    static int last_alpha = 0;
    
    if (pix.isNull() || last_color != color || last_alpha != alpha)
    {
        QImage img(16, 16, 32);
        img.setAlphaBuffer(false);
        img.fill(((uint)(alpha & 0xFF) << 24) | (color.rgb() & 0xFFFFFF));
        img.setAlphaBuffer(true);
        pix.convertFromImage(img);
        last_color = color;
        last_alpha = alpha;
    }

    p->drawTiledPixmap(r, pix);
}

} // namespace

