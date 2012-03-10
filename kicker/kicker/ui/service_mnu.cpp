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

#include <typeinfo>
#include <qcursor.h>
#include <qbitmap.h>
#include <qpixmap.h>
#include <qimage.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kprocess.h>
#include <krun.h>
#include <kservicegroup.h>
#include <ksycoca.h>
#include <ksycocaentry.h>
#include <kservice.h>
#include <kurldrag.h>
#include <kio/job.h>

#include "global.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "menumanager.h"
#include "popupmenutitle.h"
#include "panelbutton.h"
#include "recentapps.h"
#include "service_mnu.h"
#include "service_mnu.moc"

PanelServiceMenu::PanelServiceMenu(const QString & label, const QString & relPath, QWidget * parent,
                                   const char * name, bool addmenumode, const QString & insertInlineHeader)
    : KPanelMenu(label, parent, name),
      relPath_(relPath),
      insertInlineHeader_( insertInlineHeader ),
      clearOnClose_(false),
      addmenumode_(addmenumode),
      popupMenu_(0)
{
    excludeNoDisplay_=true;

    connect(KSycoca::self(), SIGNAL(databaseChanged()),
            SLOT(slotClearOnClose()));
    connect(this, SIGNAL(aboutToHide()), this, SLOT(slotClose()));
}

PanelServiceMenu::~PanelServiceMenu()
{
    clearSubmenus();
}


void PanelServiceMenu::setExcludeNoDisplay( bool flag )
{
  excludeNoDisplay_=flag;
}

void PanelServiceMenu::showMenu()
{
    activateParent(QString::null);
}

// the initialization is split in initialize() and
// doInitialize() so that a subclass does not have to
// redo all the service parsing (see e.g. kicker/menuext/prefmenu)

void PanelServiceMenu::initialize()
{
    if (initialized()) return;

    setInitialized(true);
    entryMap_.clear();
    clear();

    clearSubmenus();
    doInitialize();
}

