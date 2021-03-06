/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * Copyright (c) 2005 Lubos Lunak <l.lunak@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <X11/Xlib.h>

#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif

static Display *dpy;
static Display *qt_xdisplay()
{
    return dpy;
}
static Window qt_xrootwin()
{
    return DefaultRootWindow(dpy);
}

bool isEmpty(const char *str)
{
    if(str == NULL)
        return true;
    while(isspace(*str))
        ++str;
    return *str == '\0';
}

int main(int argc, char *argv[])
{
    if(argc != 3)
        return 1;
    dpy = XOpenDisplay(NULL);
    if(dpy == NULL)
        return 2;
    int ret = 0;
#ifdef HAVE_XCURSOR
    const char *theme = argv[1];
    const char *size = argv[2];

    // Note: If you update this code, update kapplymousetheme as well.

    // use a default value for theme only if it's not configured at all, not even in X resources
    if(isEmpty(theme) && isEmpty(XGetDefault(qt_xdisplay(), "Xcursor", "theme")) && isEmpty(XcursorGetTheme(qt_xdisplay())))
    {
        theme = "default";
        ret = 10; // means to switch to default
    }

    // Apply the KDE cursor theme to ourselves
    if(!isEmpty(theme))
        XcursorSetTheme(qt_xdisplay(), theme);

    if(!isEmpty(size))
        XcursorSetDefaultSize(qt_xdisplay(), atoi(size));

    // Load the default cursor from the theme and apply it to the root window.
    Cursor handle = XcursorLibraryLoadCursor(qt_xdisplay(), "left_ptr");
    XDefineCursor(qt_xdisplay(), qt_xrootwin(), handle);
    XFreeCursor(qt_xdisplay(), handle); // Don't leak the cursor

#else
    (void)qt_xdisplay();
    (void)qt_xrootwin();
    (void)argv;
#endif
    XCloseDisplay(dpy);
    return ret;
}
