/*
 * Copyright (C) 2012,2014 Serghei Amelian <serghei.amelian@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include <upower.h>
#include <qtimer.h>
#include <kdebug.h>

#include <config.h>

#ifdef WITH_LOGIND
#include <dbus/qdbusconnection.h>
#include <dbus/qdbuserror.h>
#include <dbus/qdbusmessage.h>
#include <klocale.h>
#include <kmessagebox.h>
#endif

#include "powersources.h"

#include "upowerbackend.h"


struct UPowerBackend::Private
{

#if UP_CHECK_VERSION(0, 99, 0)
    static void up_client_changed_cb(UpClient *client, GParamSpec *, UPowerBackend *upb)
    {
#else
    static void up_client_changed_cb(UpClient *client, UPowerBackend *upb)
    {
#endif
        emit upb->powerSourcesChanged(upb->d->ps);
    }


#if UP_CHECK_VERSION(0, 99, 0)
    static void up_device_changed_cb(UpDevice *device, GParamSpec *, UPowerBackend *upb)
    {
#else
    static void up_device_changed_cb(UpDevice *device, UPowerBackend *upb)
    {
#endif
        upb->d->updatePowerSource(device);
        emit upb->powerSourcesChanged(upb->d->ps);
    }


    void updatePowerSource(UpDevice *device)
    {
        guint kind;
        ::g_object_get(device, "kind", &kind, NULL);

        switch(kind)
        {
            case UP_DEVICE_KIND_LINE_POWER:
                ::g_object_get(device, "online", &ps.powerLine.online, NULL);
                break;
            case UP_DEVICE_KIND_BATTERY:
                ::g_object_get(device, "percentage", &ps.battery.percentage, "capacity", &ps.battery.capacity, "state", &ps.battery.state,
                               "time-to-empty", &ps.battery.timeToEmpty, "time-to-full", &ps.battery.timeToFull, "energy-rate",
                               &ps.battery.energyRate, NULL);
                break;
        }
    }


    UpClient *client;
    PowerSources ps;
};


UPowerBackend::UPowerBackend()
{
#if !defined(GLIB_VERSION_2_36)
    ::g_type_init();
#endif

    d = new Private;
    d->client = ::up_client_new();

    // the upower calls are blocking, we won't to freeze the kicker at startup
    QTimer::singleShot(1000, this, SLOT(initialize()));
}


UPowerBackend::~UPowerBackend()
{
    ::g_object_unref(d->client);
    delete d;
}


void UPowerBackend::suspend()
{
    GError *error;
    // up_client_about_to_sleep_sync() is crashing: https://bugs.freedesktop.org/show_bug.cgi?id=54976
    // gboolean ret = ::up_client_about_to_sleep_sync(d->client, UP_SLEEP_KIND_SUSPEND, NULL, &error);
    gboolean ret = true;
    if(ret)
        QTimer::singleShot(1000, this, SLOT(suspendDelayed()));
    else
    {
        kdDebug() << error;
        ::g_error_free(error);
    }
}


void UPowerBackend::hibernate()
{
    GError *error;
    // up_client_about_to_sleep_sync() is crashing: https://bugs.freedesktop.org/show_bug.cgi?id=54976
    // gboolean ret = ::up_client_about_to_sleep_sync(d->client, UP_SLEEP_KIND_HIBERNATE, NULL, &error);
    gboolean ret = true;
    if(ret)
        QTimer::singleShot(1000, this, SLOT(hibernateDelayed()));
    else
    {
        kdDebug() << error;
        ::g_error_free(error);
    }
}


void UPowerBackend::initialize()
{
    GError *error = NULL;
    GPtrArray *devices;

#if UP_CHECK_VERSION(0, 99, 0)
    ::g_signal_connect(d->client, "notify", G_CALLBACK(d->up_client_changed_cb), this);
#else
    ::g_signal_connect(d->client, "changed", G_CALLBACK(d->up_client_changed_cb), this);
#endif

#ifdef HAVE_UP_CLIENT_ENUMERATE_DEVICES_SYNC
    // cache power devices (NOTE: this function is now deprecated)
    gboolean ret = ::up_client_enumerate_devices_sync(d->client, NULL, &error);
    if(!ret)
        goto out;
#endif

    // initial update of power sources properties
    devices = ::up_client_get_devices(d->client);
    for(int i = 0; i < devices->len; i++)
    {
        UpDevice *device = static_cast< UpDevice * >(g_ptr_array_index(devices, i));
        d->updatePowerSource(device);
#if UP_CHECK_VERSION(0, 99, 0)
        ::g_signal_connect(device, "notify", G_CALLBACK(d->up_device_changed_cb), this);
#else
        ::g_signal_connect(device, "changed", G_CALLBACK(d->up_device_changed_cb), this);
#endif
    }
    ::g_ptr_array_unref(devices);

    d->ps.daemon.connected = true;
    kdDebug() << "upower backend has been initialized." << endl;

    // send first signal to the consumer (i.e. for icon update)
    emit powerSourcesChanged(d->ps);
out:
    if(error)
    {
        kdDebug() << "failed to enumerate power devices: " << error->message;
        ::g_error_free(error);

        // retry the initialization later
        QTimer::singleShot(5000, this, SLOT(initialize()));
    }
}


void UPowerBackend::suspendDelayed()
{
#ifdef WITH_LOGIND
    QString error;
    if(!sendCmdToLogind("Suspend", error))
        KMessageBox::error(0, i18n("Unable to suspend the machine.\nReason: %1").arg(error), i18n("Suspend error"));
#else
    GError *error;
    gboolean ret = ::up_client_suspend_sync(d->client, NULL, &error);

    if(!ret)
    {
        kdDebug() << error;
        ::g_error_free(error);
    }
#endif
}


void UPowerBackend::hibernateDelayed()
{
#ifdef WITH_LOGIND
    QString error;
    if(!sendCmdToLogind("Hibernate", error))
        KMessageBox::error(0, i18n("Unable to hibernate the machine.\nReason: %1").arg(error), i18n("Hibernate error"));
#else
    GError *error;
    gboolean ret = ::up_client_hibernate_sync(d->client, NULL, &error);

    if(!ret)
    {
        kdDebug() << error;
        ::g_error_free(error);
    }
#endif
}


#ifdef WITH_LOGIND
bool UPowerBackend::sendCmdToLogind(const QString &cmd, QString &error)
{
    QDBusConnection dbusConnection = QDBusConnection::addConnection(QDBusConnection::SystemBus);
    if(!dbusConnection.isConnected())
    {
        error = dbusConnection.lastError().message();
        return false;
    }

    QDBusMessage dbusMessage = QDBusMessage::methodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", cmd);
    dbusMessage << QDBusData::fromBool(false);

    // we need reply only to catch errors
    dbusConnection.sendWithReply(dbusMessage);

    if(dbusConnection.lastError().type() != 0)
    {
        error = dbusConnection.lastError().message();
        return false;
    }

    return true;
}
#endif


#include "upowerbackend.moc"
