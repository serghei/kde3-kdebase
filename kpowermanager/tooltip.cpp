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

#include <kiconloader.h>
#include <klocale.h>

#include "kickertip.h"

#include "tooltip.h"


static int groupSpace = 1;
static int decimals = 1;


Tooltip::Tooltip(const QString &title, const QString &icon, KickerTip::Data &data) : tip(data.subtext)
{
    data.message = title;
    data.icon = DesktopIcon(icon, KIcon::SizeMedium);
    // data.direction = popupDirection();
    data.duration = 0;

    tip = QString::null;
}


void Tooltip::addGroup(const QString &groupName)
{
    group = QString::null;

    if(!tip.isEmpty())
        for(int i = 0; i <= groupSpace; i++)
            group += "<br>";

    group += "<nobr><b>&nbsp;&nbsp;" + groupName + ":</b>&nbsp;&nbsp;</nobr>";
}


void Tooltip::addItem(const QString &label, const QString &value, const QString &suffix)
{
    if(!group.isEmpty())
    {
        tip += group;
        group = QString::null;
    }
    tip += "<br><nobr>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" + label + ": <b>" + value + "</b>" + suffix + "&nbsp;&nbsp;</nobr>";
}


void Tooltip::addItemPercent(const QString &label, double value)
{
    addItem(label, QString::number(value, 'f', decimals), "%");
}


void Tooltip::addItemTime(const QString &label, Q_INT64 seconds)
{
    QString time, suff;

    int h = seconds / 3600;
    int m = (seconds - h * 3600) / 60;

    if(h > 0)
        time.sprintf("%d:%02d", h, m);
    else
        time = QString::number(m);

    if(1 == h && 0 == m)
        suff = i18n("hour");
    else if(1 <= h)
        suff = i18n("hours");
    else if(1 == m)
        suff = i18n("minute");
    else
        suff = i18n("minutes");

    addItem(label, time, " " + suff);
}
