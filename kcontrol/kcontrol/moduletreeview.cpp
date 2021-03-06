/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <qheader.h>
#include <qimage.h>
#include <qpainter.h>
#include <qbitmap.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <qwhatsthis.h>
#include <qbitmap.h>

#include "moduletreeview.h"
#include "moduletreeview.moc"
#include "modules.h"
#include "global.h"

static QPixmap appIcon(const QString &iconName)
{
    QString path;
    QPixmap normal = KGlobal::iconLoader()->loadIcon(iconName, KIcon::Small, 0, KIcon::DefaultState, &path, true);
    // make sure they are not larger than KIcon::SizeSmall
    if(normal.width() > KIcon::SizeSmall || normal.height() > KIcon::SizeSmall)
    {
        QImage tmp = normal.convertToImage();
        tmp = tmp.smoothScale(KIcon::SizeSmall, KIcon::SizeSmall);
        normal.convertFromImage(tmp);
    }
    return normal;
}

class ModuleTreeWhatsThis : public QWhatsThis {
public:
    ModuleTreeWhatsThis(ModuleTreeView *tree) : QWhatsThis(tree), treeView(tree)
    {
    }
    ~ModuleTreeWhatsThis(){};


    QString text(const QPoint &p)
    {
        ModuleTreeItem *i = (ModuleTreeItem *)treeView->itemAt(p);
        if(i && i->module())
        {
            return i->module()->comment();
        }
        else if(i)
        {
            return i18n("The %1 configuration group. Click to open it.").arg(i->text(0));
        }
        return i18n("This treeview displays all available control modules. Click on one of the modules to receive more detailed information.");
    }

private:
    ModuleTreeView *treeView;
};

ModuleTreeView::ModuleTreeView(ConfigModuleList *list, QWidget *parent, const char *name) : KListView(parent, name), _modules(list)
{
    addColumn(QString::null);
    setColumnWidthMode(0, QListView::Maximum);
    setAllColumnsShowFocus(true);
    setResizeMode(QListView::AllColumns);
    setRootIsDecorated(true);
    setHScrollBarMode(AlwaysOff);
    header()->hide();

    new ModuleTreeWhatsThis(this);

    connect(this, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotItemSelected(QListViewItem *)));
}

void ModuleTreeView::fill()
{
    clear();

    QStringList subMenus = _modules->submenus(KCGlobal::baseGroup());
    for(QStringList::ConstIterator it = subMenus.begin(); it != subMenus.end(); ++it)
    {
        QString path = *it;
        ModuleTreeItem *menu = new ModuleTreeItem(this);
        menu->setGroup(path);
        fill(menu, path);
    }

    ConfigModule *module;
    QPtrList< ConfigModule > moduleList = _modules->modules(KCGlobal::baseGroup());
    for(module = moduleList.first(); module != 0; module = moduleList.next())
    {
        new ModuleTreeItem(this, module);
    }
}

void ModuleTreeView::fill(ModuleTreeItem *parent, const QString &parentPath)
{
    QStringList subMenus = _modules->submenus(parentPath);
    for(QStringList::ConstIterator it = subMenus.begin(); it != subMenus.end(); ++it)
    {
        QString path = *it;
        ModuleTreeItem *menu = new ModuleTreeItem(parent);
        menu->setGroup(path);
        fill(menu, path);
    }

    ConfigModule *module;
    QPtrList< ConfigModule > moduleList = _modules->modules(parentPath);
    for(module = moduleList.first(); module != 0; module = moduleList.next())
    {
        new ModuleTreeItem(parent, module);
    }
}


QSize ModuleTreeView::sizeHint() const
{
    return QListView::sizeHint().boundedTo(QSize(fontMetrics().maxWidth() * 35, QWIDGETSIZE_MAX));
}

void ModuleTreeView::makeSelected(ConfigModule *module)
{
    ModuleTreeItem *item = static_cast< ModuleTreeItem * >(firstChild());

    updateItem(item, module);
}

void ModuleTreeView::updateItem(ModuleTreeItem *item, ConfigModule *module)
{
    while(item)
    {
        if(item->childCount() != 0)
            updateItem(static_cast< ModuleTreeItem * >(item->firstChild()), module);
        if(item->module() == module)
        {
            setSelected(item, true);
            break;
        }
        item = static_cast< ModuleTreeItem * >(item->nextSibling());
    }
}

