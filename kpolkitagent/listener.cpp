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
 * Based on polkit-gnome, developed by David Zeuthen <davidz@redhat.com>
 */

#include <klocale.h>
#include <kdebug.h>

#include "listener.h"
#include "authenticator.h"


struct _PolkitKDE3Listener
{
    PolkitAgentListener parent_instance;

    /* we support multiple authenticators - they are simply queued up */
    GList *authenticators;

    PolkitKDE3Authenticator *active_authenticator;
};


struct _PolkitKDE3ListenerClass
{
    PolkitAgentListenerClass parent_class;
};


static void polkit_kde3_listener_initiate_authentication(PolkitAgentListener *listener, const gchar *action_id, const gchar *message,
                                                         const gchar *icon_name, PolkitDetails *details, const gchar *cookie, GList *identities,
                                                         GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);

static gboolean polkit_kde3_listener_initiate_authentication_finish(PolkitAgentListener *listener, GAsyncResult *res, GError **error);


G_DEFINE_TYPE(PolkitKDE3Listener, polkit_kde3_listener, POLKIT_AGENT_TYPE_LISTENER);


static void polkit_kde3_listener_init(PolkitKDE3Listener *listener)
{
}


static void polkit_kde3_listener_finalize(GObject *object)
{
    if(G_OBJECT_CLASS(polkit_kde3_listener_parent_class)->finalize != NULL)
        G_OBJECT_CLASS(polkit_kde3_listener_parent_class)->finalize(object);
}


static void polkit_kde3_listener_class_init(PolkitKDE3ListenerClass *klass)
{
    GObjectClass *gobject_class;
    PolkitAgentListenerClass *listener_class;

    gobject_class = G_OBJECT_CLASS(klass);
    listener_class = POLKIT_AGENT_LISTENER_CLASS(klass);

    gobject_class->finalize = polkit_kde3_listener_finalize;

    listener_class->initiate_authentication = polkit_kde3_listener_initiate_authentication;
    listener_class->initiate_authentication_finish = polkit_kde3_listener_initiate_authentication_finish;
}


PolkitAgentListener *polkit_kde3_listener_new(void)
{
    return POLKIT_AGENT_LISTENER(g_object_new(POLKIT_KDE3_TYPE_LISTENER, NULL));
}


typedef struct
{
    PolkitKDE3Listener *listener;
    PolkitKDE3Authenticator *authenticator;

    GSimpleAsyncResult *simple;
    GCancellable *cancellable;

    gulong cancel_id;
} AuthData;


static AuthData *auth_data_new(PolkitKDE3Listener *listener, PolkitKDE3Authenticator *authenticator, GSimpleAsyncResult *simple,
                               GCancellable *cancellable)
{
    AuthData *data;

    data = g_new0(AuthData, 1);
    data->listener = static_cast< PolkitKDE3Listener * >(g_object_ref(listener));
    data->authenticator = static_cast< PolkitKDE3Authenticator * >(g_object_ref(authenticator));
    data->simple = static_cast< GSimpleAsyncResult * >(g_object_ref(simple));
    data->cancellable = static_cast< GCancellable * >(g_object_ref(cancellable));
    return data;
}


static void auth_data_free(AuthData *data)
{
    g_object_unref(data->listener);
    g_object_unref(data->authenticator);
    g_object_unref(data->simple);
    if(data->cancellable != NULL && data->cancel_id > 0)
        g_signal_handler_disconnect(data->cancellable, data->cancel_id);
    g_object_unref(data->cancellable);
    g_free(data);
}


static void maybe_initiate_next_authenticator(PolkitKDE3Listener *listener)
{
    if(listener->active_authenticator == NULL && listener->authenticators != NULL)
    {
        polkit_kde3_authenticator_initiate(POLKIT_KDE3_AUTHENTICATOR(listener->authenticators->data));
        listener->active_authenticator = static_cast< PolkitKDE3Authenticator * >(listener->authenticators->data);
    }
}


static void authenticator_completed(PolkitKDE3Authenticator *authenticator, gboolean gained_authorization, gboolean dismissed, gpointer user_data)
{
    AuthData *data = static_cast< AuthData * >(user_data);

    data->listener->authenticators = g_list_remove(data->listener->authenticators, authenticator);

    if(authenticator == data->listener->active_authenticator)
        data->listener->active_authenticator = NULL;

    g_object_unref(authenticator);

    if(dismissed)
    {
        g_simple_async_result_set_error(data->simple, POLKIT_ERROR, POLKIT_ERROR_CANCELLED, "%s",
                                        i18n("Authentication dialog was dismissed by the user").ascii());
    }
    g_simple_async_result_complete(data->simple);
    g_object_unref(data->simple);

    maybe_initiate_next_authenticator(data->listener);

    auth_data_free(data);
}


static void cancelled_cb(GCancellable *cancellable, gpointer user_data)
{
    AuthData *data = static_cast< AuthData * >(user_data);

    polkit_kde3_authenticator_cancel(data->authenticator);
}


static void polkit_kde3_listener_initiate_authentication(PolkitAgentListener *agent_listener, const gchar *action_id, const gchar *message,
                                                         const gchar *icon_name, PolkitDetails *details, const gchar *cookie, GList *identities,
                                                         GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data)
{
    kdDebug() << "polkit_kde3_listener_initiate_authentication()" << endl;


    PolkitKDE3Listener *listener = POLKIT_KDE3_LISTENER(agent_listener);
    GSimpleAsyncResult *simple;
    PolkitKDE3Authenticator *authenticator;
    AuthData *data;

    simple = g_simple_async_result_new(G_OBJECT(listener), callback, user_data, (gpointer)polkit_kde3_listener_initiate_authentication);

    authenticator = polkit_kde3_authenticator_new(action_id, message, icon_name, details, cookie, identities);

    if(authenticator == NULL)
    {
        g_simple_async_result_set_error(simple, POLKIT_ERROR, POLKIT_ERROR_FAILED, "Error creating authentication object");
        g_simple_async_result_complete(simple);
        return;
    }

    data = auth_data_new(listener, authenticator, simple, cancellable);

    g_signal_connect(authenticator, "completed", G_CALLBACK(authenticator_completed), data);

    if(cancellable != NULL)
    {
        data->cancel_id = g_signal_connect(cancellable, "cancelled", G_CALLBACK(cancelled_cb), data);
    }

    listener->authenticators = g_list_append(listener->authenticators, authenticator);

    maybe_initiate_next_authenticator(listener);
}


static gboolean polkit_kde3_listener_initiate_authentication_finish(PolkitAgentListener *listener, GAsyncResult *res, GError **error)
{
    GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);

    g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == polkit_kde3_listener_initiate_authentication);

    if(g_simple_async_result_propagate_error(simple, error))
        return FALSE;

    return TRUE;
}
