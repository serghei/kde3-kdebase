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

#include "extensionmanager.h"
#include "pluginmanager.h"

#include "addextension_mnu.h"
#include "addextension_mnu.moc"

PanelAddExtensionMenu::PanelAddExtensionMenu(QWidget *parent, const char *name) : QPopupMenu(parent, name)
{
    setCheckable(true);
    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
}

void PanelAddExtensionMenu::slotAboutToShow()
{
    clear();

    extensions = PluginManager::extensions();

    AppletInfo::List::const_iterator it = extensions.constBegin();
    for(int i = 0; it != extensions.constEnd(); ++it, ++i)
    {
        const AppletInfo &ai = (*it);
        insertItem(ai.name().replace("&", "&&"), i);
        if(ai.isUniqueApplet() && PluginManager::the()->hasInstance(ai))
        {
            setItemEnabled(i, false);
            setItemChecked(i, true);
        }
    }
}

void PanelAddExtensionMenu::slotExec(int id)
{
    ExtensionManager::the()->addExtension(extensions[id].desktopFile());
}
