/*
    Naughty applet - Runaway process monitor for the KDE panel

    Copyright 2000 Rik Hemsley (rikkus) <rik@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef NAUGHTY_H
#define NAUGHTY_H

#include <kpanelapplet.h>
#include <qstringlist.h>

#include "simplebutton.h"

class NaughtyProcessMonitor;
class QPushButton;

class NaughtyApplet : public KPanelApplet {
    Q_OBJECT

public:
    NaughtyApplet(const QString &configFile, Type t = Normal, int actions = 0, QWidget *parent = 0, const char *name = 0);

    ~NaughtyApplet();

    virtual int widthForHeight(int h) const;
    virtual int heightForWidth(int w) const;

signals:

    void layoutChanged();

protected slots:

    void slotWarn(ulong pid, const QString &name);
    void slotLoad(uint);
    void slotPreferences();

protected:
    virtual void about();
    virtual void preferences();
    virtual void loadSettings();
    virtual void saveSettings();

private:
    NaughtyProcessMonitor *monitor_;
    SimpleButton *button_;
    QStringList ignoreList_;
};

#endif