void PanelServiceMenu::fillMenu(KServiceGroup::Ptr& _root,
                                KServiceGroup::List& _list,
                                const QString& /* _relPath */,
                                int& id)
{
    QStringList suppressGenericNames = _root->suppressGenericNames();

    KServiceGroup::List::ConstIterator it = _list.begin();
    bool separatorNeeded = false;
    for (; it != _list.end(); ++it)
    {
        KSycocaEntry * e = *it;

        if (e->isType(KST_KServiceGroup))
        {

            KServiceGroup::Ptr g(static_cast<KServiceGroup *>(e));
            QString groupCaption = g->caption();

           // Avoid adding empty groups.
            KServiceGroup::Ptr subMenuRoot = KServiceGroup::group(g->relPath());

            int nbChildCount = subMenuRoot->childCount();
            if (nbChildCount == 0 && !g->showEmptyMenu())
            {
                continue;
            }

            QString inlineHeaderName = g->showInlineHeader() ? groupCaption : "";
            // Item names may contain ampersands. To avoid them being converted
            // to accelerators, replace them with two ampersands.
            groupCaption.replace("&", "&&");

            if ( nbChildCount == 1 && g->allowInline() && g->inlineAlias())
            {
                KServiceGroup::Ptr element = KServiceGroup::group(g->relPath());
                if ( element )
                {
                    //just one element
                    KServiceGroup::List listElement = element->entries(true, excludeNoDisplay_, true, KickerSettings::menuEntryFormat() == KickerSettings::DescriptionAndName || KickerSettings::menuEntryFormat() == KickerSettings::DescriptionOnly);
                    KSycocaEntry * e1 = *( listElement.begin() );
                    if ( e1->isType( KST_KService ) )
                    {
                        if (separatorNeeded)
                        {
                            insertSeparator();
                            separatorNeeded = false;
                        }

                        KService::Ptr s(static_cast<KService *>(e1));
                        insertMenuItem(s, id++, -1, &suppressGenericNames);
                        continue;
                    }
                }
            }

            if (g->allowInline() && ((nbChildCount <= g->inlineValue() ) ||   (g->inlineValue() == 0)))
            {
                //inline all entries
                KServiceGroup::Ptr rootElement = KServiceGroup::group(g->relPath());

                if (!rootElement || !rootElement->isValid())
                {
                    break;
                }

                KServiceGroup::List listElement = rootElement->entries(true, excludeNoDisplay_, true, KickerSettings::menuEntryFormat() == KickerSettings::DescriptionAndName || KickerSettings::menuEntryFormat() == KickerSettings::DescriptionOnly);

                if ( !g->inlineAlias() && !inlineHeaderName.isEmpty() )
                {
                    int mid = insertItem(new PopupMenuTitle(inlineHeaderName, font()), id + 1, id); 
                    id++;
                    setItemEnabled( mid, false );
                }

                fillMenu( rootElement, listElement,  g->relPath(), id );
                continue;
            }

            // Ignore dotfiles.
            if ((g->name().at(0) == '.'))
            {
                continue;
            }

            PanelServiceMenu * m =
                newSubMenu(g->name(), g->relPath(), this, g->name().utf8(), inlineHeaderName);
            m->setCaption(groupCaption);

            QIconSet iconset = KickerLib::menuIconSet(g->icon());

            if (separatorNeeded)
            {
                insertSeparator();
                separatorNeeded = false;
            }

            int newId = insertItem(iconset, groupCaption, m, id++);
            entryMap_.insert(newId, static_cast<KSycocaEntry*>(g));
            // We have to delete the sub menu our selves! (See Qt docs.)
            subMenus.append(m);
        }
        else if (e->isType(KST_KService))
        {
            if (separatorNeeded)
            {
                insertSeparator();
                separatorNeeded = false;
            }

            KService::Ptr s(static_cast<KService *>(e));
            insertMenuItem(s, id++, -1, &suppressGenericNames);
        }
        else if (e->isType(KST_KServiceSeparator))
        {
            separatorNeeded = true;
        }
    }
#if 0
    // WABA: tear off handles don't work together with dynamically updated
    // menus. We can't update the menu while torn off, and we don't know
    // when it is torn off.
    if ( count() > 0  && !relPath_.isEmpty() )
      if (KGlobalSettings::insertTearOffHandle())
        insertTearOffHandle();
#endif
}

void PanelServiceMenu::clearSubmenus()
{
    for (PopupMenuList::const_iterator it = subMenus.constBegin();
         it != subMenus.constEnd();
         ++it)
    {
        delete *it;
    }
    subMenus.clear();
}

void PanelServiceMenu::doInitialize()
{

    // Set the startposition outside the panel, so there is no drag initiated
    // when we use drag and click to select items. A drag is only initiated when
    // you click to open the menu, and then press and drag an item.
    startPos_ = QPoint(-1,-1);

    // We ask KSycoca to give us all services (sorted).
    KServiceGroup::Ptr root = KServiceGroup::group(relPath_);

    if (!root || !root->isValid())
        return;

    KServiceGroup::List list = root->entries(true, excludeNoDisplay_, true, KickerSettings::menuEntryFormat() == KickerSettings::DescriptionAndName || KickerSettings::menuEntryFormat() == KickerSettings::DescriptionOnly);

    if (list.isEmpty()) {
        setItemEnabled(insertItem(i18n("No Entries")), false);
        return;
    }

    int id = serviceMenuStartId();

    if (addmenumode_) {
        int mid = insertItem(KickerLib::menuIconSet("ok"), i18n("Add This Menu"), id++);
        entryMap_.insert(mid, static_cast<KSycocaEntry*>(root));

        if (relPath_ == "")
        {
            insertItem(KickerLib::menuIconSet("exec"), i18n("Add Non-KDE Application"),
                       this, SLOT(addNonKDEApp()));
        }

        if (list.count() > 0) {
            insertSeparator();
            id++;
        }
    }
    if ( !insertInlineHeader_.isEmpty() )
    {
        int mid = insertItem(new PopupMenuTitle(insertInlineHeader_, font()), -1, 0);
        setItemEnabled( mid, false );
    }
    fillMenu( root, list, relPath_, id );
}

