/*****************************************************************

Copyright (c) 2001-2004 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef NOSLOTS
# define DEF( name, key3, key4, target, fnSlot ) \
   keys->insert( name, i18n(name), QString::null, key3, key4, target, SLOT(fnSlot) )
#else
# define DEF( name, key3, key4, target, fnSlot ) \
   keys->insert( name, i18n(name), QString::null, key3, key4 )
#endif
#define WIN KKey::QtWIN

#ifdef KICKER_ALL_BINDINGS
#define LAUNCH_MENU
#define SHOW_DESKTOP
#endif

#ifdef LAUNCH_MENU
	keys->insert("Program:kicker", i18n("Panel"));
	DEF(I18N_NOOP("Popup Launch Menu" ), ALT+Qt::Key_F1, WIN+Qt::Key_Menu,
                      MenuManager::the(), kmenuAccelActivated());
#endif

#ifdef SHOW_DESKTOP
	DEF(I18N_NOOP( "Toggle Showing Desktop" ), ALT+CTRL+Qt::Key_D, WIN+CTRL+Qt::Key_D,
            this, slotToggleShowDesktop());
#endif

#undef DEF
#undef WIN
