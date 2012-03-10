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

#include <qpopupmenu.h>

#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>

#include "global.h"
#include "appletop_mnu.h"
#include "kicker.h"

#include "container_base.h"
#include "container_base.moc"

BaseContainer::BaseContainer( QPopupMenu* appletOpMenu, QWidget* parent, const char * name )
  : QWidget( parent, name )
  , _dir(KPanelApplet::Up)
  , _orient(Horizontal)
  , _alignment(KPanelExtension::LeftTop)
  , _fspace(0)
  , _moveOffset(QPoint(0,0))
  , _aid(QString::null)
  , _actions(0)
  , m_immutable(false)
  , _opMnu(0)
  , _appletOpMnu(appletOpMenu)
{}

BaseContainer::~BaseContainer()
{
    delete _opMnu;
}

void BaseContainer::reparent(QWidget* parent, WFlags f, const QPoint& p, bool showIt)
{
    emit takeme(this);
    QWidget::reparent(parent, f, p, showIt);
}

bool BaseContainer::isImmutable() const
{
    return m_immutable || Kicker::the()->isImmutable();
}

void BaseContainer::setImmutable(bool immutable)
{
    m_immutable = immutable;
    clearOpMenu();
}

void BaseContainer::loadConfiguration( KConfigGroup& group )
{
    setFreeSpace( QMIN( group.readDoubleNumEntry( "FreeSpace2", 0 ), 1 ) );
    doLoadConfiguration( group );
}

void BaseContainer::saveConfiguration(KConfigGroup& group,
                                      bool layoutOnly) const
{
    if (isImmutable())
    {
        return;
    }

    // write positioning info
    group.writeEntry( "FreeSpace2", freeSpace() );
    // write type specific info
    doSaveConfiguration( group, layoutOnly );
}

void BaseContainer::configure(KPanelExtension::Orientation o,
                              KPanelApplet::Direction d)
{
    setBackgroundOrigin(AncestorOrigin);
    setOrientation(o);
    setPopupDirection(d);
    configure();
}

void BaseContainer::slotRemoved(KConfig* config)
{
    if (!config)
    {
        config = KGlobal::config();
    }

    config->deleteGroup(appletId().latin1());
    config->sync();
}

void BaseContainer::setAlignment(KPanelExtension::Alignment a)
{
    if (_alignment == a)
    {
        return;
    }

    _alignment = a;
    alignmentChange(a);
}

QPopupMenu* BaseContainer::opMenu()
{
    if (_opMnu == 0)
    {
        _opMnu = createOpMenu();
    }

    return KickerLib::reduceMenu(_opMnu);
}

void BaseContainer::clearOpMenu()
{
    delete _opMnu;
    _opMnu = 0;
}

