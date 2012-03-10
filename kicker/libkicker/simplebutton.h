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

#ifndef SIMPLEBUTTON_H
#define SIMPLEBUTTON_H

#include <qbutton.h>
#include <qpixmap.h>

#include <kdemacros.h>

class KDE_EXPORT SimpleButton : public QButton
{
    Q_OBJECT

    public:
        SimpleButton(QWidget *parent, const char *name = 0);
        void setPixmap(const QPixmap &pix);
        void setOrientation(Qt::Orientation orientaton);
        QSize sizeHint() const;
        QSize minimumSizeHint() const;

    protected:
        void drawButton( QPainter *p );
        void drawButtonLabel( QPainter *p );
        void generateIcons();

        void enterEvent( QEvent *e );
        void leaveEvent( QEvent *e );
        void resizeEvent( QResizeEvent *e );

    protected slots:
        virtual void slotSettingsChanged( int category );
        virtual void slotIconChanged( int group );

    private:
        bool m_highlight;
        QPixmap m_normalIcon;
        QPixmap m_activeIcon;
        QPixmap m_disabledIcon;
        Qt::Orientation m_orientation;
        class SimpleButtonPrivate;
        SimpleButtonPrivate* d;
};

class KDE_EXPORT SimpleArrowButton: public SimpleButton
{
    Q_OBJECT
    
    public:
        SimpleArrowButton(QWidget *parent = 0, Qt::ArrowType arrow = Qt::UpArrow, const char *name = 0);
        virtual ~SimpleArrowButton() {};
        QSize sizeHint() const;
    
    protected:
        virtual void enterEvent( QEvent *e );
        virtual void leaveEvent( QEvent *e );
        virtual void drawButton(QPainter *p);
        Qt::ArrowType arrowType() const;
    
    public slots:
        void setArrowType(Qt::ArrowType a);
    
    private:
        Qt::ArrowType _arrow;
        bool _inside;
};


#endif // HIDEBUTTON_H

// vim:ts=4:sw=4:et
