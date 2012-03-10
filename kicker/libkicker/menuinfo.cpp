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

#include "menuinfo.h"

#include <qfile.h>
#include <qwidget.h>

#include <kapplication.h>
#include <ksimpleconfig.h>
#include <klibloader.h>
#include <kstandarddirs.h>
#include <kpanelmenu.h>
#include <kparts/componentfactory.h>

MenuInfo::MenuInfo(const QString& desktopFile)
{
    KSimpleConfig df(locate("data", QString::fromLatin1("kicker/menuext/%1").arg(desktopFile)));
    df.setGroup("Desktop Entry");

    QStringList list = df.readListEntry("X-KDE-AuthorizeAction");
    if (kapp && !list.isEmpty())
    {
       for(QStringList::ConstIterator it = list.begin();
           it != list.end();
           ++it)
       {
          if (!kapp->authorize((*it).stripWhiteSpace()))
             return;
       }
    }

    name_ = df.readEntry("Name");
    comment_ = df.readEntry("Comment");
    icon_ = df.readEntry("Icon");
    library_ = df.readEntry("X-KDE-Library");
    desktopfile_ = desktopFile;
}

KPanelMenu* MenuInfo::load(QWidget *parent, const char *name)
{
    if (library_.isEmpty())
        return 0;

    return KParts::ComponentFactory::createInstanceFromLibrary<KPanelMenu>(
               QFile::encodeName( library_ ),
               parent, name );
}
