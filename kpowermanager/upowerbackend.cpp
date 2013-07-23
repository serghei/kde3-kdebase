/*
 * Copyright (C) 2012 Serghei Amelian <serghei.amelian@gmail.com>
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

#define UPOWER_ENABLE_DEPRECATED 1

#include <upower.h>
#include <qtimer.h>
#include <kdebug.h>

#include "powersources.h"

#include "upowerbackend.h"


struct UPowerBackend::Private {

    static void up_client_changed_cb(UpClient *client, UPowerBackend *upb) {
        emit upb->powerSourcesChanged(upb->d->ps);
    }

    static void up_device_changed_cb(UpClient *client, UpDevice *device, UPowerBackend *upb) {
        upb->d->updatePowerSource(device);
        emit upb->powerSourcesChanged(upb->d->ps);
    }

    void updatePowerSource(UpDevice *device) {
        guint kind;
        ::g_object_get(device, "kind", &kind, NULL);

        switch(kind) {
            case UP_DEVICE_KIND_LINE_POWER:
                    ::g_object_get(device,
                        "online", &ps.powerLine.online,
                        NULL);
                break;
            case UP_DEVICE_KIND_BATTERY:
                    ::g_object_get(device,
                        "percentage", &ps.battery.percentage,
                        "capacity", &ps.battery.capacity,
                        "state", &ps.battery.state,
                        "time-to-empty", &ps.battery.timeToEmpty,
                        "time-to-full", &ps.battery.timeToFull,
                        "energy-rate", &ps.battery.energyRate,
                        NULL);
                break;
        }
    }

    UpClient *client;
    PowerSources ps;
};


UPowerBackend::UPowerBackend()
{
    ::g_type_init();

    d = new Private;
    d->client = ::up_client_new();

    ::g_signal_connect(d->client, "changed", G_CALLBACK(d->up_client_changed_cb), this);
    ::g_signal_connect(d->client, "device-changed", G_CALLBACK(d->up_device_changed_cb), this);

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
    else {
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
    else {
        kdDebug() << error;
        ::g_error_free(error);
    }
}


void UPowerBackend::initialize()
{
    GError *error = NULL;
    GPtrArray *devices;

    // cache power devices
    gboolean ret = ::up_client_enumerate_devices_sync(d->client, NULL, &error);
    if(!ret) goto out;

    // initial update of power sources properties
    devices = ::up_client_get_devices(d->client);
    for(int i = 0; i < devices->len; i++) {
        d->updatePowerSource(static_cast<UpDevice*>(g_ptr_array_index(devices, i)));
    }
    ::g_ptr_array_unref(devices);

    d->ps.daemon.connected = true;
    kdDebug() << "upower backend has been initialized." << endl;

    // send first signal to the consumer (i.e. for icon update)
    emit powerSourcesChanged(d->ps);
out:
    if(error) {
        kdDebug() << "failed to enumerate power devices: " << error->message;
        ::g_error_free(error);

        // retry the initialization later
        QTimer::singleShot(5000, this, SLOT(initialize()));
    }
}


void UPowerBackend::suspendDelayed()
{
    GError *error;
    gboolean ret = ::up_client_suspend_sync(d->client, NULL, &error);

    if(!ret) {
        kdDebug() << error;
        ::g_error_free(error);
    }
}


void UPowerBackend::hibernateDelayed()
{
    GError *error;
    gboolean ret = ::up_client_hibernate_sync(d->client, NULL, &error);

    if(!ret) {
        kdDebug() << error;
        ::g_error_free(error);
    }
}


#include "upowerbackend.moc"
