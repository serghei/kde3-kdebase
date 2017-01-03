/*
 * Copyright (C) 2012 Serghei Amelian <serghei.amelian@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _PANEL_APPLET_H_
#define _PANEL_APPLET_H_

#include <qpixmap.h>
#include <kpanelapplet.h>

#include <config.h>

#ifdef HAVE_XRANDR
#include "backlight.h"
#endif

#include "cpumonitor.h"
#include "upowerbackend.h"


struct PowerSources;
class Tooltip;


class PanelApplet : public KPanelApplet, public KickerTip::Client {

    Q_OBJECT

public:
    PanelApplet(QWidget *parent, const QString &configFile);
    ~PanelApplet();

    int widthForHeight(int height) const;
    int heightForWidth(int width) const;

protected slots:
    void about();
    void updateCpuUsageIcon();
    void updateIcon(const PowerSources &ps);

protected:
    void updateKickerTip(KickerTip::Data &data);
    void mousePressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void paintEvent(QPaintEvent *e);

private:
    QPixmap icon;
    CpuMonitor cpuMonitor;
    PowerSources ps;
    UPowerBackend backend;
#ifdef HAVE_XRANDR
    Backlight backlight;
#endif
};


#endif
