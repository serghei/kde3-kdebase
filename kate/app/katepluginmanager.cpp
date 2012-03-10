/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

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

#include "katepluginmanager.h"
#include "katepluginmanager.moc"

#include "kateapp.h"
#include "katemainwindow.h"

#include "../interfaces/application.h"

#include <kconfig.h>
#include <qstringlist.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <qfile.h>

KatePluginManager::KatePluginManager(QObject *parent) : QObject (parent)
{
  m_pluginManager = new Kate::PluginManager (this);
  setupPluginList ();

  loadConfig ();
  loadAllEnabledPlugins ();
}

KatePluginManager::~KatePluginManager()
{
  // first write config
  writeConfig ();

  // than unload the plugins
  unloadAllPlugins ();
}

KatePluginManager *KatePluginManager::self()
{
  return KateApp::self()->pluginManager ();
}

void KatePluginManager::setupPluginList ()
{
  QValueList<KService::Ptr> traderList= KTrader::self()->query("Kate/Plugin", "(not ('Kate/ProjectPlugin' in ServiceTypes)) and (not ('Kate/InitPlugin' in ServiceTypes))");

  for(KTrader::OfferList::Iterator it(traderList.begin()); it != traderList.end(); ++it)
  {
    KService::Ptr ptr = (*it);

    QString pVersion = ptr->property("X-Kate-Version").toString();

//    if ((pVersion >= "2.5") && (pVersion <= KateApp::kateVersion(false)))
    if (pVersion == "2.5")
    {
      KatePluginInfo info;

      info.load = false;
      info.service = ptr;
      info.plugin = 0L;

      m_pluginList.push_back (info);
    }
  }
}

void KatePluginManager::loadConfig ()
{
  KateApp::self()->config()->setGroup("Kate Plugins");

  for (unsigned int i=0; i < m_pluginList.size(); ++i)
    m_pluginList[i].load =  KateApp::self()->config()->readBoolEntry (m_pluginList[i].service->library(), false) ||
                            KateApp::self()->config()->readBoolEntry (m_pluginList[i].service->property("X-Kate-PluginName").toString(),false);
}

void KatePluginManager::writeConfig ()
{
  KateApp::self()->config()->setGroup("Kate Plugins");

  for (unsigned int i=0; i < m_pluginList.size(); ++i)
  {
    QString saveName=m_pluginList[i].service->property("X-Kate-PluginName").toString();

    if (saveName.isEmpty())
      saveName = m_pluginList[i].service->library();

    KateApp::self()->config()->writeEntry (saveName, m_pluginList[i].load);
  }
}

void KatePluginManager::loadAllEnabledPlugins ()
{
  for (unsigned int i=0; i < m_pluginList.size(); ++i)
  {
    if  (m_pluginList[i].load)
      loadPlugin (&m_pluginList[i]);
    else
      unloadPlugin (&m_pluginList[i]);
  }
}

void KatePluginManager::unloadAllPlugins ()
{
  for (unsigned int i=0; i < m_pluginList.size(); ++i)
  {
    if  (m_pluginList[i].plugin)
      unloadPlugin (&m_pluginList[i]);
  }
}

void KatePluginManager::enableAllPluginsGUI (KateMainWindow *win)
{
  for (unsigned int i=0; i < m_pluginList.size(); ++i)
  {
    if  (m_pluginList[i].load)
      enablePluginGUI (&m_pluginList[i], win);
  }
}

void KatePluginManager::disableAllPluginsGUI (KateMainWindow *win)
{
  for (unsigned int i=0; i < m_pluginList.size(); ++i)
  {
    if  (m_pluginList[i].load)
      disablePluginGUI (&m_pluginList[i], win);
  }
}

void KatePluginManager::loadPlugin (KatePluginInfo *item)
{
  QString pluginName=item->service->property("X-Kate-PluginName").toString();

  if (pluginName.isEmpty())
    pluginName=item->service->library();

  item->load = (item->plugin = Kate::createPlugin (QFile::encodeName(item->service->library()), Kate::application(), 0, pluginName));
}

void KatePluginManager::unloadPlugin (KatePluginInfo *item)
{
  disablePluginGUI (item);
  if (item->plugin) delete item->plugin;
  item->plugin = 0L;
  item->load = false;
}

void KatePluginManager::enablePluginGUI (KatePluginInfo *item, KateMainWindow *win)
{
  if (!item->plugin) return;
  if (!Kate::pluginViewInterface(item->plugin)) return;

  Kate::pluginViewInterface(item->plugin)->addView(win->mainWindow());
}

void KatePluginManager::enablePluginGUI (KatePluginInfo *item)
{
  if (!item->plugin) return;
  if (!Kate::pluginViewInterface(item->plugin)) return;

  for (uint i=0; i< KateApp::self()->mainWindows(); i++)
  {
    Kate::pluginViewInterface(item->plugin)->addView(KateApp::self()->mainWindow(i)->mainWindow());
  }
}

void KatePluginManager::disablePluginGUI (KatePluginInfo *item, KateMainWindow *win)
{
  if (!item->plugin) return;
  if (!Kate::pluginViewInterface(item->plugin)) return;

  Kate::pluginViewInterface(item->plugin)->removeView(win->mainWindow());
}

void KatePluginManager::disablePluginGUI (KatePluginInfo *item)
{
  if (!item->plugin) return;
  if (!Kate::pluginViewInterface(item->plugin)) return;

  for (uint i=0; i< KateApp::self()->mainWindows(); i++)
  {
    Kate::pluginViewInterface(item->plugin)->removeView(KateApp::self()->mainWindow(i)->mainWindow());
  }
}

Kate::Plugin *KatePluginManager::plugin(const QString &name)
{
  for (unsigned int i=0; i < m_pluginList.size(); ++i)
  {
    KatePluginInfo *info = &m_pluginList[i];
    QString pluginName=info->service->property("X-Kate-PluginName").toString();
    if (pluginName.isEmpty())
       pluginName=info->service->library();
    if  (pluginName==name)
    {
      if (info->plugin)
        return info->plugin;
      else
        break;
    }
  }
  return 0;
}

bool KatePluginManager::pluginAvailable(const QString &){return false;}
class Kate::Plugin *KatePluginManager::loadPlugin(const QString &,bool ){return 0;}
void KatePluginManager::unloadPlugin(const QString &,bool){;}
