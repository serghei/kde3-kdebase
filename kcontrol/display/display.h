/* This file is part of the KDE project
   Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <kcmodule.h>

class QTabWidget;

class KCMDisplay : public KCModule {
    Q_OBJECT

public:
    KCMDisplay(QWidget *parent, const char *name, const QStringList &);
    void load();
    void save();

private slots:
    void moduleChanged(bool isChanged);

private:
    void addTab(const QString &name, const QString &label);

    QTabWidget *m_tabs;
    QMap< KCModule *, bool > m_modules;
    bool m_changed;
};

#endif // DISPLAY_H
