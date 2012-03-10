/* This file is part of the KDE project
   Copyright (C) 2003-2004 Nadeem Hasan <nhasan@kde.org>
   Copyright (C) 2004-2005 Aaron J. Seigo <aseigo@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "simplebutton.h"

#include <qpainter.h>
#include <qstyle.h>

#include <kapplication.h>
#include <kcursor.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kicontheme.h>
#include <kipc.h>
#include <kstandarddirs.h>

SimpleButton::SimpleButton(QWidget *parent, const char *name)
    : QButton(parent, name),
      m_highlight(false),
      m_orientation(Qt::Horizontal)
{
    setBackgroundOrigin( AncestorOrigin );

    connect( kapp, SIGNAL( settingsChanged( int ) ),
       SLOT( slotSettingsChanged( int ) ) );
    connect( kapp, SIGNAL( iconChanged( int ) ),
       SLOT( slotIconChanged( int ) ) );

    kapp->addKipcEventMask( KIPC::SettingsChanged );
    kapp->addKipcEventMask( KIPC::IconChanged );

    slotSettingsChanged( KApplication::SETTINGS_MOUSE );
}

void SimpleButton::setPixmap(const QPixmap &pix)
{
    QButton::setPixmap(pix);
    generateIcons();
    update();
}

void SimpleButton::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    update();
}

QSize SimpleButton::sizeHint() const
{
    const QPixmap* pm = pixmap();

    if (!pm)
        return QButton::sizeHint();
    else
        return QSize(pm->width() + KDialog::spacingHint(), pm->height() + KDialog::spacingHint());
}

QSize SimpleButton::minimumSizeHint() const
{
    const QPixmap* pm = pixmap();

    if (!pm)
        return QButton::minimumSizeHint();
    else
        return QSize(pm->width(), pm->height());
}

void SimpleButton::drawButton( QPainter *p )
{
    drawButtonLabel(p);
}

void SimpleButton::drawButtonLabel( QPainter *p )
{
    if (!pixmap())
    {
        return;
    }

    QPixmap pix = isEnabled() ? (m_highlight? m_activeIcon : m_normalIcon) : m_disabledIcon;

    if (isOn() || isDown())
    {
        pix = pix.convertToImage().smoothScale(pix.width() - 2,
                                               pix.height() - 2);
    }

    int h = height();
    int w = width();
    int ph = pix.height();
    int pw = pix.width();
    int margin = KDialog::spacingHint();
    QPoint origin(margin / 2, margin / 2);

    if (ph < (h - margin))
    {
        origin.setY((h - ph) / 2);
    }

    if (pw < (w - margin))
    {
        origin.setX((w - pw) / 2);
    }

    p->drawPixmap(origin, pix);
}

void SimpleButton::generateIcons()
{
    if (!pixmap())
    {
        return;
    }

    QImage image = pixmap()->convertToImage();
    KIconEffect effect;

    m_normalIcon = effect.apply(image, KIcon::Panel, KIcon::DefaultState);
    m_activeIcon = effect.apply(image, KIcon::Panel, KIcon::ActiveState);
    m_disabledIcon = effect.apply(image, KIcon::Panel, KIcon::DisabledState);
    
    updateGeometry();
}

void SimpleButton::slotSettingsChanged(int category)
{
    if (category != KApplication::SETTINGS_MOUSE)
    {
        return;
    }

    bool changeCursor = KGlobalSettings::changeCursorOverIcon();

    if (changeCursor)
    {
        setCursor(KCursor::handCursor());
    }
    else
    {
        unsetCursor();
    }
}

void SimpleButton::slotIconChanged( int group )
{
    if (group != KIcon::Panel)
    {
        return;
    }

    generateIcons();
    update();
}

void SimpleButton::enterEvent( QEvent *e )
{
    m_highlight = true;

    repaint( false );
    QButton::enterEvent( e );
}

void SimpleButton::leaveEvent( QEvent *e )
{
    m_highlight = false;

    repaint( false );
    QButton::enterEvent( e );
}

void SimpleButton::resizeEvent( QResizeEvent * )
{
    generateIcons();
}


SimpleArrowButton::SimpleArrowButton(QWidget *parent, Qt::ArrowType arrow, const char *name)
    : SimpleButton(parent, name)
{
    setBackgroundOrigin(AncestorOrigin);
    _arrow = arrow;
    _inside = false;
}

QSize SimpleArrowButton::sizeHint() const
{
    return QSize( 12, 12 );
}

void SimpleArrowButton::setArrowType(Qt::ArrowType a)
{
    if (_arrow != a) 
    {
        _arrow = a;
        update();
    }
}

Qt::ArrowType SimpleArrowButton::arrowType() const
{
    return _arrow;
}

void SimpleArrowButton::drawButton( QPainter *p )
{
    QRect r(1, 1, width() - 2, height() - 2);
    
    QStyle::PrimitiveElement pe = QStyle::PE_ArrowLeft;
    switch (_arrow)
    {
        case Qt::LeftArrow: pe = QStyle::PE_ArrowLeft; break;
        case Qt::RightArrow: pe = QStyle::PE_ArrowRight; break;
        case Qt::UpArrow: pe = QStyle::PE_ArrowUp; break;
        case Qt::DownArrow: pe = QStyle::PE_ArrowDown; break;
    }
    
    int flags = QStyle::Style_Default | QStyle::Style_Enabled;
    if (isDown() || isOn())	flags |= QStyle::Style_Down;
    style().drawPrimitive(pe, p, r, colorGroup(), flags);
}

void SimpleArrowButton::enterEvent( QEvent *e )
{
    _inside = true;
    SimpleButton::enterEvent( e );
    update();
}

void SimpleArrowButton::leaveEvent( QEvent *e )
{
    _inside = false;
    SimpleButton::enterEvent( e );
    update();
}

#include "simplebutton.moc"

// vim:ts=4:sw=4:et
