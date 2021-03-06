/* This file is part of the KDE project
   Copyright (c) 2005 Kevin Ottens <ervin ipsquad net>

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

#ifndef HOMEIMPL_H
#define HOMEIMPL_H

#include <kio/global.h>
#include <kio/job.h>
#include <kurl.h>
#include <kuser.h>

#include <qstring.h>

class HomeImpl : public QObject {
    Q_OBJECT

public:
    HomeImpl();
    bool parseURL(const KURL &url, QString &name, QString &path) const;
    bool realURL(const QString &name, const QString &path, KURL &url);

    bool statHome(const QString &name, KIO::UDSEntry &entry);
    bool listHomes(QValueList< KIO::UDSEntry > &list);

    void createTopLevelEntry(KIO::UDSEntry &entry) const;

private slots:
    void slotStatResult(KIO::Job *job);

private:
    void createHomeEntry(KIO::UDSEntry &entry, const KUser &user);

    KIO::UDSEntry extractUrlInfos(const KURL &url);
    KIO::UDSEntry m_entryBuffer;


    long m_effectiveUid;
};

#endif