void PanelServiceMenu::configChanged()
{
    deinitialize();
}

void PanelServiceMenu::insertMenuItem(KService::Ptr & s, int nId,
                                      int nIndex/*= -1*/,
                                      const QStringList *suppressGenericNames /* = 0 */,
                                      const QString & aliasname)
{
    QString serviceName = (aliasname.isEmpty() ? s->name() : aliasname).simplifyWhiteSpace();
    QString comment = s->genericName().simplifyWhiteSpace();

    if (!comment.isEmpty())
    {
        if (KickerSettings::menuEntryFormat() == KickerSettings::NameAndDescription)
        {
            if ((!suppressGenericNames ||
                 !suppressGenericNames->contains(s->untranslatedGenericName())) &&
                serviceName.find(comment, 0, true) == -1)
            {
                if (comment.find(serviceName, 0, true) == -1)
                {
                    serviceName = i18n("Entries in K-menu: %1 app name, %2 description", "%1 - %2").arg(serviceName, comment);
                }
                else
                {
                    serviceName = comment;
                }
            }
        }
        else if (KickerSettings::menuEntryFormat() == KickerSettings::DescriptionAndName)
        {
            serviceName = i18n("Entries in K-menu: %1 description, %2 app name", "%1 (%2)").arg(comment, serviceName);
        }
        else if (KickerSettings::menuEntryFormat() == KickerSettings::DescriptionOnly)
        {
            serviceName = comment;
        }
    }

    // restrict menu entries to a sane length
    if ( serviceName.length() > 60 ) {
      serviceName.truncate( 57 );
      serviceName += "...";
    }

    // check for NoDisplay
    if (s->noDisplay())
        return;

    // ignore dotfiles.
    if ((serviceName.at(0) == '.'))
        return;

    // item names may contain ampersands. To avoid them being converted
    // to accelerators, replace them with two ampersands.
    serviceName.replace("&", "&&");

    int newId = insertItem(KickerLib::menuIconSet(s->icon()), serviceName, nId, nIndex);
    entryMap_.insert(newId, static_cast<KSycocaEntry*>(s));
}

void PanelServiceMenu::activateParent(const QString &child)
{
    PanelServiceMenu *parentMenu = dynamic_cast<PanelServiceMenu*>(parent());
    if (parentMenu)
    {
        parentMenu->activateParent(relPath_);
    }
    else
    {
        PanelPopupButton *kButton = MenuManager::the()->findKButtonFor(this);
        if (kButton)
        {
            adjustSize();
            kButton->showMenu();
        }
        else
        {
            show();
        }
    }

    if (!child.isEmpty())
    {
        EntryMap::Iterator mapIt;
        for ( mapIt = entryMap_.begin(); mapIt != entryMap_.end(); ++mapIt )
        {
            KServiceGroup *g = dynamic_cast<KServiceGroup *>(static_cast<KSycocaEntry*>(mapIt.data()));

            // if the dynamic_cast fails, we are looking at a KService entry
            if (g && (g->relPath() == child))
            {
               activateItemAt(indexOf(mapIt.key()));
               return;
            }
        }
    }
}

bool PanelServiceMenu::highlightMenuItem( const QString &menuItemId )
{
    initialize();

    // Check menu itself
    EntryMap::Iterator mapIt;
    for ( mapIt = entryMap_.begin(); mapIt != entryMap_.end(); ++mapIt )
    {
        // Skip recent files menu
        if (mapIt.key() >= serviceMenuEndId())
        {
            continue;
        }
        KService *s = dynamic_cast<KService *>(
            static_cast<KSycocaEntry*>(mapIt.data()));
        if (s && (s->menuId() == menuItemId))
        {
            activateParent(QString::null);
            int index = indexOf(mapIt.key());
            setActiveItem(index);

            // Warp mouse pointer to location of active item
            QRect r = itemGeometry(index);
            QCursor::setPos(mapToGlobal(QPoint(r.x()+ r.width() - 15,
                r.y() + r.height() - 5)));
            return true;
        }
    }

    for(PopupMenuList::iterator it = subMenus.begin();
        it != subMenus.end();
        ++it)
    {
        PanelServiceMenu *serviceMenu = dynamic_cast<PanelServiceMenu*>(*it);
        if (serviceMenu && serviceMenu->highlightMenuItem(menuItemId))
            return true;
    }
    return false;
}

