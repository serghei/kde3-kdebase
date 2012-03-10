/*****************************************************************

Copyright (c) 1996-2004 the kicker authors. See file AUTHORS.

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

#include <assert.h>
#include <math.h>

#include <qapplication.h>
#include <kdebug.h>
#include <kglobal.h>

#include "containerarea.h"
#include "containerarealayout.h"
#include "container_applet.h"
#include "container_button.h"

class ContainerAreaLayoutIterator : public QGLayoutIterator
{
    public:
        ContainerAreaLayoutIterator(ContainerAreaLayout::ItemList *l)
            : m_idx(0), m_list(l)
        {
        }

        QLayoutItem* current()
        {
            return m_idx < int(m_list->count()) ? (*m_list->at(m_idx))->item : 0;
        }

        QLayoutItem* next()
        {
            m_idx++;
            return current();
        }

        QLayoutItem* takeCurrent()
        {
            QLayoutItem* item = 0;
            ContainerAreaLayout::ItemList::iterator b = m_list->at(m_idx);
            if (b != m_list->end())
            {
                ContainerAreaLayoutItem* layoutItem = *b;
                item = layoutItem->item;
                layoutItem->item = 0;
                m_list->erase(b);
                delete layoutItem;
            }
            return item;
        }

    private:
        int m_idx;
        ContainerAreaLayout::ItemList* m_list;
};


int ContainerAreaLayoutItem::heightForWidth(int w) const
{
    BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());
    if (container)
    {
        return container->heightForWidth(w);
    }
    else
    {
        return item->sizeHint().height();
    }
}

int ContainerAreaLayoutItem::widthForHeight(int h) const
{
    BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());
    if (container)
    {
        return container->widthForHeight(h);
    }
    else
    {
        return item->sizeHint().width();
    }
}

bool ContainerAreaLayoutItem::isStretch() const
{
    BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());
    return container ? container->isStretch() : false;
}

double ContainerAreaLayoutItem::freeSpaceRatio() const
{
    BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());
    if (container)
        return kClamp(container->freeSpace(), 0.0, 1.0);
    else
        return m_freeSpaceRatio;
}

void ContainerAreaLayoutItem::setFreeSpaceRatio(double ratio)
{
    BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());
    if (container)
        container->setFreeSpace(ratio);
    else
        m_freeSpaceRatio = ratio;
}

Qt::Orientation ContainerAreaLayoutItem::orientation() const
{
    return m_layout->orientation();
}

QRect ContainerAreaLayoutItem::geometryR() const
{
    return m_layout->transform(geometry());
}

void ContainerAreaLayoutItem::setGeometryR(const QRect& r)
{
    setGeometry(m_layout->transform(r));
}

int ContainerAreaLayoutItem::widthForHeightR(int h) const
{
    if (orientation() == Horizontal)
    {
        return widthForHeight(h);
    }
    else
    {
        return heightForWidth(h);
    }
}

int ContainerAreaLayoutItem::widthR() const
{
    if (orientation() == Horizontal)
    {
        return geometry().width();
    }
    else
    {
        return geometry().height();
    }
}

int ContainerAreaLayoutItem::heightR() const
{
    if (orientation() == Horizontal)
    {
        return geometry().height();
    }
    else
    {
        return geometry().width();
    }
}

int ContainerAreaLayoutItem::leftR() const
{
    if (orientation() == Horizontal)
    {
        if (QApplication::reverseLayout())
            return m_layout->geometry().right() - geometry().right();
        else
            return geometry().left();
    }
    else
    {
        return geometry().top();
    }
}

int ContainerAreaLayoutItem::rightR() const
{
    if (orientation() == Horizontal)
    {
        if (QApplication::reverseLayout())
            return m_layout->geometry().right() - geometry().left();
        else
            return geometry().right();
    }
    else
    {
        return geometry().bottom();
    }
}


ContainerAreaLayout::ContainerAreaLayout(QWidget* parent)
    : QLayout(parent),
      m_orientation(Horizontal),
      m_stretchEnabled(true)
{
}

void ContainerAreaLayout::addItem(QLayoutItem* item)
{
    m_items.append(new ContainerAreaLayoutItem(item, this));
}

void ContainerAreaLayout::insertIntoFreeSpace(QWidget* widget, QPoint insertionPoint)
{
    if (!widget)
    {
        return;
    }

    add(widget);
    ContainerAreaLayoutItem* item = m_items.last();

    if (!item)
    {
        // this should never happen as we just added the item above
        // but we do this to be safe.
        return;
    }

    ItemList::iterator currentIt = m_items.begin();
    if (currentIt == m_items.end())
    {
        // this shouldn't happen either, but again... we try and be safe
        return;
    }

    ItemList::iterator nextIt = m_items.begin();
    ++nextIt;

    if (nextIt == m_items.end())
    {
        // first item in!
        item->setGeometryR(QRect(insertionPoint.x(), insertionPoint.y(), widget->width(), widget->height()));
        updateFreeSpaceValues();
        return;
    }

    int insPos = (orientation() == Horizontal) ? insertionPoint.x(): insertionPoint.y();
    Item* current = *currentIt;
    Item* next = *nextIt;

    for (; nextIt != m_items.end(); ++currentIt, ++nextIt)
    {
        next = *nextIt;
        current = *currentIt;
        if (current == item || next == item)
        {
            continue;
        }

        if (insPos == 0)
        {
            if (current->rightR() + 3 < next->leftR())
            {
                insPos = current->rightR();
                break;
            }
        }
        else
        {
            if (currentIt == m_items.begin() &&
                (current->leftR() > insPos ||
                 (current->leftR() <= insPos) &&
                 (current->rightR() > insPos)))
            {
                break;
            }

            if ((current->rightR() < insPos) && (next->leftR() > insPos))
            {
                // Free space available at insertion point!
                if (insPos + item->widthR() > next->leftR())
                {
                    // We have overlap on the right, move to the left
                    insPos = next->leftR() - item->widthR();
                    if (insPos < current->rightR())
                    {
                        // We have overlap on the left as well, move to the right
                        insPos = current->rightR();
                        // We don't fit entirely, let updateFreeSpaceValues sort it out
                    }
                }
                current = next;
                break;
            }

            if ((next->leftR() <= insPos) && (next->rightR() > insPos))
            {
                // Insert at the location of next
                current = next;
                insPos = next->leftR();
                break;
            }
        }
    }

    QRect geom = item->geometryR();
    geom.moveLeft(insPos);
    item->setGeometryR(geom);
    widget->setGeometry(transform(geom)); // widget isn't shown, layout not active yet

    if (current)
    {
        m_items.erase(m_items.fromLast());
        ItemList::iterator insertIt = m_items.find(current);

        if (insertIt == m_items.begin())
        {
            m_items.push_front(item);
        }
        else if (insertIt == m_items.end())
        {
            // yes, we just removed it from the end, but
            // if we remove it afterwards and it insertIt
            // was our item then we end up with a bad iterator
            m_items.append(item);
        }
        else
        {
            m_items.insert(insertIt, item);
        }
    }

    updateFreeSpaceValues();
}

QStringList ContainerAreaLayout::listItems() const
{
    QStringList items;
    for (ItemList::const_iterator it = m_items.constBegin();
         it != m_items.constEnd(); ++it)
    {
        QLayoutItem* item = (*it)->item;
        BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());

        if (!container)
        {
            continue;
        }

        AppletContainer* applet = dynamic_cast<AppletContainer*>(container);
        if (applet)
        {
            items.append(applet->info().desktopFile());
        }
        else
        {
            // button containers don't give us anything useful that isn't
            // i18n'd (e.g. all service buttons and url buttons simply report
            // "URL" as their tileName()) which would require that we
            // extend PanelButton to be more expressive to get something more
            // instead i just cop'd out and use the visible name. good enough.
            items.append(container->visibleName());
        }
    }

    return items;
}

QWidget* ContainerAreaLayout::widgetAt(int index) const
{
    if (index < 0 || index >= (int)m_items.count())
    {
        return 0;
    }

    return m_items[index]->item->widget();
}

QSize ContainerAreaLayout::sizeHint() const
{
    const int size = KickerLib::sizeValue(KPanelExtension::SizeNormal);

    if (orientation() == Horizontal)
    {
        return QSize(widthForHeight(size), size);
    }
    else
    {
        return QSize(size, heightForWidth(size));
    }
}

QSize ContainerAreaLayout::minimumSize() const
{
    const int size = KickerLib::sizeValue(KPanelExtension::SizeTiny);

    if (orientation() == Horizontal)
    {
        return QSize(widthForHeight(size), size);
    }
    else
    {
        return QSize(size, heightForWidth(size));
    }
}

QLayoutIterator ContainerAreaLayout::iterator()
{
    return QLayoutIterator(new ContainerAreaLayoutIterator(&m_items));
}

void ContainerAreaLayout::setGeometry(const QRect& rect)
{
    //RESEARCH: when can we short curcuit this?
    //          maybe a dirty flag to be set when we have containers
    //          that needs laying out?

    QLayout::setGeometry(rect);

    float totalFreeSpace = kMax(0, widthR() - widthForHeightR(heightR()));
    int occupiedSpace = 0;

    ItemList::const_iterator it = m_items.constBegin();
    while (it != m_items.constEnd())
    {
        ContainerAreaLayoutItem* cur  = *it;
        ++it;
        ContainerAreaLayoutItem* next = (it != m_items.constEnd()) ? *it : 0;

        double fs = cur->freeSpaceRatio();
        double freeSpace = fs * totalFreeSpace;
        int pos = int(rint(freeSpace)) + occupiedSpace;

        int w = cur->widthForHeightR(heightR());
        occupiedSpace += w;
        if (m_stretchEnabled && cur->isStretch())
        {
            if (next)
            {
                double nfs = next->freeSpaceRatio();
                w += int((nfs - fs)*totalFreeSpace);
            }
            else
            {
                w = widthR() - pos;
            }
        }
        cur->setGeometryR(QRect(pos, 0, w, heightR()));
    }
}

int ContainerAreaLayout::widthForHeight(int h) const
{
    int width = 0;
    ItemList::const_iterator it = m_items.constBegin();
    for (; it != m_items.constEnd(); ++it)
    {
        width += kMax(0, (*it)->widthForHeight(h));
    }
    return width;
}

int ContainerAreaLayout::heightForWidth(int w) const
{
    int height = 0;
    ItemList::const_iterator it = m_items.constBegin();
    for (; it != m_items.constEnd(); ++it)
    {
        height += kMax(0, (*it)->heightForWidth(w));
    }
    return height;
}

void ContainerAreaLayout::setStretchEnabled(bool enable)
{
    if (m_stretchEnabled != enable)
    {
        m_stretchEnabled = enable;
        activate();
    }
}

void ContainerAreaLayout::updateFreeSpaceValues()
{
    int freeSpace =
        kMax(0, widthR() - widthForHeightR(heightR()));

    double fspace = 0;
    for (ItemList::const_iterator it = m_items.constBegin();
         it != m_items.constEnd();
         ++it)
    {
        int distance = distanceToPreviousItem(it);
        if (distance < 0) distance = 0;
        fspace += distance;
        double ssf = ( freeSpace == 0 ? 0 : fspace/freeSpace );
        if (ssf > 1) ssf = 1;
        if (ssf < 0) ssf = 0;
        (*it)->setFreeSpaceRatio(ssf);
    }
}

int ContainerAreaLayout::distanceToPreviousItem(ItemList::const_iterator it) const
{
    assert(it != m_items.constEnd());

    ContainerAreaLayoutItem* cur  = *it;
    --it;
    ContainerAreaLayoutItem* prev = (it != m_items.constEnd()) ? *it : 0;

    return prev ? cur->leftR() - prev->leftR() - prev->widthForHeightR(heightR()) :
                  cur->leftR() - leftR();
}

void ContainerAreaLayout::moveContainerSwitch(QWidget* container, int distance)
{
    const bool horizontal    = orientation() == Horizontal;
    const bool reverseLayout = QApplication::reverseLayout();

    if (horizontal && reverseLayout)
        distance = - distance;

    const bool forward = distance > 0;

    // Get the iterator 'it' pointing to 'moving'.
    ItemList::const_iterator it = m_items.constBegin();
    while (it != m_items.constEnd() && (*it)->item->widget() != container)
    {
        ++it;
    }

    if (it == m_items.constEnd())
    {
        return;
    }

    ContainerAreaLayoutItem* moving = *it;
    forward ? ++it : --it;
    ContainerAreaLayoutItem* next = (it != m_items.constEnd()) ? *it : 0;
    ContainerAreaLayoutItem* last = moving;

    while (next)
    {
        // Calculate the position and width of the virtual container
        // containing 'moving' and 'next'.
        int tpos = forward ? next->leftR() - moving->widthR()
                           : next->leftR();
        int tsize = moving->widthR() + next->widthR();

        // Determine the middle of the containers.
        int tmiddle = tpos + tsize / 2;
        int movingMiddle = moving->leftR() + distance + moving->widthR() / 2;

        // Check if the middle of 'moving' has moved past the middle of the
        // virtual container.
        if (!forward && movingMiddle > tmiddle
          || forward && movingMiddle < tmiddle)
            break;

        // Move 'next' to the other side of 'moving'.
        QRect geom = next->geometryR();
        if (forward)
            geom.moveLeft(geom.left() - moving->widthR());
        else
            geom.moveLeft(geom.left() + moving->widthR());
        next->setGeometryR(geom);

        // Store 'next'. It may become null in the next iteration, but we
        // need its value later.
        last = next;
        forward ? ++it : --it;
        next = (it != m_items.constEnd()) ? *it : 0;
    }

    int newPos = moving->leftR() + distance;
    if (last != moving)
    {
        // The case that moving has switched position with at least one other container.
        newPos = forward ? kMax(newPos, last->rightR() + 1)
                         : kMin(newPos, last->leftR() - moving->widthR());

        // Move 'moving' to its new position in the container list.
        ItemList::iterator itMoving = m_items.find(moving);

        if (itMoving != m_items.end())
        {
            ItemList::iterator itLast = itMoving;
            if (forward)
            {
                ++itLast;
                ++itLast;
            }
            else
            {
                --itLast;
            }

            m_items.erase(itMoving);

            if (itLast == m_items.end())
            {
                if (forward)
                {
                    m_items.append(moving);
                }
                else
                {
                    m_items.push_front(moving);
                }
            }
            else
            {
                m_items.insert(itLast, moving);
            }
        }
    }
    else if (next)
    {
        // Make sure that the moving container will not overlap the next one.
        newPos = forward ? kMin(newPos, next->leftR() - moving->widthR())
                         : kMax(newPos, next->rightR() + 1);
    }

    // Move the container to its new position and prevent it from moving outside the panel.
    QRect geom = moving->geometryR();
    distance = kClamp(newPos, 0, widthR() - moving->widthR());
    geom.moveLeft(distance);
    moving->setGeometryR(geom);

    // HACK - since the menuapplet is not movable by the user, make sure it's always left-aligned
    ItemList::const_iterator prev = m_items.constEnd();
    for( ItemList::const_iterator it = m_items.constBegin();
         it != m_items.constEnd();
         ( prev = it ), ++it )
    {
        if( BaseContainer* container = dynamic_cast<BaseContainer*>((*it)->item->widget()))
            if(AppletContainer* applet = dynamic_cast<AppletContainer*>(container))
                if( applet->info().desktopFile() == "menuapplet.desktop" )
                {
                    QRect geom = (*it)->geometryR();
                    if( prev != m_items.constEnd())
                        geom.moveLeft( (*prev)->rightR() + 1 );
                    else
                        geom.moveLeft( 0 );
                    (*it)->setGeometryR( geom );
                }
    }

    updateFreeSpaceValues();
}

int ContainerAreaLayout::moveContainerPush(QWidget* a, int distance)
{
    const bool horizontal    = orientation() == Horizontal;
    const bool reverseLayout = QApplication::reverseLayout();

    // Get the iterator 'it' pointing to the layoutitem representing 'a'.
    ItemList::const_iterator it = m_items.constBegin();
    while (it != m_items.constEnd() && (*it)->item->widget() != a)
    {
        ++it;
    }

    if (it != m_items.constEnd())
    {
        if (horizontal && reverseLayout)
        {
            distance = -distance;
        }

        int retVal = moveContainerPushRecursive(it, distance);
        updateFreeSpaceValues();
        if (horizontal && reverseLayout)
        {
            retVal = -retVal;
        }
        return retVal;
    }
    else
    {
        return 0;
    }
}

int ContainerAreaLayout::moveContainerPushRecursive(ItemList::const_iterator it,
                                                    int distance)
{
    if (distance == 0)
        return 0;

    const bool forward = distance > 0;

    int available; // Space available for the container to move.
    int moved;     // The actual distance the container will move.
    ContainerAreaLayoutItem* cur  = *it;
    forward ? ++it : --it;
    ContainerAreaLayoutItem* next = (it != m_items.constEnd()) ? *it : 0;

    if (!next)
    {
        available = forward ? rightR() - cur->rightR()
                            : -cur->leftR();
    }
    else
    {
        available = forward ? next->leftR()  - cur->rightR() - 1
                            : next->rightR() - cur->leftR()  + 1;

        if (!forward && distance < available
          || forward && distance > available)
            available += moveContainerPushRecursive(it, distance - available);
    }
    moved = forward ? kMin(distance, available)
                    : kMax(distance, available);

    QRect geom = cur->geometryR();
    geom.moveLeft(geom.left() + moved);
    cur->setGeometryR(geom);

    return moved;
}

QRect ContainerAreaLayout::transform(const QRect& r) const
{
    if (orientation() == Horizontal)
    {
        if (QApplication::reverseLayout())
        {
            QRect t = r;
            t.moveLeft(geometry().right() - r.right());
            return t;
        }
        else
        {
            return r;
        }
    }
    else
    {
        return QRect(r.y(), r.x(), r.height(), r.width());
    }
}

int ContainerAreaLayout::widthForHeightR(int h) const
{
    if (orientation() == Horizontal)
    {
        return widthForHeight(h);
    }
    else
    {
        return heightForWidth(h);
    }
}

int ContainerAreaLayout::widthR() const
{
    if (orientation() == Horizontal)
    {
        return geometry().width();
    }
    else
    {
        return geometry().height();
    }
}

int ContainerAreaLayout::heightR() const
{
    if (orientation() == Horizontal)
    {
        return geometry().height();
    }
    else
    {
        return geometry().width();
    }
}

int ContainerAreaLayout::leftR() const
{
    if (orientation() == Horizontal)
        return geometry().left();
    else
        return geometry().top();
}

int ContainerAreaLayout::rightR() const
{
    if (orientation() == Horizontal)
        return geometry().right();
    else
        return geometry().bottom();
}

