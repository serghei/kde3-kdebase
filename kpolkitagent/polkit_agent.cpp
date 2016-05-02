/*
 * Copyright (C) 2012 Serghei Amelian <serghei.amelian@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <polkitagent/polkitagent.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "listener.h"

#include "polkit_agent.h"


// the Authority
static PolkitAuthority *authority = NULL;

// the session we are servicing
static PolkitSubject *session = NULL;


PolkitAgent::PolkitAgent(const QCString &obj)
    : KDEDModule(obj), listener(NULL)
{
    initialize();
}


PolkitAgent::~PolkitAgent()
{
    cleanup();
}


void PolkitAgent::initialize()
{
    ::g_type_init();

    GError *error = NULL;
    authority = ::polkit_authority_get_sync(NULL /* GCancellable* */, &error);
    if(NULL == authority) {
        kdDebug() << "Error getting authority: " << error->message << endl;
        goto err;
    }

    listener = ::polkit_kde3_listener_new();

    error = NULL;
    session = ::polkit_unix_session_new_for_process_sync (::getpid(), NULL, &error);
    if(NULL != error) {
        kdDebug() << "Unable to determine the session we are in: " << error->message << endl;
        goto err;
    }

    error = NULL;
    if(!::polkit_agent_listener_register(
                listener,
                POLKIT_AGENT_REGISTER_FLAGS_NONE,
                session,
                "/org/kde3/PolicyKit1/AuthenticationAgent",
                NULL, /* GCancellable */
                &error))
    {
        kdDebug() << "Cannot register authentication agent: " << error->message << endl;
        goto err;
    }

    return;

err:
    if(error) {
        ::g_error_free(error);
    }
    cleanup();
}


void PolkitAgent::cleanup()
{
    if(NULL != authority) {
        ::g_object_unref(authority);
        authority = NULL;
    }

    if(NULL != session) {
        ::g_object_unref(session);
        session = NULL;
    }

    if(NULL != listener) {
        ::g_object_unref(listener);
        listener = NULL;
    }
}


extern "C"
{
    KDE_EXPORT KDEDModule *create_polkit_agent(const QCString &name)
    {
        KGlobal::locale()->insertCatalogue("polkit_agent");
        return new PolkitAgent(name);
    }
}
