/*
 * Copyright (C) 2012-2014 Serghei Amelian <serghei.amelian@gmail.com>
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

#ifndef _UPOWER_BACKEND_H_
#define _UPOWER_BACKEND_H_


#include <qobject.h>
#include <config.h>


struct PowerSources;


class UPowerBackend : public QObject {

    Q_OBJECT

public:
    UPowerBackend();
    ~UPowerBackend();

public slots:
    void suspend();
    void hibernate();

private slots:
    void initialize();
    void suspendDelayed();
    void hibernateDelayed();

#ifdef WITH_LOGIND
private:
    bool sendCmdToLogind(const QString &cmd, QString &error);
#endif

signals:
    void powerSourcesChanged(const PowerSources&);

private:
    struct Private; Private *d;
};


#endif
