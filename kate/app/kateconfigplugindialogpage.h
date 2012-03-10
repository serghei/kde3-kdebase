/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>

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

#ifndef __KATE_CONFIGPLUGINDIALOGPAGE_H__
#define __KATE_CONFIGPLUGINDIALOGPAGE_H__

#include "katemain.h"
#include "katepluginmanager.h"

#include <klistview.h>

#include <qvbox.h>

class KatePluginListItem;

class KatePluginListView : public KListView
{
  Q_OBJECT

  friend class KatePluginListItem;

  public:
    KatePluginListView (QWidget *parent = 0, const char *name = 0);

  signals:
    void stateChange(KatePluginListItem *, bool);

  private:
    void stateChanged(KatePluginListItem *, bool);
};

class KateConfigPluginPage: public QVBox
{
  Q_OBJECT

  public:
    KateConfigPluginPage(QWidget *parent, class KateConfigDialog *dialog);
    ~KateConfigPluginPage(){;};

  private:
    class KateConfigDialog *myDialog;

  signals:
    void changed();

  private slots:
    void stateChange(KatePluginListItem *, bool);

    void loadPlugin (KatePluginListItem *);
    void unloadPlugin (KatePluginListItem *);
};

#endif
