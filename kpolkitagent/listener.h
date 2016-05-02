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

#ifndef _POLKIT_KDE3_LISTENER_H_
#define _POLKIT_KDE3_LISTENER_H_

#include <polkitagent/polkitagent.h>

G_BEGIN_DECLS

#define POLKIT_KDE3_TYPE_LISTENER (polkit_kde3_listener_get_type())
#define POLKIT_KDE3_LISTENER(o) (G_TYPE_CHECK_INSTANCE_CAST((o), POLKIT_KDE3_TYPE_LISTENER, PolkitKDE3Listener))
#define POLKIT_KDE3_LISTENER_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), POLKIT_KDE3_TYPE_LISTENER, PolkitKDE3ListenerClass))
#define POLKIT_KDE3_LISTENER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), POLKIT_KDE3_TYPE_LISTENER, PolkitKDE3ListenerClass))
#define POLKIT_KDE3_IS_LISTENER(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), POLKIT_KDE3_TYPE_LISTENER))
#define POLKIT_KDE3_IS_LISTENER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), POLKIT_KDE3_TYPE_LISTENER))

typedef struct _PolkitKDE3Listener PolkitKDE3Listener;
typedef struct _PolkitKDE3ListenerClass PolkitKDE3ListenerClass;

GType polkit_kde3_listener_get_type(void) G_GNUC_CONST;
PolkitAgentListener *polkit_kde3_listener_new(void);

G_END_DECLS

#endif /* _POLKIT_KDE3_LISTENER_H_ */
