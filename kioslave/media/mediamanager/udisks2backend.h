/* This file is part of the KDE3 Fork Project
   Copyright (c) 2012 Serghei Amelian <serghei.amelian@gmail.com>

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

#ifndef _UDISKS2BACKEND_H_
#define _UDISKS2BACKEND_H_

#include "backendbase.h"


namespace UDisks2 {
class ObjectManager;
}


class UDisks2Backend : public BackendBase {
public:
    UDisks2Backend(MediaList &mediaList);
    ~UDisks2Backend();

    bool initialize();

    QString mount(const QString &name);
    QString unmount(const QString &name);

private:
    UDisks2::ObjectManager *d;
};


#endif
