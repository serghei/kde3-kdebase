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

#include <qlayout.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qstyle.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "simplebutton.h"
#include "panner.h"
#include "panner.moc"


Panner::Panner( QWidget* parent, const char* name )
    : QScrollView( parent, name ),
      _luSB(0),
      _rdSB(0)
{
    KGlobal::locale()->insertCatalogue("libkicker");
    setBackgroundOrigin( AncestorOrigin );

    _updateScrollButtonsTimer = new QTimer(this);
    connect(_updateScrollButtonsTimer, SIGNAL(timeout()), this, SLOT(reallyUpdateScrollButtons()));
    
    setResizePolicy(Manual);
    setVScrollBarMode( QScrollView::AlwaysOff );
    setHScrollBarMode( QScrollView::AlwaysOff );

    viewport()->setBackgroundMode( PaletteBackground );
    viewport()->setBackgroundOrigin( AncestorOrigin );

    // layout
    _layout = new QBoxLayout(this, QBoxLayout::LeftToRight);
    _layout->addWidget(viewport(), 1);
    setOrientation(Horizontal);
}

Panner::~Panner() 
{
}

void Panner::setupButtons()
{
    if (orientation() == Horizontal)
    {
        if (_luSB)
        {
            _luSB->setArrowType(Qt::LeftArrow);
            _rdSB->setArrowType(Qt::RightArrow);
            _luSB->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
            _rdSB->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
            QToolTip::add(_luSB, i18n("Scroll left"));
            QToolTip::add(_rdSB, i18n("Scroll right"));
            setMinimumSize(24, 0);
        }
        _layout->setDirection(QBoxLayout::LeftToRight);
    }
    else
    {
        if (_luSB)
        {
            _luSB->setArrowType(Qt::UpArrow);
            _rdSB->setArrowType(Qt::DownArrow);
            _luSB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
            _rdSB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
            QToolTip::add(_luSB, i18n("Scroll up"));
            QToolTip::add(_rdSB, i18n("Scroll down"));
            setMinimumSize(0, 24);
        }
        _layout->setDirection(QBoxLayout::TopToBottom);
    }

    if (isVisible())
    {
        // we need to manually redo the layout if we are visible
        // otherwise let the toolkit decide when to do this
        _layout->activate();
    }
}

void Panner::setOrientation(Orientation o)
{
    _orient = o;
    setupButtons();
    reallyUpdateScrollButtons();
}

void Panner::resizeEvent( QResizeEvent* e )
{
    QScrollView::resizeEvent( e );
    updateScrollButtons();
}

void Panner::scrollRightDown()
{
    if(orientation() == Horizontal) // scroll right
        scrollBy( 40, 0 );
    else // scroll down
        scrollBy( 0, 40 );
}

void Panner::scrollLeftUp()
{
    if(orientation() == Horizontal) // scroll left
        scrollBy( -40, 0 );
    else // scroll up
        scrollBy( 0, -40 );
}

void Panner::createScrollButtons()
{
    if (_luSB)
    {
        return;
    }

    // left/up scroll button
    _luSB = new SimpleArrowButton(this);
    _luSB->installEventFilter(this);
    _luSB->setAutoRepeat(true);
    _luSB->setMinimumSize(12, 12);
    _luSB->hide();
    _layout->addWidget(_luSB);
    connect(_luSB, SIGNAL(clicked()), SLOT(scrollLeftUp()));

    // right/down scroll button
    _rdSB = new SimpleArrowButton(this);
    _rdSB->installEventFilter(this);
    _rdSB->setAutoRepeat(true);
    _rdSB->setMinimumSize(12, 12);
    _rdSB->hide();
    _layout->addWidget(_rdSB);
    connect(_rdSB, SIGNAL(clicked()), SLOT(scrollRightDown()));

    // set up the buttons
    setupButtons();
}

void Panner::reallyUpdateScrollButtons()
{
    int delta = 0;
    
    _updateScrollButtonsTimer->stop();

    if (orientation() == Horizontal)
    {
        delta = contentsWidth() - width();
    }
    else
    {
        delta = contentsHeight() - height();
    }

    if (delta > 1)
    {
        createScrollButtons();

        // since the buttons may be visible but of the wrong size
        // we need to do this every single time
        _luSB->show();
        _rdSB->show();

        if (orientation() == Horizontal)
        {
            setMargins(0, 0, _luSB->width() + _rdSB->width(), 0);
        }
        else
        {
            setMargins(0, 0, 0, _luSB->height() + _rdSB->height());
        }
    }
    else if (_luSB && _luSB->isVisibleTo(this))
    {
        _luSB->hide();
        _rdSB->hide();
        setMargins(0, 0, 0, 0);
    }
}

void Panner::updateScrollButtons()
{
    _updateScrollButtonsTimer->start(200, true);
}

void Panner::resizeContents( int w, int h )
{
    QScrollView::resizeContents( w, h );
    updateScrollButtons();
}
