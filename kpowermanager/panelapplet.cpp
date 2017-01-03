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

#include <math.h>

#include <qpainter.h>
#include <qtooltip.h>

#include <kaboutapplication.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kickertip.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include "powersources.h"
#include "tooltip.h"

#include "panelapplet.h"


static const char *appName = "kpowermanager";


static QString qBatteryStatusToText(int state)
{
    switch(state)
    {
        case PowerSources::Battery::Charging:
            return i18n("charging");
        case PowerSources::Battery::Discharging:
            return i18n("discharging");
        case PowerSources::Battery::Empty:
            return i18n("empty");
        case PowerSources::Battery::FullCharged:
            return i18n("fully charged");
        case PowerSources::Battery::PendingCharge:
            return i18n("pending charge");
        case PowerSources::Battery::PendingDischarge:
            return i18n("pending discharge");
    }

    return i18n("unknown");
}


PanelApplet::PanelApplet(QWidget *parent, const QString &configFile)
    : KPanelApplet(configFile, KPanelApplet::Normal, KPanelApplet::About, parent, appName)
{
    updateIcon(PowerSources());

    connect(&cpuMonitor, SIGNAL(cpuUsageChanged()), SLOT(updateCpuUsageIcon()));
    connect(&backend, SIGNAL(powerSourcesChanged(const PowerSources &)), SLOT(updateIcon(const PowerSources &)));

    installEventFilter(KickerTip::the());
}


PanelApplet::~PanelApplet()
{
}


int PanelApplet::widthForHeight(int height) const
{
    return 42;
}


int PanelApplet::heightForWidth(int width) const
{
    return 22;
}


void PanelApplet::about()
{
    KAboutData data(appName, I18N_NOOP("KPowerManager"), "1.1", I18N_NOOP("The KDE3 Power Manager"), KAboutData::License_GPL_V2,
                    "(c) 2012-2017, KDE3 Desktop Environment");

    data.setHomepage("http://github.com/serghei");

    data.addAuthor("Serghei Amelian", I18N_NOOP("Developer & maintainer"), "serghei.amelian@gmail.com");

    KAboutApplication dialog(&data);
    dialog.exec();
}


void PanelApplet::updateCpuUsageIcon()
{
    // update the tooltip
    KickerTip::Client::updateKickerTip();

    // repaint the applet area
    update();
}


void PanelApplet::updateIcon(const PowerSources &ps)
{
    this->ps = ps;

    const char *iconName = "ac";
    KIcon::States iconState = KIcon::DisabledState;

    if(ps.daemon.connected)
    {
        iconState = KIcon::ActiveState;
        switch(ps.battery.state)
        {
            case PowerSources::Battery::Charging:
                iconName = "battery_charging";
                break;
            case PowerSources::Battery::Discharging:
                iconName = "battery";
                break;
        }
    }

    QPixmap pixmap = KIconLoader(appName).loadIcon(iconName, KIcon::Toolbar, 0, iconState);

    // draw battery percentage
    if(PowerSources::Battery::Charging == ps.battery.state || PowerSources::Battery::Discharging == ps.battery.state)
    {
        QPainter p(&pixmap);
        int level = static_cast< int >(round(16. * ps.battery.percentage / 100.));
        p.fillRect(15, 4 + (16 - level), 5, level, QColor(0x00, 0xff, 0x00));
    }

    // update the tooltip
    KickerTip::Client::updateKickerTip();

    // repaint the applet area
    icon = pixmap;
    update();
}


void PanelApplet::updateKickerTip(KickerTip::Data &data)
{
    Tooltip tip("KPowerManager", appName, data);

    if(!ps.daemon.connected)
    {
        data.subtext = i18n("KPowerManger is not connected to UPower daemon.");
        return;
    }

    // Power AC
    tip.addGroup(i18n("Power AC"));
    tip.addItem(i18n("Status"), ps.powerLine.online ? i18n("online") : i18n("unplugged"));

    // Battery
    tip.addGroup(i18n("Battery"));
    tip.addItem(i18n("Status"), qBatteryStatusToText(ps.battery.state));
    tip.addItemPercent(i18n("Charged"), ps.battery.percentage);
    tip.addItemPercent(i18n("Capacity"), ps.battery.capacity);
    if(0 < ps.battery.timeToEmpty)
        tip.addItemTime(i18n("Time to empty"), ps.battery.timeToEmpty);
    if(0 < ps.battery.timeToFull)
        tip.addItemTime(i18n("Time to full"), ps.battery.timeToFull);
    if(!ps.powerLine.online)
        tip.addItem(i18n("Energy rate"), QString::number(ps.battery.energyRate, 'f', 1), " W");

    // CPU monitor
    tip.addGroup(i18n("CPU Monitor"));
    tip.addItemPercent(i18n("CPU Usage"), cpuMonitor.cpuUsage());

// Backlight
#ifdef HAVE_XRANDR
    if(backlight.maxBrightness() > 0)
    {
        tip.addGroup("Backlight");
        tip.addItem(i18n("Brightness level"), QString::number(backlight.brightness() + 1), i18n(" (of %2)").arg(backlight.maxBrightness() + 1));
    }
#endif
}


void PanelApplet::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::RightButton)
    {
        KPopupMenu menu(this);

        menu.insertTitle(SmallIcon(appName), i18n("KPowerManager"));

        menu.insertItem(SmallIcon("memory"), i18n("&Suspend"), &backend, SLOT(suspend()));
        menu.insertItem(SmallIcon("hdd_unmount"), i18n("Hi&bernate"), &backend, SLOT(hibernate()));
        menu.insertSeparator();

        KPopupMenu helpSubmenu(this);
        menu.insertItem(SmallIcon("help"), i18n("&Help"), &helpSubmenu);
        helpSubmenu.insertItem(SmallIcon(appName), i18n("&About KPowerManager"), this, SLOT(about()));

        menu.exec(this->mapToGlobal(e->pos()));
    }
}


void PanelApplet::wheelEvent(QWheelEvent *e)
{
#ifdef HAVE_XRANDR
    int level = backlight.brightness() + e->delta() / 120;

    if(backlight.minBrightness() <= level && backlight.maxBrightness() >= level)
    {
        backlight.setBrightness(level);

        // update the tooltip
        KickerTip::Client::updateKickerTip();
    }
#endif
}


void PanelApplet::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    // prepare x/y coords for centering icon
    int x = (width() - icon.width()) / 2;
    int y = (height() - icon.height()) / 2;

    // draw the icon on applet area
    p.drawPixmap(0, KMAX(y, 0), icon);

    // draw "disabled" bars of cpu usage
    p.setPen(QColor(0xb0, 0xb0, 0xb0));
    for(int i = 0; i < 21; i += 2)
        p.drawLine(26, y + 21 - i, 40, y + 21 - i);

    int cpuUsageLines = static_cast<int>(ceil(static_cast<double>(cpuMonitor.cpuUsage()) * 21. / 100.));

    // draw the "active" bars of cpu usage
    p.setPen(QColor(0x00, 0x00, 0x00));
    for(int i = 0; i < cpuUsageLines; i += 2)
        p.drawLine(26, y + 21 - i, 40, y + 21 - i);
}


extern "C" {
KDE_EXPORT KPanelApplet *init(QWidget *parent, const QString &configFile)
{
    KGlobal::locale()->insertCatalogue(appName);
    return new PanelApplet(parent, configFile);
}
}


#include "panelapplet.moc"