void PanelServiceMenu::slotExec(int id)
{
    if (!entryMap_.contains(id))
    {
        return;
    }

    KSycocaEntry * e = entryMap_[id];

    kapp->propagateSessionManager();

    KService::Ptr service = static_cast<KService *>(e);
    KApplication::startServiceByDesktopPath(service->desktopEntryPath(),
      QStringList(), 0, 0, 0, "", true);

    updateRecentlyUsedApps(service);
    startPos_ = QPoint(-1,-1);
}

void PanelServiceMenu::mousePressEvent(QMouseEvent * ev)
{
    startPos_ = ev->pos();
    KPanelMenu::mousePressEvent(ev);
}

void PanelServiceMenu::mouseReleaseEvent(QMouseEvent * ev)
{
    if (ev->button() == RightButton && !Kicker::the()->isKioskImmutable())
    {
        int id = idAt( ev->pos() );

        if (id < serviceMenuStartId())
        {
            return;
        }

        if (!entryMap_.contains(id))
        {
            kdDebug(1210) << "Cannot find service with menu id " << id << endl;
            return;
        }

        contextKSycocaEntry_ = entryMap_[id];

        delete popupMenu_;
        popupMenu_ = new KPopupMenu(this);
        connect(popupMenu_, SIGNAL(activated(int)), SLOT(slotContextMenu(int)));
        bool hasEntries = false;

        switch (contextKSycocaEntry_->sycocaType())
        {
        case KST_KService:
            if (kapp->authorize("editable_desktop_icons"))
            {
                hasEntries = true;
                popupMenu_->insertItem(SmallIconSet("desktop"),
                    i18n("Add Item to Desktop"), AddItemToDesktop);
            }
            if (kapp->authorizeKAction("kicker_rmb") && !Kicker::the()->isImmutable())
            {
                hasEntries = true;
                popupMenu_->insertItem(SmallIconSet("kicker"),
                    i18n("Add Item to Main Panel"), AddItemToPanel);
            }
            if (kapp->authorizeKAction("menuedit"))
            {
                hasEntries = true;
                popupMenu_->insertItem(SmallIconSet("kmenuedit"),
                    i18n("Edit Item"), EditItem);
            }
            if (kapp->authorize("run_command"))
            {
                hasEntries = true;
                popupMenu_->insertItem(SmallIconSet("run"),
                    i18n("Put Into Run Dialog"), PutIntoRunDialog);
            }
            break;

        case KST_KServiceGroup:
            if (kapp->authorize("editable_desktop_icons"))
            {
                hasEntries = true;
                popupMenu_->insertItem(SmallIconSet("desktop"),
                    i18n("Add Menu to Desktop"), AddMenuToDesktop);
            }
            if (kapp->authorizeKAction("kicker_rmb") && !Kicker::the()->isImmutable())
            {
                hasEntries = true;
                popupMenu_->insertItem(SmallIconSet("kicker"),
                    i18n("Add Menu to Main Panel"), AddMenuToPanel);
            }
            if (kapp->authorizeKAction("menuedit"))
            {
                hasEntries = true;
                popupMenu_->insertItem(SmallIconSet("kmenuedit"),
                    i18n("Edit Menu"), EditMenu);
            }
            break;

        default:
            break;
        }

        if (hasEntries)
        {
            popupMenu_->popup(this->mapToGlobal(ev->pos()));
            return;
        }
    }

    delete popupMenu_;
    popupMenu_ = 0;

    KPanelMenu::mouseReleaseEvent(ev);
}

extern int kicker_screen_number;

