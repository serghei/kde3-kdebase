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

#include <polkit/polkit.h>
#include <polkitagent/polkitagent.h>

#include <qeventloop.h>

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>

#include "dialog.h"

#include "authenticator.h"


struct _PolkitKDE3Authenticator
{
    GObject parent_instance;

    PolkitAuthority *authority;
    gchar *cookie;

    gboolean gained_authorization;
    gboolean was_cancelled;
    gboolean new_user_selected;

    PolkitAgentSession *session;
    AuthenticationDialog *dialog;
    GMainLoop *loop;
};


struct _PolkitKDE3AuthenticatorClass
{
    GObjectClass parent_class;
};


enum
{
    COMPLETED_SIGNAL,
    LAST_SIGNAL,
};

static guint _signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE(PolkitKDE3Authenticator, polkit_kde3_authenticator, G_TYPE_OBJECT);


static void polkit_kde3_authenticator_init(PolkitKDE3Authenticator *authenticator)
{
}


static void polkit_kde3_authenticator_finalize(GObject *object)
{
    PolkitKDE3Authenticator *authenticator;

    authenticator = POLKIT_KDE3_AUTHENTICATOR(object);

    if(authenticator->authority != NULL)
        g_object_unref(authenticator->authority);

    g_free(authenticator->cookie);

    if(authenticator->session != NULL)
        g_object_unref(authenticator->session);
    if(authenticator->dialog != NULL)
        delete authenticator->dialog;
    if(authenticator->loop != NULL)
        g_main_loop_unref(authenticator->loop);

    if(G_OBJECT_CLASS(polkit_kde3_authenticator_parent_class)->finalize != NULL)
        G_OBJECT_CLASS(polkit_kde3_authenticator_parent_class)->finalize(object);
}


static void polkit_kde3_authenticator_class_init(PolkitKDE3AuthenticatorClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->finalize = polkit_kde3_authenticator_finalize;

    _signals[COMPLETED_SIGNAL] = g_signal_new("completed", POLKIT_KDE3_TYPE_AUTHENTICATOR, G_SIGNAL_RUN_LAST, 0, /* class offset     */
                                              NULL,                                                              /* accumulator      */
                                              NULL,                                                              /* accumulator data */
                                              g_cclosure_marshal_generic, G_TYPE_NONE, 2, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);
}


static PolkitActionDescription *get_desc_for_action(PolkitAuthority *authority, const QString &action_id)
{
    GList *action_descs;
    GList *l;
    PolkitActionDescription *result;

    result = NULL;

    action_descs = ::polkit_authority_enumerate_actions_sync(authority, NULL, NULL);

    for(l = action_descs; l != NULL; l = l->next)
    {
        PolkitActionDescription *action_desc = POLKIT_ACTION_DESCRIPTION(l->data);

        if(::polkit_action_description_get_action_id(action_desc) == action_id)
        {
            result = static_cast< PolkitActionDescription * >(g_object_ref(action_desc));
            goto out;
        }
    }

out:
    g_list_foreach(action_descs, (GFunc)g_object_unref, NULL);
    g_list_free(action_descs);

    return result;
}


// static void on_dialog_deleted(GtkWidget *widget,
//                   GdkEvent  *event,
//                   gpointer   user_data)
//{
//    PolkitKDE3Authenticator *authenticator = POLKIT_KDE3_AUTHENTICATOR(user_data);

//    polkit_kde3_authenticator_cancel(authenticator);
//}


static void on_user_selected(GObject *object, GParamSpec *pspec, gpointer user_data)
{
    PolkitKDE3Authenticator *authenticator = POLKIT_KDE3_AUTHENTICATOR(user_data);

    /* clear any previous messages */
    //    polkit_kde3_authentication_dialog_set_info_message(POLKIT_KDE3_AUTHENTICATION_DIALOG(authenticator->dialog), "");

    polkit_kde3_authenticator_cancel(authenticator);
    authenticator->new_user_selected = TRUE;
}


