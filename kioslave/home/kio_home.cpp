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

#include <stdlib.h>

#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kcmdlineargs.h>
#include <kglobal.h>


#include "kio_home.h"

static const KCmdLineOptions options[] = {{"+protocol", I18N_NOOP("Protocol name"), 0},
                                          {"+pool", I18N_NOOP("Socket name"), 0},
                                          {"+app", I18N_NOOP("Socket name"), 0},
                                          KCmdLineLastOption};

extern "C" {
int KDE_EXPORT kdemain(int argc, char **argv)
{
    // KApplication is necessary to use other ioslaves
    putenv(strdup("SESSION_MANAGER="));
    KCmdLineArgs::init(argc, argv, "kio_home", 0, 0, 0, 0);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app(false, false);
    // We want to be anonymous even if we use DCOP
    app.dcopClient()->attach();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    HomeProtocol slave(args->arg(0), args->arg(1), args->arg(2));
    slave.dispatchLoop();
    return 0;
}
}


HomeProtocol::HomeProtocol(const QCString &protocol, const QCString &pool, const QCString &app) : ForwardingSlaveBase(protocol, pool, app)
{
}

HomeProtocol::~HomeProtocol()
{
}

bool HomeProtocol::rewriteURL(const KURL &url, KURL &newUrl)
{
    QString name, path;

    if(!m_impl.parseURL(url, name, path))
    {
        error(KIO::ERR_MALFORMED_URL, url.prettyURL());
        return false;
    }


    if(!m_impl.realURL(name, path, newUrl))
    {
        error(KIO::ERR_MALFORMED_URL, url.prettyURL());
        return false;
    }

    return true;
}


void HomeProtocol::listDir(const KURL &url)
{
    kdDebug() << "HomeProtocol::listDir: " << url << endl;

    if(url.path().length() <= 1)
    {
        listRoot();
        return;
    }

    QString name, path;
    bool ok = m_impl.parseURL(url, name, path);

    if(!ok)
    {
        error(KIO::ERR_MALFORMED_URL, url.prettyURL());
        return;
    }

    ForwardingSlaveBase::listDir(url);
}

void HomeProtocol::listRoot()
{
    KIO::UDSEntry entry;

    KIO::UDSEntryList home_entries;
    bool ok = m_impl.listHomes(home_entries);

    if(!ok) // can't happen
    {
        error(KIO::ERR_UNKNOWN, "");
        return;
    }

    totalSize(home_entries.count() + 1);

    m_impl.createTopLevelEntry(entry);
    listEntry(entry, false);

    KIO::UDSEntryListIterator it = home_entries.begin();
    KIO::UDSEntryListIterator end = home_entries.end();

    for(; it != end; ++it)
    {
        listEntry(*it, false);
    }

    entry.clear();
    listEntry(entry, true);

    finished();
}

void HomeProtocol::stat(const KURL &url)
{
    kdDebug() << "HomeProtocol::stat: " << url << endl;

    QString path = url.path();
    if(path.isEmpty() || path == "/")
    {
        // The root is "virtual" - it's not a single physical directory
        KIO::UDSEntry entry;
        m_impl.createTopLevelEntry(entry);
        statEntry(entry);
        finished();
        return;
    }

    QString name;
    bool ok = m_impl.parseURL(url, name, path);

    if(!ok)
    {
        error(KIO::ERR_MALFORMED_URL, url.prettyURL());
        return;
    }

    if(path.isEmpty())
    {
        KIO::UDSEntry entry;

        if(m_impl.statHome(name, entry))
        {
            statEntry(entry);
            finished();
        }
        else
        {
            error(KIO::ERR_DOES_NOT_EXIST, url.prettyURL());
        }
    }
    else
    {
        ForwardingSlaveBase::stat(url);
    }
}