void PanelServiceMenu::slotContextMenu(int selected)
{
    KProcess *proc;
    KService::Ptr service;
    KServiceGroup::Ptr g;
    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);

    KURL src,dest;
    KIO::CopyJob *job;
    KDesktopFile *df;

    switch (selected) {
        case AddItemToDesktop:
	    service = static_cast<KService *>(contextKSycocaEntry_);

	    src.setPath( KGlobal::dirs()->findResource( "apps", service->desktopEntryPath() ) );
	    dest.setPath( KGlobalSettings::desktopPath() );
	    dest.setFileName( src.fileName() );

            job = KIO::copyAs( src, dest );
            job->setDefaultPermissions( true );
	    break;

        case AddItemToPanel: {
            QCString appname = "kicker";
            if ( kicker_screen_number )
                appname.sprintf("kicker-screen-%d", kicker_screen_number);
            service = static_cast<KService *>(contextKSycocaEntry_);
            kapp->dcopClient()->send(appname, "Panel", "addServiceButton(QString)", service->desktopEntryPath());
            break;
        }

	case EditItem:
            proc = new KProcess(this);
            *proc << KStandardDirs::findExe(QString::fromLatin1("kmenuedit"));
            *proc << "/"+relPath_ << static_cast<KService *>(contextKSycocaEntry_)->menuId();
            proc->start();
	    break;

        case PutIntoRunDialog: {
            close();
            QCString appname = "kdesktop";
            if ( kicker_screen_number )
                appname.sprintf("kdesktop-screen-%d", kicker_screen_number);
            service = static_cast<KService *>(contextKSycocaEntry_);
            kapp->updateRemoteUserTimestamp( appname );
            kapp->dcopClient()->send(appname, "default", "popupExecuteCommand(QString)", service->exec());
            break;
        }

	case AddMenuToDesktop:
	    g = static_cast<KServiceGroup *>(contextKSycocaEntry_);
            dest.setPath( KGlobalSettings::desktopPath() );
	    dest.setFileName( g->caption() );

	    df = new KDesktopFile( dest.path() );
            df->writeEntry( "Icon", g->icon() );
            df->writePathEntry( "URL", "programs:/"+g->name() );
	    df->writeEntry( "Name", g->caption() );
	    df->writeEntry( "Type", "Link" );
            df->sync();
	    delete df;

	    break;

        case AddMenuToPanel: {
            QCString appname = "kicker";
            if ( kicker_screen_number )
                appname.sprintf("kicker-screen-%d", kicker_screen_number);

            g = static_cast<KServiceGroup *>(contextKSycocaEntry_);
            ds << "foo" << g->relPath();
            kapp->dcopClient()->send("kicker", "Panel", "addServiceMenuButton(QString,QString)", ba);
            break;
        }

        case EditMenu:
            proc = new KProcess(this);
            *proc << KStandardDirs::findExe(QString::fromLatin1("kmenuedit"));
            *proc << "/"+static_cast<KServiceGroup *>(contextKSycocaEntry_)->relPath();
            proc->start();
	    break;

	default:
	    break;
	}
}

void PanelServiceMenu::mouseMoveEvent(QMouseEvent * ev)
{
    KPanelMenu::mouseMoveEvent(ev);

    if (Kicker::the()->isKioskImmutable())
        return;

    if ( (ev->state() & LeftButton ) != LeftButton )
        return;

    QPoint p = ev->pos() - startPos_;
    if (p.manhattanLength() <= QApplication::startDragDistance() )
        return;

    int id = idAt(startPos_);

    // Don't drag items we didn't create.
    if (id < serviceMenuStartId())
        return;

    if (!entryMap_.contains(id)) {
        kdDebug(1210) << "Cannot find service with menu id " << id << endl;
        return;
    }

    KSycocaEntry * e = entryMap_[id];

    QPixmap icon;
    KURL url;

    switch (e->sycocaType()) {

        case KST_KService:
        {
            icon = static_cast<KService *>(e)->pixmap(KIcon::Small);
            QString filePath = static_cast<KService *>(e)->desktopEntryPath();
            if (filePath[0] != '/')
            {
                filePath = locate("apps", filePath);
            }
            url.setPath(filePath);
            break;
        }

        case KST_KServiceGroup:
        {
            icon = KGlobal::iconLoader()
                   ->loadIcon(static_cast<KServiceGroup *>(e)->icon(), KIcon::Small);
            url = "programs:/" + static_cast<KServiceGroup *>(e)->relPath();
            break;
        }

        default:
        {
            return;
            break;
        }
    }

    // If the path to the desktop file is relative, try to get the full
    // path from KStdDirs.

    KURLDrag *d = new KURLDrag(KURL::List(url), this);
    connect(d, SIGNAL(destroyed()), this, SLOT(slotDragObjectDestroyed()));
    d->setPixmap(icon);
    d->dragCopy();

    // Set the startposition outside the panel, so there is no drag initiated
    // when we use drag and click to select items. A drag is only initiated when
    // you click to open the menu, and then press and drag an item.
    startPos_ = QPoint(-1,-1);
}

