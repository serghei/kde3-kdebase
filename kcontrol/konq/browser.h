/* This file is part of the KDE project
   Copyright (C) 2002 Waldo Bastian <bastian@kde.org>

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


#ifndef __KBROWSER_OPTIONS_H__
#define __KBROWSER_OPTIONS_H__

#include <kcmodule.h>

class KConfig;
class QTabWidget;

//-----------------------------------------------------------------------------

class KBrowserOptions : public KCModule {
    Q_OBJECT
public:
    KBrowserOptions(KConfig *config, QString group, QWidget *parent = 0, const char *name = 0);

    virtual void load();
    virtual void save();
    virtual void defaults();
    virtual QString quickHelp() const;

private:
    KCModule *appearance;
    KCModule *behavior;
    KCModule *previews;
    KCModule *kuick;
    QTabWidget *m_tab;
};

#endif