PolkitKDE3Authenticator *polkit_kde3_authenticator_new(const gchar *action_id, const gchar *message, const gchar *icon_name, PolkitDetails *details,
                                                       const gchar *cookie, GList *identities)
{
    PolkitKDE3Authenticator *authenticator;
    PolkitActionDescription *action_desc;
    GError *error;
    char **keys;

    QMap< QString, QString > mapDetails;
    QStringList listUsers;


    authenticator = POLKIT_KDE3_AUTHENTICATOR(g_object_new(POLKIT_KDE3_TYPE_AUTHENTICATOR, NULL));

    error = NULL;
    authenticator->authority = polkit_authority_get_sync(NULL /* GCancellable* */, &error);
    if(authenticator->authority == NULL)
    {
        g_critical("Error getting authority: %s", error->message);
        g_error_free(error);
        goto error;
    }

    authenticator->cookie = g_strdup(cookie);

    action_desc = get_desc_for_action(authenticator->authority, action_id);
    if(action_desc == NULL)
        goto error;

    mapDetails.insert(i18n("Action"), action_id);
    mapDetails.insert(i18n("Vendor"), QString("<a href='%2'>%1</a>")
                                          .arg(::polkit_action_description_get_vendor_name(action_desc))
                                          .arg(polkit_action_description_get_vendor_url(action_desc)));

    ::g_object_unref(action_desc);

    if(details)
    {
        keys = ::polkit_details_get_keys(details);
        for(int i = 0; keys[i]; i++)
        {
            const char *key = keys[i];
            if(::g_str_has_prefix(key, "polkit."))
                continue;
            const char *value = ::polkit_details_lookup(details, key);
            mapDetails.insert(key, value);
        }
        ::g_strfreev(keys);
    }

    for(GList *l = identities; l != NULL; l = l->next)
    {
        PolkitUnixUser *unixUser = POLKIT_UNIX_USER(l->data);
        listUsers << polkit_unix_user_get_name(unixUser);
    }

    authenticator->dialog = new AuthenticationDialog(action_id, icon_name, message, mapDetails, listUsers);

    //    g_signal_connect(authenticator->dialog,
    //                      "delete-event",
    //                      G_CALLBACK(on_dialog_deleted),
    //                      authenticator);
    //    g_signal_connect(authenticator->dialog,
    //                      "notify::selected-user",
    //                      G_CALLBACK(on_user_selected),
    //                      authenticator);*/

    return authenticator;

error:
    g_object_unref(authenticator);
    return NULL;
}


static void session_request(PolkitAgentSession *session, const char *request, gboolean /*echo_on*/, gpointer user_data)
{
    PolkitKDE3Authenticator *authenticator = POLKIT_KDE3_AUTHENTICATOR(user_data);
    AuthenticationDialog *authenticationDialog = authenticator->dialog;

    // Fix up, and localize, password prompt if it's password auth
    if(::g_ascii_strncasecmp(request, "password:", 9) == 0)
        authenticationDialog->setPasswordLabel(i18n("Password:"));
    else
        authenticationDialog->setPasswordLabel(request);

    if(authenticator->dialog->exec())
        ::polkit_agent_session_response(authenticator->session, authenticationDialog->password().ascii());
    else
        ::polkit_kde3_authenticator_cancel(authenticator);
}


static void session_show_error(PolkitAgentSession *session, const gchar *msg, gpointer user_data)
{
    //    PolkitKDE3Authenticator *authenticator = POLKIT_KDE3_AUTHENTICATOR(user_data);
    //    gchar *s;

    //    s = g_strconcat("<b>", msg, "</b>", NULL);
    //    polkit_kde3_authentication_dialog_set_info_message(POLKIT_KDE3_AUTHENTICATION_DIALOG(authenticator->dialog), s);
    //    g_free(s);
}


