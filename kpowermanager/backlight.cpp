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

#include <qpaintdevice.h>
#include <kdebug.h>
#include <klocale.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include "backlight.h"


struct Backlight::Private {
    Atom backlight;
    XRRScreenResources *resources;
    RROutput output;
    int minLevel, maxLevel;
};


Backlight::Backlight()
{
    QString error;

    d = new Private();
    d->resources = NULL;
    d->minLevel = d->maxLevel = 0;

    Display *xdisplay = QPaintDevice::x11AppDisplay();
    Window xroot = QPaintDevice::x11AppRootWindow();

    d->backlight = ::XInternAtom(xdisplay, "Backlight", True);
    if(None == d->backlight) {
        error = "Video output have no backlight property.";
        goto err;
    }

    d->resources = ::XRRGetScreenResourcesCurrent(xdisplay, xroot);
    if(!d->resources) {
        error = "No Randr resources available.";
        goto err;
    }

    // cache min/max brightness
    for(int i = 0; i < d->resources->noutput; i++) {
        d->output = d->resources->outputs[i];
        XRRPropertyInfo *info = ::XRRQueryOutputProperty(xdisplay, d->output, d->backlight);

        if(info) {
            if(info->range && 2 == info->num_values) {
                d->minLevel = info->values[0];
                d->maxLevel = info->values[1];
                ::XFree(info);
                break;
            }
            ::XFree(info);
        }
    }

    kdDebug() << "brightness control has been initialized." << endl;
    return;

err:
    d->backlight = None;
    kdDebug() << error << " " << "The brightness control has been disabled." << endl;
}


Backlight::~Backlight()
{
    if(d->resources)
        ::XRRFreeScreenResources(d->resources);

    delete d;
}


int Backlight::minBrightness()
{
    if(None == d->backlight)
        return 0;

    return d->minLevel;
}


int Backlight::maxBrightness()
{
    if(None == d->backlight)
        return 0;

    return d->maxLevel;
}


int Backlight::brightness()
{
    if(None == d->backlight)
        return 0;

    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *prop;
    Atom actual_type;
    int actual_format;
    long value = 0;

    int ret = ::XRRGetOutputProperty(
        QPaintDevice::x11AppDisplay(), d->output, d->backlight,
        0, 4, False, False, None,
        &actual_type, &actual_format,
        &nitems, &bytes_after, &prop);

    if(Success == ret) {
        if(XA_INTEGER == actual_type && 1 == nitems && 32 == actual_format)
            value = *((long*)prop);
        ::XFree(prop);
    }

    return value;
}


void Backlight::setBrightness(int level)
{
    if(None == d->backlight)
        return;

    ::XRRChangeOutputProperty(
        QPaintDevice::x11AppDisplay(), d->output, d->backlight,
        XA_INTEGER, 32, PropModeReplace, (unsigned char*)&level, 1);
}
