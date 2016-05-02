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

#ifndef _TOOLTIP_H_
#define _TOOLTIP_H_


class Tooltip {
public:
    Tooltip(const QString &title, const QString &icon, KickerTip::Data &data);

    void addGroup(const QString &groupName);
    void addItem(const QString &label, const QString &value, const QString &suffix = QString::null);

    void addItemPercent(const QString &label, double value);
    void addItemTime(const QString &label, Q_INT64 seconds);

private:
    QString &tip;
    QString group;
};


#endif
