/* This file is part of the KDE Project
   Copyright (c) 2005 K�vin Ottens <ervin ipsquad net>

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

#include "homedirnotifymodule.h"

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

HomeDirNotifyModule::HomeDirNotifyModule(const QCString &obj) : KDEDModule(obj)
{
}

extern "C" {
KDE_EXPORT KDEDModule *create_homedirnotify(const QCString &obj)
{
    KGlobal::locale()->insertCatalogue("kio_home");
    return new HomeDirNotifyModule(obj);
}
}
