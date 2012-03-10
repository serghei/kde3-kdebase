/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "pluginconfiginterfaceextension.h"
#include "pluginconfiginterfaceextension.moc"

#include "plugin.h"

namespace Kate
{

class PrivatePluginConfigInterfaceExtension
{
  public:
    PrivatePluginConfigInterfaceExtension() {}
     ~PrivatePluginConfigInterfaceExtension() {}
};

}

using namespace Kate;

PluginConfigPage::PluginConfigPage ( QWidget *parent, const char *name ) : QWidget (parent, name) { }
 
PluginConfigPage::~PluginConfigPage () { }

unsigned int PluginConfigInterfaceExtension::globalPluginConfigInterfaceExtensionNumber = 0;

PluginConfigInterfaceExtension::PluginConfigInterfaceExtension()
{
  globalPluginConfigInterfaceExtensionNumber++;
  myPluginConfigInterfaceExtensionNumber = globalPluginConfigInterfaceExtensionNumber++;

  d = new PrivatePluginConfigInterfaceExtension();
}

PluginConfigInterfaceExtension::~PluginConfigInterfaceExtension()
{
  delete d;
}

unsigned int PluginConfigInterfaceExtension::pluginConfigInterfaceExtensionNumber () const
{
  return myPluginConfigInterfaceExtensionNumber;
}
                      
PluginConfigInterfaceExtension *Kate::pluginConfigInterfaceExtension (Plugin *plugin)
{                       
  if (!plugin)
    return 0;

  return static_cast<PluginConfigInterfaceExtension*>(plugin->qt_cast("Kate::PluginConfigInterfaceExtension"));
}