/*
void ModuleTreeView::expandItem(QListViewItem *item, QPtrList<QListViewItem> *parentList)
{
  while (item)
    {
      setOpen(item, parentList->contains(item));

          if (item->childCount() != 0)
                expandItem(item->firstChild(), parentList);
      item = item->nextSibling();
    }
}
*/
void ModuleTreeView::makeVisible(ConfigModule *module)
{
    QString path = _modules->findModule(module);
    if(path.startsWith(KCGlobal::baseGroup()))
        path = path.mid(KCGlobal::baseGroup().length());

    QStringList groups = QStringList::split('/', path);

    ModuleTreeItem *item = 0;
    QStringList::ConstIterator it;
    for(it = groups.begin(); it != groups.end(); ++it)
    {
        if(item)
            item = static_cast< ModuleTreeItem * >(item->firstChild());
        else
            item = static_cast< ModuleTreeItem * >(firstChild());

        while(item)
        {
            if(item->tag() == *it)
            {
                setOpen(item, true);
                break;
            }
            item = static_cast< ModuleTreeItem * >(item->nextSibling());
        }
        if(!item)
            break; // Not found (?)
    }

    // make the item visible
    if(item)
        ensureItemVisible(item);
}

void ModuleTreeView::slotItemSelected(QListViewItem *item)
{
    if(!item)
        return;

    if(static_cast< ModuleTreeItem * >(item)->module())
    {
        emit moduleSelected(static_cast< ModuleTreeItem * >(item)->module());
        return;
    }
    else
    {
        emit categorySelected(item);
    }

    setOpen(item, !item->isOpen());

    /*
    else
      {
        QPtrList<QListViewItem> parents;

        QListViewItem* i = item;
        while(i)
          {
            parents.append(i);
            i = i->parent();
          }

        //int oy1 = item->itemPos();
        //int oy2 = mapFromGlobal(QCursor::pos()).y();
        //int offset = oy2 - oy1;

        expandItem(firstChild(), &parents);

        //int x =mapFromGlobal(QCursor::pos()).x();
        //int y = item->itemPos() + offset;
        //QCursor::setPos(mapToGlobal(QPoint(x, y)));
      }
    */
}

void ModuleTreeView::keyPressEvent(QKeyEvent *e)
{
    if(!currentItem())
        return;

    if(e->key() == Key_Return || e->key() == Key_Enter || e->key() == Key_Space)
    {
        // QCursor::setPos(mapToGlobal(QPoint(10, currentItem()->itemPos()+5)));
        slotItemSelected(currentItem());
    }
    else
        KListView::keyPressEvent(e);
}


ModuleTreeItem::ModuleTreeItem(QListViewItem *parent, ConfigModule *module)
    : QListViewItem(parent), _module(module), _tag(QString::null), _maxChildIconWidth(0)
{
    if(_module)
    {
        setText(0, " " + module->moduleName());
        _icon = module->icon();
        setPixmap(0, appIcon(_icon));
    }
}

ModuleTreeItem::ModuleTreeItem(QListView *parent, ConfigModule *module)
    : QListViewItem(parent), _module(module), _tag(QString::null), _maxChildIconWidth(0)
{
    if(_module)
    {
        setText(0, " " + module->moduleName());
        _icon = module->icon();
        setPixmap(0, appIcon(_icon));
    }
}

ModuleTreeItem::ModuleTreeItem(QListViewItem *parent, const QString &text)
    : QListViewItem(parent, " " + text), _module(0), _tag(QString::null), _maxChildIconWidth(0)
{
}

ModuleTreeItem::ModuleTreeItem(QListView *parent, const QString &text)
    : QListViewItem(parent, " " + text), _module(0), _tag(QString::null), _maxChildIconWidth(0)
{
}

void ModuleTreeItem::setPixmap(int column, const QPixmap &pm)
{
    if(!pm.isNull())
    {
        ModuleTreeItem *p = dynamic_cast< ModuleTreeItem * >(parent());
        if(p)
            p->regChildIconWidth(pm.width());
    }

    QListViewItem::setPixmap(column, pm);
}

void ModuleTreeItem::regChildIconWidth(int width)
{
    if(width > _maxChildIconWidth)
        _maxChildIconWidth = width;
}

void ModuleTreeItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align)
{
    if(!pixmap(0))
    {
        int offset = 0;
        ModuleTreeItem *parentItem = dynamic_cast< ModuleTreeItem * >(parent());
        if(parentItem)
        {
            offset = parentItem->maxChildIconWidth();
        }

        if(offset > 0)
        {
            QPixmap pixmap(offset, offset);
            pixmap.fill(Qt::color0);
            pixmap.setMask(pixmap.createHeuristicMask());
            QBitmap mask(pixmap.size(), true);
            pixmap.setMask(mask);
            QListViewItem::setPixmap(0, pixmap);
        }
    }

    QListViewItem::paintCell(p, cg, column, width, align);
}


void ModuleTreeItem::setGroup(const QString &path)
{
    KServiceGroup::Ptr group = KServiceGroup::group(path);
    QString defName = path.left(path.length() - 1);
    int pos = defName.findRev('/');
    if(pos >= 0)
        defName = defName.mid(pos + 1);
    if(group && group->isValid())
    {
        _icon = group->icon();
        setPixmap(0, appIcon(_icon));
        setText(0, " " + group->caption());
        setTag(defName);
        setCaption(group->caption());
    }
    else
    {
        // Should not happen: Installation problem
        // Let's try to fail softly.
        setText(0, " " + defName);
        setTag(defName);
    }
}