static void session_show_info(PolkitAgentSession *session, const gchar *msg, gpointer user_data)
{
    //    PolkitKDE3Authenticator *authenticator = POLKIT_KDE3_AUTHENTICATOR(user_data);
    //    gchar *s;

    //    s = g_strconcat("<b>", msg, "</b>", NULL);

    //    polkit_kde3_authentication_dialog_set_info_message(POLKIT_KDE3_AUTHENTICATION_DIALOG(authenticator->dialog), s);
    //    g_free(s);
}


static void session_completed(PolkitAgentSession *session, gboolean gained_authorization, gpointer user_data)
{
    PolkitKDE3Authenticator *authenticator = POLKIT_KDE3_AUTHENTICATOR(user_data);

    authenticator->gained_authorization = gained_authorization;

    ::g_main_loop_quit(authenticator->loop);
}


static gboolean do_initiate(gpointer user_data)
{
    PolkitKDE3Authenticator *authenticator = POLKIT_KDE3_AUTHENTICATOR(user_data);
    PolkitIdentity *identity;
    gint num_tries;

    //    if(!polkit_kde3_authentication_dialog_run_until_user_is_selected(POLKIT_KDE3_AUTHENTICATION_DIALOG(authenticator->dialog)))
    //    {
    //        /* user cancelled the dialog */
    //        /*g_debug("User cancelled before selecting a user");*/
    //        authenticator->was_cancelled = TRUE;
    //        goto out;
    //    }

    authenticator->loop = ::g_main_loop_new(NULL, TRUE);

    num_tries = 0;

try_again:
    identity = polkit_unix_user_new_for_name(authenticator->dialog->username().ascii(), NULL);

    authenticator->session = polkit_agent_session_new(identity, authenticator->cookie);

    g_object_unref(identity);

    g_signal_connect(authenticator->session, "request", G_CALLBACK(session_request), authenticator);

    g_signal_connect(authenticator->session, "show-info", G_CALLBACK(session_show_info), authenticator);

    g_signal_connect(authenticator->session, "show-error", G_CALLBACK(session_show_error), authenticator);

    g_signal_connect(authenticator->session, "completed", G_CALLBACK(session_completed), authenticator);


    ::polkit_agent_session_initiate(authenticator->session);
    ::g_main_loop_run(authenticator->loop);

    if(authenticator->new_user_selected)
    {
        /*g_debug("New user selected");*/
        authenticator->new_user_selected = FALSE;
        g_object_unref(authenticator->session);
        authenticator->session = NULL;
        goto try_again;
    }

    num_tries++;

    if(!authenticator->gained_authorization && !authenticator->was_cancelled)
    {
        if(authenticator->dialog != NULL)
        {
            //            gchar *s;

            //            s = g_strconcat("<b>", _("Authentication Failure"), "</b>", NULL);
            //            polkit_kde3_authentication_dialog_set_info_message(
            //                        POLKIT_KDE3_AUTHENTICATION_DIALOG(authenticator->dialog),
            //                        s);
            //            g_free(s);
            //            /// gtk_widget_queue_draw(authenticator->dialog);

            //            /* shake the dialog to indicate error */
            //            polkit_kde3_authentication_dialog_indicate_error(POLKIT_KDE3_AUTHENTICATION_DIALOG(authenticator->dialog));

            if(num_tries < 3)
            {
                g_object_unref(authenticator->session);
                authenticator->session = NULL;
                goto try_again;
            }
        }
    }

out:
    g_signal_emit_by_name(authenticator, "completed", authenticator->gained_authorization, authenticator->was_cancelled);

    g_object_unref(authenticator);

    return FALSE;
}


void polkit_kde3_authenticator_initiate(PolkitKDE3Authenticator *authenticator)
{
    /* run from idle since we're going to block the main loop in the dialog(which has a recursive mainloop) */
    ::g_idle_add(do_initiate, ::g_object_ref(authenticator));
}


void polkit_kde3_authenticator_cancel(PolkitKDE3Authenticator *authenticator)
{
    if(authenticator->dialog)
        authenticator->dialog->close();

    authenticator->was_cancelled = TRUE;

    if(authenticator->session != NULL)
    {
        ::polkit_agent_session_cancel(authenticator->session);
    }
}
