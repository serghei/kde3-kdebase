/* This file is part of the KDE project
   Copyright (C) 2002 Lubos Lunak <l.lunak@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KONQUEROR_PRELOADER_H
#define _KONQUEROR_PRELOADER_H

#include <kdedmodule.h>
#include <qtimer.h>

class KonqyPreloader : public KDEDModule {
    Q_OBJECT
    K_DCOP
public:
    KonqyPreloader(const QCString &obj);
    virtual ~KonqyPreloader();
    k_dcop : bool registerPreloadedKonqy(QCString id, int screen);
    QCString getPreloadedKonqy(int screen);
    ASYNC unregisterPreloadedKonqy(QCString id);
    void reconfigure();
    void unloadAllPreloaded();
private slots:
    void appRemoved(const QCString &id);
    void checkAlwaysPreloaded();

private:
    void updateCount();
    struct KonqyData
    {
        KonqyData(){}; // for QValueList
        KonqyData(const QCString &id_P, int screen_P) : id(id_P), screen(screen_P)
        {
        }
        QCString id;
        int screen;
    };
    typedef QValueList< KonqyData > InstancesList;
    InstancesList instances;
    QTimer check_always_preloaded_timer;
};

#endif