void PanelServiceMenu::dragEnterEvent(QDragEnterEvent *event)
{
    // Set the DragObject's target to this widget. This is needed because the
    // widget doesn't accept drops, but we want to determine if the drag object
    // is dropped on it. This avoids closing on accidental drags. If this
    // widget accepts drops in the future, these lines can be removed.
    if (event->source() == this)
    {
        KURLDrag::setTarget(this);
    }
    event->ignore();
}

void PanelServiceMenu::dragLeaveEvent(QDragLeaveEvent *)
{
    // see PanelServiceMenu::dragEnterEvent why this is nescessary
    if (!frameGeometry().contains(QCursor::pos()))
    {
        KURLDrag::setTarget(0);
    }
}

void PanelServiceMenu::slotDragObjectDestroyed()
{
    if (KURLDrag::target() != this)
    {
        // we need to re-enter the event loop before calling close() here
        // this gets called _before_ the drag object is destroyed, so we are
        // still in its event loop. closing the menu before that event loop is
        // exited may result in getting hung up in it which in turn prevents
        // the execution of any code after the original exec() statement
        // though the panels themselves continue on otherwise normally
        // (we just have some sort of nested event loop)
        QTimer::singleShot(0, this, SLOT(close()));
    }
}

PanelServiceMenu *PanelServiceMenu::newSubMenu(const QString & label, const QString & relPath,
                                               QWidget * parent, const char * name, const QString& _inlineHeader)
{
    return new PanelServiceMenu(label, relPath, parent, name, false,_inlineHeader);
}

void PanelServiceMenu::slotClearOnClose()
{
    if (!initialized()) return;

    if (!isVisible()){
        clearOnClose_ = false;
        slotClear();
    } else {
        clearOnClose_ = true;
    }
}

void PanelServiceMenu::slotClose()
{
    if (clearOnClose_)
    {
        clearOnClose_ = false;
        slotClear();
    }

    delete popupMenu_;
    popupMenu_ = 0;
}

void PanelServiceMenu::slotClear()
{
    if (isVisible())
    {
        // QPopupMenu's aboutToHide() is emitted before the popup is really hidden,
        // and also before a click in the menu is handled, so do the clearing
        // only after that has been handled
        QTimer::singleShot(100, this, SLOT(slotClear()));
        return;
    }

    entryMap_.clear();
    KPanelMenu::slotClear();

    for (PopupMenuList::iterator it = subMenus.begin();
         it != subMenus.end();
         ++it)
    {
        delete *it;
    }
    subMenus.clear();
}

void PanelServiceMenu::selectFirstItem()
{
    setActiveItem(indexOf(serviceMenuStartId()));
}

// updates "recent" section of KMenu
void PanelServiceMenu::updateRecentlyUsedApps(KService::Ptr &service)
{
    QString strItem(service->desktopEntryPath());

    // don't add an item from root kmenu level
    if (!strItem.contains('/'))
    {
        return;
    }

    // add it into recent apps list
    RecentlyLaunchedApps::the().appLaunched(strItem);
    RecentlyLaunchedApps::the().save();
    RecentlyLaunchedApps::the().m_bNeedToUpdate = true;
}

