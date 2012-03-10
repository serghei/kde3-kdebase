/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

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

#include <klocale.h>
#include <kglobal.h>

#include "kicker.h"
#include "extensionmanager.h"
#include "pluginmanager.h"

#include "panelmenuiteminfo.h"
#include "removeextension_mnu.h"
#include "removeextension_mnu.moc"

static const int REMOVEALLID = 1000;

PanelRemoveExtensionMenu::PanelRemoveExtensionMenu( QWidget *parent, const char *name )
    : QPopupMenu( parent, name )
{
    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
}

PanelRemoveExtensionMenu::PanelRemoveExtensionMenu()
{
}

void PanelRemoveExtensionMenu::slotAboutToShow()
{
    int id = 0;

    clear();
    m_containers = ExtensionManager::the()->containers();
    QValueList<PanelMenuItemInfo> items;

    ExtensionList::iterator itEnd = m_containers.end();
    for (ExtensionList::iterator it = m_containers.begin(); it != itEnd; ++it)
    {
        const AppletInfo info = (*it)->info();
        QString name = info.name().replace("&", "&&");
        switch ((*it)->position())
        {
            case KPanelExtension::Top:
                name = i18n("%1 (Top)").arg(name);
            break;
            case KPanelExtension::Right:
                name = i18n("%1 (Right)").arg(name);
            break;
            case KPanelExtension::Bottom:
                name = i18n("%1 (Bottom)").arg(name);
            break;
            case KPanelExtension::Left:
                name = i18n("%1 (Left)").arg(name);
            break;
            case KPanelExtension::Floating:
                name = i18n("%1 (Floating)").arg(name);
            break;
         }
        items.append(PanelMenuItemInfo(QString::null, name, id));
        ++id;
    }

    qHeapSort(items);
    QValueList<PanelMenuItemInfo>::iterator itEnd2 = items.end();
    for (QValueList<PanelMenuItemInfo>::iterator it = items.begin(); it != itEnd2; ++it)
    {
        (*it).plug(this);
    }

    if (m_containers.count() > 1)
    {
        insertSeparator();
        insertItem(i18n("All"), REMOVEALLID);
    }
}

void PanelRemoveExtensionMenu::slotExec( int id )
{
    if (id == REMOVEALLID)
    {
        ExtensionManager::the()->removeAllContainers();
    }
    else if (m_containers.at(id) != m_containers.end())
    {
        ExtensionManager::the()->removeContainer(*m_containers.at(id));
    }
}

