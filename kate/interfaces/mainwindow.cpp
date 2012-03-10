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

#include "mainwindow.h"
#include "mainwindow.moc"

#include "documentmanager.h"
#include "plugin.h"
#include "viewmanager.h"
#include "toolviewmanager.h"
#include "pluginmanager.h"

#include "../app/katemainwindow.h"
#include "../app/kateviewmanager.h"

namespace Kate
{

class PrivateMainWindow
  {
  public:
    PrivateMainWindow ()
    {
    }

    ~PrivateMainWindow ()
    {

    }

    KateMainWindow *win;
  };

MainWindow::MainWindow (void *mainWindow) : QObject ((KateMainWindow*) mainWindow)
{
  d = new PrivateMainWindow;
  d->win = (KateMainWindow*) mainWindow;
}

MainWindow::~MainWindow ()
{
  delete d;
}

KXMLGUIFactory *MainWindow::guiFactory() const
{
  return d->win->guiFactory();
}

ViewManager *MainWindow::viewManager () const
{
  return d->win->viewManager ()->viewManager ();
}

class QWidget *MainWindow::centralWidget() const
{
  return d->win->centralWidget();
}

ToolViewManager *MainWindow::toolViewManager () const
{
  return d->win->toolViewManager ();
}

}

// kate: space-indent on; indent-width 2; replace-tabs on;
