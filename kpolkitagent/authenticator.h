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

#ifndef _POLKIT_KDE3_AUTHENTICATOR_H_
#define _POLKIT_KDE3_AUTHENTICATOR_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define POLKIT_KDE3_TYPE_AUTHENTICATOR (polkit_kde3_authenticator_get_type())
#define POLKIT_KDE3_AUTHENTICATOR(o) (G_TYPE_CHECK_INSTANCE_CAST((o), POLKIT_KDE3_TYPE_AUTHENTICATOR, PolkitKDE3Authenticator))
#define POLKIT_KDE3_AUTHENTICATOR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), POLKIT_KDE3_TYPE_AUTHENTICATOR, PolkitKDE3AuthenticatorClass))
#define POLKIT_KDE3_AUTHENTICATOR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), POLKIT_KDE3_TYPE_AUTHENTICATOR, PolkitKDE3AuthenticatorClass))
#define POLKIT_KDE3_IS_AUTHENTICATOR(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), POLKIT_KDE3_TYPE_AUTHENTICATOR))
#define POLKIT_KDE3_IS_AUTHENTICATOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), POLKIT_KDE3_TYPE_AUTHENTICATOR))

typedef struct _PolkitKDE3Authenticator PolkitKDE3Authenticator;
typedef struct _PolkitKDE3AuthenticatorClass PolkitKDE3AuthenticatorClass;

GType polkit_kde3_authenticator_get_type(void) G_GNUC_CONST;

PolkitKDE3Authenticator *polkit_kde3_authenticator_new(const gchar *action_id, const gchar *message, const gchar *icon_name, PolkitDetails *details,
                                                       const gchar *cookie, GList *identities);

void polkit_kde3_authenticator_initiate(PolkitKDE3Authenticator *authenticator);
void polkit_kde3_authenticator_cancel(PolkitKDE3Authenticator *authenticator);
const gchar *polkit_kde3_authenticator_get_cookie(PolkitKDE3Authenticator *authenticator);

G_END_DECLS

#endif // _POLKIT_KDE3_AUTHENTICATOR_H_
