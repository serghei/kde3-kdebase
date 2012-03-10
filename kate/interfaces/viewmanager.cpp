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

#include "viewmanager.h"
#include "viewmanager.moc"

#include "plugin.h"
#include "documentmanager.h"
#include "toolviewmanager.h"
#include "pluginmanager.h"

#include "../app/kateviewmanager.h"

namespace Kate
{

class PrivateViewManager
  {
  public:
    PrivateViewManager ()
    {
    }

    ~PrivateViewManager ()
    {    
    }          
        
    KateViewManager *viewMan; 
  };
            
ViewManager::ViewManager (void *viewManager) : QObject ((KateViewManager*) viewManager)
{
  d = new PrivateViewManager ();
  d->viewMan = (KateViewManager*) viewManager;
}

ViewManager::~ViewManager ()
{
  delete d;
}

View *ViewManager::activeView()
{
  return d->viewMan->activeView();
}

void ViewManager::activateView ( uint documentNumber )
{
  d->viewMan->activateView( documentNumber );
}

void ViewManager::openURL (const KURL &url)
{
  d->viewMan->openURL (url, QString::null, true);
}

}

